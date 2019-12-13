#include "joystick.h"

int xbox_fd, len = -1;
xbox_map_t map;
xbox_init(xbox_fd, &map);
len = xbox_map_read(xbox_fd, &map);
xbox_close(xbox_fd);


static PyObject *Extest_xbox_init(PyObject *self, PyObject *args)
{
    int fd;
    return (PyObject *)Py_BuildValue("")
}


static PyObject *Extest_xbox_map_read(PyObject *self, PyObject *args)
{
    int fd;
    return (PyObject *)Py_BuildValue("")
}


static PyObject *Extest_xbox_close(PyObject *self, PyObject *args)
{
    int fd;
    return (PyObject *)Py_BuildValue("")
}

static PyMethodDef JoystickMethods[] =
{
    {"xbox_init",     joystick_xbox_init,     METH_VARARGS, "init"},
    {"xbox_map_read", joystick_xbox_map_read, METH_VARARGS, "read"},
    {"xbox_close",    joystick_xbox_init,     METH_VARARGS, "close"},
    {NULL, NULL, 0, NULL}
}


static struct PyModuleDef ExtestMoudle =
{
    PyModuleDef_HEAD_INIT,
    "joystick",
    NULL,
    -1,
    JoystickMethods
}


void PyInit_Joystick()
{
    PyModule_Create(& JoystickMoudle)
}

