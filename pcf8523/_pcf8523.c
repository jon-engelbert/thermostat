#include <Python.h>
#include "pcf8523.h"

/* Docstrings */
static char module_docstring[] =
        "Copyright (c) 2015 Multi-Dimensional Visual Echo LLC.";
static char pcf8523_docstring[] = "Blah.";

/* Available functions */
static PyObject *pcf8523_startup(PyObject *self, PyObject *args);
static PyObject *pcf8523_reset(PyObject *self, PyObject *args);
static PyObject *pcf8523_fetch_reg(PyObject *self, PyObject *args);
static PyObject *pcf8523_set_reg(PyObject *self, PyObject *args);
static PyObject *pcf8523_fetch_chip_time(PyObject *self, PyObject *args);
static PyObject *pcf8523_set_chip_time(PyObject *self, PyObject *args);
static PyObject *pcf8523_check_battery(PyObject *self, PyObject *args);
static PyObject *pcf8523_set_systime_from_chip(PyObject *self, PyObject *args);
static PyObject *pcf8523_set_chip_to_systime(PyObject *self, PyObject *args);

/* Module specification */
static PyMethodDef module_methods[] = {
    {"startup", pcf8523_startup, METH_VARARGS, pcf8523_docstring},
    {"reset", pcf8523_reset, METH_VARARGS, pcf8523_docstring},
    {"fetch_reg", pcf8523_fetch_reg, METH_VARARGS, pcf8523_docstring},
    {"set_reg", pcf8523_set_reg, METH_VARARGS, pcf8523_docstring},
    {"fetch_chip_time", pcf8523_fetch_chip_time, METH_VARARGS, pcf8523_docstring},
    {"set_chip_time", pcf8523_set_chip_time, METH_VARARGS, pcf8523_docstring},
    {"check_battery", pcf8523_check_battery, METH_VARARGS, pcf8523_docstring},
    {"set_systime_from_chip", pcf8523_set_systime_from_chip, METH_VARARGS, pcf8523_docstring},
    {"set_chip_to_systime", pcf8523_set_chip_to_systime, METH_VARARGS, pcf8523_docstring},
    {NULL, NULL, 0, NULL}
};

/* Initialize the module */
PyMODINIT_FUNC initpcf8523(void)
{
    PyObject *m = Py_InitModule3("pcf8523", module_methods, module_docstring);
    if (m == NULL)
        return;

    /* Do init stuff here */
}

static PyObject *pcf8523_startup(PyObject *self, PyObject *args)
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

static PyObject *pcf8523_reset(PyObject *self, PyObject *args)
{
    reset();

    /* Build the output tuple */
    PyObject *ret = Py_BuildValue("i", 0);
    return ret;
}

static PyObject *pcf8523_fetch_reg(PyObject *self, PyObject *args)
{
    unsigned char reg;
    int regnum;
    
    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "i", &regnum))
    {
        PyErr_SetString(PyExc_RuntimeError, "ERROR: bad parameters.");
        return NULL;
    }

    reg = fetch_reg(regnum);

    PyObject *ret = Py_BuildValue("B", reg);
    return ret;
}

static PyObject *pcf8523_set_reg(PyObject *self, PyObject *args)
{
    int regnum;
    int value;
    int ok;
    
    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "ii", &regnum, &value))
    {
        PyErr_SetString(PyExc_RuntimeError, "ERROR: bad parameters.");
        return NULL;
    }

    ok = set_reg(regnum, value);

    PyObject *ret = Py_BuildValue("i", ok);
    return ret;
}

static PyObject *pcf8523_fetch_chip_time(PyObject *self, PyObject *args)
{
    unsigned char yr,mo,dow,dy,ho,mi,sc;
    
    if (fetch_chip_time(&yr, &mo, &dow, &dy, &ho, &mi, &sc) < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "ERROR: bad fetch.");
        return NULL;
    }
    
    PyObject *ret = Py_BuildValue("BBBBBBB", yr,mo,dow,dy,ho,mi,sc);
    return ret;
}

static PyObject *pcf8523_set_chip_time(PyObject *self, PyObject *args)
{
    int ok;
    unsigned char yr,mo,dow,dy,ho,mi,sc;
    
    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "bbbbbbb", &yr, &mo, &dow, &dy, &ho, &mi, &sc))
    {
        PyErr_SetString(PyExc_RuntimeError, "ERROR: bad parameters.");
        return NULL;
    }

    ok = set_chip_time(yr,mo,dow,dy,ho,mi,sc);

    PyObject *ret = Py_BuildValue("i", ok);
    return ret;
}

static PyObject *pcf8523_check_battery(PyObject *self, PyObject *args)
{
    char ok;
    
    ok = check_battery(); 
    if (ok < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "ERROR: battery check failed, result unknown.");
    }
    
    PyObject *ret = Py_BuildValue("b", ok);
    return ret;
}

static PyObject *pcf8523_set_systime_from_chip(PyObject *self, PyObject *args)
{
    char ok;
    
    ok = set_systime_from_chip(); 
    if (ok < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "ERROR: set system time failed.");
    }
    
    PyObject *ret = Py_BuildValue("b", ok);
    return ret;
}

static PyObject *pcf8523_set_chip_to_systime(PyObject *self, PyObject *args)
{
    char ok;
    
    ok = set_chip_to_systime(); 
    if (ok < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "ERROR: set chip time failed.");
    }
    
    PyObject *ret = Py_BuildValue("b", ok);
    return ret;
}
