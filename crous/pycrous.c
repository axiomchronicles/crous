#include <Python.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "include/crous.h"

/* ============================================================================
   PYTHON MODULE ERRORS
   ============================================================================ */

static PyObject *CrousError = NULL;
static PyObject *CrousEncodeError = NULL;
static PyObject *CrousDecodeError = NULL;

/* ============================================================================
   PYTHON VALUE -> CROUS VALUE CONVERSION
   ============================================================================ */

static crous_value* pyobj_to_crous(PyObject *obj, crous_err_t *err);

static crous_value* pyobj_to_crous(PyObject *obj, crous_err_t *err) {
    *err = CROUS_OK;
    
    /* None */
    if (obj == Py_None) {
        return crous_value_new_null();
    }
    
    /* Booleans */
    if (PyBool_Check(obj)) {
        return crous_value_new_bool(obj == Py_True ? 1 : 0);
    }
    
    /* Integers */
    if (PyLong_Check(obj)) {
        long val = PyLong_AsLong(obj);
        if (PyErr_Occurred()) {
            *err = CROUS_ERR_DECODE;
            return NULL;
        }
        return crous_value_new_int((int64_t)val);
    }
    
    /* Floats */
    if (PyFloat_Check(obj)) {
        double val = PyFloat_AsDouble(obj);
        if (PyErr_Occurred()) {
            *err = CROUS_ERR_DECODE;
            return NULL;
        }
        return crous_value_new_float(val);
    }
    
    /* Strings */
    if (PyUnicode_Check(obj)) {
        Py_ssize_t len;
        const char *data = PyUnicode_AsUTF8AndSize(obj, &len);
        if (!data) {
            *err = CROUS_ERR_DECODE;
            return NULL;
        }
        return crous_value_new_string(data, (size_t)len);
    }
    
    /* Bytes */
    if (PyBytes_Check(obj)) {
        char *data;
        Py_ssize_t len;
        if (PyBytes_AsStringAndSize(obj, &data, &len) < 0) {
            *err = CROUS_ERR_DECODE;
            return NULL;
        }
        return crous_value_new_bytes((uint8_t *)data, (size_t)len);
    }
    
    /* Lists */
    if (PyList_Check(obj)) {
        Py_ssize_t size = PyList_Size(obj);
        if (size < 0) {
            *err = CROUS_ERR_DECODE;
            return NULL;
        }
        
        crous_value *list = crous_value_new_list((size_t)size);
        if (!list) {
            *err = CROUS_ERR_OOM;
            return NULL;
        }
        
        for (Py_ssize_t i = 0; i < size; i++) {
            PyObject *item = PyList_GetItem(obj, i);
            if (!item) {
                crous_value_free_tree(list);
                *err = CROUS_ERR_DECODE;
                return NULL;
            }
            
            crous_value *citem = pyobj_to_crous(item, err);
            if (*err != CROUS_OK) {
                crous_value_free_tree(list);
                return NULL;
            }
            
            if (crous_value_list_append(list, citem) != CROUS_OK) {
                crous_value_free_tree(list);
                crous_value_free_tree(citem);
                *err = CROUS_ERR_OOM;
                return NULL;
            }
        }
        
        return list;
    }
    
    /* Tuples */
    if (PyTuple_Check(obj)) {
        Py_ssize_t size = PyTuple_Size(obj);
        if (size < 0) {
            *err = CROUS_ERR_DECODE;
            return NULL;
        }
        
        crous_value *tuple = crous_value_new_tuple((size_t)size);
        if (!tuple) {
            *err = CROUS_ERR_OOM;
            return NULL;
        }
        
        for (Py_ssize_t i = 0; i < size; i++) {
            PyObject *item = PyTuple_GetItem(obj, i);
            if (!item) {
                crous_value_free_tree(tuple);
                *err = CROUS_ERR_DECODE;
                return NULL;
            }
            
            crous_value *citem = pyobj_to_crous(item, err);
            if (*err != CROUS_OK) {
                crous_value_free_tree(tuple);
                return NULL;
            }
            
            if (crous_value_list_append(tuple, citem) != CROUS_OK) {
                crous_value_free_tree(tuple);
                crous_value_free_tree(citem);
                *err = CROUS_ERR_OOM;
                return NULL;
            }
        }
        
        return tuple;
    }
    
    /* Dictionaries */
    if (PyDict_Check(obj)) {
        crous_value *dict = crous_value_new_dict(0);
        if (!dict) {
            *err = CROUS_ERR_OOM;
            return NULL;
        }
        
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        
        while (PyDict_Next(obj, &pos, &key, &value)) {
            if (!PyUnicode_Check(key)) {
                crous_value_free_tree(dict);
                PyErr_SetString(CrousEncodeError, "Dictionary keys must be strings");
                *err = CROUS_ERR_INVALID_TYPE;
                return NULL;
            }
            
            Py_ssize_t klen;
            const char *kdata = PyUnicode_AsUTF8AndSize(key, &klen);
            if (!kdata) {
                crous_value_free_tree(dict);
                *err = CROUS_ERR_DECODE;
                return NULL;
            }
            
            crous_value *cval = pyobj_to_crous(value, err);
            if (*err != CROUS_OK) {
                crous_value_free_tree(dict);
                return NULL;
            }
            
            if (crous_value_dict_set_binary(dict, kdata, (size_t)klen, cval) != CROUS_OK) {
                crous_value_free_tree(dict);
                crous_value_free_tree(cval);
                *err = CROUS_ERR_OOM;
                return NULL;
            }
        }
        
        return dict;
    }
    
    /* Unsupported type */
    PyErr_Format(CrousEncodeError, "Unsupported type for encoding: %s", 
                 Py_TYPE(obj)->tp_name);
    *err = CROUS_ERR_INVALID_TYPE;
    return NULL;
}

