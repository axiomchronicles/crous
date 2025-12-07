#include <Python.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static PyObject *CrousError = NULL;
static PyObject *CrousEncodeError = NULL;
static PyObject *CrousDecodeError = NULL;

// registry vibes
static PyObject *_serializer_registry = NULL;
static PyObject *_decoder_registry = NULL;

// type codes fr fr
#define CROUS_TYPE_NULL       0x00
#define CROUS_TYPE_BOOL_FALSE 0x01
#define CROUS_TYPE_BOOL_TRUE  0x02
#define CROUS_TYPE_INT        0x03
#define CROUS_TYPE_FLOAT      0x04
#define CROUS_TYPE_STR        0x05
#define CROUS_TYPE_BYTES      0x06
#define CROUS_TYPE_LIST       0x07
#define CROUS_TYPE_DICT       0x08

// magic sauce
#define CROUS_MAGIC           0x4352 
#define CROUS_VERSION         0x02

// buffer era
typedef struct {
    uint8_t *data;
    size_t pos;
    size_t capacity;
} Buffer;

// spawn new buffer no cap
static Buffer* buffer_create(size_t initial_size) {
    Buffer *buf = (Buffer *)malloc(sizeof(Buffer));
    if (!buf) return NULL;
    
    buf->data = (uint8_t *)malloc(initial_size);
    if (!buf->data) {
        free(buf);
        return NULL;
    }
    
    buf->pos = 0;
    buf->capacity = initial_size;
    return buf;
}

// cleanup szn
static void buffer_free(Buffer *buf) {
    if (buf) {
        free(buf->data);
        free(buf);
    }
}

// make room fr
static int buffer_ensure_capacity(Buffer *buf, size_t needed) {
    if (buf->pos + needed <= buf->capacity) {
        return 1;
    }
    // double it
    size_t new_capacity = buf->capacity * 2;
    while (new_capacity < buf->pos + needed) {
        new_capacity *= 2;
    }
    
    uint8_t *new_data = (uint8_t *)realloc(buf->data, new_capacity);
    if (!new_data) {
        return 0;
    }
    
    buf->data = new_data;
    buf->capacity = new_capacity;
    return 1;
}

// yeet a byte
static int buffer_write_u8(Buffer *buf, uint8_t val) {
    if (!buffer_ensure_capacity(buf, 1)) return 0;
    buf->data[buf->pos++] = val;
    return 1;
}

// 32-bit energy
static int buffer_write_u32(Buffer *buf, uint32_t val) {
    if (!buffer_ensure_capacity(buf, 4)) return 0;
    buf->data[buf->pos++] = (val >> 24) & 0xFF;
    buf->data[buf->pos++] = (val >> 16) & 0xFF;
    buf->data[buf->pos++] = (val >> 8) & 0xFF;
    buf->data[buf->pos++] = val & 0xFF;
    return 1;
}

// float go brrr
static int buffer_write_f64(Buffer *buf, double val) {
    if (!buffer_ensure_capacity(buf, 8)) return 0;
    uint8_t *bytes = (uint8_t *)&val;
    for (int i = 0; i < 8; i++) {
        buf->data[buf->pos++] = bytes[i];
    }
    return 1;
}

// dump bytes in there
static int buffer_write_bytes(Buffer *buf, const uint8_t *data, size_t len) {
    if (!buffer_ensure_capacity(buf, len)) return 0;
    memcpy(buf->data + buf->pos, data, len);
    buf->pos += len;
    return 1;
}

// encode that data
static int encode_object(Buffer *buf, PyObject *obj);

// string into bytes no 🧢
static int encode_string(Buffer *buf, PyObject *str) {
    Py_ssize_t len;
    const char *data = PyUnicode_AsUTF8AndSize(str, &len);
    if (!data) return 0;
    
    if (!buffer_write_u8(buf, CROUS_TYPE_STR)) return 0;
    if (!buffer_write_u32(buf, (uint32_t)len)) return 0;
    if (!buffer_write_bytes(buf, (const uint8_t *)data, len)) return 0;
    
    return 1;
}

// bytes are just bytes fr
static int encode_bytes(Buffer *buf, PyObject *bytes_obj) {
    Py_ssize_t len = PyBytes_Size(bytes_obj);
    if (len < 0) return 0;
    
    if (!buffer_write_u8(buf, CROUS_TYPE_BYTES)) return 0;
    if (!buffer_write_u32(buf, (uint32_t)len)) return 0;
    if (!buffer_write_bytes(buf, (uint8_t *)PyBytes_AsString(bytes_obj), len)) return 0;
    
    return 1;
}

