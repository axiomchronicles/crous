/**
 * Crous - High-performance binary serialization for Node.js
 * 
 * TypeScript type definitions for the Crous serialization library.
 * 
 * @packageDocumentation
 */

/**
 * Custom error class for Crous-related errors
 */
export class CrousError extends Error {
    constructor(message: string);
}

/**
 * Error thrown during encoding/serialization
 */
export class CrousEncodeError extends CrousError {
    constructor(message: string);
}

/**
 * Error thrown during decoding/deserialization
 */
export class CrousDecodeError extends CrousError {
    constructor(message: string);
}

/**
 * Function type for custom serializers
 * @template T - The type being serialized
 */
export type SerializerFunction<T = any> = (obj: T) => any;

/**
 * Function type for custom decoders
 * @template T - The type being deserialized
 */
export type DecoderFunction<T = any> = (value: any) => T;

/**
 * Function type for object hooks (post-processing dictionaries)
 */
export type ObjectHook = (obj: Record<string, any>) => any;

/**
 * Function type for default serializer fallback
 */
export type DefaultFunction = (obj: any) => any;

/**
 * Options for dumps() function
 */
export interface DumpsOptions {
    /**
     * Optional function to handle custom types that aren't natively supported
     */
    default?: DefaultFunction;
    
    /**
     * Whether to allow custom serializers (default: true)
     */
    allowCustom?: boolean;
}

/**
 * Options for loads() function
 */
export interface LoadsOptions {
    /**
     * Optional function to post-process dictionary objects during deserialization
     */
    objectHook?: ObjectHook;
}

/**
 * Options for dump() function
 */
export interface DumpOptions extends DumpsOptions {
}

/**
 * Options for load() function
 */
export interface LoadOptions extends LoadsOptions {
}

/**
 * Encoder class for custom serialization control
 */
export class CrousEncoder {
    /**
     * Create a new encoder instance
     * @param options - Encoder options
     */
    constructor(options?: {
        default?: DefaultFunction;
        allowCustom?: boolean;
    });
    
    /**
     * Encode an object to binary format
     * @param obj - The object to encode
     * @returns Binary encoded data
     */
    encode(obj: any): Buffer;
}

/**
 * Decoder class for custom deserialization control
 */
export class CrousDecoder {
    /**
     * Create a new decoder instance
     * @param options - Decoder options
     */
    constructor(options?: {
        objectHook?: ObjectHook;
    });
    
    /**
     * Decode binary data to an object
     * @param data - The binary data to decode
     * @returns Deserialized object
     */
    decode(data: Buffer): any;
}

/**
 * Version information structure
 */
export interface VersionInfo {
    major: number;
    minor: number;
    patch: number;
    string: string;
    tuple: [number, number, number];
    hex: number;
}

/**
 * Serialize a JavaScript value to binary format
 * 
 * @param obj - The object to serialize
 * @param options - Serialization options
 * @returns Binary encoded data as a Buffer
 * @throws {CrousEncodeError} If encoding fails
 * 
 * @example
 * ```typescript
 * import { dumps } from 'crous';
 * 
 * const data = { name: 'Alice', age: 30 };
 * const binary = dumps(data);
 * console.log(binary); // <Buffer ...>
 * ```
 */
export function dumps(obj: any, options?: DumpsOptions): Buffer;

/**
 * Deserialize binary data to a JavaScript value
 * 
 * @param data - Binary data to deserialize
 * @param options - Deserialization options
 * @returns Deserialized JavaScript value
 * @throws {CrousDecodeError} If decoding fails
 * 
 * @example
 * ```typescript
 * import { loads, dumps } from 'crous';
 * 
 * const binary = dumps({ name: 'Alice' });
 * const data = loads(binary);
 * console.log(data); // { name: 'Alice' }
 * ```
 */
export function loads(data: Buffer, options?: LoadsOptions): any;

/**
 * Serialize a JavaScript value and write to a file
 * 
 * @param obj - The object to serialize
 * @param filepath - Path to the output file or writable stream
 * @param options - Serialization options
 * @throws {CrousEncodeError} If encoding fails
 * 
 * @example
 * ```typescript
 * import { dump } from 'crous';
 * import * as fs from 'fs';
 * 
 * const data = { name: 'Alice', age: 30 };
 * 
 * // Write to file path
 * dump(data, 'output.crous');
 * 
 * // Write to stream
 * const stream = fs.createWriteStream('output.crous');
 * dump(data, stream);
 * ```
 */
