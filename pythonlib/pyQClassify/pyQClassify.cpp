#include <Python.h>
#include <structmember.h>

#include <iostream>
#include <stdio.h>
#include <string>
#include "config/config.hpp"
#include <Interfaces/cpp/LemInterface.hpp>
#include "qclassify/qclassify.hpp"
#include "qclassify/qclassify_impl.hpp"
#include "qclassify/htmlmark.hpp"

#define PY_DEFAULT_ARGUMENT_INIT(name, value, ret) \
    PyObject *name = NULL; \
    static PyObject *default_##name = NULL; \
    if (! default_##name) { \
        default_##name = value; \
        if (! default_##name) { \
            PyErr_SetString(PyExc_RuntimeError, "Can not create default value for " #name); \
            return ret; \
        } \
    }

#define PY_DEFAULT_ARGUMENT_SET(name) if (! name) name = default_##name; \
    Py_INCREF(name)

using namespace gogo;

typedef struct {
	PyObject_HEAD

	const char* config;
	bool is_initialized;

	const PhraseSearcher *m_psrch;
	XmlConfig* m_cfg;
	PhraseCollectionLoader *m_ldr;
	QCHtmlMarker *m_marker;
} PyAgent;

static LemInterface* m_pLem = NULL;

static PyObject* PyExc_QClassifyError;


void initMarkup(PyAgent* self) {
	/* Dependencies:
		m_ldr <- m_pLem, m_cfg
		m_psrch <- m_ldr
		m_marker <- m_psrch, m_cfg */

	// prepare search
	self->m_ldr->setLemmatizer(m_pLem);
	if (!self->m_ldr->loadByConfig(self->m_cfg)) {
		self->is_initialized = 0;
		return;
	}
	self->m_psrch = self->m_ldr->getSearcher();

	// init markup
	self->m_marker->setPhraseSearcher(self->m_psrch);
	self->m_marker->loadSettings(self->m_cfg);

	self->is_initialized = 1;
}


static int PyAgent_init(PyAgent *self, PyObject *args) {

	const char *fname;

	if (!PyArg_ParseTuple(args, "s", &fname)) {
		return NULL;
	}

	// fprintf(stderr, "%s\n", fname);

	self->config = fname;
	self->is_initialized = 0;

	self->m_cfg = new XmlConfig();
	self->m_ldr = new PhraseCollectionLoader();
	self->m_marker = new QCHtmlMarker();

	try {
		if (!m_pLem){
			m_pLem = new LemInterface(true /* UTF8 */);
		}
	}
	catch(...){
		PyErr_SetString(PyExc_QClassifyError, "liblemmatizer can not be initialized");
		return -1;
	}

	// load config
	try {
		if (!self->m_cfg->Load(fname)) {
			PyErr_SetString(PyExc_QClassifyError, "Unable to load config.");
			return -1;
		}
	}
	catch(...) {
		PyErr_SetString(PyExc_QClassifyError, "Unknown error while loading config file");
		return -1;
	}

	initMarkup(self);
	return 0;
}

static void PyAgent_dealloc(PyAgent* self) {
	delete self->m_marker;
	self->m_marker = NULL;

	delete self->m_ldr;
	self->m_ldr = NULL;

	delete self->m_cfg;
	self->m_cfg = NULL;

	self->ob_type->tp_free((PyObject*)self);
}


static PyObject* PyAgent_markup(PyAgent* self, PyObject *args, PyObject *kwargs) {
        if (!self->is_initialized) {
                PyErr_SetString(PyExc_QClassifyError, "Unable to load PhraseCollectionLoader config");
                return NULL;
        }

        std::string out;

        PY_DEFAULT_ARGUMENT_INIT(url_to_exclude,  PyUnicode_FromString(""),  NULL);

        PyObject* UInputText;

        static const char *kwlist[] = {"text", "url_to_exclude", NULL};

        if (!PyArg_ParseTupleAndKeywords(args, kwargs, "U|U", const_cast<char **>(kwlist), &UInputText, &url_to_exclude))
                return NULL;
        PY_DEFAULT_ARGUMENT_SET(url_to_exclude);
        PyObject* UTFInput = PyUnicode_AsEncodedString(UInputText, "UTF-8", NULL);

        char* text = PyString_AsString(UTFInput);
        char* url_to_exclude_str = PyString_AsString(url_to_exclude);
        QCHtmlMarker::MarkupSettings st = self->m_marker->getConfigSettings();
        try {
                self->m_marker->markup((std::string)text, out, st, (std::string)url_to_exclude_str);
        }
        catch(...) {
                PyErr_SetString(PyExc_QClassifyError, "Unable to markup text");
                Py_DECREF(UTFInput);
                Py_DECREF(url_to_exclude);
                return NULL;
        }

        Py_DECREF(UTFInput);
        Py_DECREF(url_to_exclude);
        return PyUnicode_FromString(out.c_str());
}

static PyObject* PyAgent_index2file(PyAgent* self) {
	try {
		PhraseCollectionIndexer idx(m_pLem);
		idx.indexByConfig(self->m_cfg);
		idx.save();
	}
	catch (...) {
		PyErr_SetString(PyExc_QClassifyError, "Indexing error");
		return NULL;
	}

	initMarkup(self);

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject* PyAgent_getIndexFileName(PyAgent* self) {
	std::string s;
	self->m_cfg->GetStr("QueryQualifier", "IndexFile", s, "phrases.idx");
	return PyString_FromString(s.c_str());
}


static PyObject* PyAgent_version(PyAgent* self) {
	return PyInt_FromLong(qcls_impl::QCLASSIFY_INDEX_VERSION);
}

static PyMethodDef PyAgent_methods[] =
{
	{ "markup", (PyCFunction)PyAgent_markup, METH_KEYWORDS, "Markup text" },
	{ "index2file", (PyCFunction)PyAgent_index2file, METH_NOARGS, "Builds index file" },
	{ "get_index", (PyCFunction)PyAgent_getIndexFileName, METH_NOARGS, "Index file path" },
	{ "version", (PyCFunction)PyAgent_version, METH_NOARGS, "C library version" },
	{ NULL,  NULL, 0, NULL }  /* Sentinel */
};

static PyMemberDef PyAgent_members[] = {
	{ "config", T_STRING, offsetof(PyAgent, config), 0, "Config file" },
    { NULL, 0, 0, 0, NULL }  /* Sentinel */
};

static PyTypeObject PyAgentType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /* ob_size */
	"QClassifyAgent",          /* tp_name */
	sizeof(PyAgent),           /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)PyAgent_dealloc, /* tp_dealloc */
	0,                         /* tp_print */
	0,                         /* tp_getattr */
	0,                         /* tp_setattr */
	0,                         /* tp_compare */
	0,                         /* tp_repr */
	0,                         /* tp_as_number */
	0,                         /* tp_as_sequence */
	0,                         /* tp_as_mapping */
	0,                         /* tp_hash */
	0,                         /* tp_call */
	0,                         /* tp_str */
	0,                         /* tp_getattro */
	0,                         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags*/
	"pyQClassify class",       /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	PyAgent_methods,           /* tp_methods */
	PyAgent_members,           /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)PyAgent_init,    /* tp_init */
	0,                         /* tp_alloc */
	0,                         /* tp_new */
};


static PyMethodDef module_functions[] = {
	{ NULL, NULL, 0, NULL }
};

// This function is called to initialize the module.

PyMODINIT_FUNC
initpyQClassify(void)
{
	PyObject* module;

	// Create the module
	module = Py_InitModule3("pyQClassify", module_functions, "Python wrapper around QClassify library.");
	if (module == NULL)
		return;

	// Fill in some slots in the type, and make it ready
	PyAgentType.tp_new = PyType_GenericNew;
	if (PyType_Ready(&PyAgentType) < 0) {
		return;
	}

	// Add the type to the module.
	Py_INCREF(&PyAgentType);
	PyModule_AddObject(module, "QClassifyAgent", (PyObject*)&PyAgentType);

	// Add exception to the module.
	PyExc_QClassifyError = PyErr_NewException((char *)"pyQClassify.QClassifyError", NULL, NULL);
	Py_INCREF(PyExc_QClassifyError);
	PyModule_AddObject(module, "QClassifyError", PyExc_QClassifyError);
}
