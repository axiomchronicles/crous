#!/usr/bin/env python3
"""
Crous Benchmark Suite — Rigorous comparison against JSON, MessagePack, Protocol Buffers

Runs serialization/deserialization benchmarks on multiple real-world-style datasets:
1. Small API payload (user profile)
2. Medium nested config
3. Large dataset (1000 records)
4. Binary-heavy payload
5. Deep nesting

Reports: ops/sec, file size, memory usage
"""
import json
import time
import sys
import platform
import os
import struct
import statistics

import crous
import msgpack

# For protobuf, we use protobuf's Struct for schema-less comparison
from google.protobuf import struct_pb2, json_format

# ─── Datasets ───────────────────────────────────────────────────────

DATASETS = {}

# 1. Small API payload
DATASETS["small_api"] = {
    "id": 12345,
    "name": "Alice Johnson",
    "email": "alice@example.com",
    "age": 29,
    "verified": True,
    "scores": [98.5, 95.0, 100.0, 87.5],
    "address": {
        "street": "123 Main St",
        "city": "San Francisco",
        "state": "CA",
        "zip": "94102"
    },
    "tags": ["developer", "python", "systems"]
}

# 2. Medium nested config
DATASETS["config"] = {
    "server": {
        "host": "0.0.0.0",
        "port": 8080,
        "workers": 4,
        "timeout": 30,
        "ssl": {
            "enabled": True,
            "cert_path": "/etc/ssl/certs/server.pem",
            "key_path": "/etc/ssl/private/server.key",
            "protocols": ["TLSv1.2", "TLSv1.3"]
        }
    },
    "database": {
        "primary": {
            "host": "db-primary.internal",
            "port": 5432,
            "name": "myapp_production",
            "pool_size": 20,
            "pool_timeout": 5
        },
        "replica": {
            "host": "db-replica.internal",
            "port": 5432,
            "name": "myapp_production",
            "pool_size": 10,
            "read_only": True
        }
    },
    "cache": {
        "backend": "redis",
        "host": "cache.internal",
        "port": 6379,
        "ttl": 3600,
        "prefix": "myapp:"
    },
    "logging": {
        "level": "INFO",
        "format": "%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        "handlers": ["console", "file", "syslog"],
        "file_path": "/var/log/myapp/app.log",
        "max_bytes": 10485760,
        "backup_count": 5
    },
    "features": {
        "rate_limiting": True,
        "auth_enabled": True,
        "cors_origins": ["https://app.example.com", "https://admin.example.com"],
        "max_upload_mb": 50
    }
}

# 3. Large dataset (1000 records)
DATASETS["large_records"] = {
    "metadata": {
        "total": 1000,
        "page": 1,
        "per_page": 1000,
        "generated_at": "2026-02-16T00:00:00Z"
    },
    "records": [
        {
            "id": i,
            "uuid": f"550e8400-e29b-41d4-a716-{i:012d}",
            "name": f"User {i}",
            "email": f"user{i}@example.com",
            "age": 20 + (i % 50),
            "score": 50.0 + (i % 50) + (i % 7) * 0.1,
            "active": i % 3 != 0,
            "department": ["engineering", "marketing", "sales", "support", "hr"][i % 5],
            "permissions": ["read", "write"] if i % 2 == 0 else ["read"],
            "metadata": {
                "created": f"2025-{(i % 12) + 1:02d}-{(i % 28) + 1:02d}",
                "logins": i * 3 + 7,
                "last_ip": f"192.168.{i % 256}.{(i * 7) % 256}"
            }
        }
        for i in range(1000)
    ]
}

# 4. Numeric-heavy payload
DATASETS["numeric"] = {
    "matrix": [[float(i * 100 + j) for j in range(100)] for i in range(100)],
    "integers": list(range(10000)),
    "floats": [i * 0.001 for i in range(10000)],
    "stats": {
        "mean": 4999.5,
        "std": 2886.895680,
        "min": 0,
        "max": 9999,
        "quartiles": [2499.75, 4999.5, 7499.25]
    }
}

# 5. String-heavy payload 
DATASETS["text_heavy"] = {
    "articles": [
        {
            "id": i,
            "title": f"Article {i}: Understanding Binary Serialization in Modern Systems",
            "body": f"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Record {i} of the dataset.",
            "author": f"Author {i % 20}",
            "tags": [f"tag{j}" for j in range(i % 5 + 1)],
            "published": i % 4 != 0,
            "views": i * 127 + 42
        }
        for i in range(200)
    ]
}