// loop through the list
static int encode_list(Buffer *buf, PyObject *list) {
    Py_ssize_t len = PyList_Size(list);
    if (len < 0) return 0;
    
    if (!buffer_write_u8(buf, CROUS_TYPE_LIST)) return 0;
    if (!buffer_write_u32(buf, (uint32_t)len)) return 0;
    
    // each item gets the treatment
    for (Py_ssize_t i = 0; i < len; i++) {
        PyObject *item = PyList_GetItem(list, i);
        if (!item) return 0;
        if (!encode_object(buf, item)) return 0;
    }
    
    return 1;
}

// dict go brrr
static int encode_dict(Buffer *buf, PyObject *dict) {
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    Py_ssize_t len = PyDict_Size(dict);
    
    if (!buffer_write_u8(buf, CROUS_TYPE_DICT)) return 0;
    if (!buffer_write_u32(buf, (uint32_t)len)) return 0;
    
    // keys must be strings periodt
    while (PyDict_Next(dict, &pos, &key, &value)) {
        if (!PyUnicode_Check(key)) {
            PyErr_SetString(CrousEncodeError, "Dict keys must be strings");
            return 0;
        }
        
        if (!encode_string(buf, key)) return 0;
        if (!encode_object(buf, value)) return 0;
    }
    
    return 1;
}

// main encoding logic
static int encode_object(Buffer *buf, PyObject *obj) {
    // the void
    if (obj == Py_None) {
        return buffer_write_u8(buf, CROUS_TYPE_NULL);
    }
    // cap check
    else if (obj == Py_True) {
        return buffer_write_u8(buf, CROUS_TYPE_BOOL_TRUE);
    }
    else if (obj == Py_False) {
        return buffer_write_u8(buf, CROUS_TYPE_BOOL_FALSE);
    }
    // number szn
    else if (PyLong_Check(obj)) {
        long val = PyLong_AsLong(obj);
        if (val == -1 && PyErr_Occurred()) return 0;
        
        if (!buffer_write_u8(buf, CROUS_TYPE_INT)) return 0;
        if (!buffer_write_u32(buf, (uint32_t)val)) return 0;
        return 1;
    }
    // decimal vibes
    else if (PyFloat_Check(obj)) {
        double val = PyFloat_AsDouble(obj);
        if (val == -1.0 && PyErr_Occurred()) return 0;
        
        // decimal vibes
        if (!buffer_write_u8(buf, CROUS_TYPE_FLOAT)) return 0;
        if (!buffer_write_f64(buf, val)) return 0;
        return 1;
    }
    else if (PyUnicode_Check(obj)) {
        return encode_string(buf, obj);
    }
    else if (PyBytes_Check(obj)) {
        return encode_bytes(buf, obj);
    }
    else if (PyList_Check(obj)) {
        return encode_list(buf, obj);
    }
    else if (PyDict_Check(obj)) {
        return encode_dict(buf, obj);
    }
    else {
        // check for custom handlers
        if (_serializer_registry != NULL) {
            PyObject *serializer = PyDict_GetItem(_serializer_registry, (PyObject *)Py_TYPE(obj));
            if (serializer != NULL) {
                // invoke it
                PyObject *result = PyObject_CallFunctionObjArgs(serializer, obj, NULL);
                if (result == NULL) {
                    return 0;
                }
                
                // recursive energy
                int success = encode_object(buf, result);
                Py_DECREF(result);
                return success;
            }
        }
        
        PyErr_Format(CrousEncodeError, 
            "Object of type %.100s is not Crous-serializable",
            Py_TYPE(obj)->tp_name);
        return 0;
    }
}

// decode that data

// reader goes beep boop
typedef struct {
    const uint8_t *data;
    size_t pos;
    size_t size;
} Reader;

// grab a byte
static uint8_t reader_read_u8(Reader *r, int *success) {
    if (r->pos >= r->size) {
        *success = 0;
        return 0;
    }
    return r->data[r->pos++];
}

// read 4 bytes fam
static uint32_t reader_read_u32(Reader *r, int *success) {
    if (r->pos + 4 > r->size) {
        *success = 0;
        return 0;
    }
    uint32_t val = 0;
    // big endian energy
    val |= ((uint32_t)r->data[r->pos++]) << 24;
    val |= ((uint32_t)r->data[r->pos++]) << 16;
    val |= ((uint32_t)r->data[r->pos++]) << 8;
    val |= ((uint32_t)r->data[r->pos++]);
    return val;
}

// float time
static double reader_read_f64(Reader *r, int *success) {
    if (r->pos + 8 > r->size) {
        *success = 0;
        return 0.0;
    }
    double val;
    uint8_t *bytes = (uint8_t *)&val;
    for (int i = 0; i < 8; i++) {
        bytes[i] = r->data[r->pos++];
    }
    return val;
}

// yoink them bytes
static const uint8_t* reader_read_bytes(Reader *r, size_t len, int *success) {
    if (r->pos + len > r->size) {
        *success = 0;
        return NULL;
    }
    const uint8_t *data = r->data + r->pos;
    r->pos += len;
    return data;
}

