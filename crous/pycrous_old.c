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
   CUSTOM SERIALIZER/DECODER REGISTRIES
   ============================================================================ */

/* Dictionary mapping Python types to serializer functions */
static PyObject *custom_serializers = NULL;

/* Dictionary mapping tag integers to decoder functions */
static PyObject *custom_decoders = NULL;

/* Tag counter for auto-assignment */
static uint32_t next_custom_tag = 100;

/* Dictionary mapping Python types to their assigned tags */
static PyObject *type_to_tag = NULL;

/* ============================================================================
   FORWARD DECLARATIONS
   ============================================================================ */

static crous_value* pyobj_to_crous_with_default(PyObject *obj, PyObject *default_func, crous_err_t *err);
static PyObject* crous_to_pyobj_with_hook(const crous_value *v, PyObject *object_hook);

/* ============================================================================
   CUSTOM SERIALIZER IMPLEMENTATION
   ============================================================================ */

/**
 * Try to serialize an object using custom serializers.
 * Returns: new crous_value* on success, NULL if no custom serializer or on error.
 * Sets *handled = 1 if a custom serializer was found (even if it failed).
 */
static crous_value* try_custom_serializer(PyObject *obj, PyObject *default_func, 
                                          crous_err_t *err, int *handled) {
    *handled = 0;
    
    if (!custom_serializers || PyDict_Size(custom_serializers) == 0) {
        return NULL;
    }
    
    /* Get the type of the object */
    PyObject *obj_type = (PyObject *)Py_TYPE(obj);
    
    /* Look up serializer for this type */
    PyObject *serializer = PyDict_GetItem(custom_serializers, obj_type);
    
    if (!serializer) {
        /* Try checking base classes (MRO lookup) */
        PyObject *mro = PyType_GetMro((PyTypeObject *)obj_type);
        if (mro) {
            Py_ssize_t mro_len = PyTuple_Size(mro);
            for (Py_ssize_t i = 0; i < mro_len; i++) {
                PyObject *base = PyTuple_GetItem(mro, i);
                serializer = PyDict_GetItem(custom_serializers, base);
                if (serializer) break;
            }
        }
    }
    
    if (!serializer && default_func && default_func != Py_None) {
        /* Use the default function as fallback */
        serializer = default_func;
    }
    
    if (!serializer) {
        return NULL;
    }
    
    *handled = 1;
    
    /* Call the serializer function */
    PyObject *result = PyObject_CallFunctionObjArgs(serializer, obj, NULL);
    if (!result) {
        *err = CROUS_ERR_ENCODE;
        return NULL;
    }
    
    /* Get the tag for this type */
    uint32_t tag = 100;  /* Default custom tag */
    PyObject *tag_obj = PyDict_GetItem(type_to_tag, obj_type);
    if (tag_obj && PyLong_Check(tag_obj)) {
        tag = (uint32_t)PyLong_AsUnsignedLong(tag_obj);
    }
    
    /* Recursively convert the result */
    crous_value *inner = pyobj_to_crous_with_default(result, NULL, err);
    Py_DECREF(result);
    
    if (!inner) {
        return NULL;
    }
    
    /* Wrap in tagged value */
    crous_value *tagged = crous_value_new_tagged(tag, inner);
    if (!tagged) {
        crous_value_free_tree(inner);
        *err = CROUS_ERR_OOM;
        return NULL;
    }
    
    return tagged;
}

/* ============================================================================
   PYTHON VALUE -> CROUS VALUE CONVERSION
   ============================================================================ */