# ─── Protobuf helpers (schema-less via Struct) ──────────────────────

def to_protobuf_struct(data):
    """Convert Python dict to protobuf Struct (schema-less comparison)."""
    s = struct_pb2.Struct()
    json_format.ParseDict(data, s)
    return s

def from_protobuf_struct(serialized_bytes):
    """Deserialize protobuf Struct bytes back."""
    s = struct_pb2.Struct()
    s.ParseFromString(serialized_bytes)
    return json_format.MessageToDict(s)


# ─── Benchmark Runner ───────────────────────────────────────────────

def benchmark_serialize(func, data, iterations=500):
    """Benchmark serialization, return (ops_per_sec, encoded_size, raw_times)."""
    # Warmup
    for _ in range(min(50, iterations)):
        result = func(data)
    
    times = []
    encoded = None
    for _ in range(iterations):
        start = time.perf_counter_ns()
        encoded = func(data)
        end = time.perf_counter_ns()
        times.append((end - start) / 1e9)
    
    total = sum(times)
    ops_per_sec = iterations / total
    size = len(encoded) if isinstance(encoded, (bytes, bytearray)) else len(encoded.encode('utf-8') if isinstance(encoded, str) else encoded)
    
    return ops_per_sec, size, times


def benchmark_deserialize(serialize_func, deserialize_func, data, iterations=500):
    """Benchmark deserialization, return (ops_per_sec, raw_times)."""
    encoded = serialize_func(data)
    
    # Warmup
    for _ in range(min(50, iterations)):
        deserialize_func(encoded)
    
    times = []
    for _ in range(iterations):
        start = time.perf_counter_ns()
        deserialize_func(encoded)
        end = time.perf_counter_ns()
        times.append((end - start) / 1e9)
    
    total = sum(times)
    ops_per_sec = iterations / total
    
    return ops_per_sec, times


def format_ops(ops):
    if ops >= 1_000_000:
        return f"{ops/1_000_000:.1f}M"
    elif ops >= 1_000:
        return f"{ops/1_000:.1f}K"
    else:
        return f"{ops:.0f}"


def format_size(size_bytes):
    if size_bytes >= 1024 * 1024:
        return f"{size_bytes / (1024*1024):.2f} MB"
    elif size_bytes >= 1024:
        return f"{size_bytes / 1024:.2f} KB"
    else:
        return f"{size_bytes} B"


# ─── Main ───────────────────────────────────────────────────────────

