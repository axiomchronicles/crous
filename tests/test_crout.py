"""
test_crout.py - Comprehensive tests for the CROUT text format.

CROUT (Compact Readable Object Utility Text) is a human-readable
serialization format for Crous values.  Format:

    CROUT1
    [@ <tok>=<key>]
    <root-value>

Value syntax:
    N            null
    T / F        bool
    i<dec>       integer
    f<dec>       float
    s<len>:<str> string  (length-prefixed, binary-safe)
    b<len>:<hex> bytes   (length-prefixed, hex-encoded)
    [v , v]      list
    (v , v)      tuple
    {k:v , k:v}  dict
    #<tag>:<val> tagged value
"""

import math
import pytest
import crous

# Alias for the decode error
CrousDecodeError = crous.CrousDecodeError


# =============================================================================
# Scalar round-trips
# =============================================================================

class TestCroutScalars:
    """Scalar types: null, bool, int, float, string, bytes."""

    def test_null_roundtrip(self):
        text = crous.dumps_text(None)
        assert "N" in text
        assert crous.loads_text(text) is None

    def test_bool_true(self):
        text = crous.dumps_text(True)
        assert "T" in text
        assert crous.loads_text(text) is True

    def test_bool_false(self):
        text = crous.dumps_text(False)
        assert "F" in text
        assert crous.loads_text(text) is False

    def test_int_zero(self):
        text = crous.dumps_text(0)
        assert crous.loads_text(text) == 0

    def test_int_positive(self):
        for v in [1, 42, 255, 65535, 2**31 - 1]:
            assert crous.loads_text(crous.dumps_text(v)) == v

    def test_int_negative(self):
        for v in [-1, -42, -255, -(2**31)]:
            assert crous.loads_text(crous.dumps_text(v)) == v

    def test_int_large(self):
        """Large 64-bit ints."""
        for v in [2**62, -(2**62)]:
            assert crous.loads_text(crous.dumps_text(v)) == v

    def test_float_basic(self):
        for v in [0.0, 1.5, -3.14, 1e10, -1e-10]:
            result = crous.loads_text(crous.dumps_text(v))
            assert abs(result - v) < 1e-9, f"{result} != {v}"

    def test_float_nan(self):
        text = crous.dumps_text(float("nan"))
        assert "fnan" in text
        result = crous.loads_text(text)
        assert math.isnan(result)

    def test_float_inf(self):
        text = crous.dumps_text(float("inf"))
        assert "finf" in text
        result = crous.loads_text(text)
        assert result == float("inf")

    def test_float_neg_inf(self):
        text = crous.dumps_text(float("-inf"))
        assert "f-inf" in text
        result = crous.loads_text(text)
        assert result == float("-inf")

    def test_string_basic(self):
        text = crous.dumps_text("hello")
        assert "s5:hello" in text
        assert crous.loads_text(text) == "hello"

    def test_string_empty(self):
        text = crous.dumps_text("")
        assert "s0:" in text
        assert crous.loads_text(text) == ""

    def test_string_unicode(self):
        s = "café ☕ 日本語"
        result = crous.loads_text(crous.dumps_text(s))
        assert result == s

    def test_string_with_newlines(self):
        s = "line1\nline2\ttab"
        assert crous.loads_text(crous.dumps_text(s)) == s

    def test_string_with_special_chars(self):
        """Strings containing CROUT syntax characters."""
        for s in ["{}", "[]", "()", "s5:", "b3:", "i42", "fnan", "T", "N", "@ a=b"]:
            assert crous.loads_text(crous.dumps_text(s)) == s

    def test_bytes_basic(self):
        text = crous.dumps_text(b"\x00\x01\x02\xff")
        assert "b4:000102ff" in text
        assert crous.loads_text(text) == b"\x00\x01\x02\xff"

    def test_bytes_empty(self):
        text = crous.dumps_text(b"")
        assert "b0:" in text
        assert crous.loads_text(text) == b""

    def test_bytes_ascii(self):
        text = crous.dumps_text(b"ABC")
        result = crous.loads_text(text)
        assert result == b"ABC"

    def test_bytes_all_values(self):
        """Round-trip all 256 byte values."""
        data = bytes(range(256))
        result = crous.loads_text(crous.dumps_text(data))
        assert result == data


