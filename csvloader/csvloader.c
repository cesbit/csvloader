/* C-CSV module
 *
 * This module is created especially for SiriDB and should be used with caution.
 * We do make some assumptions about the data so be careful!
 *
 * 2015, Jeroen van der Heijden (Transceptor Technology)
 */

#include <Python.h>
#include <stdbool.h>
#include <math.h>
#include <stddef.h>

#define INIT_ALLOC_SZ 64

/* Docstrings */
static char module_docstring[] =
    "csvloader module in C for quick reading csv input and returning correct\n"
    "typed data fields.";

static char loads_docstring[] =
    "Returns a 2 dimensional array for the given CSV content.\n"
    "\n"
    "Each field in the CSV will be type casted to either a integer, float,\n"
    "string or None value. An empty field will be casted to None, except when\n"
    "the field explicit has an empty string defined by two. In that case we\n"
    "will return an empty string instead of None.";


/* Available functions */
static PyObject * csvloader_loads(PyObject * self, PyObject * args);

/* Other functions */
static char * replace_str(const char * content, Py_ssize_t * length);

/* Module specification */
static PyMethodDef module_methods[] =
{
    {"loads", (PyCFunction)csvloader_loads, METH_VARARGS, loads_docstring},
    {NULL, NULL, 0, NULL}
};

static char * quoted_str = NULL;
static Py_ssize_t quoted_len = 0;

#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "csvloader",       /* m_name */
        module_docstring,  /* m_doc */
        -1,                /* m_size */
        module_methods,    /* m_methods */
        NULL,              /* m_reload */
        NULL,              /* m_traverse */
        NULL,              /* m_clear */
        NULL,              /* m_free */
    };

    /* Initialize the module */
    PyMODINIT_FUNC PyInit_csvloader(void)
    {
        return PyModule_Create(&moduledef);
    }
#else
    PyMODINIT_FUNC initcsvloader(void)
    {
        PyObject *m = Py_InitModule3(
                "csvloader",
                module_methods,
                module_docstring);
        if (m == NULL) return;
    }
#endif

#if PY_MAJOR_VERSION >= 3
#define PyXXX_FromStringAndSize PyUnicode_FromStringAndSize
#define PyXXX_FromSsize_t PyLong_FromSsize_t
#else
#define PyXXX_FromStringAndSize PyString_FromStringAndSize
#define PyXXX_FromSsize_t PyInt_FromSsize_t
#endif

/*
 * Returns a string or NULL in case of an error (PyErr will be set)
 */
static char * replace_str(const char * content, Py_ssize_t * length)
{
    *length -= 2;
    if (quoted_len < *length)
    {
        char * tmp = (char *) realloc(quoted_str, *length);
        if (tmp == NULL)
        {
            PyErr_SetString(PyExc_MemoryError, "Memory re-allocation error.");
            return NULL;
        }
        quoted_str = tmp;
    }

    content++;
    char * dp = quoted_str;

    for (Py_ssize_t i = *length; i--; content++, dp++)
    {
        if (*content == '"')
        {
            i--;
            content++;
            (*length)--;
        }
        *dp = *content;
    }

    return quoted_str;
}

