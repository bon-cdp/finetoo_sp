#!/usr/bin/env python3
"""
Thought to Finetoo Schema Converter

Converts unstructured thoughts/text into Finetoo-compatible schemas
with semantic flags and available operations.

This bridges the logical consistency scorer with the Finetoo structuring protocol.
"""

import json
from typing import List, Dict, Set, Any
from dataclasses import dataclass, asdict
from logical_consistency_scorer import LogicalConsistencyScorer, LogicalStatement


@dataclass
class FinetooProperty:
    """Represents a property in the Finetoo schema."""
    name: str
    type: str  # "string", "boolean", "relation", etc.
    unique: bool = False
    comparable: bool = False
    indexed: bool = False
    aggregable: bool = False
    # Semantic extensions
    entity_identifier: bool = False
    logical_relation: bool = False
    property_attribute: bool = False


@dataclass
class FinetooNode:
    """Represents a node type in the Finetoo schema."""
    name: str
    properties: List[FinetooProperty]
    description: str = ""


@dataclass
class FinetooEdge:
    """Represents an edge type in the Finetoo schema."""
    name: str
    source_type: str
    target_type: str
    properties: List[FinetooProperty]
    description: str = ""


@dataclass
class FinetooSchema:
    """Complete Finetoo schema."""
    nodes: List[FinetooNode]
    edges: List[FinetooEdge]
    metadata: Dict[str, Any]