static crous_value* pyobj_to_crous_with_default(PyObject *obj, PyObject *default_func, crous_err_t *err) {
    *err = CROUS_OK;
    
    /* None */
    if (obj == Py_None) {
        return crous_value_new_null();
    }
    
    /* Booleans (must check before int since bool is subclass of int) */
    if (PyBool_Check(obj)) {
        return crous_value_new_bool(obj == Py_True ? 1 : 0);
    }
    
    /* Integers - use proper overflow handling */
    if (PyLong_Check(obj)) {
        int overflow = 0;
        long long val = PyLong_AsLongLongAndOverflow(obj, &overflow);
        
        if (overflow != 0) {
            /* Value doesn't fit in long long, try custom serializer or fail */
            int handled = 0;
            crous_value *result = try_custom_serializer(obj, default_func, err, &handled);
            if (handled) return result;
            
            PyErr_SetString(CrousEncodeError, "Integer value too large to serialize");
            *err = CROUS_ERR_OVERFLOW;
            return NULL;
        }
        
        if (PyErr_Occurred()) {
            *err = CROUS_ERR_ENCODE;
            return NULL;
        }
        
        return crous_value_new_int((int64_t)val);
    }
    
    /* Floats */
    if (PyFloat_Check(obj)) {
        double val = PyFloat_AsDouble(obj);
        if (PyErr_Occurred()) {
            *err = CROUS_ERR_ENCODE;
            return NULL;
        }
        return crous_value_new_float(val);
    }
    
    /* Strings */
    if (PyUnicode_Check(obj)) {
        Py_ssize_t len;
        const char *data = PyUnicode_AsUTF8AndSize(obj, &len);
        if (!data) {
            *err = CROUS_ERR_ENCODE;
            return NULL;
        }
        return crous_value_new_string(data, (size_t)len);
    }
    
    /* Bytes */
    if (PyBytes_Check(obj)) {
        char *data;
        Py_ssize_t len;
        if (PyBytes_AsStringAndSize(obj, &data, &len) < 0) {
            *err = CROUS_ERR_ENCODE;
            return NULL;
        }
        return crous_value_new_bytes((uint8_t *)data, (size_t)len);
    }
    
    /* Bytearray */
    if (PyByteArray_Check(obj)) {
        char *data = PyByteArray_AsString(obj);
        Py_ssize_t len = PyByteArray_Size(obj);
        if (!data) {
            *err = CROUS_ERR_ENCODE;
            return NULL;
        }
        return crous_value_new_bytes((uint8_t *)data, (size_t)len);
    }
    
    /* Lists */
    if (PyList_Check(obj)) {
        Py_ssize_t size = PyList_Size(obj);
        if (size < 0) {
            *err = CROUS_ERR_ENCODE;
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
                *err = CROUS_ERR_ENCODE;
                return NULL;
            }
            
            crous_value *citem = pyobj_to_crous_with_default(item, default_func, err);
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
            *err = CROUS_ERR_ENCODE;
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
                *err = CROUS_ERR_ENCODE;
                return NULL;
            }
            
            crous_value *citem = pyobj_to_crous_with_default(item, default_func, err);
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
                *err = CROUS_ERR_ENCODE;
                return NULL;
            }
            
            crous_value *cval = pyobj_to_crous_with_default(value, default_func, err);
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
    
    /* Sets - convert to list with tag */
    if (PySet_Check(obj) || PyFrozenSet_Check(obj)) {
        int handled = 0;
        crous_value *result = try_custom_serializer(obj, default_func, err, &handled);
        if (handled) return result;
        
        /* Default: convert to list */
        PyObject *as_list = PySequence_List(obj);
        if (!as_list) {
            *err = CROUS_ERR_ENCODE;
            return NULL;
        }
        
        crous_value *list_val = pyobj_to_crous_with_default(as_list, default_func, err);
        Py_DECREF(as_list);
        
        if (*err != CROUS_OK) return NULL;
        
        /* Wrap in tagged value with set tag (90 for set, 91 for frozenset) */
        uint32_t tag = PyFrozenSet_Check(obj) ? 91 : 90;
        crous_value *tagged = crous_value_new_tagged(tag, list_val);
        if (!tagged) {
            crous_value_free_tree(list_val);
            *err = CROUS_ERR_OOM;
            return NULL;
        }
        return tagged;
    }
    
    /* Try custom serializer for unsupported types */
    int handled = 0;
    crous_value *result = try_custom_serializer(obj, default_func, err, &handled);
    if (handled) return result;
    
    /* Unsupported type */
    PyErr_Format(CrousEncodeError, "Unsupported type for encoding: %s", 
                 Py_TYPE(obj)->tp_name);
    *err = CROUS_ERR_INVALID_TYPE;
    return NULL;
}

/* Legacy function for backwards compatibility */
static crous_value* pyobj_to_crous(PyObject *obj, crous_err_t *err) {
    return pyobj_to_crous_with_default(obj, NULL, err);
}

/* ============================================================================
   CROUS VALUE -> PYTHON VALUE CONVERSION
   ============================================================================ */

