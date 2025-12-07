"""
test_nested.py - Deep nesting and complex structure tests

Tests deeply nested structures and stress tests for nesting depth/breadth.
"""

import pytest
import crous


class TestDeepNesting:
    """Test deeply nested structures."""

    def test_nest_depth_10(self):
        """Test 10 levels of nesting."""
        data = {'l1': {'l2': {'l3': {'l4': {'l5': {'l6': {'l7': {'l8': {'l9': {'l10': 'value'}}}}}}}}}}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_nest_depth_50(self):
        """Test 50 levels of nesting."""
        data = {}
        current = data
        for i in range(50):
            current[f'level_{i}'] = {}
            current = current[f'level_{i}']
        current['value'] = 'deep'
        
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_nest_depth_100(self):
        """Test 100 levels of nesting."""
        data = {}
        current = data
        for i in range(100):
            current['next'] = {}
            current = current['next']
        current['bottom'] = 'value'
        
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_nested_lists(self):
        """Test deeply nested lists."""
        data = [[[[[['value']]]]]]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_nested_mixed_containers(self):
        """Test mixed nesting of lists and dicts."""
        data = {'a': {'b': [{'c': [1, 2, {'d': [3, 4]}]}]}}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_wide_nesting_many_keys(self):
        """Test dict with many keys at each level."""
        data = {}
        for i in range(100):
            data[f'key_{i}'] = {f'inner_{j}': j for j in range(100)}
        
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data


class TestBreadthStress:
    """Test structures with large breadth (many items at same level)."""

    def test_large_list(self):
        """Test list with 10,000 elements."""
        data = list(range(10000))
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_very_large_list(self):
        """Test list with 100,000 elements."""
        data = list(range(100000))
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_large_dict(self):
        """Test dict with 10,000 keys."""
        data = {f'key_{i}': i for i in range(10000)}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_very_large_dict(self):
        """Test dict with 100,000 keys."""
        data = {f'key_{i}': i for i in range(100000)}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_of_large_strings(self):
        """Test list containing large strings."""
        data = ['x' * 10000 for _ in range(1000)]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data
        assert len(result) == 1000
        assert all(len(s) == 10000 for s in result)

    def test_dict_with_large_values(self):
        """Test dict with large value strings."""
        data = {f'key_{i}': 'x' * 10000 for i in range(100)}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_large_bytes_list(self):
        """Test list of large bytes objects."""
        data = [bytes(range(256)) * 100 for _ in range(100)]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data


class TestComplexStructures:
    """Test realistically complex data structures."""

    def test_nested_table_structure(self):
        """Test structure resembling nested tables."""
        data = {
            'table1': [
                {'id': 1, 'name': 'row1', 'columns': [1, 2, 3]},
                {'id': 2, 'name': 'row2', 'columns': [4, 5, 6]},
            ],
            'table2': [
                {'id': 1, 'data': {'x': 1, 'y': 2}},
                {'id': 2, 'data': {'x': 3, 'y': 4}},
            ]
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_graph_like_structure(self):
        """Test structure resembling graph with nodes."""
        data = {
            'nodes': [
                {'id': 1, 'label': 'A', 'edges': [2, 3]},
                {'id': 2, 'label': 'B', 'edges': [1, 3]},
                {'id': 3, 'label': 'C', 'edges': [1, 2]},
            ],
            'metadata': {'nodes_count': 3, 'edges_count': 6}
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_tree_like_structure(self):
        """Test structure resembling tree."""
        data = {
            'value': 'root',
            'children': [
                {
                    'value': 'left',
                    'children': [
                        {'value': 'left-left', 'children': []},
                        {'value': 'left-right', 'children': []},
                    ]
                },
                {
                    'value': 'right',
                    'children': [
                        {'value': 'right-left', 'children': []},
                        {'value': 'right-right', 'children': []},
                    ]
                }
            ]
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_realistic_json_like_data(self):
        """Test realistic JSON-like data structure."""
        data = {
            'users': [
                {
                    'id': 1,
                    'username': 'alice',
                    'email': 'alice@example.com',
                    'profile': {
                        'bio': 'Software engineer',
                        'location': 'San Francisco',
                        'tags': ['python', 'rust', 'go'],
                    },
                    'posts': [
                        {
                            'id': 101,
                            'title': 'First post',
                            'content': 'Hello world',
                            'comments': [
                                {'author': 'bob', 'text': 'Nice!'},
                                {'author': 'charlie', 'text': 'Great!'},
                            ]
                        }
                    ]
                },
                {
                    'id': 2,
                    'username': 'bob',
                    'email': 'bob@example.com',
                    'profile': {
                        'bio': 'Data scientist',
                        'location': 'New York',
                        'tags': ['python', 'ml', 'data'],
                    },
                    'posts': []
                }
            ],
            'metadata': {
                'total_users': 2,
                'total_posts': 1,
            }
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data


class TestNestingInvariants:
    """Test invariants that should hold for nested structures."""

    def test_nested_list_type_preservation(self):
        """Test that types are preserved in nested lists."""
        data = [[1, 'two', 3.0], ['a', 2, 3.14]]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert isinstance(result[0][0], int)
        assert isinstance(result[0][1], str)
        assert isinstance(result[0][2], float)

    def test_nested_dict_value_types(self):
        """Test that value types are preserved in nested dicts."""
        data = {'outer': {'int': 1, 'str': 'hello', 'float': 3.14}}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert isinstance(result['outer']['int'], int)
        assert isinstance(result['outer']['str'], str)
        assert isinstance(result['outer']['float'], float)

    def test_nested_containers_independence(self):
        """Test that nested containers don't interfere with each other."""
        data = [
            {'a': [1, 2, 3]},
            {'b': [4, 5, 6]},
            {'c': [7, 8, 9]},
        ]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert result[0]['a'] == [1, 2, 3]
        assert result[1]['b'] == [4, 5, 6]
        assert result[2]['c'] == [7, 8, 9]

    def test_nested_empty_containers(self):
        """Test that empty containers at different nesting levels work."""
        data = {
            'empty_list': [],
            'empty_dict': {},
            'nested': {
                'empty_list': [],
                'empty_dict': {},
            }
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data
