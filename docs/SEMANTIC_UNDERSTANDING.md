# Semantic Understanding in Finetoo

## Overview

Finetoo achieves semantic understanding of engineering documents through **schema-driven metadata**, not hard-coded rules. The schema encodes the "meaning" of properties, and operations emerge automatically.

## Part vs. Annotation: The Fundamental Distinction

### The Challenge

In a CAD drawing, blocks serve many purposes:
- **Parts**: Physical components (flanges, valves, bolts)
- **Annotations**: Symbols, title blocks, revision clouds, dimension markers
- **Reference**: Border, scale indicators, north arrows

Traditional systems either:
1. Hard-code rules ("if name contains VALVE, it's a part") - brittle!
2. Treat everything the same - loses meaning!

### Finetoo's Schema-Driven Approach

```protobuf
message PropertyMetadata {
  string name = 1;
  Type type = 2;

  // Semantic flags
  bool unique = 3;          // Can identify entities uniquely
  bool comparable = 4;      // Can detect differences
  bool indexed = 5;         // Can filter efficiently
  bool aggregable = 6;      // Can count/sum

  // Future extensions for semantic classification
  // bool part_identifier = 7;
  // bool dimensional_attribute = 8;
  // bool tolerance_specification = 9;
}
```

### How Parts Are Identified

1. **Schema Analysis**:
   - Blocks with `comparable=true` content_hash → can detect divergence
   - INSERT entities with `type="INSERT"` → part instances
   - Edge type `REFERENCES` connects INSERT → Block → part definition

2. **Traversal Operation**:
   ```
   TRAVERSE(entity, REFERENCES, Block)
   ```
   This discovers: "entity 3F2 references Block *U282" → *U282 is a part!

3. **Aggregation Operation**:
   ```
   AGGREGATE(COUNT, INSERTs, GROUP_BY block_name)
   ```
   This computes: "*U282 appears 14 times" → BOM quantity!

**Key Insight**: We didn't hard-code "INSERT means part". The schema describes the relationship, and the operation discovers it.

## Multi-Format Semantic Richness

Different formats encode semantics differently:

### DXF (2D Drawings)

**Semantic Metadata Available**:
- Block definitions (parts)
- INSERT references (part instances)
- DIMENSION entities (tolerances)
- Layer organization (functional grouping)

**Limitations**:
- No explicit assembly structure
- No material properties
- Limited tolerance specifications
- 2D only (no true 3D relationships)

### STEP (3D Models + Assemblies)

**Semantic Metadata Available** (much richer!):
- PRODUCT entities (explicit parts with part numbers)
- PRODUCT_DEFINITION_FORMATION (versions/revisions)
- PRODUCT_DEFINITION_CONTEXT (design/manufacturing intent)
- MATERIAL_DESIGNATION (metals, plastics, composites)
- GEOMETRIC_TOLERANCE (full GD&T specifications)
- ASSEMBLY relationships (parent-child hierarchy)
- SHAPE_REPRESENTATION (3D geometry)

**Example STEP Data**:
```step
#10=PRODUCT('FLANGE-8IN-150LB','Flange 8 inch 150lb','ASME B16.5',(#20));
#20=PRODUCT_CONTEXT('',#2,'mechanical');
#30=MATERIAL_DESIGNATION('','Stainless Steel 316',(#10));
#40=GEOMETRIC_TOLERANCE(.PERPENDICULARITY.,#50,#60,0.005);
```

**Schema Extraction**:
```
Node: Product
  Properties:
    part_number: "FLANGE-8IN-150LB" (unique=true, part_identifier=true)
    description: "Flange 8 inch 150lb"
    standard: "ASME B16.5"
    material: "Stainless Steel 316" (material_specification=true)

Edge: PRODUCT --MADE_OF--> MATERIAL
Edge: PRODUCT --HAS_TOLERANCE--> GEOMETRIC_TOLERANCE
```

**Discovered Operations**:
- `MATCH parts WHERE material="Stainless Steel 316"` (material-based BOM!)
- `FILTER tolerances WHERE type=PERPENDICULARITY AND value<0.01` (tight tolerances)
- `TRAVERSE product, HAS_CHILD, assembly` (exploded BOM with nesting!)

### STL (3D Mesh)

**Semantic Metadata Available** (minimal):
- Triangle vertices and normals
- Solid names (sometimes)

**Computed Properties**:
- Volume (via mesh integration)
- Surface area
- Bounding box
- Mesh quality (watertight, normal consistency)

**Use Cases**:
- Geometry validation: "Is this part 3D printable?"
- Weight estimation: "Given material density, compute mass"
- Collision detection: "Do parts interfere?"