static PyObject* decode_object(Reader *r, int *success);

// string decode era
static PyObject* decode_string(Reader *r, int *success) {
    uint32_t len = reader_read_u32(r, success);
    if (!*success) {
        PyErr_SetString(CrousDecodeError, "Truncated string length");
        return NULL;
    }
    
    const uint8_t *data = reader_read_bytes(r, len, success);
    if (!*success) {
        PyErr_SetString(CrousDecodeError, "Truncated string data");
        return NULL;
    }
    
    return PyUnicode_FromStringAndSize((const char *)data, len);
}

// bytes mode activated
static PyObject* decode_bytes(Reader *r, int *success) {
    uint32_t len = reader_read_u32(r, success);
    if (!*success) {
        PyErr_SetString(CrousDecodeError, "Truncated bytes length");
        return NULL;
    }
    
    const uint8_t *data = reader_read_bytes(r, len, success);
    if (!*success) {
        PyErr_SetString(CrousDecodeError, "Truncated bytes data");
        return NULL;
    }
    
    return PyBytes_FromStringAndSize((const char *)data, len);
}

// list construction szn
static PyObject* decode_list(Reader *r, int *success) {
    uint32_t len = reader_read_u32(r, success);
    if (!*success) {
        PyErr_SetString(CrousDecodeError, "Truncated list length");
        return NULL;
    }
    
    PyObject *list = PyList_New(len);
    if (!list) return NULL;
    
    // populate that list
    for (uint32_t i = 0; i < len; i++) {
        PyObject *item = decode_object(r, success);
        if (!*success) {
            Py_DECREF(list);
            return NULL;
        }
        PyList_SetItem(list, i, item);
    }
    
    return list;
}

// dict time baby
static PyObject* decode_dict(Reader *r, int *success) {
    uint32_t len = reader_read_u32(r, success);
    if (!*success) {
        PyErr_SetString(CrousDecodeError, "Truncated dict length");
        return NULL;
    }
    
    PyObject *dict = PyDict_New();
    if (!dict) return NULL;
    
    // fill it up
    for (uint32_t i = 0; i < len; i++) {
        uint8_t key_type = reader_read_u8(r, success);
        if (!*success) {
            Py_DECREF(dict);
            PyErr_SetString(CrousDecodeError, "Truncated dict key type");
            return NULL;
        }
        
        if (key_type != CROUS_TYPE_STR) {
            Py_DECREF(dict);
            PyErr_SetString(CrousDecodeError, "Dict keys must be strings");
            return NULL;
        }
        
        PyObject *key = decode_string(r, success);
        if (!*success) {
            Py_DECREF(dict);
            return NULL;
        }
        
        PyObject *value = decode_object(r, success);
        if (!*success) {
            Py_DECREF(key);
            Py_DECREF(dict);
            return NULL;
        }
        
        PyDict_SetItem(dict, key, value);
        Py_DECREF(key);
        Py_DECREF(value);
    }
    
    return dict;
}

// main decode dispatch
static PyObject* decode_object(Reader *r, int *success) {
    uint8_t type = reader_read_u8(r, success);
    if (!*success) {
        PyErr_SetString(CrousDecodeError, "Truncated type byte");
        return NULL;
    }
    
    // type check time
    switch (type) {
        case CROUS_TYPE_NULL:
            Py_RETURN_NONE;
        
        case CROUS_TYPE_BOOL_FALSE:
            Py_RETURN_FALSE;
        
        case CROUS_TYPE_BOOL_TRUE:
            Py_RETURN_TRUE;
        
        case CROUS_TYPE_INT: {
            uint32_t val = reader_read_u32(r, success);
            if (!*success) {
                PyErr_SetString(CrousDecodeError, "Truncated int");
                return NULL;
            }
            return PyLong_FromLong((long)val);
        }
        
        case CROUS_TYPE_FLOAT: {
            double val = reader_read_f64(r, success);
            if (!*success) {
                PyErr_SetString(CrousDecodeError, "Truncated float");
                return NULL;
            }
            return PyFloat_FromDouble(val);
        }
        
        case CROUS_TYPE_STR:
            return decode_string(r, success);
        
        case CROUS_TYPE_BYTES:
            return decode_bytes(r, success);
        
        case CROUS_TYPE_LIST:
            return decode_list(r, success);
        
        case CROUS_TYPE_DICT:
            return decode_dict(r, success);
        
        default: {
            PyErr_Format(CrousDecodeError, "Unknown type byte: 0x%02x", type);
            *success = 0;
            return NULL;
        }
    }
}

// documentation nation