class ThoughtToFinetoo:
    """Converts thoughts/text to Finetoo schemas."""

    def __init__(self, verbose=False):
        self.verbose = verbose
        self.scorer = LogicalConsistencyScorer(verbose=False)

    def convert_to_schema(self, text: str) -> FinetooSchema:
        """
        Convert unstructured text to a Finetoo schema.

        Args:
            text: Raw text containing thoughts/statements

        Returns:
            FinetooSchema object
        """
        # First, score the consistency
        consistency_result = self.scorer.compute_consistency_score(text)

        # Parse statements and build entity graph
        statements = self.scorer.statements
        entities = self.scorer.entities

        # Generate schema from entities
        nodes = self._generate_nodes(entities, statements)
        edges = self._generate_edges(entities, statements)

        # Add metadata including consistency score
        metadata = {
            'consistency_score': consistency_result['score'],
            'cohomological_obstruction': consistency_result['residual'],
            'n_statements': len(statements),
            'n_entities': len(entities),
            'contradictions': consistency_result['contradictions']
        }

        return FinetooSchema(nodes=nodes, edges=edges, metadata=metadata)

    def _generate_nodes(self, entities: Dict, statements: List[LogicalStatement]) -> List[FinetooNode]:
        """Generate node types from entities."""
        nodes = []

        # Create a node type for "Entity" (the base type)
        entity_properties = [
            FinetooProperty(
                name="name",
                type="string",
                unique=True,
                comparable=True,
                indexed=True,
                entity_identifier=True
            ),
            FinetooProperty(
                name="type",
                type="string",
                comparable=True,
                indexed=True
            )
        ]

        # Collect all unique properties across entities
        all_properties = set()
        for entity in entities.values():
            all_properties.update(entity.properties)

        # Add properties as node properties
        for prop in sorted(all_properties):
            if prop.startswith('not_'):
                # Handle negated properties
                base_prop = prop[4:]
                entity_properties.append(
                    FinetooProperty(
                        name=f"lacks_{base_prop}",
                        type="boolean",
                        comparable=True,
                        property_attribute=True
                    )
                )
            else:
                entity_properties.append(
                    FinetooProperty(
                        name=f"has_{prop}",
                        type="boolean",
                        comparable=True,
                        aggregable=True,
                        property_attribute=True
                    )
                )

        nodes.append(FinetooNode(
            name="Entity",
            properties=entity_properties,
            description="Base entity type representing concepts in the knowledge graph"
        ))

        # Create specific node types for entities that appear as types
        entity_types = set()
        for stmt in statements:
            if stmt.relation == 'is':
                entity_types.add(stmt.object)

        for entity_type in sorted(entity_types):
            if entity_type in entities:
                # This is both an entity and a type
                type_properties = [
                    FinetooProperty(
                        name="name",
                        type="string",
                        unique=True,
                        entity_identifier=True
                    )
                ]

                # Add properties specific to this type
                if entity_type in entities:
                    for prop in entities[entity_type].properties:
                        if not prop.startswith('not_'):
                            type_properties.append(
                                FinetooProperty(
                                    name=prop,
                                    type="string",
                                    comparable=True,
                                    property_attribute=True
                                )
                            )

                nodes.append(FinetooNode(
                    name=entity_type.capitalize(),
                    properties=type_properties,
                    description=f"Specialized type: {entity_type}"
                ))

        return nodes

    def _generate_edges(self, entities: Dict, statements: List[LogicalStatement]) -> List[FinetooEdge]:
        """Generate edge types from relationships."""
        edges = []

        # Collect all relation types
        relation_types = set()
        for entity in entities.values():
            relation_types.update(entity.relations.keys())

        # Create edges for each relation type
        for rel_type in sorted(relation_types):
            if rel_type == 'is':
                edges.append(FinetooEdge(
                    name="IS_A",
                    source_type="Entity",
                    target_type="Entity",
                    properties=[
                        FinetooProperty(
                            name="confidence",
                            type="float",
                            comparable=True,
                            aggregable=True
                        )
                    ],
                    description="Taxonomic/type relationship"
                ))
            elif rel_type == 'is_not':
                edges.append(FinetooEdge(
                    name="IS_NOT",
                    source_type="Entity",
                    target_type="Entity",
                    properties=[
                        FinetooProperty(
                            name="confidence",
                            type="float",
                            comparable=True
                        )
                    ],
                    description="Negative type relationship"
                ))

        # Add property relationships as edges
        edges.append(FinetooEdge(
            name="HAS_PROPERTY",
            source_type="Entity",
            target_type="Property",
            properties=[
                FinetooProperty(
                    name="value",
                    type="string",
                    comparable=True
                )
            ],
            description="Entity has a specific property"
        ))

        return edges

    def generate_operations(self, schema: FinetooSchema) -> List[Dict[str, Any]]:
        """
        Generate available operations based on the schema.

        Returns list of operation definitions that can be executed.
        """
        operations = []

        # MATCH operations for unique properties
        for node in schema.nodes:
            for prop in node.properties:
                if prop.unique:
                    operations.append({
                        'type': 'MATCH',
                        'node_type': node.name,
                        'property': prop.name,
                        'description': f"Find {node.name} by {prop.name}"
                    })

        # FILTER operations for comparable properties
        for node in schema.nodes:
            for prop in node.properties:
                if prop.comparable:
                    operations.append({
                        'type': 'FILTER',
                        'node_type': node.name,
                        'property': prop.name,
                        'operators': ['EQUALS', 'NOT_EQUALS'],
                        'description': f"Filter {node.name} by {prop.name}"
                    })

        # TRAVERSE operations for edges
        for edge in schema.edges:
            operations.append({
                'type': 'TRAVERSE',
                'edge_type': edge.name,
                'source': edge.source_type,
                'target': edge.target_type,
                'description': f"Traverse {edge.name} from {edge.source_type} to {edge.target_type}"
            })

        # AGGREGATE operations for aggregable properties
        for node in schema.nodes:
            aggregable_props = [p for p in node.properties if p.aggregable]
            if aggregable_props:
                operations.append({
                    'type': 'AGGREGATE',
                    'node_type': node.name,
                    'properties': [p.name for p in aggregable_props],
                    'functions': ['COUNT', 'GROUP_BY'],
                    'description': f"Aggregate {node.name} data"
                })

        return operations

    def export_schema(self, schema: FinetooSchema, format: str = 'json') -> str:
        """
        Export schema to various formats.

        Args:
            schema: FinetooSchema object
            format: 'json' or 'protobuf' (text format)

        Returns:
            Serialized schema string
        """
        if format == 'json':
            return self._export_json(schema)
        elif format == 'protobuf':
            return self._export_protobuf(schema)
        else:
            raise ValueError(f"Unsupported format: {format}")

    def _export_json(self, schema: FinetooSchema) -> str:
        """Export schema as JSON."""
        schema_dict = {
            'nodes': [
                {
                    'name': node.name,
                    'description': node.description,
                    'properties': [asdict(prop) for prop in node.properties]
                }
                for node in schema.nodes
            ],
            'edges': [
                {
                    'name': edge.name,
                    'source_type': edge.source_type,
                    'target_type': edge.target_type,
                    'description': edge.description,
                    'properties': [asdict(prop) for prop in edge.properties]
                }
                for edge in schema.edges
            ],
            'metadata': schema.metadata
        }
        return json.dumps(schema_dict, indent=2)

    def _export_protobuf(self, schema: FinetooSchema) -> str:
        """Export schema as protobuf text format."""
        lines = []
        lines.append("// Auto-generated Finetoo Schema from thoughts")
        lines.append(f"// Consistency Score: {schema.metadata['consistency_score']:.1f}%")
        lines.append(f"// Cohomological Obstruction: {schema.metadata['cohomological_obstruction']:.4e}")
        lines.append("")

        # Node messages
        for node in schema.nodes:
            lines.append(f"message {node.name} {{")
            if node.description:
                lines.append(f"  // {node.description}")

            for i, prop in enumerate(node.properties, 1):
                type_map = {
                    'string': 'string',
                    'boolean': 'bool',
                    'float': 'float',
                    'int': 'int32'
                }
                proto_type = type_map.get(prop.type, 'string')

                # Add semantic comments
                comments = []
                if prop.unique:
                    comments.append("unique")
                if prop.comparable:
                    comments.append("comparable")
                if prop.indexed:
                    comments.append("indexed")
                if prop.aggregable:
                    comments.append("aggregable")

                if comments:
                    lines.append(f"  {proto_type} {prop.name} = {i};  // {', '.join(comments)}")
                else:
                    lines.append(f"  {proto_type} {prop.name} = {i};")

            lines.append("}")
            lines.append("")

        # Edge messages
        for edge in schema.edges:
            lines.append(f"message {edge.name} {{")
            lines.append(f"  // {edge.description}")
            lines.append(f"  string source_{edge.source_type.lower()} = 1;")
            lines.append(f"  string target_{edge.target_type.lower()} = 2;")

            for i, prop in enumerate(edge.properties, 3):
                type_map = {
                    'string': 'string',
                    'boolean': 'bool',
                    'float': 'float',
                    'int': 'int32'
                }
                proto_type = type_map.get(prop.type, 'string')
                lines.append(f"  {proto_type} {prop.name} = {i};")

            lines.append("}")
            lines.append("")

        return "\n".join(lines)