static int loads(PyObject * grid, Py_ssize_t length, const char * content)
{
    int rc;
    char c;
    Py_ssize_t word_len, replace_len;
    bool in_quotes, is_quoted, is_int, is_float;
    PyObject * row;
    PyObject * obj;

    if ((row = PyList_New(0)) == NULL)
    {
        return -1;
    }

    /* Initialize variables. */
    in_quotes = false;
    is_quoted = false;
    is_int = true;
    is_float = true;
    word_len = 0;

    /* Loop through the content using the length */
    while (length--)
    {
        /* Get the current char. */
        c = content[word_len];

        /* Check if we have an not-quoted comma or end-of-line character. */
        if (!in_quotes && (c == ',' || c == '\n'))
        {
            /* Set the correct object type */
            if (!word_len)
            {
                obj = Py_None;
                Py_INCREF(obj);
            }
            else if (is_int)
            {
                obj = PyXXX_FromSsize_t((Py_ssize_t) strtoll(content, NULL, 0));
            }
            else if (is_float)
            {
                obj = PyFloat_FromDouble(strtod(content, NULL));
            }
            else if (is_quoted)
            {
                replace_len = word_len;
                if (replace_str(content, &replace_len) == NULL)
                {
                    Py_DECREF(row);
                    return -1;
                }
                obj = PyXXX_FromStringAndSize(quoted_str, replace_len);
            }
            else
            {
                obj = PyXXX_FromStringAndSize(content, word_len);
            }

            if (obj == NULL)
            {
                Py_DECREF(row);
                return -1;
            }

            /* Append value to the row. */
            rc = PyList_Append(row, obj);
            Py_DECREF(obj);

            if (rc == -1)
            {
                Py_DECREF(row);
                return -1;
            }

            /* Update variables. */
            content += word_len + 1;
            word_len = 0;
            is_quoted = false;
            is_int = true;
            is_float = true;

            /* Do some stuff when we have an end-of-line detected. */
            if (c == '\n')
            {
                rc = PyList_Append(grid, row);
                Py_DECREF(row);

                row = PyList_New(0);
                if (rc == -1 || row == NULL)
                {
                    Py_DECREF(grid);
                    return -1;
                }
            }

            continue;
        }

        if (c == '"')
        {
            if (!word_len)
            {
                is_quoted = true;
            }
            if (is_quoted)
            {
                in_quotes = !in_quotes;
            }
        }
        else if (is_quoted && !in_quotes)
        {
            Py_DECREF(row);
            PyErr_SetString(PyExc_ValueError, "Wrong string escaping found");
            return -1;
        }

        if (is_float && !isdigit(c) && (word_len != 0 || c != '-'))
        {
            if (is_float && c == '.')
            {
                if (is_int)
                {
                    is_int = false;
                }
                else
                {
                    is_float = false;
                }
            }
            else
            {
                is_int = false;
                is_float = false;
            }
        }
        word_len++;
    }
    if (in_quotes)
    {
        Py_DECREF(row);
        PyErr_SetString(PyExc_ValueError, "Wrong string escaping found");
        return -1;
    }

    /* Set the correct object type */
    if (!word_len)
    {
        obj = Py_None;
        Py_INCREF(obj);
    }
    else if (is_int)
    {
        obj = PyXXX_FromSsize_t((Py_ssize_t) strtoll(content, NULL, 0));
    }
    else if (is_float)
    {
        obj = PyFloat_FromDouble(strtod(content, NULL));
    }
    else if (is_quoted)
    {
        replace_len = word_len;
        if (replace_str(content, &replace_len) == NULL)
        {
            Py_DECREF(row);
            return -1;
        }
        obj = PyXXX_FromStringAndSize(quoted_str, replace_len);
    }
    else
    {
        obj = PyXXX_FromStringAndSize(content, word_len);
    }

    if (obj == NULL)
    {
        Py_DECREF(row);
        return -1;
    }

    /* Append the last value to the row. */
    rc = PyList_Append(row, obj);
    Py_DECREF(obj);

    if (rc == 0)
    {
        /* Append the last row to the grid. */
        rc = PyList_Append(grid, row);
        Py_DECREF(row);
    }

    return rc;
}

static PyObject * csvloader_loads(PyObject * self, PyObject * args)
{
    int arg_size;
    char * content = NULL;
    PyObject * obj;
    Py_ssize_t length;

    /* Check if we have exactly one argument. */
    arg_size = PyTuple_GET_SIZE(args);

    if (arg_size != 1)
    {
        PyErr_SetString(
                PyExc_TypeError,
                "loads() missing 1 required positional argument");
        return NULL;
    }

    /* Check for a valid string and set content and length variable. */
    obj = PyTuple_GET_ITEM(args, 0);

#if PY_MAJOR_VERSION >= 3
    if (!PyUnicode_Check(obj))
#else
    if (!PyString_Check(obj))
#endif
    {
        PyErr_SetString(
                PyExc_TypeError,
                "loads() first argument must be a string.");
        return NULL;
    }

#if PY_MAJOR_VERSION >= 3
    if ((content = PyUnicode_AsUTF8AndSize(obj, &length)) == NULL)
#else
    if (PyString_AsStringAndSize(obj, &content, &length))
#endif
    {
        return NULL;  /* PyErr is set */
    }

    /* Create grid and first row. */
    if ((obj = PyList_New(0)) == NULL)
    {
        return NULL;
    }

    quoted_len = INIT_ALLOC_SZ;
    quoted_str = (char *) malloc(quoted_len);

    if (quoted_str == NULL || loads(obj, length, content))
    {
        Py_DECREF(obj);
        obj = NULL;
    }

    free(quoted_str);

    return obj;
}



