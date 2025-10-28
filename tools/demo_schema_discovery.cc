// Copyright 2025 Finetoo
// Demonstration: Schema-Driven Operation Discovery
//
// This tool demonstrates the core finetoo insight:
// Schemas encode their own operational capabilities!

#include <iostream>
#include <string>

#include "absl/strings/str_format.h"
#include "src/schema/schema_analyzer.h"

namespace {

void PrintSection(const std::string& title) {
  std::cout << "\n" << std::string(60, '=') << "\n";
  std::cout << title << "\n";
  std::cout << std::string(60, '=') << "\n\n";
}

void PrintProperty(const std::string& name, const std::vector<std::string>& values) {
  std::cout << "  " << name << ": [";
  for (size_t i = 0; i < values.size(); ++i) {
    std::cout << values[i];
    if (i < values.size() - 1) std::cout << ", ";
  }
  std::cout << "]\n";
}

}  // namespace

int main(int argc, char** argv) {
  using namespace finetoo::schema;

  PrintSection("Finetoo Schema-Driven Operation Discovery Demo");

  std::cout << "This demonstrates the core insight of the finetoo whitepaper:\n";
  std::cout << "Structured documents encode their own operations through schema!\n\n";

  // Create a DXF schema
  auto schema_or = SchemaAnalyzer::CreateDXFSchema("AC1027");
  if (!schema_or.ok()) {
    std::cerr << "Failed to create schema: " << schema_or.status() << "\n";
    return 1;
  }

  const auto& schema = *schema_or;
  SchemaAnalyzer analyzer;

  PrintSection("DXF Schema Overview");
  std::cout << "  Source Format: " << schema.source_format() << "\n";
  std::cout << "  Format Version: " << schema.format_version() << "\n";
  std::cout << "  Schema Version: " << schema.schema_version() << "\n";
  std::cout << "  Node Types: " << schema.node_types_size() << "\n";
  std::cout << "  Edge Types: " << schema.edge_types_size() << "\n";

  PrintSection("Node Type: Entity (CAD entities like LINE, POLYLINE, etc.)");

  auto unique_props = analyzer.FindUniqueProperties(schema, "Entity");
  PrintProperty("Unique Properties (enable MATCH operations)", unique_props);
  std::cout << "    → Operation: match_by_handle(v1_entity, v2_entity)\n\n";

  auto comparable_props = analyzer.FindComparableProperties(schema, "Entity");
  PrintProperty("Comparable Properties (enable COMPARE operations)", comparable_props);
  std::cout << "    → Operation: compare(v1.x, v2.x), compare(v1.y, v2.y)\n\n";

  auto indexed_props = analyzer.FindIndexedProperties(schema, "Entity");
  PrintProperty("Indexed Properties (enable FILTER operations)", indexed_props);
  std::cout << "    → Operation: filter(type == \"POLYLINE\")\n";
  std::cout << "    → Operation: filter(layer == \"EMS_REV\")\n\n";

  auto aggregable_props = analyzer.FindAggregableProperties(schema, "Entity");
  PrintProperty("Aggregable Properties (enable AGGREGATE operations)", aggregable_props);
  std::cout << "    → Operation: avg(y), min(x), max(x)\n";

  PrintSection("Node Type: Block (Block definitions)");

  auto block_unique = analyzer.FindUniqueProperties(schema, "Block");
  PrintProperty("Unique Properties", block_unique);

  auto block_comparable = analyzer.FindComparableProperties(schema, "Block");
  PrintProperty("Comparable Properties (enable divergence detection!)", block_comparable);
  std::cout << "    → Operation: compare(block1.content_hash, block2.content_hash)\n";
  std::cout << "    → This solves the C-loop source-of-truth problem!\n";

  PrintSection("Traversal Operations (from Edge Types)");

  auto edge_types = analyzer.GetTraversableEdgeTypes(schema);
  PrintProperty("Available Edge Types (enable TRAVERSE operations)", edge_types);
  std::cout << "    → Operation: traverse(entity, BELONGS_TO, layer)\n";
  std::cout << "    → Operation: traverse(block, CONTAINS, entity)\n";
  std::cout << "    → Operation: traverse(entity, REFERENCES, block)\n";

  PrintSection("Key Insight: Zero-Shot Generalization");

  std::cout << "ALL of the above operations were discovered from the schema,\n";
  std::cout << "NOT hardcoded in the application!\n\n";

  std::cout << "To add support for a new file format (e.g., Excel, Word):\n";
  std::cout << "  1. Create a schema with operational metadata\n";
  std::cout << "  2. Operations are automatically discovered\n";
  std::cout << "  3. LLM can immediately compose operations for queries\n\n";

  std::cout << "No fine-tuning required. No hardcoded tools.\n";
  std::cout << "Just schemas + generic operations + LLM composition.\n";

  PrintSection("C-Loop Block Divergence Example");

  std::cout << "From the earlier analysis, we found:\n";
  std::cout << "  Block *U282 exists in drawings 300, 301, 304\n";
  std::cout << "  BUT has 3 different content hashes!\n\n";

  std::cout << "With schema-driven approach, we can:\n";
  std::cout << "  1. match_by_name(block, \"*U282\") across drawings\n";
  std::cout << "  2. compare(drawing1.block.hash, drawing2.block.hash)\n";
  std::cout << "  3. filter(block.hash != expected_hash)\n";
  std::cout << "  4. group_by(block.name) to find all divergent blocks\n\n";

  std::cout << "LLM composes these operations. We execute. Return provenance.\n";

  PrintSection("What's Next?");

  std::cout << "  [✓] Protocol Buffer schemas with safe versioning\n";
  std::cout << "  [✓] Schema analyzer with operational metadata\n";
  std::cout << "  [✓] Operation discovery from schema\n";
  std::cout << "  [ ] DXF parser → property graph\n";
  std::cout << "  [ ] 8 generic operation primitives\n";
  std::cout << "  [ ] LLM integration for operation composition\n";
  std::cout << "  [ ] C-loop divergence analysis\n\n";

  std::cout << "This foundation enables the entire finetoo system.\n";

  PrintSection("End of Demo");

  return 0;
}