# =============================================================================
# Container round-trips
# =============================================================================

class TestCroutContainers:
    """Lists, tuples, dicts, and nested structures."""

    def test_list_basic(self):
        data = [1, 2, 3]
        assert crous.loads_text(crous.dumps_text(data)) == data

    def test_list_empty(self):
        data = []
        assert crous.loads_text(crous.dumps_text(data)) == data

    def test_list_mixed_types(self):
        data = [None, True, False, 42, 3.14, "hello", b"\xff"]
        result = crous.loads_text(crous.dumps_text(data))
        assert result[0] is None
        assert result[1] is True
        assert result[2] is False
        assert result[3] == 42
        assert abs(result[4] - 3.14) < 1e-9
        assert result[5] == "hello"
        assert result[6] == b"\xff"

    def test_list_nested(self):
        data = [[1, 2], [3, [4, 5]]]
        assert crous.loads_text(crous.dumps_text(data)) == data

    def test_tuple_basic(self):
        data = (10, 20, 30)
        result = crous.loads_text(crous.dumps_text(data))
        assert isinstance(result, tuple)
        assert result == data

    def test_tuple_empty(self):
        data = ()
        result = crous.loads_text(crous.dumps_text(data))
        assert isinstance(result, tuple)
        assert result == ()

    def test_tuple_single(self):
        data = (42,)
        result = crous.loads_text(crous.dumps_text(data))
        assert isinstance(result, tuple)
        assert result == (42,)

    def test_dict_basic(self):
        data = {"name": "alice", "age": 30}
        result = crous.loads_text(crous.dumps_text(data))
        assert result == data

    def test_dict_empty(self):
        data = {}
        assert crous.loads_text(crous.dumps_text(data)) == data

    def test_dict_nested(self):
        data = {"a": {"b": {"c": "deep"}}}
        assert crous.loads_text(crous.dumps_text(data)) == data

    def test_dict_many_keys(self):
        data = {f"key_{i}": i for i in range(50)}
        assert crous.loads_text(crous.dumps_text(data)) == data

    def test_dict_mixed_values(self):
        data = {
            "null": None,
            "bool": True,
            "int": -42,
            "float": 2.718,
            "str": "value",
            "bytes": b"\xde\xad",
            "list": [1, 2],
            "tuple": (3, 4),
            "dict": {"inner": True},
        }
        result = crous.loads_text(crous.dumps_text(data))
        assert result["null"] is None
        assert result["bool"] is True
        assert result["int"] == -42
        assert abs(result["float"] - 2.718) < 1e-9
        assert result["str"] == "value"
        assert result["bytes"] == b"\xde\xad"
        assert result["list"] == [1, 2]
        assert result["tuple"] == (3, 4)
        assert result["dict"] == {"inner": True}

    def test_list_of_dicts(self):
        data = [{"x": 1}, {"x": 2}, {"x": 3}]
        assert crous.loads_text(crous.dumps_text(data)) == data

    def test_deeply_nested(self):
        """Moderately deep nesting should work."""
        data = {"a": None}
        for _ in range(50):
            data = {"level": data}
        assert crous.loads_text(crous.dumps_text(data)) == data


# =============================================================================
# Token table
# =============================================================================

