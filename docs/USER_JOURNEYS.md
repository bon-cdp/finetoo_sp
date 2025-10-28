# Finetoo User Journeys

This document details the interactive conversational workflows for the two primary use cases in Phase 1.

## Journey 1: Bill of Materials (BOM) Generation

### Starting the Session

```bash
$ finetoo
Finetoo v0.1.0 - Interactive Engineering Document Assistant
Type 'help' for available commands, 'exit' to quit.

finetoo>
```

### Complete Workflow Example

```
You: Generate a BOM from all drawings in binder_test_no_refs/

Finetoo: I'll scan the directory and parse all DXF files...

        [Progress indicators]
        ✓ 18066-G-300.dxf (17MB) - 477 blocks, 5,643 entities
        ✓ 18066-G-301.dxf (34MB) - 892 blocks, 11,234 entities
        ✓ 18066-G-302.dxf (2.8MB) - 123 blocks, 1,891 entities
        ✓ 18066-G-304.dxf (6MB) - 345 blocks, 4,012 entities
        ✓ 18066-G-305.dxf (1.7MB) - 89 blocks, 987 entities
        ✓ 18066-G-306.dxf (15MB) - 421 blocks, 5,234 entities
        ✓ 18066-G-307.dxf (2.5MB) - 156 blocks, 1,976 entities

        Analysis complete!

        Found 247 unique parts across all drawings
        Total INSERT references: 1,432

        ⚠ Warning: Detected 3 blocks with divergent content:
          1. Block *U282 (appears in 3 drawings, 2 different versions)
          2. Block *U445 (appears in 4 drawings, 3 different versions)
          3. Block *VALVE_10 (appears in 2 drawings, 2 different versions)

        Would you like to:
        1. See the full BOM
        2. Investigate divergences first
        3. Export BOM to file

You: Investigate divergences first

Finetoo: Let's start with Block *U282 (highest impact - 14 total instances)

        Version Analysis:

        Version 1 (hash: a3f2c14b7e...)
          Found in: 18066-G-300.dxf (4 instances)
                    18066-G-304.dxf (2 instances)
          Entities: 23 (12 LINEs, 8 ARCs, 3 DIMENSIONs)

        Version 2 (hash: b7e8d49a2c...)
          Found in: 18066-G-301.dxf (8 instances)
          Entities: 24 (12 LINEs, 9 ARCs, 3 DIMENSIONs)

        Key differences detected:
          - Arc radius changed: 2.00" → 2.25" (entity handle 3F2B)
          - Dimension value changed: "3.500" → "3.750" (entity handle 3F2A)
          - One additional arc entity in Version 2

        These are significant geometric changes. Which version should be canonical?
        [1] Version 1 (G-300, G-304)
        [2] Version 2 (G-301)
        [3] Skip for now

You: 2

Finetoo: Marked Version 2 as canonical for Block *U282

        This means:
          - G-300 and G-304 use non-canonical version (6 instances need attention)
          - G-301 is correct (8 instances)

        Moving to next divergence: Block *U445

        [Similar analysis...]

You: Show me the BOM now

Finetoo: Generating Bill of Materials with full provenance...

        ╔═══════════════════════════════════════════════════════════════════╗
        ║  BILL OF MATERIALS - Project 18066 C-Loop                         ║
        ║  Generated: 2025-01-15 14:32:18                                   ║
        ║  Source: binder_test_no_refs/ (7 drawings)                        ║
        ╚═══════════════════════════════════════════════════════════════════╝

        Part Name          | Qty | Drawings        | Status      | Notes
        ───────────────────┼─────┼─────────────────┼─────────────┼──────────
        *U282              |  14 | 300,301,304     | ⚠ Divergent | Use G-301
        *U445              |  23 | 300,301,304,306 | ⚠ Divergent | Review req'd
        *VALVE_10          |   8 | 305,307         | ⚠ Divergent | Minor diff
        *FLANGE_8IN        |  42 | All             | ✓ Consistent|
        *BOLT_M12          | 168 | All             | ✓ Consistent|
        *NUT_M12           | 168 | All             | ✓ Consistent|
        ...

        Total unique parts: 247
        Total instances: 1,432
        Divergent blocks: 3

        Export options:
        1. JSON (with full provenance and handles)
        2. CSV (simplified for spreadsheets)
        3. Excel (with tabs for divergences)
        4. PDF report

You: Export to JSON

Finetoo: Exporting BOM to bom_18066_20250115.json...
        ✓ Complete! (24 KB)

        File includes:
          - All 247 parts with quantities
          - INSERT handle references
          - Drawing provenance
          - Divergence warnings
          - Canonical version selections

        You can import this into your PLM system or process further.
```

## Journey 2: Inspection Checklist Generation

### Complete Workflow Example

