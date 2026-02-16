"""
test_thread_safety.py - Thread safety tests for Crous

Validates that the registry_lock protects custom serializer/decoder
registries under concurrent access.
"""

import pytest
import crous
import threading
import time
from datetime import datetime
from decimal import Decimal


class TestConcurrentDumpsLoads:
    """Verify dumps/loads can be called from many threads simultaneously."""

    def test_concurrent_dumps_loads_basic(self):
        """Stress-test dumps/loads from 8 threads, 200 iterations each."""
        errors = []

        def worker(tid):
            try:
                for i in range(200):
                    data = {"thread": tid, "iter": i, "values": [1, 2.5, "hello", None, True]}
                    binary = crous.dumps(data)
                    result = crous.loads(binary)
                    assert result["thread"] == tid
                    assert result["iter"] == i
            except Exception as e:
                errors.append((tid, e))

        threads = [threading.Thread(target=worker, args=(t,)) for t in range(8)]
        for t in threads:
            t.start()
        for t in threads:
            t.join()

        assert errors == [], f"Thread errors: {errors}"

    def test_concurrent_dumps_loads_nested(self):
        """Stress-test nested structures from multiple threads."""
        errors = []

        def worker(tid):
            try:
                for i in range(100):
                    data = {
                        "level1": {
                            "level2": [{"key": f"t{tid}_i{i}", "val": i * 1.1}]
                        }
                    }
                    binary = crous.dumps(data)
                    result = crous.loads(binary)
                    assert result["level1"]["level2"][0]["key"] == f"t{tid}_i{i}"
            except Exception as e:
                errors.append((tid, e))

        threads = [threading.Thread(target=worker, args=(t,)) for t in range(8)]
        for t in threads:
            t.start()
        for t in threads:
            t.join()

        assert errors == [], f"Thread errors: {errors}"


class TestConcurrentRegistration:
    """Verify register/unregister don't crash under concurrent access."""

    def test_register_unregister_serializer_concurrent(self):
        """Multiple threads registering and unregistering serializers."""
        errors = []

        def register_worker(tid):
            try:
                class MyType:
                    pass

                def ser(obj):
                    return f"type_{tid}"

                for _ in range(50):
                    crous.register_serializer(MyType, ser)
                    crous.unregister_serializer(MyType)
            except Exception as e:
                errors.append((tid, "register", e))

        threads = [threading.Thread(target=register_worker, args=(t,)) for t in range(8)]
        for t in threads:
            t.start()
        for t in threads:
            t.join()

        assert errors == [], f"Thread errors: {errors}"

    def test_register_unregister_decoder_concurrent(self):
        """Multiple threads registering and unregistering decoders."""
        errors = []

        def register_worker(tid):
            try:
                tag = 100 + tid
                def dec(data):
                    return data

                for _ in range(50):
                    crous.register_decoder(tag, dec)
                    crous.unregister_decoder(tag)
            except Exception as e:
                errors.append((tid, "decoder", e))

        threads = [threading.Thread(target=register_worker, args=(t,)) for t in range(8)]
        for t in threads:
            t.start()
        for t in threads:
            t.join()

        assert errors == [], f"Thread errors: {errors}"


class TestConcurrentSerializerUsage:
    """Test using custom serializers from multiple threads."""

    def test_dumps_with_custom_serializer_concurrent(self):
        """Serialize custom types from many threads while serializer is registered."""
        errors = []

        def datetime_ser(obj):
            if isinstance(obj, datetime):
                return obj.isoformat()
            raise TypeError()

        crous.register_serializer(datetime, datetime_ser)

        def worker(tid):
            try:
                for i in range(100):
                    dt = datetime(2023, 1, 1, tid, i % 60, 0)
                    binary = crous.dumps(dt)
                    result = crous.loads(binary)
                    assert result == dt.isoformat()
            except Exception as e:
                errors.append((tid, e))

        try:
            threads = [threading.Thread(target=worker, args=(t,)) for t in range(8)]
            for t in threads:
                t.start()
            for t in threads:
                t.join()
        finally:
            crous.unregister_serializer(datetime)

        assert errors == [], f"Thread errors: {errors}"

    def test_register_while_dumping(self):
        """One thread registers/unregisters while others dump built-in types."""
        errors = []
        stop = threading.Event()

        def dumper(tid):
            try:
                while not stop.is_set():
                    data = {"key": tid, "list": [1, 2, 3]}
                    binary = crous.dumps(data)
                    result = crous.loads(binary)
                    assert result["key"] == tid
            except Exception as e:
                errors.append((tid, e))

        def registrar():
            try:
                for _ in range(100):
                    def ser(obj):
                        return str(obj)
                    crous.register_serializer(Decimal, ser)
                    time.sleep(0.001)
                    crous.unregister_serializer(Decimal)
            except Exception as e:
                errors.append(("registrar", e))

        dump_threads = [threading.Thread(target=dumper, args=(t,)) for t in range(4)]
        reg_thread = threading.Thread(target=registrar)

        for t in dump_threads:
            t.start()
        reg_thread.start()

        reg_thread.join()
        stop.set()

        for t in dump_threads:
            t.join()

        assert errors == [], f"Thread errors: {errors}"