static PyObject* crous_to_pyobj_with_hook(const crous_value *v, PyObject *object_hook) {
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
                PyObject *item = crous_to_pyobj_with_hook(crous_value_list_get(v, i), object_hook);
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
                PyObject *item = crous_to_pyobj_with_hook(crous_value_list_get(v, i), object_hook);
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
                
                PyObject *val = crous_to_pyobj_with_hook(entry->value, object_hook);
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
            
            /* Apply object_hook if provided */
            if (object_hook && object_hook != Py_None) {
                PyObject *result = PyObject_CallFunctionObjArgs(object_hook, dict, NULL);
                Py_DECREF(dict);
                return result;
            }
            
            return dict;
        }
        
        case CROUS_TYPE_TAGGED: {
            uint32_t tag = crous_value_get_tag(v);
            const crous_value *inner = crous_value_get_tagged_inner(v);
            
            /* Check for built-in tag types */
            if (tag == 90) {
                /* Set */
                PyObject *list = crous_to_pyobj_with_hook(inner, object_hook);
                if (!list) return NULL;
                PyObject *set = PySet_New(list);
                Py_DECREF(list);
                return set;
            } else if (tag == 91) {
                /* Frozenset */
                PyObject *list = crous_to_pyobj_with_hook(inner, object_hook);
                if (!list) return NULL;
                PyObject *fset = PyFrozenSet_New(list);
                Py_DECREF(list);
                return fset;
            }
            
            /* Check for custom decoder */
            if (custom_decoders) {
                PyObject *tag_key = PyLong_FromUnsignedLong(tag);
                if (tag_key) {
                    PyObject *decoder = PyDict_GetItem(custom_decoders, tag_key);
                    Py_DECREF(tag_key);
                    
                    if (decoder) {
                        /* Get the inner value as Python object */
                        PyObject *inner_py = crous_to_pyobj_with_hook(inner, object_hook);
                        if (!inner_py) return NULL;
                        
                        /* Call the decoder */
                        PyObject *result = PyObject_CallFunctionObjArgs(decoder, inner_py, NULL);
                        Py_DECREF(inner_py);
                        return result;
                    }
                }
            }
            
            /* No decoder found, return inner value */
            return crous_to_pyobj_with_hook(inner, object_hook);
        }
        
        default:
            PyErr_SetString(CrousError, "Unknown crous value type");
            return NULL;
    }
}

/* Legacy function */
static PyObject* crous_to_pyobj(const crous_value *v) {
    return crous_to_pyobj_with_hook(v, NULL);
}

/* ============================================================================
   CROUSENCODER CLASS
   ============================================================================ */

typedef struct {
    PyObject_HEAD
    PyObject *default_func;
    int allow_custom;
} CrousEncoderObject;

static void CrousEncoder_dealloc(CrousEncoderObject *self) {
    Py_XDECREF(self->default_func);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject* CrousEncoder_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    CrousEncoderObject *self = (CrousEncoderObject *)type->tp_alloc(type, 0);
    if (self) {
        self->default_func = NULL;
        self->allow_custom = 1;
    }
    return (PyObject *)self;
}

static int CrousEncoder_init(CrousEncoderObject *self, PyObject *args, PyObject *kwargs) {
    static char *kwlist[] = {"default", "allow_custom", NULL};
    PyObject *default_func = NULL;
    int allow_custom = 1;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Op", kwlist, 
                                      &default_func, &allow_custom)) {
        return -1;
    }
    
    Py_XINCREF(default_func);
    Py_XDECREF(self->default_func);
    self->default_func = default_func;
    self->allow_custom = allow_custom;
    
    return 0;
}

static PyObject* CrousEncoder_encode(CrousEncoderObject *self, PyObject *args) {
    PyObject *obj;
    
    if (!PyArg_ParseTuple(args, "O", &obj)) {
        return NULL;
    }
    
    crous_err_t err = CROUS_OK;
    crous_value *value = pyobj_to_crous_with_default(obj, self->default_func, &err);
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

static PyMethodDef CrousEncoder_methods[] = {
    {"encode", (PyCFunction)CrousEncoder_encode, METH_VARARGS, 
     "Encode a Python object to Crous binary format."},
    {NULL, NULL, 0, NULL}
};

static PyTypeObject CrousEncoderType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "crous.CrousEncoder",
    .tp_doc = "Crous encoder class for serializing Python objects to binary format.",
    .tp_basicsize = sizeof(CrousEncoderObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = CrousEncoder_new,
    .tp_init = (initproc)CrousEncoder_init,
    .tp_dealloc = (destructor)CrousEncoder_dealloc,
    .tp_methods = CrousEncoder_methods,
};

/* ============================================================================
   CROUSDECODER CLASS
   ============================================================================ */

typedef struct {
    PyObject_HEAD
    PyObject *object_hook;
} CrousDecoderObject;

static void CrousDecoder_dealloc(CrousDecoderObject *self) {
    Py_XDECREF(self->object_hook);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject* CrousDecoder_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    CrousDecoderObject *self = (CrousDecoderObject *)type->tp_alloc(type, 0);
    if (self) {
        self->object_hook = NULL;
    }
    return (PyObject *)self;
}

static int CrousDecoder_init(CrousDecoderObject *self, PyObject *args, PyObject *kwargs) {
    static char *kwlist[] = {"object_hook", NULL};
    PyObject *object_hook = NULL;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O", kwlist, &object_hook)) {
        return -1;
    }
    
    Py_XINCREF(object_hook);
    Py_XDECREF(self->object_hook);
    self->object_hook = object_hook;
    
    return 0;
}