def demo():
    """Demonstrate thought to Finetoo conversion."""

    print("=" * 80)
    print(" Thought to Finetoo Schema Converter Demo ".center(80))
    print("=" * 80)

    converter = ThoughtToFinetoo(verbose=True)

    # Example: Convert philosophical thoughts
    text = """
    consciousness is phenomenon
    awareness is phenomenon
    thought is process
    memory is storage
    consciousness has awareness
    consciousness has thought
    thought has memory
    awareness is not memory
    process is not storage
    """

    print("\nInput thoughts:")
    print("-" * 40)
    print(text.strip())

    print("\n\nConverting to Finetoo schema...")
    print("-" * 40)

    schema = converter.convert_to_schema(text)

    # Display metadata
    print(f"\nMetadata:")
    print(f"  Consistency Score: {schema.metadata['consistency_score']:.1f}%")
    print(f"  Cohomological Obstruction: {schema.metadata['cohomological_obstruction']:.4e}")
    print(f"  Entities: {schema.metadata['n_entities']}")
    print(f"  Statements: {schema.metadata['n_statements']}")

    if schema.metadata['contradictions']:
        print(f"  Contradictions: {len(schema.metadata['contradictions'])}")
        for c in schema.metadata['contradictions']:
            print(f"    - {c[0]} vs {c[1]}")

    # Display schema
    print("\nGenerated Schema:")
    print("-" * 40)
    print(f"Nodes: {len(schema.nodes)}")
    for node in schema.nodes:
        print(f"  - {node.name} ({len(node.properties)} properties)")

    print(f"\nEdges: {len(schema.edges)}")
    for edge in schema.edges:
        print(f"  - {edge.name}: {edge.source_type} -> {edge.target_type}")

    # Generate operations
    operations = converter.generate_operations(schema)
    print(f"\nAvailable Operations: {len(operations)}")
    for op in operations[:5]:  # Show first 5
        print(f"  - {op['type']}: {op['description']}")
    if len(operations) > 5:
        print(f"  ... and {len(operations) - 5} more")

    # Export examples
    print("\n\nExport Formats:")
    print("-" * 40)

    print("\n1. JSON Format (snippet):")
    json_export = converter.export_schema(schema, 'json')
    print(json_export[:500] + "..." if len(json_export) > 500 else json_export)

    print("\n2. Protobuf Format (snippet):")
    proto_export = converter.export_schema(schema, 'protobuf')
    print(proto_export[:500] + "..." if len(proto_export) > 500 else proto_export)

    print("\n" + "=" * 80)
    print("Conversion complete! The schema can now be used with Finetoo agents.")
    print("=" * 80)


if __name__ == "__main__":
    demo()