export function dump(obj: any, filepath: string | NodeJS.WritableStream, options?: DumpOptions): void;

/**
 * Deserialize a JavaScript value from a file
 * 
 * @param filepath - Path to the input file or readable stream
 * @param options - Deserialization options
 * @returns Deserialized JavaScript value
 * @throws {CrousDecodeError} If decoding fails
 * 
 * @example
 * ```typescript
 * import { load } from 'crous';
 * import * as fs from 'fs';
 * 
 * // Read from file path
 * const data = load('input.crous');
 * 
 * // Read from stream
 * const stream = fs.createReadStream('input.crous');
 * const data2 = load(stream);
 * ```
 */
export function load(filepath: string | NodeJS.ReadableStream, options?: LoadOptions): any;

/**
 * Register a custom serializer for a specific type
 * 
 * @param type - The constructor/class to register a serializer for
 * @param serializer - Function to convert instances to serializable values
 * @throws {TypeError} If serializer is not a function
 * 
 * @example
 * ```typescript
 * import { registerSerializer, dumps } from 'crous';
 * 
 * class Point {
 *     constructor(public x: number, public y: number) {}
 * }
 * 
 * registerSerializer(Point, (point) => {
 *     return { x: point.x, y: point.y };
 * });
 * 
 * const binary = dumps(new Point(10, 20));
 * ```
 */
export function registerSerializer<T = any>(
    type: new (...args: any[]) => T,
    serializer: SerializerFunction<T>
): void;

/**
 * Unregister a custom serializer for a specific type
 * 
 * @param type - The constructor/class to unregister
 * 
 * @example
 * ```typescript
 * import { unregisterSerializer } from 'crous';
 * 
 * class Point {}
 * unregisterSerializer(Point);
 * ```
 */
export function unregisterSerializer<T = any>(
    type: new (...args: any[]) => T
): void;

/**
 * Register a custom decoder for a specific tag
 * 
 * @param tag - The tag identifier (integer)
 * @param decoder - Function to convert tagged values to objects
 * @throws {TypeError} If decoder is not a function
 * 
 * @example
 * ```typescript
 * import { registerDecoder, loads } from 'crous';
 * 
 * class Point {
 *     constructor(public x: number, public y: number) {}
 * }
 * 
 * registerDecoder(100, (value) => {
 *     return new Point(value.x, value.y);
 * });
 * 
 * const data = loads(binary);
 * ```
 */
export function registerDecoder<T = any>(
    tag: number,
    decoder: DecoderFunction<T>
): void;

/**
 * Unregister a custom decoder for a specific tag
 * 
 * @param tag - The tag identifier to unregister
 * 
 * @example
 * ```typescript
 * import { unregisterDecoder } from 'crous';
 * 
 * unregisterDecoder(100);
 * ```
 */
export function unregisterDecoder(tag: number): void;

/**
 * Get version information about the Crous library
 * 
 * @returns Version information object
 * 
 * @example
 * ```typescript
 * import { versionInfo } from 'crous';
 * 
 * const info = versionInfo();
 * console.log(`Crous v${info.string}`);
 * ```
 */
export function versionInfo(): VersionInfo;

/**
 * Module version string
 */
export const version: string;

/**
 * Module version tuple [major, minor, patch]
 */
export const versionTuple: [number, number, number];

// Re-export as default for CommonJS compatibility
declare const crous: {
    dumps: typeof dumps;
    loads: typeof loads;
    dump: typeof dump;
    load: typeof load;
    registerSerializer: typeof registerSerializer;
    unregisterSerializer: typeof unregisterSerializer;
    registerDecoder: typeof registerDecoder;
    unregisterDecoder: typeof unregisterDecoder;
    versionInfo: typeof versionInfo;
    CrousEncoder: typeof CrousEncoder;
    CrousDecoder: typeof CrousDecoder;
    CrousError: typeof CrousError;
    CrousEncodeError: typeof CrousEncodeError;
    CrousDecodeError: typeof CrousDecodeError;
    version: string;
    versionTuple: [number, number, number];
};

export default crous;