```
You: Create an inspection checklist from 18066-G-300.dxf

Finetoo: Analyzing drawing 18066-G-300.dxf for inspection requirements...

        ✓ Parsed successfully (477 blocks, 5,643 entities)
        ✓ Found 94 DIMENSION entities
        ✓ Identified 4 GD&T callouts (perpendicularity, flatness, position)

        Analyzing dimensions for criticality...

        Dimension Classification:
          Critical (tolerance ≤ 0.01"):    12 dimensions
          Standard (tolerance ≤ 0.05"):    31 dimensions
          Non-critical (tolerance > 0.05"): 51 dimensions

        Default: Including only critical dimensions (12)

        Would you like to:
        1. Preview critical dimensions
        2. Adjust criticality threshold
        3. Include all dimensions
        4. Proceed with default

You: Preview critical dimensions

Finetoo: Critical Dimensions Preview (tolerance ≤ 0.01"):

        1. Bore Diameter
           Value: 3.500" ±0.005"
           Handle: 2A1B
           Layer: DIMENSIONS
           Note: Primary shaft interface

        2. Flange Thickness
           Value: 0.750" ±0.008"
           Handle: 2B3C
           Layer: DIMENSIONS
           Note: Sealing surface

        3. Bolt Circle Diameter
           Value: 8.000" ±0.010"
           Handle: 2C5D
           Layer: DIMENSIONS
           Note: Bolt pattern positioning

        4. Face Flatness (GD&T)
           Value: 0.003" total
           Handle: 3D2E
           Type: Flatness callout

        [8 more dimensions...]

        These dimensions will be preserved in the inspection drawing.
        All other geometry will be simplified.

You: Also include GD&T callouts

Finetoo: Adding all GD&T annotations to inspection checklist...

        Found 4 geometric tolerances:
          ✓ Perpendicularity: 0.005" @ datum A (handle 3D2)
          ✓ Position: Ø0.010" @ datum A,B (handle 3D7)
          ✓ Flatness: 0.003" (handle 3E1)
          ✓ Concentricity: Ø0.008" @ datum C (handle 3F4)

        All will be included in output.

You: Generate the checklist

Finetoo: Generating inspection checklist drawing...

        Process:
        [1/5] Extracting critical dimensions and GD&T... ✓
        [2/5] Simplifying geometry (removing 4,891 of 5,643 entities)... ✓
        [3/5] Preserving dimension annotations and leaders... ✓
        [4/5] Adding inspection metadata (checkboxes, table)... ✓
        [5/5] Writing DXF file... ✓

        Output: inspection_18066-G-300.dxf (2.1 MB, 752 entities)
        Reduction: 87% smaller, preserves all critical information

        Inspection Drawing Contents:
          - 12 critical dimensions with tolerances
          - 4 GD&T callouts with datum references
          - Simplified outline geometry
          - Inspection table with checkboxes
          - Sign-off area

        You can now:
        1. Open in AutoCAD/LibreCAD for review
        2. Export to PDF for printing
        3. Adjust dimension selection
        4. Generate for additional drawings

You: Export to PDF

Finetoo: Converting to PDF for printing...

        ✓ PDF generated: inspection_18066-G-300.pdf

        Format: Letter size (8.5" x 11")
        Scale: Fit to page
        Orientation: Landscape

        The PDF is ready for print distribution to QC team.
```

## Additional Conversational Capabilities

### Cross-Drawing Queries

```
You: Which parts appear in all 7 drawings?

Finetoo: Analyzing cross-drawing part usage...

        Parts appearing in ALL 7 drawings (common components):

        1. *FLANGE_8IN (42 total instances)
        2. *BOLT_M12 (168 total instances)
        3. *NUT_M12 (168 total instances)
        4. *WASHER_M12 (168 total instances)
        5. *GASKET_8IN (14 total instances)

        These are your standard components across the entire assembly.
```

### Dimension Queries

```
You: Find all dimensions greater than 10 inches in G-301

Finetoo: Searching for dimensions > 10" in 18066-G-301.dxf...

        Found 8 dimensions:

        1. 12.500" ±0.050" (handle 4A2B) - Overall length
        2. 14.750" ±0.100" (handle 4C3D) - Flange span
        3. 16.000" ±0.020" (handle 4E5F) - Bolt circle
        4. 11.250" ±0.030" (handle 4G6H) - Mounting width
        ...

        Export options: CSV, JSON, or add to inspection checklist?
```

## Technical Implementation Notes

These conversational interactions are powered by:

1. **SchemaAnalyzer**: Discovers available operations from DXF schema
2. **OperationExecutor**: Executes MATCH, FILTER, TRAVERSE, AGGREGATE primitives
3. **PartIdentifier**: Semantic classification of blocks as parts
4. **BlockAnalyzer**: SHA-256 hashing and divergence detection
5. **DimensionExtractor**: Parses DIMENSION entities and tolerances
6. **BOMGenerator / DXFWriter**: Output generation with full provenance

All operations work on the property graph representation, ensuring consistent results and full traceability back to source handles.
