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

using namespace gogo;

#define ESTATUS_INDEXERROR  3
#define ESTATUS_LOADERROR   4

typedef struct {
	PyObject_HEAD

	const char* config;
	bool configured;
	bool ready;

    // const PhraseSearcher *m_psrch;
	// PhraseCollectionLoader m_ldr;
    // LemInterface *m_pLem;
	// static int lem_nrefs;
    XmlConfig* m_cfg;
    // std::string m_req;
	// QCHtmlMarker m_marker;
	// PhraseSearcher::res_t m_clsRes;
} PyAgent;

static PyObject* PyExc_QClassifyError;

static int PyAgent_init(PyAgent *self, PyObject *args) {

	const char *fname;

	if (!PyArg_ParseTuple(args, "s", &fname)) {
		return NULL;
	}

	// fprintf(stderr, "%s\n", fname);

	self->config = fname;
	self->configured = 0;
	self->ready = 0;

	// init lemmatizer
	// self->m_psrch = NULL;
    // try {
	// 	self->m_pLem = new LemInterface(true /* UTF8 */);
    // }
    // catch(...) {
    //     PyErr_SetString(PyExc_QClassifyError, "Error occured while initializing lemmatizer library");
    //     return -1;
    // }

	self->m_cfg = new(&self->m_cfg) XmlConfig();

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

    return 0;
}

static void PyAgent_dealloc(PyAgent* self) {
	// delete self->m_pLem;
	// self->m_pLem = NULL;
	// delete self->m_cfg;
	// self->m_cfg = NULL;
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject* PyAgent_markup(PyAgent* self, PyObject *args, PyObject *kwds) {
	PyObject* UnicodeInput;

	static char *kwlist[] = {(char *)"text", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "U", kwlist, &UnicodeInput))
		return NULL;

	PyObject* UTFInput = PyUnicode_AsEncodedString(UnicodeInput, "UTF-8", NULL);

	char* text= PyString_AsString(UTFInput);
	Py_DECREF(UTFInput);

	return PyUnicode_FromString(text);
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
    {"markup", (PyCFunction)PyAgent_markup, METH_KEYWORDS, "Markup text"},
    {"get_index", (PyCFunction)PyAgent_getIndexFileName, METH_NOARGS, "Index file path"},
    // {"initMarkup", (PyCFunction)PyAgent_initMarkup, METH_NOARGS, "Initialize markup"},
    {"version", (PyCFunction)PyAgent_version, METH_NOARGS, "C library version"},
    // {"loadConfig", (PyCFunction)PyAgent_loadConfig, METH_KEYWORDS, "Loads config from file"},
    // {"index2file", (PyCFunction)PyAgent_index2file, METH_NOARGS, "Builds index file"},
    // {"classifyPhrase", (PyCFunction)PyAgent_classifyPhrase, METH_KEYWORDS, ""},
    // {"firstForm", (PyCFunction)PyAgent_firstForm, METH_KEYWORDS, "Return first form of given keyword"},
    { NULL,  NULL, 0, NULL }  /* Sentinel */
};

static PyMemberDef PyAgent_members[] = {
	{ "config", T_STRING, offsetof(PyAgent, config), 0, "Config file" },
    { NULL, 0, 0, 0, NULL }  /* Sentinel */
};

static PyTypeObject PyAgentType = {
   PyObject_HEAD_INIT(NULL)
   0,                         /* ob_size */
   "libpyQClassify.Agent",    /* tp_name */
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
initlibpyQClassify(void)
{
	PyObject* module;

	// Create the module
	module = Py_InitModule3("libpyQClassify", module_functions, "Python wrapper around QClassify library.");
	if (module == NULL)
		return;

	// Fill in some slots in the type, and make it ready
	PyAgentType.tp_new = PyType_GenericNew;
	if (PyType_Ready(&PyAgentType) < 0) {
		return;
	}

	// Add the type to the module.
	Py_INCREF(&PyAgentType);
	PyModule_AddObject(module, "Agent", (PyObject*)&PyAgentType);

	// Add exception to the module.
	PyExc_QClassifyError = PyErr_NewException((char *)"libpyQClassify.QClassifyError", NULL, NULL);
	Py_INCREF(PyExc_QClassifyError);
	PyModule_AddObject(module, "QClassifyError", PyExc_QClassifyError);
}