class TestCroutTokenTable:
    """Token table compression for repeated dict keys."""

    def test_tokens_appear_for_repeated_keys(self):
        """Keys used >= 2 times should get token assignments."""
        data = [{"name": "a", "age": 1}, {"name": "b", "age": 2}]
        text = crous.dumps_text(data)
        # Should have token lines like "@ a=name\n@ b=age\n"
        assert "@ " in text
        assert "=name" in text
        assert "=age" in text

    def test_tokens_not_used_for_single_occurrence(self):
        """Keys used only once should NOT get tokens."""
        data = {"unique_key": 42}
        text = crous.dumps_text(data)
        assert "@ " not in text.split("\n", 2)[1]  # no token line after header

    def test_no_tokens_mode(self):
        """use_tokens=False suppresses the token table entirely."""
        data = [{"name": "a"}, {"name": "b"}, {"name": "c"}]
        text = crous.dumps_text(data, use_tokens=False)
        assert "@ " not in text
        assert crous.loads_text(text) == data

    def test_token_table_round_trip(self):
        """Data with tokens round-trips correctly."""
        data = [
            {"name": "alice", "age": 30, "city": "NYC"},
            {"name": "bob", "age": 25, "city": "LA"},
            {"name": "carol", "age": 35, "city": "SF"},
        ]
        text = crous.dumps_text(data)
        result = crous.loads_text(text)
        assert result == data

    def test_many_repeated_keys(self):
        """Stress test: many distinct keys all repeated."""
        keys = [f"field_{i}" for i in range(30)]
        row = {k: i for i, k in enumerate(keys)}
        data = [row, row, row]
        text = crous.dumps_text(data)
        result = crous.loads_text(text)
        assert result == data

    def test_token_threshold(self):
        """Default threshold=2: keys used only once are not tokenized."""
        data = [
            {"common": 1, "rare1": 10},
            {"common": 2, "rare2": 20},
        ]
        text = crous.dumps_text(data)
        lines = text.split("\n")
        token_lines = [l for l in lines if l.startswith("@ ")]
        # "common" should have a token, "rare1" and "rare2" should not
        assert any("=common" in l for l in token_lines)
        assert not any("=rare1" in l for l in token_lines)
        assert not any("=rare2" in l for l in token_lines)
        assert crous.loads_text(text) == data


# =============================================================================
# Pretty mode
# =============================================================================

class TestCroutPrettyMode:
    """Pretty printing with indentation."""

    def test_pretty_basic(self):
        data = {"a": 1, "b": 2}
        text = crous.dumps_text(data, pretty=True)
        assert "\n" in text
        assert "  " in text  # default indent=2
        assert crous.loads_text(text) == data

    def test_pretty_nested(self):
        data = {"outer": {"inner": [1, 2, 3]}}
        text = crous.dumps_text(data, pretty=True)
        assert crous.loads_text(text) == data

    def test_pretty_custom_indent(self):
        data = {"key": "value"}
        text = crous.dumps_text(data, pretty=True, indent=4)
        assert "    " in text  # 4-space indent
        assert crous.loads_text(text) == data

    def test_pretty_empty_containers(self):
        data = {"d": {}, "l": [], "t": ()}
        text = crous.dumps_text(data, pretty=True)
        assert crous.loads_text(text) == data

    def test_pretty_list_of_dicts(self):
        data = [{"x": 1}, {"x": 2}]
        text = crous.dumps_text(data, pretty=True)
        assert crous.loads_text(text) == data

    def test_compact_vs_pretty(self):
        """Compact output should be shorter than pretty."""
        data = {"a": [1, 2, 3], "b": {"c": True}}
        compact = crous.dumps_text(data)
        pretty = crous.dumps_text(data, pretty=True)
        assert len(compact) < len(pretty)


# =============================================================================
# CROUT ↔ FLUX (binary) conversion
# =============================================================================

