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

#define ESTATUS_OK          0
#define ESTATUS_CONFERROR   1
#define ESTATUS_INITERROR   2
#define ESTATUS_INDEXERROR  3
#define ESTATUS_LOADERROR   4

// typedef struct {
//     const PhraseSearcher *m_psrch;
//     PhraseCollectionLoader m_ldr;
//     static LemInterface *m_pLem;
//     static int lem_nrefs;
//     XmlConfig m_cfg;
//     std::string m_req;
//     QCHtmlMarker m_marker;
//     PhraseSearcher::res_t m_clsRes;
// } CAgent;

typedef struct {
	PyObject_HEAD
	const char* config;
} PyAgent;

static PyObject* PyExc_QClassifyError;



static int PyAgent_init(PyAgent *self, PyObject *args) {

	const char *fname;

	if (!PyArg_ParseTuple(args, "s", &fname)) {
		return NULL;
	}

	// std::cout << fname << std::endl;
	self->config = fname;

    return 0;
}

static void PyAgent_dealloc(PyAgent* self) {
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

static PyObject* PyAgent_version(PyAgent* self) {
    return PyInt_FromLong(qcls_impl::QCLASSIFY_INDEX_VERSION);
}

static PyMethodDef PyAgent_methods[] =
{
    // {"initMarkup", (PyCFunction)PyAgent_initMarkup, METH_NOARGS, "Initialize markup"},
    {"version", (PyCFunction)PyAgent_version, METH_NOARGS, "C library version"},
    // {"loadConfig", (PyCFunction)PyAgent_loadConfig, METH_KEYWORDS, "Loads config from file"},
    // {"index2file", (PyCFunction)PyAgent_index2file, METH_NOARGS, "Builds index file"},
    // {"getIndexFileName", (PyCFunction)PyAgent_getIndexFileName, METH_NOARGS, "Index file path"},
    // {"classifyPhrase", (PyCFunction)PyAgent_classifyPhrase, METH_KEYWORDS, ""},
    {"markup", (PyCFunction)PyAgent_markup, METH_KEYWORDS, "Markup text"},
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