static PyObject* CrousDecoder_decode(CrousDecoderObject *self, PyObject *args) {
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
    
    PyObject *result = crous_to_pyobj_with_hook(value, self->object_hook);
    crous_value_free_tree(value);
    return result;
}

static PyMethodDef CrousDecoder_methods[] = {
    {"decode", (PyCFunction)CrousDecoder_decode, METH_VARARGS,
     "Decode Crous binary data to a Python object."},
    {NULL, NULL, 0, NULL}
};

static PyTypeObject CrousDecoderType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "crous.CrousDecoder",
    .tp_doc = "Crous decoder class for deserializing binary data to Python objects.",
    .tp_basicsize = sizeof(CrousDecoderObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = CrousDecoder_new,
    .tp_init = (initproc)CrousDecoder_init,
    .tp_dealloc = (destructor)CrousDecoder_dealloc,
    .tp_methods = CrousDecoder_methods,
};

/* ============================================================================
   PYTHON MODULE FUNCTIONS
   ============================================================================ */

static PyObject* py_dumps(PyObject *self, PyObject *args, PyObject *kwargs) {
    PyObject *obj;
    PyObject *default_func = NULL;
    PyObject *encoder = NULL;
    int allow_custom = 1;
    static char *kwlist[] = {"obj", "default", "encoder", "allow_custom", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|OOp", kwlist, 
                                      &obj, &default_func, &encoder, &allow_custom)) {
        return NULL;
    }
    
    crous_err_t err = CROUS_OK;
    crous_value *value = pyobj_to_crous_with_default(obj, default_func, &err);
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

static PyObject* py_loads(PyObject *self, PyObject *args, PyObject *kwargs) {
    const uint8_t *buf;
    Py_ssize_t buf_size;
    PyObject *object_hook = NULL;
    PyObject *decoder = NULL;
    static char *kwlist[] = {"data", "object_hook", "decoder", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y#|OO", kwlist, 
                                      &buf, &buf_size, &object_hook, &decoder)) {
        return NULL;
    }
    
    crous_value *value = NULL;
    crous_err_t err = crous_decode(buf, (size_t)buf_size, &value);
    
    if (err != CROUS_OK) {
        PyErr_SetString(CrousDecodeError, crous_err_str(err));
        if (value) crous_value_free_tree(value);
        return NULL;
    }
    
    PyObject *result = crous_to_pyobj_with_hook(value, object_hook);
    crous_value_free_tree(value);
    return result;
}

static PyObject* py_dump(PyObject *self, PyObject *args, PyObject *kwargs) {
    PyObject *obj;
    PyObject *fp;
    PyObject *default_func = NULL;
    static char *kwlist[] = {"obj", "fp", "default", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|O", kwlist, 
                                      &obj, &fp, &default_func)) {
        return NULL;
    }
    
    /* Convert Python object to crous value */
    crous_err_t err = CROUS_OK;
    crous_value *value = pyobj_to_crous_with_default(obj, default_func, &err);
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
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O", kwlist, 
                                      &fp, &object_hook)) {
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
    PyObject *result = crous_to_pyobj_with_hook(value, object_hook);
    crous_value_free_tree(value);
    return result;
}

static PyObject* py_dumps_stream(PyObject *self, PyObject *args, PyObject *kwargs) {
    PyObject *obj;
    PyObject *fp;
    PyObject *default_func = NULL;
    static char *kwlist[] = {"obj", "fp", "default", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|O", kwlist, 
                                      &obj, &fp, &default_func)) {
        return NULL;
    }
    
    /* Same as dump */
    return py_dump(self, args, kwargs);
}