class TestCroutFluxConversion:
    """text_to_flux() and flux_to_text() helpers."""

    def test_text_to_flux_basic(self):
        data = {"hello": "world"}
        text = crous.dumps_text(data)
        flux_bytes = crous.text_to_flux(text)
        assert isinstance(flux_bytes, bytes)
        assert len(flux_bytes) > 0
        # Decode via binary loads to verify
        result = crous.loads(flux_bytes)
        assert result == data

    def test_flux_to_text_basic(self):
        data = {"answer": 42}
        binary = crous.dumps(data)
        text = crous.flux_to_text(binary)
        assert isinstance(text, str)
        assert text.startswith("CROUT1")
        result = crous.loads_text(text)
        assert result == data

    def test_roundtrip_crout_flux_crout(self):
        data = [1, "two", 3.0, None, True]
        text1 = crous.dumps_text(data)
        flux = crous.text_to_flux(text1)
        text2 = crous.flux_to_text(flux)
        assert crous.loads_text(text2) == data

    def test_roundtrip_binary_crout_binary(self):
        data = {"key": [1, 2, 3], "nested": {"a": True}}
        bin1 = crous.dumps(data)
        text = crous.flux_to_text(bin1)
        bin2 = crous.text_to_flux(text)
        assert crous.loads(bin2) == data

    def test_flux_to_text_pretty(self):
        data = {"x": 1}
        binary = crous.dumps(data)
        text = crous.flux_to_text(binary, pretty=True)
        assert "\n" in text
        assert crous.loads_text(text) == data

    def test_flux_to_text_no_tokens(self):
        data = [{"k": 1}, {"k": 2}]
        binary = crous.dumps(data)
        text = crous.flux_to_text(binary, use_tokens=False)
        assert "@ " not in text
        assert crous.loads_text(text) == data

    def test_text_to_flux_with_tokens(self):
        """CROUT text with token table converts correctly to binary."""
        data = [{"name": "a"}, {"name": "b"}]
        text = crous.dumps_text(data)  # will have tokens
        assert "@ " in text
        flux = crous.text_to_flux(text)
        assert crous.loads(flux) == data

    def test_conversion_preserves_bytes(self):
        data = {"raw": b"\xde\xad\xbe\xef"}
        text = crous.dumps_text(data)
        flux = crous.text_to_flux(text)
        assert crous.loads(flux) == data

    def test_conversion_preserves_tuple(self):
        data = {"coords": (1, 2, 3)}
        text = crous.dumps_text(data)
        flux = crous.text_to_flux(text)
        result = crous.loads(flux)
        assert result["coords"] == (1, 2, 3)
        assert isinstance(result["coords"], tuple)


# =============================================================================
# Tagged values
# =============================================================================

class TestCroutTaggedValues:
    """Tagged values: #<tag>:<value>."""

    def test_tagged_round_trip(self):
        """Tagged values via binary round-trip."""
        # Create tagged value through binary format, convert to CROUT
        inner = {"data": 42}
        binary = crous.dumps(inner)
        # We can test tagged via flux_to_text if binary has tagged values
        # For now, test that the format itself works via direct text
        text = "CROUT1\n#1:i42"
        result = crous.loads_text(text)
        # Result should be a tagged value; exact Python representation
        # depends on the library
        assert result is not None

    def test_tagged_in_container(self):
        text = "CROUT1\n[#0:s5:hello , #1:i99]"
        result = crous.loads_text(text)
        assert isinstance(result, list)
        assert len(result) == 2


# =============================================================================
# Edge cases and error handling
# =============================================================================