/* ============================================================================
   CROUS VALUE -> PYTHON VALUE CONVERSION
   ============================================================================ */

static PyObject* crous_to_pyobj(const crous_value *v);

static PyObject* crous_to_pyobj(const crous_value *v) {
    if (!v) {
        Py_RETURN_NONE;
    }
    
    switch (crous_value_get_type(v)) {
        case CROUS_TYPE_NULL:
            Py_RETURN_NONE;
        
        case CROUS_TYPE_BOOL:
            if (crous_value_get_bool(v)) {
                Py_RETURN_TRUE;
            } else {
                Py_RETURN_FALSE;
            }
        
        case CROUS_TYPE_INT:
            return PyLong_FromLongLong(crous_value_get_int(v));
        
        case CROUS_TYPE_FLOAT:
            return PyFloat_FromDouble(crous_value_get_float(v));
        
        case CROUS_TYPE_STRING: {
            size_t len;
            const char *data = crous_value_get_string(v, &len);
            return PyUnicode_FromStringAndSize(data, (Py_ssize_t)len);
        }
        
        case CROUS_TYPE_BYTES: {
            size_t len;
            const uint8_t *data = crous_value_get_bytes(v, &len);
            return PyBytes_FromStringAndSize((const char *)data, (Py_ssize_t)len);
        }
        
        case CROUS_TYPE_LIST: {
            size_t size = crous_value_list_size(v);
            PyObject *list = PyList_New((Py_ssize_t)size);
            if (!list) return NULL;
            
            for (size_t i = 0; i < size; i++) {
                PyObject *item = crous_to_pyobj(crous_value_list_get(v, i));
                if (!item) {
                    Py_DECREF(list);
                    return NULL;
                }
                PyList_SetItem(list, (Py_ssize_t)i, item);
            }
            return list;
        }
        
        case CROUS_TYPE_TUPLE: {
            size_t size = crous_value_list_size(v);
            PyObject *tuple = PyTuple_New((Py_ssize_t)size);
            if (!tuple) return NULL;
            
            for (size_t i = 0; i < size; i++) {
                PyObject *item = crous_to_pyobj(crous_value_list_get(v, i));
                if (!item) {
                    Py_DECREF(tuple);
                    return NULL;
                }
                PyTuple_SetItem(tuple, (Py_ssize_t)i, item);
            }
            return tuple;
        }
        
        case CROUS_TYPE_DICT: {
            PyObject *dict = PyDict_New();
            if (!dict) return NULL;
            
            size_t size = crous_value_dict_size(v);
            for (size_t i = 0; i < size; i++) {
                const crous_dict_entry *entry = crous_value_dict_get_entry(v, i);
                if (!entry) {
                    Py_DECREF(dict);
                    return NULL;
                }
                
                PyObject *key = PyUnicode_FromStringAndSize(entry->key, (Py_ssize_t)entry->key_len);
                if (!key) {
                    Py_DECREF(dict);
                    return NULL;
                }
                
                PyObject *val = crous_to_pyobj(entry->value);
                if (!val) {
                    Py_DECREF(key);
                    Py_DECREF(dict);
                    return NULL;
                }
                
                if (PyDict_SetItem(dict, key, val) < 0) {
                    Py_DECREF(key);
                    Py_DECREF(val);
                    Py_DECREF(dict);
                    return NULL;
                }
                
                Py_DECREF(key);
                Py_DECREF(val);
            }
            return dict;
        }
        
        case CROUS_TYPE_TAGGED: {
            /* For now, just return the inner value. Tagged types can be extended. */
            return crous_to_pyobj(crous_value_get_tagged_inner(v));
        }
        
        default:
            PyErr_SetString(CrousError, "Unknown crous value type");
            return NULL;
    }
}