static PyObject* py_loads_stream(PyObject *self, PyObject *args, PyObject *kwargs) {
    PyObject *fp;
    PyObject *object_hook = NULL;
    static char *kwlist[] = {"fp", "object_hook", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O", kwlist, 
                                      &fp, &object_hook)) {
        return NULL;
    }
    
    /* Same as load */
    return py_load(self, args, kwargs);
}

/* ============================================================================
   CUSTOM SERIALIZER/DECODER REGISTRATION
   ============================================================================ */

static PyObject* py_register_serializer(PyObject *self, PyObject *args) {
    PyObject *type_obj;
    PyObject *func;
    
    if (!PyArg_ParseTuple(args, "OO", &type_obj, &func)) {
        return NULL;
    }
    
    if (!PyType_Check(type_obj)) {
        PyErr_SetString(PyExc_TypeError, "First argument must be a type");
        return NULL;
    }
    
    if (!PyCallable_Check(func)) {
        PyErr_SetString(PyExc_TypeError, "Second argument must be callable");
        return NULL;
    }
    
    /* Initialize registries if needed */
    if (!custom_serializers) {
        custom_serializers = PyDict_New();
        if (!custom_serializers) return NULL;
    }
    if (!type_to_tag) {
        type_to_tag = PyDict_New();
        if (!type_to_tag) return NULL;
    }
    
    /* Add to registry */
    if (PyDict_SetItem(custom_serializers, type_obj, func) < 0) {
        return NULL;
    }
    
    /* Assign a tag to this type */
    PyObject *tag_obj = PyLong_FromUnsignedLong(next_custom_tag++);
    if (!tag_obj) return NULL;
    
    if (PyDict_SetItem(type_to_tag, type_obj, tag_obj) < 0) {
        Py_DECREF(tag_obj);
        return NULL;
    }
    Py_DECREF(tag_obj);
    
    Py_RETURN_NONE;
}

static PyObject* py_unregister_serializer(PyObject *self, PyObject *args) {
    PyObject *type_obj;
    
    if (!PyArg_ParseTuple(args, "O", &type_obj)) {
        return NULL;
    }
    
    if (!PyType_Check(type_obj)) {
        PyErr_SetString(PyExc_TypeError, "Argument must be a type");
        return NULL;
    }
    
    if (custom_serializers) {
        PyDict_DelItem(custom_serializers, type_obj);
        PyErr_Clear();  /* Ignore if key not found */
    }
    
    if (type_to_tag) {
        PyDict_DelItem(type_to_tag, type_obj);
        PyErr_Clear();
    }
    
    Py_RETURN_NONE;
}

static PyObject* py_register_decoder(PyObject *self, PyObject *args) {
    unsigned int tag;
    PyObject *func;
    
    if (!PyArg_ParseTuple(args, "IO", &tag, &func)) {
        return NULL;
    }
    
    if (!PyCallable_Check(func)) {
        PyErr_SetString(PyExc_TypeError, "Second argument must be callable");
        return NULL;
    }
    
    /* Initialize registry if needed */
    if (!custom_decoders) {
        custom_decoders = PyDict_New();
        if (!custom_decoders) return NULL;
    }
    
    /* Add to registry */
    PyObject *tag_key = PyLong_FromUnsignedLong(tag);
    if (!tag_key) return NULL;
    
    if (PyDict_SetItem(custom_decoders, tag_key, func) < 0) {
        Py_DECREF(tag_key);
        return NULL;
    }
    
    Py_DECREF(tag_key);
    Py_RETURN_NONE;
}

static PyObject* py_unregister_decoder(PyObject *self, PyObject *args) {
    unsigned int tag;
    
    if (!PyArg_ParseTuple(args, "I", &tag)) {
        return NULL;
    }
    
    if (custom_decoders) {
        PyObject *tag_key = PyLong_FromUnsignedLong(tag);
        if (tag_key) {
            PyDict_DelItem(custom_decoders, tag_key);
            Py_DECREF(tag_key);
            PyErr_Clear();  /* Ignore if key not found */
        }
    }
    
    Py_RETURN_NONE;
}

/* ============================================================================
   MODULE SETUP
   ============================================================================ */

