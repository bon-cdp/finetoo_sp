# Logical Consistency Scorer with Wreath-Sheaf Cohomology

## Overview

This tool quantifies the logical consistency of natural language statements using the wreath-sheaf framework's cohomological obstruction. While wreath-sheaf isn't optimal for generating human-like text, it excels at measuring logical coherence through its mathematical foundations.

**Key Insight**: The cohomological obstruction (residual error from the global linear system) directly quantifies logical inconsistency. A score of 0 indicates perfect consistency, while higher scores reveal contradictions.

## Architecture

```
Unstructured Text → Parse → Entity Graph → Wreath-Sheaf Analysis → Consistency Score
                                    ↓
                              Finetoo Schema → Operations → Agents
```

### Components

1. **`logical_consistency_scorer.py`**: Core module that:
   - Parses natural language into logical statements
   - Builds entity relationship graphs
   - Encodes statements as vectors for sheaf analysis
   - Computes cohomological obstruction score

2. **`consistency_cli.py`**: Command-line interface for:
   - Interactive consistency checking
   - File-based batch processing
   - JSON output for integration

3. **`thought_to_finetoo.py`**: Finetoo integration that:
   - Converts thoughts to structured schemas
   - Generates semantic metadata flags
   - Produces available operations
   - Exports to JSON/Protobuf formats

4. **`test_consistency.py`**: Comprehensive test suite covering:
   - Perfect consistency scenarios
   - Direct contradictions
   - Transitive relationships
   - Real-world knowledge patterns

## Installation

```bash
# Ensure you're in the finetoo_sp directory
cd /home/shakil/Documents/finetoo_sp

# The tool uses existing wreath-sheaf implementations
# No additional dependencies needed beyond what's already installed
```

## Usage

### Basic CLI Usage

```bash
# Interactive mode
cd experiments/logical_consistency
python consistency_cli.py

# File input
python consistency_cli.py -f statements.txt

# Pipe input
echo "dog is animal
cat is animal
dog is cat" | python consistency_cli.py

# JSON output for integration
python consistency_cli.py -f statements.txt --json

# Verbose mode with detailed metrics
python consistency_cli.py -f statements.txt --verbose
```

### Python API

```python
from logical_consistency_scorer import LogicalConsistencyScorer

# Create scorer
scorer = LogicalConsistencyScorer()

# Score text
text = """
dog is animal
cat is animal
dog has fur
cat has fur
dog is not cat
"""

result = scorer.compute_consistency_score(text)

print(f"Consistency: {result['score']:.1f}%")
print(f"Cohomological Obstruction: {result['residual']:.4e}")
```

### Finetoo Schema Generation

```python
from thought_to_finetoo import ThoughtToFinetoo

# Convert thoughts to schema
converter = ThoughtToFinetoo()

text = """
consciousness is phenomenon
thought is process
consciousness has thought
"""

schema = converter.convert_to_schema(text)

# Export to different formats
json_schema = converter.export_schema(schema, 'json')
proto_schema = converter.export_schema(schema, 'protobuf')

# Generate operations
operations = converter.generate_operations(schema)
```

## Examples

### Example 1: Perfectly Consistent

```
Input:
dog is animal
cat is animal
dog has fur
cat has fur
dog is not cat

Output:
Consistency Score: 100.0%
Cohomological Obstruction: 0.0000e+00
Analysis: Perfect logical consistency! All 5 statements are coherent.
```

### Example 2: Direct Contradiction

```
Input:
dog is animal
dog is not animal

Output:
Consistency Score: 0.0%
Cohomological Obstruction: 1.4142e+00
Contradictions:
  - "dog is animal" vs "dog is not animal"
Analysis: Low logical consistency. 1 contradictions found.
```

### Example 3: Complex Relationships

```
Input:
mammal is animal
dog is mammal
cat is mammal
bird is animal
bird is not mammal
dog and cat have fur
bird has feathers

Output:
Consistency Score: 95.2%
Cohomological Obstruction: 2.3451e-02
Analysis: High logical consistency with minor tensions.
```

## How It Works

### 1. Statement Parsing

Supports patterns:
- `X is Y` (type/identity)
- `X is not Y` (negation)
- `X has Y` (property)
- `X lacks Y` (absence)
- `X and Y have Z` (shared property)

### 2. Entity Graph Construction

Creates a knowledge graph with:
- **Entities**: Subjects and objects from statements
- **Properties**: Attributes entities have/lack
- **Relations**: is/is_not connections between entities

### 3. Wreath-Sheaf Analysis

- **Patches**: Groups of statements by relation type
- **Vectors**: Encode entity presence, relations, properties
- **Gluing**: Enforce consistency between patches
- **Solution**: Solve global linear system
- **Obstruction**: Residual error = inconsistency measure

### 4. Scoring Interpretation

- **0-10% obstruction**: Perfect consistency ✅
- **10-30% obstruction**: High consistency with minor tensions 🟢
- **30-60% obstruction**: Moderate consistency, review needed 🟡
- **60-100% obstruction**: Poor consistency, major conflicts 🔴

## Integration with Finetoo

The consistency score feeds into the Finetoo structuring protocol:

1. **Schema Generation**: Thoughts → Entities → Node types
2. **Semantic Flags**: Properties marked as comparable, unique, etc.
3. **Operations**: MATCH, FILTER, TRAVERSE, AGGREGATE emerge
4. **Agent Guidance**: Consistency score helps agents process thoughts

### Generated Schema Example

```protobuf
message Entity {
  string name = 1;  // unique, comparable, indexed
  string type = 2;  // comparable, indexed
  bool has_fur = 3;  // comparable, aggregable
}

message IS_A {
  string source_entity = 1;
  string target_entity = 2;
  float confidence = 3;  // comparable, aggregable
}
```

## Testing

Run the comprehensive test suite:

```bash
cd experiments/logical_consistency
python test_consistency.py
```

Tests cover:
- Perfect consistency
- Direct contradictions
- Property conflicts
- Transitive relationships
- Circular references
- Real-world scenarios

## Performance

- **Complexity**: O(n²) for n statements (due to pairwise comparisons)
- **Speed**: ~100 statements/second on modern hardware
- **Memory**: Linear in number of entities and properties
- **FHE Compatible**: Depth 0 operations only

## Use Cases

1. **Digital Brain Organization**: Structure unorganized thoughts
2. **Knowledge Base Validation**: Check for logical conflicts
3. **Agent Pre-processing**: Score inputs before processing
4. **Documentation Analysis**: Verify technical docs consistency
5. **Belief System Analysis**: Identify contradictory beliefs

## Future Enhancements

- [ ] Support for temporal logic (X was Y, X becomes Z)
- [ ] Probabilistic statements (X might be Y)
- [ ] Conditional logic (if X then Y)
- [ ] Multi-valued logic (X is somewhat Y)
- [ ] Real-time incremental scoring
- [ ] Visual knowledge graph representation
- [ ] Integration with LLMs for explanation generation

## Theory Background

The wreath-sheaf framework combines:
- **Wreath Products**: Position-dependent transformations
- **Sheaf Theory**: Local-to-global consistency
- **Cohomology**: Obstruction to global solutions

The residual from solving the unified linear system quantifies the "cohomological obstruction" - essentially how much the local patches fail to glue into a consistent global structure.

## License

Part of the Finetoo Structuring Protocol project.

## Contact

For questions about the logical consistency scorer or wreath-sheaf theory:
shakilflynn@gmail.com