PyDoc_STRVAR(crous_dumps_doc,
"dumps(obj, *, default=None, allow_custom=True) -> bytes\n\n"
"Serialize obj to Crous binary format.\n\n"
"Args:\n"
"    obj: Python object to serialize.\n"
"    default: Callable for non-serializable objects (unused, for compatibility).\n"
"    allow_custom: Whether to allow custom types (default True).\n\n"
"Returns:\n"
"    bytes: Crous-encoded binary data.\n\n"
"Raises:\n"
"    CrousEncodeError: If object cannot be serialized.\n\n"
"Example:\n"
"    >>> crous.dumps({'key': 'value', 'num': 42})\n"
"    b'CR...'\n"
);

PyDoc_STRVAR(crous_loads_doc,
"loads(data, *, object_hook=None, decoder=None) -> object\n\n"
"Deserialize Crous binary data to Python object.\n\n"
"Args:\n"
"    data: Bytes-like object with Crous-encoded data.\n"
"    object_hook: Callable for dict post-processing (unused, for compatibility).\n"
"    decoder: Decoder instance (unused, for compatibility).\n\n"
"Returns:\n"
"    Deserialized Python object.\n\n"
"Raises:\n"
"    CrousDecodeError: If data is malformed or truncated.\n\n"
"Example:\n"
"    >>> crous.loads(b'CR...')\n"
"    {'key': 'value', 'num': 42}\n"
);

PyDoc_STRVAR(crous_dump_doc,
"dump(obj, fp, *, default=None) -> None\n\n"
"Serialize obj to a file-like object.\n\n"
"Args:\n"
"    obj: Python object to serialize.\n"
"    fp: File path (str) or file-like object with write() method.\n"
"    default: Callable for non-serializable objects.\n\n"
"Returns:\n"
"    None\n\n"
"Raises:\n"
"    CrousEncodeError: If serialization fails.\n"
"    IOError: If write fails.\n\n"
"Example:\n"
"    >>> with open('data.crous', 'wb') as f:\n"
"    ...     crous.dump({'key': 'value'}, f)\n"
);

PyDoc_STRVAR(crous_load_doc,
"load(fp, *, object_hook=None) -> object\n\n"
"Deserialize from a file-like object.\n\n"
"Args:\n"
"    fp: File path (str) or file-like object with read() method.\n"
"    object_hook: Callable for dict post-processing (unused, for compatibility).\n\n"
"Returns:\n"
"    Deserialized Python object.\n\n"
"Raises:\n"
"    CrousDecodeError: If data is malformed.\n"
"    IOError: If read fails.\n\n"
"Example:\n"
"    >>> with open('data.crous', 'rb') as f:\n"
"    ...     obj = crous.load(f)\n"
);

// encoder decoder fr fr

static PyObject *
CrousEncoder_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    return PyType_GenericNew(type, args, kwargs);
}

static PyTypeObject CrousEncoderType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "crous.CrousEncoder",
    .tp_doc = PyDoc_STR("Encoder class (stub for API compatibility)."),
    .tp_basicsize = 0,
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = CrousEncoder_new,
};

static PyObject *
CrousDecoder_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    return PyType_GenericNew(type, args, kwargs);
}

static PyTypeObject CrousDecoderType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "crous.CrousDecoder",
    .tp_doc = PyDoc_STR("Decoder class (stub for API compatibility)."),
    .tp_basicsize = 0,
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = CrousDecoder_new,
};

// core functions go hard

static PyObject *pycrous_dumps(PyObject *Py_UNUSED(self), PyObject *args, PyObject *kwargs) {
    PyObject *obj;
    PyObject *default_fn = NULL;
    int allow_custom = 1;
    
    static char *kwlist[] = {"obj", "default", "allow_custom", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|Oi", kwlist, 
                                     &obj, &default_fn, &allow_custom)) {
        return NULL;
    }
    
    Buffer *buf = buffer_create(256);
    if (!buf) {
        PyErr_NoMemory();
        return NULL;
    }
    
    // add the magic number
    if (!buffer_write_u8(buf, (CROUS_MAGIC >> 8) & 0xFF) ||
        !buffer_write_u8(buf, CROUS_MAGIC & 0xFF) ||
        !buffer_write_u8(buf, CROUS_VERSION)) {
        buffer_free(buf);
        PyErr_NoMemory();
        return NULL;
    }
    
    // let's encode
    if (!encode_object(buf, obj)) {
        buffer_free(buf);
        return NULL;
    }
    
    // pack it up
    PyObject *result = PyBytes_FromStringAndSize((const char *)buf->data, buf->pos);
    buffer_free(buf);
    
    return result;
}