static PyMethodDef crous_methods[] = {
    {"dumps", (PyCFunction)py_dumps, METH_VARARGS | METH_KEYWORDS, 
     "Encode Python object to CROUS binary format.\n\n"
     "Args:\n"
     "    obj: Python object to serialize\n"
     "    default: Optional callable for custom types\n"
     "    encoder: Optional encoder instance (reserved)\n"
     "    allow_custom: Whether to allow custom types (default True)\n\n"
     "Returns:\n"
     "    bytes: Binary encoded data"},
    {"loads", (PyCFunction)py_loads, METH_VARARGS | METH_KEYWORDS, 
     "Decode CROUS binary format to Python object.\n\n"
     "Args:\n"
     "    data: Bytes to decode\n"
     "    object_hook: Optional callable for dict post-processing\n"
     "    decoder: Optional decoder instance (reserved)\n\n"
     "Returns:\n"
     "    Deserialized Python object"},
    {"dump", (PyCFunction)py_dump, METH_VARARGS | METH_KEYWORDS, 
     "Serialize object to file.\n\n"
     "Args:\n"
     "    obj: Python object to serialize\n"
     "    fp: File-like object with write() method\n"
     "    default: Optional callable for custom types"},
    {"load", (PyCFunction)py_load, METH_VARARGS | METH_KEYWORDS, 
     "Deserialize object from file.\n\n"
     "Args:\n"
     "    fp: File-like object with read() method\n"
     "    object_hook: Optional callable for dict post-processing"},
    {"dumps_stream", (PyCFunction)py_dumps_stream, METH_VARARGS | METH_KEYWORDS, 
     "Serialize object to stream (same as dump for file objects)."},
    {"loads_stream", (PyCFunction)py_loads_stream, METH_VARARGS | METH_KEYWORDS, 
     "Deserialize object from stream (same as load for file objects)."},
    {"register_serializer", py_register_serializer, METH_VARARGS, 
     "Register a custom serializer for a Python type.\n\n"
     "Args:\n"
     "    typ: The Python type to register\n"
     "    func: Callable(obj) -> serializable value\n\n"
     "Example:\n"
     "    def serialize_datetime(dt):\n"
     "        return dt.isoformat()\n"
     "    crous.register_serializer(datetime, serialize_datetime)"},
    {"unregister_serializer", py_unregister_serializer, METH_VARARGS, 
     "Unregister a custom serializer.\n\n"
     "Args:\n"
     "    typ: The type to unregister"},
    {"register_decoder", py_register_decoder, METH_VARARGS, 
     "Register a custom decoder for a tagged value.\n\n"
     "Args:\n"
     "    tag: Tag identifier (int)\n"
     "    func: Callable(value) -> decoded object\n\n"
     "Example:\n"
     "    def decode_datetime(value):\n"
     "        return datetime.fromisoformat(value)\n"
     "    crous.register_decoder(100, decode_datetime)"},
    {"unregister_decoder", py_unregister_decoder, METH_VARARGS, 
     "Unregister a custom decoder.\n\n"
     "Args:\n"
     "    tag: Tag identifier to unregister"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef crous_module = {
    PyModuleDef_HEAD_INIT,
    "crous",
    "CROUS - Compact Rapid Object Utility Serialization\n\n"
    "A high-performance binary serialization format for Python.\n\n"
    "Basic usage:\n"
    "    import crous\n"
    "    data = {'key': 'value', 'number': 42}\n"
    "    binary = crous.dumps(data)\n"
    "    result = crous.loads(binary)\n",
    -1,
    crous_methods
};

PyMODINIT_FUNC PyInit_crous(void) {
    PyObject *m = PyModule_Create(&crous_module);
    if (!m) return NULL;
    
    /* Initialize type objects */
    if (PyType_Ready(&CrousEncoderType) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    if (PyType_Ready(&CrousDecoderType) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    
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
    
    /* Add encoder/decoder classes */
    Py_INCREF(&CrousEncoderType);
    if (PyModule_AddObject(m, "CrousEncoder", (PyObject *)&CrousEncoderType) < 0) {
        Py_DECREF(&CrousEncoderType);
        Py_DECREF(m);
        return NULL;
    }
    
    Py_INCREF(&CrousDecoderType);
    if (PyModule_AddObject(m, "CrousDecoder", (PyObject *)&CrousDecoderType) < 0) {
        Py_DECREF(&CrousDecoderType);
        Py_DECREF(m);
        return NULL;
    }
    
    /* Initialize custom serializer/decoder registries */
    custom_serializers = PyDict_New();
    custom_decoders = PyDict_New();
    type_to_tag = PyDict_New();
    
    return m;
}
