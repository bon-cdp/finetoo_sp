// Copyright 2025 Finetoo
// Demo: BOM Generation Using Operations (No LLM Yet)
//
// This demonstrates the core operation primitives working on property graphs
// Next step: Add Gemini to compose these operations from natural language

#include <iostream>
#include <string>

#include "src/graph/graph_builder.h"
#include "src/operations/operation_executor.h"
#include "src/parser/dxf_text_parser.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <dxf_file> [dxf_file2 ...]\n";
    return 1;
  }

  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " Finetoo BOM Generation Demo (Operation Primitives)\n";
  std::cout << "════════════════════════════════════════════════════════════\n\n";

  // Step 1: Parse DXF files and build property graphs
  std::cout << "Step 1: Parsing DXF files...\n";
  std::vector<finetoo::graph::v1::PropertyGraph> graphs;

  for (int i = 1; i < argc; i++) {
    std::string file_path = argv[i];
    std::cout << "  Parsing: " << file_path << "\n";

    finetoo::graph::GraphBuilder builder;
    auto graph_or = builder.BuildFromFile(file_path);

    if (!graph_or.ok()) {
      std::cerr << "    Error: " << graph_or.status() << "\n";
      continue;
    }

    graphs.push_back(*graph_or);
    const auto& graph = graphs.back();

    std::cout << "    ✓ " << graph.stats().node_count() << " nodes, "
              << graph.stats().edge_count() << " edges\n";
  }

  if (graphs.empty()) {
    std::cerr << "\nNo graphs to process!\n";
    return 1;
  }

  std::cout << "\n";

  // Step 2: Use FILTER operation to find all INSERT entities
  std::cout << "Step 2: Finding all INSERT entities (FILTER operation)...\n";

  for (size_t i = 0; i < graphs.size(); i++) {
    auto& graph = graphs[i];
    finetoo::operations::OperationExecutor executor(&graph);

    // Create FILTER operation: FILTER(Entity, type == "INSERT")
    finetoo::operations::v1::Operation filter_op;
    filter_op.set_type(finetoo::operations::v1::FILTER);
    filter_op.set_target_type("Entity");
    filter_op.set_property_name("type");
    (*filter_op.mutable_parameters())["operator"] = "EQUALS";
    (*filter_op.mutable_parameters())["value"] = "INSERT";

    auto result_or = executor.Execute(filter_op);
    if (!result_or.ok()) {
      std::cerr << "  Error executing FILTER: " << result_or.status() << "\n";
      continue;
    }

    const auto& result = *result_or;
    std::cout << "  Graph " << (i + 1) << ": Found " << result.node_ids_size()
              << " INSERT entities\n";
  }

  std::cout << "\n";

  // Step 3: Use TRAVERSE operation to find referenced blocks
  std::cout << "Step 3: Finding referenced blocks (TRAVERSE operation)...\n";
  std::cout << "  Following REFERENCES edges from INSERT → Block\n";

  for (size_t i = 0; i < graphs.size(); i++) {
    auto& graph = graphs[i];
    finetoo::operations::OperationExecutor executor(&graph);

    // Create TRAVERSE operation: TRAVERSE(REFERENCES edge type)
    finetoo::operations::v1::Operation traverse_op;
    traverse_op.set_type(finetoo::operations::v1::TRAVERSE);
    (*traverse_op.mutable_parameters())["edge_type"] = "REFERENCES";

    auto result_or = executor.Execute(traverse_op);
    if (!result_or.ok()) {
      std::cerr << "  Error executing TRAVERSE: " << result_or.status() << "\n";
      continue;
    }

    const auto& result = *result_or;
    std::cout << "  Graph " << (i + 1) << ": Found " << result.node_ids_size()
              << " block references\n";

    // Show sample
    if (result.node_ids_size() > 0) {
      std::cout << "    Sample blocks: ";
      for (int j = 0; j < std::min(5, result.node_ids_size()); j++) {
        std::cout << result.node_ids(j);
        if (j < std::min(4, result.node_ids_size() - 1)) std::cout << ", ";
      }
      if (result.node_ids_size() > 5) std::cout << " ...";
      std::cout << "\n";
    }
  }

  std::cout << "\n";

  // Step 4: Use AGGREGATE operation to count blocks
  std::cout << "Step 4: Counting block usage (AGGREGATE operation)...\n";
  std::cout << "  Aggregating with GROUP_BY block name\n\n";

  for (size_t i = 0; i < graphs.size(); i++) {
    auto& graph = graphs[i];
    finetoo::operations::OperationExecutor executor(&graph);

    // Create AGGREGATE operation: AGGREGATE(COUNT, GROUP_BY name)
    finetoo::operations::v1::Operation agg_op;
    agg_op.set_type(finetoo::operations::v1::AGGREGATE);
    agg_op.set_target_type("Entity");
    agg_op.set_property_name("type");
    (*agg_op.mutable_parameters())["function"] = "COUNT";
    (*agg_op.mutable_parameters())["group_by"] = "type";

    auto result_or = executor.Execute(agg_op);
    if (!result_or.ok()) {
      std::cerr << "  Error executing AGGREGATE: " << result_or.status() << "\n";
      continue;
    }

    const auto& result = *result_or;
    std::cout << "  Graph " << (i + 1) << " Entity counts:\n";

    // Show counts
    for (const auto& [type, count_str] : result.values()) {
      if (type == "INSERT" || type == "LINE" || type == "CIRCLE" ||
          type == "DIMENSION" || type == "ARC") {
        std::cout << "    " << type << ": " << count_str << "\n";
      }
    }
  }

  std::cout << "\n";
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " Demo Complete!\n";
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << "\nWhat we demonstrated:\n";
  std::cout << "  ✓ DXF → Property Graph conversion (GraphBuilder)\n";
  std::cout << "  ✓ FILTER operation (find entities by criteria)\n";
  std::cout << "  ✓ TRAVERSE operation (follow REFERENCES edges)\n";
  std::cout << "  ✓ AGGREGATE operation (count and group)\n";
  std::cout << "\nNext step: Add Gemini LLM to compose these operations\n";
  std::cout << "  from natural language queries like:\n";
  std::cout << "    \"Generate a BOM from these drawings\"\n\n";

  return 0;
}
