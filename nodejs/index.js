/**
 * Crous - High-performance binary serialization for Node.js
 * 
 * This module provides complete Crous serialization with full support
 * for Node.js native types and custom serializers.
 * 
 * @module crous
 */

'use strict';

const fs = require('fs');
const path = require('path');

// Load native addon
let native;
try {
    native = require('./build/Release/crous.node');
} catch (e) {
    try {
        native = require('./build/Debug/crous.node');
    } catch (e2) {
        throw new Error('Could not load Crous native addon. Please run: npm install');
    }
}

// Version information
const VERSION = {
    major: 2,
    minor: 0,
    patch: 0,
    get string() {
        return `${this.major}.${this.minor}.${this.patch}`;
    },
    get tuple() {
        return [this.major, this.minor, this.patch];
    },
    get hex() {
        return (this.major << 16) | (this.minor << 8) | this.patch;
    }
};

/**
 * Custom error classes
 */
class CrousError extends Error {
    constructor(message) {
        super(message);
        this.name = 'CrousError';
    }
}

class CrousEncodeError extends CrousError {
    constructor(message) {
        super(message);
        this.name = 'CrousEncodeError';
    }
}

class CrousDecodeError extends CrousError {
    constructor(message) {
        super(message);
        this.name = 'CrousDecodeError';
    }
}

/**
 * Encoder class for custom serialization control
 */
class CrousEncoder {
    constructor(options = {}) {
        this.default = options.default || null;
        this.allowCustom = options.allowCustom !== false;
    }
    
    encode(obj) {
        return native.dumps(obj, this.default);
    }
}

/**
 * Decoder class for custom deserialization control
 */
class CrousDecoder {
    constructor(options = {}) {
        this.objectHook = options.objectHook || null;
    }
    
    decode(data) {
        return native.loads(data, this.objectHook);
    }
}

/**
 * Serialize a JavaScript value to binary format
 * 
 * @param {*} obj - The object to serialize
 * @param {Object} options - Serialization options
 * @param {Function} options.default - Optional function for custom types
 * @param {boolean} options.allowCustom - Whether to allow custom serializers
 * @returns {Buffer} Binary encoded data
 * @throws {CrousEncodeError} If encoding fails
 */
function dumps(obj, options = {}) {
    try {
        return native.dumps(obj, options.default || null);
    } catch (error) {
        throw new CrousEncodeError(error.message);
    }
}

/**
 * Deserialize binary data to a JavaScript value
 * 
 * @param {Buffer} data - Binary data to deserialize
 * @param {Object} options - Deserialization options
 * @param {Function} options.objectHook - Optional function for post-processing objects
 * @returns {*} Deserialized JavaScript value
 * @throws {CrousDecodeError} If decoding fails
 */
function loads(data, options = {}) {
    try {
        if (!Buffer.isBuffer(data)) {
            data = Buffer.from(data);
        }
        return native.loads(data, options.objectHook || null);
    } catch (error) {
        throw new CrousDecodeError(error.message);
    }
}

/**
 * Serialize a JavaScript value and write to a file
 * 
 * @param {*} obj - The object to serialize
 * @param {string|stream.Writable} filepath - Path to output file or writable stream
 * @param {Object} options - Serialization options
 * @throws {CrousEncodeError} If encoding fails
 */
function dump(obj, filepath, options = {}) {
    try {
        const binary = native.dumps(obj, options.default || null);
        
        if (typeof filepath === 'string') {
            // Write to file path
            fs.writeFileSync(filepath, binary);
        } else if (filepath && typeof filepath.write === 'function') {
            // Write to stream
            filepath.write(binary);
        } else {
            throw new Error('filepath must be a string or writable stream');
        }
    } catch (error) {
        if (error instanceof CrousEncodeError) {
            throw error;
        }
        throw new CrousEncodeError(error.message);
    }
}

/**
 * Deserialize a JavaScript value from a file
 * 
 * @param {string|stream.Readable} filepath - Path to input file or readable stream
 * @param {Object} options - Deserialization options
 * @returns {*} Deserialized JavaScript value
 * @throws {CrousDecodeError} If decoding fails
 */
function load(filepath, options = {}) {
    try {
        let binary;
        
        if (typeof filepath === 'string') {
            // Read from file path
            binary = fs.readFileSync(filepath);
        } else if (filepath && typeof filepath.read === 'function') {
            // Read from stream
            const chunks = [];
            let chunk;
            while ((chunk = filepath.read()) !== null) {
                chunks.push(chunk);
            }
            binary = Buffer.concat(chunks);
        } else {
            throw new Error('filepath must be a string or readable stream');
        }
        
        return native.loads(binary, options.objectHook || null);
    } catch (error) {
        if (error instanceof CrousDecodeError) {
            throw error;
        }
        throw new CrousDecodeError(error.message);
    }
}

/**
 * Register a custom serializer for a specific type
 * 
 * @param {Function} type - The constructor/class to register a serializer for
 * @param {Function} serializer - Function to convert instances to serializable values
 * @throws {TypeError} If serializer is not a function
 */
function registerSerializer(type, serializer) {
    if (typeof serializer !== 'function') {
        throw new TypeError('Serializer must be a function');
    }
    native.registerSerializer(type, serializer);
}

/**
 * Unregister a custom serializer for a specific type
 * 
 * @param {Function} type - The constructor/class to unregister
 */
function unregisterSerializer(type) {
    native.unregisterSerializer(type);
}

/**
 * Register a custom decoder for a specific tag
 * 
 * @param {number} tag - The tag identifier (integer)
 * @param {Function} decoder - Function to convert tagged values to objects
 * @throws {TypeError} If decoder is not a function
 */
function registerDecoder(tag, decoder) {
    if (typeof decoder !== 'function') {
        throw new TypeError('Decoder must be a function');
    }
    native.registerDecoder(tag, decoder);
}

/**
 * Unregister a custom decoder for a specific tag
 * 
 * @param {number} tag - The tag identifier to unregister
 */
function unregisterDecoder(tag) {
    native.unregisterDecoder(tag);
}

/**
 * Get version information about the Crous library
 * 
 * @returns {Object} Version information object
 */
function versionInfo() {
    return {
        major: VERSION.major,
        minor: VERSION.minor,
        patch: VERSION.patch,
        string: VERSION.string,
        tuple: VERSION.tuple,
        hex: VERSION.hex
    };
}

// Export all functions and classes
module.exports = {
    // Core functions
    dumps,
    loads,
    dump,
    load,
    
    // Custom serializers/decoders
    registerSerializer,
    unregisterSerializer,
    registerDecoder,
    unregisterDecoder,
    
    // Classes
    CrousEncoder,
    CrousDecoder,
    CrousError,
    CrousEncodeError,
    CrousDecodeError,
    
    // Version info
    versionInfo,
    version: VERSION.string,
    versionTuple: VERSION.tuple,
};

// ES6 named exports for modern Node.js
if (typeof exports !== 'undefined') {
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.default = module.exports;
}