static PyObject *pycrous_loads(PyObject *Py_UNUSED(self), PyObject *args, PyObject *kwargs) {
    Py_buffer view;
    PyObject *object_hook = NULL;
    PyObject *decoder = NULL;
    
    static char *kwlist[] = {"data", "object_hook", "decoder", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*|OO", kwlist,
                                     &view, &object_hook, &decoder)) {
        return NULL;
    }
    
    Reader r;
    r.data = (const uint8_t *)view.buf;
    r.size = view.len;
    r.pos = 0;
    
    if (r.size < 3) {
        PyBuffer_Release(&view);
        PyErr_SetString(CrousDecodeError, "Data too short");
        return NULL;
    }
    
    uint16_t magic = (r.data[0] << 8) | r.data[1];
    uint8_t version = r.data[2];
    r.pos = 3;
    
    // magic check
    if (magic != CROUS_MAGIC) {
        PyBuffer_Release(&view);
        PyErr_SetString(CrousDecodeError, "Invalid magic number");
        return NULL;
    }
    
    if (version != CROUS_VERSION) {
        PyBuffer_Release(&view);
        PyErr_Format(CrousDecodeError, "Unsupported version: %d", version);
        return NULL;
    }
    
    // do the thing
    int success = 1;
    PyObject *result = decode_object(&r, &success);
    
    PyBuffer_Release(&view);
    
    if (!success) {
        return NULL;
    }
    
    return result;
}

// file io hits different

static PyObject *pycrous_dump(PyObject *Py_UNUSED(self), PyObject *args, PyObject *kwargs) {
    PyObject *obj, *fp;
    PyObject *default_fn = NULL;
    
    static char *kwlist[] = {"obj", "fp", "default", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|O", kwlist,
                                     &obj, &fp, &default_fn)) {
        return NULL;
    }
    
    // serialize first
    PyObject *dumps_args = Py_BuildValue("(O)", obj);
    if (!dumps_args) return NULL;
    
    PyObject *dumps_result = pycrous_dumps(NULL, dumps_args, kwargs);
    Py_DECREF(dumps_args);
    
    if (!dumps_result) return NULL;
    
    /* Check if fp is a string (file path) */
    if (PyUnicode_Check(fp)) {
        const char *path = PyUnicode_AsUTF8(fp);
        if (!path) {
            Py_DECREF(dumps_result);
            return NULL;
        }
        
        FILE *f = fopen(path, "wb");
        if (!f) {
            Py_DECREF(dumps_result);
            PyErr_Format(PyExc_IOError, "Cannot open file: %s", path);
            return NULL;
        }
        
        size_t data_len = PyBytes_Size(dumps_result);
        uint8_t *data = (uint8_t *)PyBytes_AsString(dumps_result);
        
        size_t written = fwrite(data, 1, data_len, f);
        int close_status = fclose(f);
        
        Py_DECREF(dumps_result);
        
        if (written != data_len || close_status != 0) {
            PyErr_Format(PyExc_IOError, "Failed to write to file: %s", path);
            return NULL;
        }
        
        Py_RETURN_NONE;
    }
    
    // file object mode
    PyObject *write_method = PyObject_GetAttrString(fp, "write");
    if (!write_method) {
        Py_DECREF(dumps_result);
        PyErr_SetString(PyExc_AttributeError, "fp has no write() method");
        return NULL;
    }
    
    PyObject *write_result = PyObject_CallFunctionObjArgs(write_method, 
                                                           dumps_result, NULL);
    Py_DECREF(write_method);
    Py_DECREF(dumps_result);
    
    if (!write_result) return NULL;
    Py_DECREF(write_result);
    
    Py_RETURN_NONE;
}

static PyObject *pycrous_load(PyObject *Py_UNUSED(self), PyObject *args, PyObject *kwargs) {
    PyObject *fp;
    PyObject *object_hook = NULL;
    
    static char *kwlist[] = {"fp", "object_hook", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O", kwlist,
                                     &fp, &object_hook)) {
        return NULL;
    }
    
    PyObject *data = NULL;
    // is it a path or file object?
    if (PyUnicode_Check(fp)) {
        const char *path = PyUnicode_AsUTF8(fp);
        if (!path) return NULL;
        
        FILE *f = fopen(path, "rb");
        if (!f) {
            PyErr_Format(PyExc_IOError, "Cannot open file: %s", path);
            return NULL;
        }
        
        fseek(f, 0, SEEK_END);
        long file_size = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        if (file_size < 0) {
            fclose(f);
            PyErr_Format(PyExc_IOError, "Cannot determine file size: %s", path);
            return NULL;
        }
        
        uint8_t *buf = (uint8_t *)malloc(file_size);
        if (!buf) {
            fclose(f);
            PyErr_NoMemory();
            return NULL;
        }
        
        size_t read_bytes = fread(buf, 1, file_size, f);
        fclose(f);
        
        if (read_bytes != (size_t)file_size) {
            free(buf);
            PyErr_Format(PyExc_IOError, "Failed to read file: %s", path);
            return NULL;
        }
        
        data = PyBytes_FromStringAndSize((const char *)buf, file_size);
        free(buf);
        
        if (!data) return NULL;
    } else {
        // file object era
        PyObject *read_method = PyObject_GetAttrString(fp, "read");
        if (!read_method) {
            PyErr_SetString(PyExc_AttributeError, "fp has no read() method");
            return NULL;
        }
        
        data = PyObject_CallObject(read_method, NULL);
        Py_DECREF(read_method);
        
        if (!data) return NULL;
    }
    
    // time to decode
    PyObject *loads_args = Py_BuildValue("(O)", data);
    Py_DECREF(data);
    if (!loads_args) return NULL;
    
    PyObject *loads_result = pycrous_loads(NULL, loads_args, kwargs);
    Py_DECREF(loads_args);
    
    return loads_result;
}