class TestCroutEdgeCases:
    """Edge cases, malformed input, and error handling."""

    def test_missing_header(self):
        with pytest.raises(CrousDecodeError):
            crous.loads_text("{s1:a:i1}")

    def test_wrong_magic(self):
        with pytest.raises(CrousDecodeError):
            crous.loads_text("CROUT99\n{s1:a:i1}")

    def test_truncated_string(self):
        with pytest.raises(CrousDecodeError):
            crous.loads_text("CROUT1\ns100:short")

    def test_truncated_bytes(self):
        with pytest.raises(CrousDecodeError):
            crous.loads_text("CROUT1\nb100:aabb")

    def test_invalid_hex_in_bytes(self):
        with pytest.raises(CrousDecodeError):
            crous.loads_text("CROUT1\nb2:ZZZZ")

    def test_unclosed_dict(self):
        with pytest.raises(CrousDecodeError):
            crous.loads_text("CROUT1\n{s1:a:i1")

    def test_unclosed_list(self):
        with pytest.raises(CrousDecodeError):
            crous.loads_text("CROUT1\n[i1 , i2")

    def test_unclosed_tuple(self):
        with pytest.raises(CrousDecodeError):
            crous.loads_text("CROUT1\n(i1 , i2")

    def test_empty_input(self):
        with pytest.raises(CrousDecodeError):
            crous.loads_text("")

    def test_header_only(self):
        """Header with no value body should still fail gracefully."""
        with pytest.raises(CrousDecodeError):
            crous.loads_text("CROUT1\n")

    def test_returns_string_type(self):
        """dumps_text always returns str, not bytes."""
        text = crous.dumps_text(42)
        assert isinstance(text, str)

    def test_starts_with_magic(self):
        text = crous.dumps_text(42)
        assert text.startswith("CROUT1\n")

    def test_binary_safe_string_with_nul(self):
        """Strings containing NUL bytes round-trip."""
        s = "hello\x00world"
        result = crous.loads_text(crous.dumps_text(s))
        assert result == s

    def test_very_long_string(self):
        s = "x" * 10000
        result = crous.loads_text(crous.dumps_text(s))
        assert result == s

    def test_very_long_bytes(self):
        b = bytes(range(256)) * 40  # 10240 bytes
        result = crous.loads_text(crous.dumps_text(b))
        assert result == b

    def test_string_key_with_equals_sign(self):
        """Dict key containing '=' should not break token table parsing."""
        data = {"a=b": 42}
        result = crous.loads_text(crous.dumps_text(data))
        assert result == data

    def test_string_key_with_colon(self):
        """Dict key containing ':' should not break key:value parsing."""
        data = {"a:b": 42}
        result = crous.loads_text(crous.dumps_text(data))
        assert result == data

    def test_string_key_with_spaces(self):
        data = {"hello world": 42}
        result = crous.loads_text(crous.dumps_text(data))
        assert result == data

    def test_single_element_list(self):
        data = [42]
        assert crous.loads_text(crous.dumps_text(data)) == data

    def test_single_element_dict(self):
        data = {"k": "v"}
        assert crous.loads_text(crous.dumps_text(data)) == data

    def test_nested_empty_containers(self):
        data = {"a": [], "b": {}, "c": ()}
        result = crous.loads_text(crous.dumps_text(data))
        assert result["a"] == []
        assert result["b"] == {}
        assert result["c"] == ()

    def test_comment_in_crout_text(self):
        """Parser should skip // comments."""
        text = "CROUT1\n// this is a comment\ni42"
        result = crous.loads_text(text)
        assert result == 42

    def test_whitespace_tolerance(self):
        """Extra whitespace between tokens should be tolerated."""
        text = "CROUT1\n[  i1  ,  i2  ,  i3  ]"
        result = crous.loads_text(text)
        assert result == [1, 2, 3]


# =============================================================================
# Cross-format consistency
# =============================================================================

class TestCroutBinaryConsistency:
    """Verify CROUT and binary formats produce the same data."""

    @pytest.mark.parametrize("data", [
        None,
        True,
        False,
        0,
        42,
        -99,
        3.14,
        "",
        "hello",
        b"",
        b"\xff\x00",
        [],
        [1, 2, 3],
        {},
        {"a": 1},
        (1, 2),
        {"nested": {"list": [1, None, "str"]}},
    ])
    def test_binary_crout_equivalence(self, data):
        """Encoding to binary then to CROUT should yield the same data."""
        binary = crous.dumps(data)
        crout_text = crous.flux_to_text(binary)
        from_crout = crous.loads_text(crout_text)

        crout_direct = crous.dumps_text(data)
        from_direct = crous.loads_text(crout_direct)

        # Both paths should produce equivalent results
        if isinstance(data, float) and math.isnan(data):
            assert math.isnan(from_crout)
            assert math.isnan(from_direct)
        else:
            assert from_crout == from_direct
