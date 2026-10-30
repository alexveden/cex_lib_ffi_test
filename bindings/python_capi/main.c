#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

#include "cex_demo.h"

typedef struct
{
    PyObject_HEAD
    cex_demo_request_t req;
} request_object_t;

static PyObject* cex_demo_error_type;
static PyObject* cex_demo_argument_error_type;
static PyObject* cex_demo_overflow_error_type;

static bool
to_int32(PyObject* value, int32_t* out)
{
    long long v = PyLong_AsLongLong(value);
    if (v == -1 && PyErr_Occurred()) { return false; }
    if (v < INT32_MIN || v > INT32_MAX) {
        PyErr_SetString(PyExc_OverflowError, "value does not fit in int32");
        return false;
    }
    *out = (int32_t)v;
    return true;
}

static void
raise_cex(cex_demo_exc_t exc)
{
    PyObject* type = cex_demo_error_type;
    if (exc == cex_demo_Error.argument) {
        type = cex_demo_argument_error_type;
    } else if (exc == cex_demo_Error.overflow) {
        type = cex_demo_overflow_error_type;
    }
    PyErr_SetString(type, exc);
}

static PyObject*
request_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    (void)args;
    (void)kwargs;
    request_object_t* self = (request_object_t*)type->tp_alloc(type, 0);
    if (!self) { return NULL; }
    self->req.value = 0;
    return (PyObject*)self;
}

static int
request_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    static char* kwlist[] = { "value", NULL };
    PyObject* value = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O", kwlist, &value)) { return -1; }
    if (value && !to_int32(value, &((request_object_t*)self)->req.value)) { return -1; }
    return 0;
}

static PyObject*
request_get_value(PyObject* self, void* closure)
{
    (void)closure;
    return PyLong_FromLong(((request_object_t*)self)->req.value);
}

static int
request_set_value(PyObject* self, PyObject* value, void* closure)
{
    (void)closure;
    if (!value) {
        PyErr_SetString(PyExc_AttributeError, "value cannot be deleted");
        return -1;
    }
    return to_int32(value, &((request_object_t*)self)->req.value) ? 0 : -1;
}

static PyObject*
request_repr(PyObject* self)
{
    return PyUnicode_FromFormat("Request(value=%d)", ((request_object_t*)self)->req.value);
}

static PyGetSetDef request_getset[] = {
    { "value", request_get_value, request_set_value, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL },
};

static PyTypeObject request_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_cex_demo_capi.Request",
    .tp_basicsize = sizeof(request_object_t),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = request_new,
    .tp_init = request_init,
    .tp_repr = request_repr,
    .tp_getset = request_getset,
};

static PyObject*
py_run(PyObject* self, PyObject* req)
{
    (void)self;
    cex_demo_exc_t exc = NULL;
    if (req == Py_None) {
        exc = cex_demo_run(NULL);
    } else if (PyObject_TypeCheck(req, &request_type)) {
        exc = cex_demo_run(&((request_object_t*)req)->req);
    } else {
        PyErr_Format(
            PyExc_TypeError,
            "run() argument must be Request or None, not %.200s",
            Py_TYPE(req)->tp_name
        );
        return NULL;
    }
    if (exc != cex_demo_Error.ok) {
        raise_cex(exc);
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject*
py_traceback(PyObject* self, PyObject* unused)
{
    (void)self;
    (void)unused;
    char buf[512];
    cex_demo_exc_t exc = cex_demo_traceback(buf, sizeof buf);
    if (exc != cex_demo_Error.ok) {
        raise_cex(exc);
        return NULL;
    }
    return PyUnicode_FromString(buf);
}

static PyMethodDef module_methods[] = {
    { "run", py_run, METH_O, NULL },
    { "traceback", py_traceback, METH_NOARGS, NULL },
    { NULL, NULL, 0, NULL },
};

static void
module_free(void* module)
{
    (void)module;
    Py_CLEAR(cex_demo_error_type);
    Py_CLEAR(cex_demo_argument_error_type);
    Py_CLEAR(cex_demo_overflow_error_type);
}

static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_cex_demo_capi",
    .m_size = -1,
    .m_methods = module_methods,
    .m_free = module_free,
};

PyMODINIT_FUNC
PyInit__cex_demo_capi(void)
{
    if (PyType_Ready(&request_type) < 0) { return NULL; }

    cex_demo_error_type = PyErr_NewException("_cex_demo_capi.CexDemoError", NULL, NULL);
    if (!cex_demo_error_type) { return NULL; }
    cex_demo_argument_error_type = PyErr_NewException(
        "_cex_demo_capi.CexDemoArgumentError",
        cex_demo_error_type,
        NULL
    );
    if (!cex_demo_argument_error_type) { return NULL; }
    cex_demo_overflow_error_type = PyErr_NewException(
        "_cex_demo_capi.CexDemoOverflowError",
        cex_demo_error_type,
        NULL
    );
    if (!cex_demo_overflow_error_type) { return NULL; }

    PyObject* module = PyModule_Create(&module_def);
    if (!module) {
        module_free(NULL);
        return NULL;
    }
    if (PyModule_AddObjectRef(module, "Request", (PyObject*)&request_type) < 0 ||
        PyModule_AddObjectRef(module, "CexDemoError", cex_demo_error_type) < 0 ||
        PyModule_AddObjectRef(module, "CexDemoArgumentError", cex_demo_argument_error_type) < 0 ||
        PyModule_AddObjectRef(module, "CexDemoOverflowError", cex_demo_overflow_error_type) < 0) {
        Py_DECREF(module);
        return NULL;
    }
    return module;
}