/* ============================================================================
   PYTHON MODULE FUNCTIONS
   ============================================================================ */

static PyObject* py_encode(PyObject *self, PyObject *args) {
    PyObject *obj;
    
    if (!PyArg_ParseTuple(args, "O", &obj)) {
        return NULL;
    }
    
    crous_err_t err = CROUS_OK;
    crous_value *value = pyobj_to_crous(obj, &err);
    if (!value) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(CrousEncodeError, crous_err_str(err));
        }
        return NULL;
    }
    
    uint8_t *buf = NULL;
    size_t size = 0;
    err = crous_encode(value, &buf, &size);
    crous_value_free_tree(value);
    
    if (err != CROUS_OK) {
        PyErr_SetString(CrousEncodeError, crous_err_str(err));
        free(buf);
        return NULL;
    }
    
    PyObject *result = PyBytes_FromStringAndSize((const char *)buf, (Py_ssize_t)size);
    free(buf);
    return result;
}

/* Alias: dumps = encode */
static PyObject* py_dumps(PyObject *self, PyObject *args) {
    return py_encode(self, args);
}

static PyObject* py_decode(PyObject *self, PyObject *args) {
    const uint8_t *buf;
    Py_ssize_t buf_size;
    
    if (!PyArg_ParseTuple(args, "y#", &buf, &buf_size)) {
        return NULL;
    }
    
    crous_value *value = NULL;
    crous_err_t err = crous_decode(buf, (size_t)buf_size, &value);
    
    if (err != CROUS_OK) {
        PyErr_SetString(CrousDecodeError, crous_err_str(err));
        if (value) crous_value_free_tree(value);
        return NULL;
    }
    
    PyObject *result = crous_to_pyobj(value);
    crous_value_free_tree(value);
    return result;
}

/* Alias: loads = decode */
static PyObject* py_loads(PyObject *self, PyObject *args) {
    return py_decode(self, args);
}

/* ============================================================================
   ENCODER/DECODER CLASSES (STUBS FOR NOW)
   ============================================================================ */

static PyObject* CrousEncoder = NULL;
static PyObject* CrousDecoder = NULL;

/* Forward declarations for stream functions */
static PyObject* py_dump(PyObject *self, PyObject *args, PyObject *kwargs);
static PyObject* py_load(PyObject *self, PyObject *args, PyObject *kwargs);