def main():
    print("=" * 80)
    print("  CROUS BENCHMARK SUITE — Rigorous Format Comparison")
    print("=" * 80)
    print()
    print(f"  Python:    {sys.version.split()[0]}")
    print(f"  Platform:  {platform.platform()}")
    print(f"  CPU:       {platform.processor()} ({platform.machine()})")
    print(f"  Crous:     {crous.version}")
    print(f"  msgpack:   {'.'.join(str(x) for x in msgpack.version)}")
    from google.protobuf import __version__ as pb_ver
    print(f"  protobuf:  {pb_ver}")
    print(f"  Iterations: 500 per benchmark")
    print()

    formats = {
        "JSON": {
            "serialize": lambda d: json.dumps(d).encode('utf-8'),
            "deserialize": lambda b: json.loads(b),
        },
        "MessagePack": {
            "serialize": lambda d: msgpack.packb(d, use_bin_type=True),
            "deserialize": lambda b: msgpack.unpackb(b, raw=False),
        },
        "Protobuf": {
            "serialize": lambda d: to_protobuf_struct(d).SerializeToString(),
            "deserialize": lambda b: from_protobuf_struct(b),
        },
        "Crous": {
            "serialize": lambda d: crous.dumps(d),
            "deserialize": lambda b: crous.loads(b),
        },
    }

    all_results = {}

    for dataset_name, dataset in DATASETS.items():
        print(f"━━━ Dataset: {dataset_name} ━━━")
        print()
        
        results = {}
        
        for fmt_name, funcs in formats.items():
            try:
                ser_ops, size, ser_times = benchmark_serialize(
                    funcs["serialize"], dataset, iterations=500
                )
                deser_ops, deser_times = benchmark_deserialize(
                    funcs["serialize"], funcs["deserialize"], dataset, iterations=500
                )
                
                results[fmt_name] = {
                    "ser_ops": ser_ops,
                    "deser_ops": deser_ops,
                    "size": size,
                    "ser_median": statistics.median(ser_times) * 1000,
                    "deser_median": statistics.median(deser_times) * 1000,
                    "ser_p99": sorted(ser_times)[int(len(ser_times)*0.99)] * 1000,
                    "deser_p99": sorted(deser_times)[int(len(deser_times)*0.99)] * 1000,
                }
            except Exception as e:
                results[fmt_name] = {"error": str(e)}
        
        # Print table
        print(f"  {'Format':<14} {'Serialize':>12} {'Deserialize':>12} {'Size':>10} {'Ser p50':>10} {'Deser p50':>10}")
        print(f"  {'':─<14} {'(ops/sec)':─>12} {'(ops/sec)':─>12} {'':─>10} {'(ms)':─>10} {'(ms)':─>10}")
        
        for fmt_name in ["JSON", "MessagePack", "Protobuf", "Crous"]:
            r = results.get(fmt_name, {})
            if "error" in r:
                print(f"  {fmt_name:<14} ERROR: {r['error'][:40]}")
            else:
                print(f"  {fmt_name:<14} {format_ops(r['ser_ops']):>12} {format_ops(r['deser_ops']):>12} {format_size(r['size']):>10} {r['ser_median']:>9.3f} {r['deser_median']:>9.3f}")
        
        # Size comparison
        json_size = results.get("JSON", {}).get("size", 0)
        if json_size > 0:
            print()
            print(f"  Size vs JSON:")
            for fmt_name in ["MessagePack", "Protobuf", "Crous"]:
                r = results.get(fmt_name, {})
                if "size" in r:
                    pct = ((r["size"] - json_size) / json_size) * 100
                    print(f"    {fmt_name:<14} {pct:>+6.1f}%  ({format_size(r['size'])})")
        
        print()
        all_results[dataset_name] = results

    # ─── Summary ─────────────────────────────────────────────
    print("=" * 80)
    print("  AGGREGATE SUMMARY")
    print("=" * 80)
    print()
    
    # Average speedup vs JSON
    for fmt_name in ["MessagePack", "Protobuf", "Crous"]:
        ser_speedups = []
        deser_speedups = []
        size_savings = []
        for ds_name, ds_results in all_results.items():
            json_r = ds_results.get("JSON", {})
            fmt_r = ds_results.get(fmt_name, {})
            if "error" not in json_r and "error" not in fmt_r:
                if json_r.get("ser_ops", 0) > 0:
                    ser_speedups.append(fmt_r["ser_ops"] / json_r["ser_ops"])
                if json_r.get("deser_ops", 0) > 0:
                    deser_speedups.append(fmt_r["deser_ops"] / json_r["deser_ops"])
                if json_r.get("size", 0) > 0:
                    size_savings.append(1 - fmt_r["size"] / json_r["size"])
        
        if ser_speedups:
            avg_ser = statistics.mean(ser_speedups)
            avg_deser = statistics.mean(deser_speedups)
            avg_size = statistics.mean(size_savings) * 100
            print(f"  {fmt_name:<14} vs JSON:")
            print(f"    Serialize:   {avg_ser:.2f}x {'faster' if avg_ser > 1 else 'slower'}")
            print(f"    Deserialize: {avg_deser:.2f}x {'faster' if avg_deser > 1 else 'slower'}")
            print(f"    Size:        {avg_size:+.1f}% ({'smaller' if avg_size > 0 else 'larger'})")
            print()

    # Output JSON for the website
    print()
    print("─── JSON DATA FOR WEBSITE ───")
    import json as _json
    website_data = {}
    for ds_name, ds_results in all_results.items():
        website_data[ds_name] = {}
        for fmt_name, r in ds_results.items():
            if "error" not in r:
                website_data[ds_name][fmt_name] = {
                    "serialize_ops": round(r["ser_ops"]),
                    "deserialize_ops": round(r["deser_ops"]),
                    "size_bytes": r["size"],
                    "ser_median_ms": round(r["ser_median"], 4),
                    "deser_median_ms": round(r["deser_median"], 4),
                }
    print(_json.dumps(website_data, indent=2))


if __name__ == "__main__":
    main()