// streaming > blocking no cap

PyDoc_STRVAR(crous_dumps_stream_doc,
"dumps_stream(obj, fp, *, default=None) -> None\n\n"
"Stream-based serialization (currently identical to dump).\n\n"
"Args:\n"
"    obj: Python object to serialize.\n"
"    fp: File-like object with write() method (opened in 'wb' mode).\n"
"    default: Callable for non-serializable objects.\n\n"
"Returns:\n"
"    None\n\n"
"Raises:\n"
"    CrousEncodeError: If serialization fails.\n"
"    IOError: If write fails.\n\n"
"Example:\n"
"    >>> with open('data.crous', 'wb') as f:\n"
"    ...     crous.dumps_stream({'key': 'value'}, f)\n"
);

static PyObject *pycrous_dumps_stream(PyObject *Py_UNUSED(self), PyObject *args, PyObject *kwargs) {
    PyObject *obj, *fp;
    PyObject *default_fn = NULL;
    
    static char *kwlist[] = {"obj", "fp", "default", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|O", kwlist,
                                     &obj, &fp, &default_fn)) {
        return NULL;
    }
    
    // make the bytes
    PyObject *dumps_args = Py_BuildValue("(O)", obj);
    if (!dumps_args) return NULL;
    
    PyObject *dumps_kwargs = PyDict_New();
    if (!dumps_kwargs) {
        Py_DECREF(dumps_args);
        return NULL;
    }
    
    if (default_fn) {
        PyDict_SetItemString(dumps_kwargs, "default", default_fn);
    }
    
    PyObject *dumps_result = pycrous_dumps(NULL, dumps_args, dumps_kwargs);
    Py_DECREF(dumps_args);
    Py_DECREF(dumps_kwargs);
    
    if (!dumps_result) return NULL;
    
    // pump it out
    PyObject *write_method = PyObject_GetAttrString(fp, "write");
    if (!write_method) {
        Py_DECREF(dumps_result);
        PyErr_SetString(PyExc_AttributeError, "fp has no write() method");
        return NULL;
    }
    
    PyObject *write_result = PyObject_CallFunctionObjArgs(write_method, 
                                                           dumps_result, NULL);
    Py_DECREF(write_method);
    Py_DECREF(dumps_result);
    
    if (!write_result) return NULL;
    Py_DECREF(write_result);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(crous_loads_stream_doc,
"loads_stream(fp, *, object_hook=None) -> object\n\n"
"Stream-based deserialization (currently identical to load).\n\n"
"Args:\n"
"    fp: File-like object with read() method (opened in 'rb' mode).\n"
"    object_hook: Callable for dict post-processing (unused, for compatibility).\n\n"
"Returns:\n"
"    Deserialized Python object.\n\n"
"Raises:\n"
"    CrousDecodeError: If data is malformed or truncated.\n"
"    IOError: If read fails.\n\n"
"Example:\n"
"    >>> with open('data.crous', 'rb') as f:\n"
"    ...     obj = crous.loads_stream(f)\n"
);

static PyObject *pycrous_loads_stream(PyObject *Py_UNUSED(self), PyObject *args, PyObject *kwargs) {
    PyObject *fp;
    PyObject *object_hook = NULL;
    
    static char *kwlist[] = {"fp", "object_hook", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O", kwlist,
                                     &fp, &object_hook)) {
        return NULL;
    }
    
    // slurp it all
    PyObject *read_method = PyObject_GetAttrString(fp, "read");
    if (!read_method) {
        PyErr_SetString(PyExc_AttributeError, "fp has no read() method");
        return NULL;
    }
    
    PyObject *data = PyObject_CallObject(read_method, NULL);
    Py_DECREF(read_method);
    
    if (!data) return NULL;
    
    // decode time
    PyObject *loads_args = PyTuple_Pack(1, data);
    if (!loads_args) {
        Py_DECREF(data);
        return NULL;
    }
    
    PyObject *loads_kwargs = PyDict_New();
    if (!loads_kwargs) {
        Py_DECREF(loads_args);
        Py_DECREF(data);
        return NULL;
    }
    
    if (object_hook) {
        PyDict_SetItemString(loads_kwargs, "object_hook", object_hook);
    }
    
    PyObject *loads_result = pycrous_loads(NULL, loads_args, loads_kwargs);
    Py_DECREF(loads_args);
    Py_DECREF(loads_kwargs);
    Py_DECREF(data);
    
    return loads_result;
}