static PyObject* py_register_serializer(PyObject *self, PyObject *args) {
    /* Stub: Not yet implemented */
    Py_RETURN_NONE;
}

static PyObject* py_unregister_serializer(PyObject *self, PyObject *args) {
    /* Stub: Not yet implemented */
    Py_RETURN_NONE;
}

static PyObject* py_register_decoder(PyObject *self, PyObject *args) {
    /* Stub: Not yet implemented */
    Py_RETURN_NONE;
}

static PyObject* py_unregister_decoder(PyObject *self, PyObject *args) {
    /* Stub: Not yet implemented */
    Py_RETURN_NONE;
}

static PyObject* py_dumps_stream(PyObject *self, PyObject *args) {
    /* Alias: dumps_stream = dump for stream compatibility */
    return py_dump(self, args, NULL);
}

static PyObject* py_loads_stream(PyObject *self, PyObject *args) {
    /* Alias: loads_stream = load for stream compatibility */
    return py_load(self, args, NULL);
}

/* ============================================================================
   FILE I/O FUNCTIONS
   ============================================================================ */

static PyObject* py_dump(PyObject *self, PyObject *args, PyObject *kwargs) {
    PyObject *obj;
    PyObject *fp;
    PyObject *default_obj = NULL;
    static char *kwlist[] = {"obj", "fp", "default", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|O", kwlist, &obj, &fp, &default_obj)) {
        return NULL;
    }
    
    /* Convert Python object to crous value */
    crous_err_t err = CROUS_OK;
    crous_value *value = pyobj_to_crous(obj, &err);
    if (!value) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(CrousEncodeError, crous_err_str(err));
        }
        return NULL;
    }
    
    /* Encode to binary */
    uint8_t *buf = NULL;
    size_t size = 0;
    err = crous_encode(value, &buf, &size);
    crous_value_free_tree(value);
    
    if (err != CROUS_OK) {
        PyErr_SetString(CrousEncodeError, crous_err_str(err));
        free(buf);
        return NULL;
    }
    
    /* Write to file object */
    PyObject *write_method = PyObject_GetAttrString(fp, "write");
    if (!write_method) {
        PyErr_SetString(PyExc_TypeError, "fp must have a write() method");
        free(buf);
        return NULL;
    }
    
    PyObject *bytes_obj = PyBytes_FromStringAndSize((const char *)buf, (Py_ssize_t)size);
    free(buf);
    if (!bytes_obj) {
        Py_DECREF(write_method);
        return NULL;
    }
    
    PyObject *result = PyObject_CallFunctionObjArgs(write_method, bytes_obj, NULL);
    Py_DECREF(bytes_obj);
    Py_DECREF(write_method);
    
    if (!result) {
        return NULL;
    }
    
    Py_DECREF(result);
    Py_RETURN_NONE;
}

static PyObject* py_load(PyObject *self, PyObject *args, PyObject *kwargs) {
    PyObject *fp;
    PyObject *object_hook = NULL;
    static char *kwlist[] = {"fp", "object_hook", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O", kwlist, &fp, &object_hook)) {
        return NULL;
    }
    
    /* Read from file object */
    PyObject *read_method = PyObject_GetAttrString(fp, "read");
    if (!read_method) {
        PyErr_SetString(PyExc_TypeError, "fp must have a read() method");
        return NULL;
    }
    
    PyObject *bytes_obj = PyObject_CallFunction(read_method, NULL);
    Py_DECREF(read_method);
    
    if (!bytes_obj) {
        return NULL;
    }
    
    if (!PyBytes_Check(bytes_obj)) {
        PyErr_SetString(PyExc_TypeError, "read() must return bytes");
        Py_DECREF(bytes_obj);
        return NULL;
    }
    
    const uint8_t *buf = (uint8_t *)PyBytes_AsString(bytes_obj);
    Py_ssize_t buf_size = PyBytes_Size(bytes_obj);
    
    /* Decode from binary */
    crous_value *value = NULL;
    crous_err_t err = crous_decode(buf, (size_t)buf_size, &value);
    
    Py_DECREF(bytes_obj);
    
    if (err != CROUS_OK) {
        PyErr_SetString(CrousDecodeError, crous_err_str(err));
        if (value) crous_value_free_tree(value);
        return NULL;
    }
    
    /* Convert crous value to Python object */
    PyObject *result = crous_to_pyobj(value);
    crous_value_free_tree(value);
    return result;
}

