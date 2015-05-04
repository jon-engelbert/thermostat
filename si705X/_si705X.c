#include <Python.h>
#include "si705X.h"

/* Docstrings */
static char module_docstring[] =
        "Copyright (c) 2015 Multi-Dimensional Visual Echo LLC.";
static char si705X_docstring[] = "Blah.";

/* Available functions */
static PyObject *si705X_startup(PyObject *self, PyObject *args);
static PyObject *si705X_reset(PyObject *self, PyObject *args);
static PyObject *si705X_get_revision(PyObject *self, PyObject *args);
static PyObject *si705X_get_ESN(PyObject *self, PyObject *args);
static PyObject *si705X_get_reg1(PyObject *self, PyObject *args);
static PyObject *si705X_set_reg1(PyObject *self, PyObject *args);
static PyObject *si705X_get_tempC(PyObject *self, PyObject *args);
static PyObject *si705X_get_tempF(PyObject *self, PyObject *args);

/* Module specification */
static PyMethodDef module_methods[] = {
    {"startup", si705X_startup, METH_VARARGS, si705X_docstring},
    {"reset", si705X_reset, METH_VARARGS, si705X_docstring},
    {"get_revision", si705X_get_revision, METH_VARARGS, si705X_docstring},
    {"get_ESN", si705X_get_ESN, METH_VARARGS, si705X_docstring},
    {"get_reg1", si705X_get_reg1, METH_VARARGS, si705X_docstring},
    {"set_reg1", si705X_set_reg1, METH_VARARGS, si705X_docstring},
    {"get_tempC", si705X_get_tempC, METH_VARARGS, si705X_docstring},
    {"get_tempF", si705X_get_tempF, METH_VARARGS, si705X_docstring},
    {NULL, NULL, 0, NULL}
};

/* Initialize the module */
PyMODINIT_FUNC initsi705X(void)
{
    PyObject *m = Py_InitModule3("si705X", module_methods, module_docstring);
    if (m == NULL)
        return;

    /* Do init stuff here */
}

static PyObject *si705X_startup(PyObject *self, PyObject *args)
{
    int bus_no;
    int addr;

    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "ii", &bus_no, &addr))
    {
        PyErr_SetString(PyExc_RuntimeError, "ERROR: bad parameters.");
        return NULL;
    }

    startup(bus_no, addr);

    /* Build the output tuple */
    PyObject *ret = Py_BuildValue("i", 0);
    return ret;
}

static PyObject *si705X_reset(PyObject *self, PyObject *args)
{
    reset();

    /* Build the output tuple */
    PyObject *ret = Py_BuildValue("i", 0);
    return ret;
}

static PyObject *si705X_get_revision(PyObject *self, PyObject *args)
{
    int revno;

    revno = get_revision();

    PyObject *ret = Py_BuildValue("i", revno);
    return ret;
}

static PyObject *si705X_get_ESN(PyObject *self, PyObject *args)
{
    unsigned long esn1;
    unsigned long esn2;
    unsigned long long esn;

    esn = get_ESN();
    esn1 = esn >> 32;
    esn2 = esn & 0xFFFFFFFF;

    PyObject *ret = Py_BuildValue("ll", esn1, esn2);
    return ret;
}

static PyObject *si705X_get_reg1(PyObject *self, PyObject *args)
{
    unsigned char reg1;

    reg1 = get_reg1();

    PyObject *ret = Py_BuildValue("B", reg1);
    return ret;
}

static PyObject *si705X_set_reg1(PyObject *self, PyObject *args)
{
    unsigned char reg1;

    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "b", &reg1))
    {
        PyErr_SetString(PyExc_RuntimeError, "ERROR: bad parameters.");
        return NULL;
    }

    set_reg1(reg1);

    /* Build the output tuple */
    PyObject *ret = Py_BuildValue("i", 0);
    return ret;
}

static PyObject *si705X_get_tempC(PyObject *self, PyObject *args)
{
    double temperature;

    temperature = get_tempC();

    /* Build the output tuple */
    PyObject *ret = Py_BuildValue("d", temperature);
    return ret;
}

static PyObject *si705X_get_tempF(PyObject *self, PyObject *args)
{
    double temperature;

    temperature = get_tempF();

    /* Build the output tuple */
    PyObject *ret = Py_BuildValue("d", temperature);
    return ret;
}