**No Part Semantics**: STL doesn't distinguish parts from annotations because it's just triangles. But we can still extract geometric properties for analysis.

## Schema Evolution: Adding Semantic Flags

As we encounter new semantic needs, we extend the schema:

### Current (v1)

```protobuf
message PropertyMetadata {
  bool unique = 3;
  bool comparable = 4;
  bool indexed = 5;
  bool aggregable = 6;
}
```

### Future (v2) - Semantic Extensions

```protobuf
message PropertyMetadata {
  // v1 flags
  bool unique = 3;
  bool comparable = 4;
  bool indexed = 5;
  bool aggregable = 6;

  // v2 semantic extensions
  bool part_identifier = 16;      // This property identifies parts
  bool dimensional_attribute = 17;  // This is a measured dimension
  bool tolerance_specification = 18; // This specifies tolerance
  bool material_property = 19;      // Material designation
  bool assembly_relationship = 20;  // Parent-child assembly link

  reserved 7 to 15;  // Safe evolution!
}
```

### Enabling New Operations

Once we add `part_identifier=true` to STEP's `PRODUCT.part_number`:

```cpp
// Automatically discovered!
auto part_props = analyzer.FindPartIdentifierProperties(schema, "Product");
// Returns: ["part_number"]

// Operation is now available:
operations.push_back(FilterOperation("part_number", STARTS_WITH, "FLANGE"));
```

**No code changes required**. The schema describes semantics, operations emerge.

## Comparison to Traditional Approaches

### Hard-Coded Rules (Traditional)

```cpp
bool is_part(const Block& block) {
  // ❌ Brittle rules that break with new naming conventions
  if (block.name.contains("VALVE")) return true;
  if (block.name.starts_with("*U")) return true;
  if (block.name.ends_with("-PART")) return true;
  if (block.layer == "PARTS") return true;
  return false;
}
```

**Problems**:
- Breaks when naming changes
- Doesn't generalize to STEP, STL, etc.
- No provenance ("why is this a part?")
- Hard to extend

### Finetoo's Schema-Driven Approach

```cpp
// ✅ Generic discovery from schema
auto edge_types = analyzer.GetTraversableEdgeTypes(schema);
if (edge_types.contains("REFERENCES")) {
  // REFERENCES edges connect INSERTs to Blocks
  // Traverse to find all parts
  auto parts = executor.Traverse(inserts, "REFERENCES", "Block");
}
```

**Advantages**:
- Works on any format with schema-described relationships
- Full provenance (every part traced to source handle)
- Extensible (add new semantic flags, operations emerge)
- Zero-shot generalization

## Practical Examples

### Example 1: BOM from DXF

```
Schema says: Block has "name" (unique=true), "content_hash" (comparable=true)
Schema says: INSERT REFERENCES Block edge exists

Operations discovered:
1. MATCH(Block, name="*U282") → find block definition
2. TRAVERSE(INSERT, REFERENCES, Block) → find all instances
3. AGGREGATE(COUNT, GROUP_BY block_name) → compute quantities
4. COMPARE(content_hash) → detect divergence

Result: BOM with divergence warnings!
```

### Example 2: BOM from STEP

```
Schema says: PRODUCT has "part_number" (unique=true, part_identifier=true)
Schema says: ASSEMBLY HAS_CHILD PRODUCT edge exists

Operations discovered:
1. MATCH(PRODUCT, part_number="FLANGE-8IN")
2. TRAVERSE(ASSEMBLY, HAS_CHILD, PRODUCT) → exploded BOM!
3. PROJECT(part_number, material, mass) → extract attributes
4. AGGREGATE(SUM, mass) → total assembly weight

Result: Nested BOM with materials and weights!
```

### Example 3: Tolerance Analysis from STEP

```
Schema says: GEOMETRIC_TOLERANCE has "value" (aggregable=true, tolerance_specification=true)

Operations discovered:
1. FILTER(GEOMETRIC_TOLERANCE, value < 0.01) → tight tolerances
2. TRAVERSE(TOLERANCE, APPLIES_TO, FEATURE) → which features
3. AGGREGATE(MIN, value) → tightest tolerance in assembly
4. GROUP_BY(type) → tolerances by type (perpendicularity, flatness, etc.)

Result: Tolerance stackup analysis with provenance!
```

## Conclusion

Finetoo's semantic understanding comes from **explicit schema metadata**, not implicit rules. This enables:

1. **Generalization**: Same operations work on DXF, STEP, STL, Excel, Word, etc.
2. **Extensibility**: Add semantic flags, operations emerge automatically
3. **Provenance**: Every result traced to source entities with handles
4. **Composability**: Operations combine like Lego blocks

**The schema is the semantic layer**. Everything else follows.