/* ============================================================================
   MODULE SETUP
   ============================================================================ */

static PyMethodDef crous_methods[] = {
    {"encode", py_encode, METH_VARARGS, "Encode Python object to CROUS binary format"},
    {"dumps", py_dumps, METH_VARARGS, "Encode Python object to CROUS binary format (alias for encode)"},
    {"decode", py_decode, METH_VARARGS, "Decode CROUS binary format to Python object"},
    {"loads", py_loads, METH_VARARGS, "Decode CROUS binary format to Python object (alias for decode)"},
    {"dump", (PyCFunction)py_dump, METH_VARARGS | METH_KEYWORDS, "Serialize object to file"},
    {"load", (PyCFunction)py_load, METH_VARARGS | METH_KEYWORDS, "Deserialize object from file"},
    {"dumps_stream", (PyCFunction)py_dumps_stream, METH_VARARGS, "Serialize object to stream"},
    {"loads_stream", (PyCFunction)py_loads_stream, METH_VARARGS, "Deserialize object from stream"},
    {"register_serializer", py_register_serializer, METH_VARARGS, "Register custom serializer"},
    {"unregister_serializer", py_unregister_serializer, METH_VARARGS, "Unregister custom serializer"},
    {"register_decoder", py_register_decoder, METH_VARARGS, "Register custom decoder"},
    {"unregister_decoder", py_unregister_decoder, METH_VARARGS, "Unregister custom decoder"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef crous_module = {
    PyModuleDef_HEAD_INIT,
    "crous",
    "CROUS - Compact Rapid Object Utility Serialization",
    -1,
    crous_methods
};

PyMODINIT_FUNC PyInit_crous(void) {
    PyObject *m = PyModule_Create(&crous_module);
    if (!m) return NULL;
    
    /* Create exception classes */
    CrousError = PyErr_NewException("crous.CrousError", NULL, NULL);
    if (!CrousError) {
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(CrousError);
    if (PyModule_AddObject(m, "CrousError", CrousError) < 0) {
        Py_DECREF(CrousError);
        Py_DECREF(m);
        return NULL;
    }
    
    CrousEncodeError = PyErr_NewException("crous.CrousEncodeError", CrousError, NULL);
    if (!CrousEncodeError) {
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(CrousEncodeError);
    if (PyModule_AddObject(m, "CrousEncodeError", CrousEncodeError) < 0) {
        Py_DECREF(CrousEncodeError);
        Py_DECREF(m);
        return NULL;
    }
    
    CrousDecodeError = PyErr_NewException("crous.CrousDecodeError", CrousError, NULL);
    if (!CrousDecodeError) {
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(CrousDecodeError);
    if (PyModule_AddObject(m, "CrousDecodeError", CrousDecodeError) < 0) {
        Py_DECREF(CrousDecodeError);
        Py_DECREF(m);
        return NULL;
    }
    
    /* Create stub encoder/decoder classes */
    CrousEncoder = (PyObject *)&PyType_Type;  /* Use type as placeholder */
    Py_INCREF(CrousEncoder);
    if (PyModule_AddObject(m, "CrousEncoder", CrousEncoder) < 0) {
        Py_DECREF(CrousEncoder);
        Py_DECREF(m);
        return NULL;
    }
    
    CrousDecoder = (PyObject *)&PyType_Type;  /* Use type as placeholder */
    Py_INCREF(CrousDecoder);
    if (PyModule_AddObject(m, "CrousDecoder", CrousDecoder) < 0) {
        Py_DECREF(CrousDecoder);
        Py_DECREF(m);
        return NULL;
    }
    
    return m;
}