// registration logic slaps

PyDoc_STRVAR(pycrous_register_serializer_doc,
"register_serializer(typ, func) -> None\n\n"
"Register a custom serializer for a type.\n\n"
"Args:\n"
"    typ: Type to register.\n"
"    func: Callable(obj) -> serializable_value.\n"
);

static PyObject *pycrous_register_serializer(PyObject *Py_UNUSED(self), 
                                            PyObject *args, PyObject *kwargs) {
    PyObject *typ = NULL;
    PyObject *func = NULL;
    
    static char *kwlist[] = {"typ", "func", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO", kwlist, &typ, &func)) {
        return NULL;
    }
    
    /* Ensure func is callable */
    if (!PyCallable_Check(func)) {
        PyErr_SetString(PyExc_TypeError, "func must be callable");
        return NULL;
    }
    
    // create registry if needed
    if (_serializer_registry == NULL) {
        _serializer_registry = PyDict_New();
        if (_serializer_registry == NULL) {
            return NULL;
        }
    }
    
    // add to registry
    if (PyDict_SetItem(_serializer_registry, typ, func) < 0) {
        return NULL;
    }
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(pycrous_unregister_serializer_doc,
"unregister_serializer(typ) -> None\n\n"
"Unregister a custom serializer.\n\n"
"Args:\n"
"    typ: Type to unregister.\n"
);

static PyObject *pycrous_unregister_serializer(PyObject *Py_UNUSED(self), 
                                              PyObject *args, PyObject *kwargs) {
    PyObject *typ = NULL;
    
    static char *kwlist[] = {"typ", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", kwlist, &typ)) {
        return NULL;
    }
    
    // nothing to remove?
    if (_serializer_registry == NULL) {
        Py_RETURN_NONE;
    }
    
    // yeet it
    if (PyDict_DelItem(_serializer_registry, typ) < 0) {
        PyErr_Clear();
    }
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(pycrous_register_decoder_doc,
"register_decoder(tag, func) -> None\n\n"
"Register a custom decoder for a tag.\n\n"
"Args:\n"
"    tag: Tag identifier (int).\n"
"    func: Callable(value) -> deserialized_object.\n"
);

static PyObject *pycrous_register_decoder(PyObject *Py_UNUSED(self), 
                                         PyObject *args, PyObject *kwargs) {
    long tag = 0;
    PyObject *func = NULL;
    PyObject *tag_obj = NULL;
    
    static char *kwlist[] = {"tag", "func", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iO", kwlist, &tag, &func)) {
        return NULL;
    }
    
    /* Ensure func is callable */
    if (!PyCallable_Check(func)) {
        PyErr_SetString(PyExc_TypeError, "func must be callable");
        return NULL;
    }
    
    // spawn the registry
    if (_decoder_registry == NULL) {
        _decoder_registry = PyDict_New();
        if (_decoder_registry == NULL) {
            return NULL;
        }
    }
    
    /* Convert tag to Python int object */
    tag_obj = PyLong_FromLong(tag);
    if (tag_obj == NULL) {
        return NULL;
    }
    
    // register it
    int result = PyDict_SetItem(_decoder_registry, tag_obj, func);
    Py_DECREF(tag_obj);
    
    if (result < 0) {
        return NULL;
    }
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(pycrous_unregister_decoder_doc,
"unregister_decoder(tag) -> None\n\n"
"Unregister a custom decoder.\n\n"
"Args:\n"
"    tag: Tag identifier (int).\n"
);

static PyObject *pycrous_unregister_decoder(PyObject *Py_UNUSED(self), 
                                           PyObject *args, PyObject *kwargs) {
    long tag = 0;
    PyObject *tag_obj = NULL;
    
    static char *kwlist[] = {"tag", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &tag)) {
        return NULL;
    }
    
    // already gone?
    if (_decoder_registry == NULL) {
        Py_RETURN_NONE;
    }
    
    // convert tag
    tag_obj = PyLong_FromLong(tag);
    if (tag_obj == NULL) {
        return NULL;
    }
    // remove it
    if (PyDict_DelItem(_decoder_registry, tag_obj) < 0) {
        PyErr_Clear();
    }
    
    Py_DECREF(tag_obj);
    Py_RETURN_NONE;
}

// all the methods

static PyMethodDef crous_methods[] = {
    {"dumps", (PyCFunction)pycrous_dumps, METH_VARARGS | METH_KEYWORDS,
     crous_dumps_doc},
    {"loads", (PyCFunction)pycrous_loads, METH_VARARGS | METH_KEYWORDS,
     crous_loads_doc},
    {"dump", (PyCFunction)pycrous_dump, METH_VARARGS | METH_KEYWORDS,
     crous_dump_doc},
    {"load", (PyCFunction)pycrous_load, METH_VARARGS | METH_KEYWORDS,
     crous_load_doc},
    {"dumps_stream", (PyCFunction)pycrous_dumps_stream, METH_VARARGS | METH_KEYWORDS,
     crous_dumps_stream_doc},
    {"loads_stream", (PyCFunction)pycrous_loads_stream, METH_VARARGS | METH_KEYWORDS,
     crous_loads_stream_doc},
    {"register_serializer", (PyCFunction)pycrous_register_serializer, 
     METH_VARARGS | METH_KEYWORDS, pycrous_register_serializer_doc},
    {"unregister_serializer", (PyCFunction)pycrous_unregister_serializer, 
     METH_VARARGS | METH_KEYWORDS, pycrous_unregister_serializer_doc},
    {"register_decoder", (PyCFunction)pycrous_register_decoder, 
     METH_VARARGS | METH_KEYWORDS, pycrous_register_decoder_doc},
    {"unregister_decoder", (PyCFunction)pycrous_unregister_decoder, 
     METH_VARARGS | METH_KEYWORDS, pycrous_unregister_decoder_doc},
    {NULL, NULL, 0, NULL}
};

// what is this tho

PyDoc_STRVAR(crous_module_doc,
"crous: High-performance binary serialization format for Python\n\n"
"Core functions:\n"
"    - dumps(obj) -> bytes: Serialize to binary.\n"
"    - loads(data) -> obj: Deserialize from binary.\n"
"    - dump(obj, fp): Serialize to file object or path.\n"
"    - load(fp) -> obj: Deserialize from file object or path.\n\n"
"Supported types:\n"
"    None, bool, int, float, str, bytes, list, dict\n\n"
"Classes:\n"
"    - CrousEncoder: Encoder (stub for API compatibility).\n"
"    - CrousDecoder: Decoder (stub for API compatibility).\n\n"
"Exceptions:\n"
"    - CrousError: Base exception.\n"
"    - CrousEncodeError: Encoding errors.\n"
"    - CrousDecodeError: Decoding errors.\n"
);

/*============================================================================
  MODULE DEFINITION
  ============================================================================*/

/* Module cleanup function */
static int crous_module_exec(PyObject *m) {
    return 0;
}

static void crous_module_free(void *module_state) {
    /* Clean up serializer registry */
    if (_serializer_registry != NULL) {
        Py_DECREF(_serializer_registry);
        _serializer_registry = NULL;
    }
    
    // decoder registry gone
    if (_decoder_registry != NULL) {
        Py_DECREF(_decoder_registry);
        _decoder_registry = NULL;
    }
}

static struct PyModuleDef crous_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "crous",
    .m_doc = crous_module_doc,
    .m_size = -1,
    .m_methods = crous_methods,
    .m_free = crous_module_free,
};

// let's go

PyMODINIT_FUNC PyInit_crous(void) {
    PyObject *m = PyModule_Create(&crous_module);
    if (m == NULL) return NULL;
    
    // exceptions fr
    CrousError = PyErr_NewException("crous.CrousError", NULL, NULL);
    if (CrousError == NULL) goto fail;
    
    CrousEncodeError = PyErr_NewException("crous.CrousEncodeError",
                                          CrousError, NULL);
    if (CrousEncodeError == NULL) goto fail;
    
    CrousDecodeError = PyErr_NewException("crous.CrousDecodeError",
                                          CrousError, NULL);
    if (CrousDecodeError == NULL) goto fail;
    
    // add em to module
    if (PyModule_AddObject(m, "CrousError", CrousError) < 0) goto fail;
    if (PyModule_AddObject(m, "CrousEncodeError", CrousEncodeError) < 0) goto fail;
    if (PyModule_AddObject(m, "CrousDecodeError", CrousDecodeError) < 0) goto fail;
    
    // types ready
    if (PyType_Ready(&CrousEncoderType) < 0) goto fail;
    if (PyType_Ready(&CrousDecoderType) < 0) goto fail;
    
    if (PyModule_AddObject(m, "CrousEncoder", (PyObject *)&CrousEncoderType) < 0) 
        goto fail;
    if (PyModule_AddObject(m, "CrousDecoder", (PyObject *)&CrousDecoderType) < 0) 
        goto fail;
    
    // version flex
    if (PyModule_AddStringConstant(m, "__version__", "2.0.0") < 0) goto fail;
    if (PyModule_AddStringConstant(m, "__author__", "Crous Contributors") < 0) 
        goto fail;
    
    return m;
    
fail:
    Py_XDECREF(m);
    return NULL;
}