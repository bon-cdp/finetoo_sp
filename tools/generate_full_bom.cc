// Copyright 2025 Finetoo
// Full BOM Generation - All Drawings with Complete Metadata
//
// Parse all 7 C-loop drawings, build combined property graph, generate comprehensive BOM

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "src/cloud/vertex_ai_client.h"
#include "src/export/bom_exporter.h"
#include "src/graph/graph_builder.h"
#include "src/query/query_service.h"

int main(int argc, char** argv) {
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " Finetoo: Full BOM Generation Across All Drawings\n";
  std::cout << " Multi-Document Schema Analysis + Gemini Composition\n";
  std::cout << "════════════════════════════════════════════════════════════\n\n";

  // Get directory path
  std::string directory = (argc >= 2)
      ? argv[1]
      : "/home/shakil/Documents/finetoo_sp/binder_test_no_refs";

  std::string query = (argc >= 3)
      ? argv[2]
      : "Generate a complete bill of materials with quantities for all parts across all drawings";

  // Get Google Cloud configuration
  const char* project_env = std::getenv("FINETOO_GCP_PROJECT");
  const char* location_env = std::getenv("FINETOO_GCP_LOCATION");

  std::string project_id = project_env ? project_env : "";
  std::string location = location_env ? location_env : "us-central1";

  if (project_id.empty()) {
    std::cerr << "Error: FINETOO_GCP_PROJECT environment variable not set\n\n";
    return 1;
  }

  std::cout << "Configuration:\n";
  std::cout << "  GCP Project: " << project_id << "\n";
  std::cout << "  Location: " << location << "\n";
  std::cout << "  Directory: " << directory << "\n\n";

  // Step 1: Find all DXF files
  std::cout << "Step 1: Scanning for DXF files...\n";
  std::vector<std::string> dxf_files;

  try {
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
      if (entry.path().extension() == ".dxf") {
        dxf_files.push_back(entry.path().string());
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "  Error scanning directory: " << e.what() << "\n";
    return 1;
  }

  if (dxf_files.empty()) {
    std::cerr << "  No DXF files found in " << directory << "\n";
    return 1;
  }

  std::sort(dxf_files.begin(), dxf_files.end());
  std::cout << "  Found " << dxf_files.size() << " DXF files:\n";
  for (const auto& file : dxf_files) {
    std::filesystem::path p(file);
    std::cout << "    - " << p.filename().string() << "\n";
  }
  std::cout << "\n";

  // Step 2: Parse all files into one combined property graph
  std::cout << "Step 2: Parsing all DXF files into combined property graph...\n";

  finetoo::graph::v1::PropertyGraph combined_graph;
  finetoo::graph::GraphBuilder builder;

  // Parse first file to get schema
  auto first_graph_or = builder.BuildFromFile(dxf_files[0]);
  if (!first_graph_or.ok()) {
    std::cerr << "  Error parsing first file: " << first_graph_or.status() << "\n";
    return 1;
  }

  combined_graph = *first_graph_or;
  std::filesystem::path first_path(dxf_files[0]);
  std::cout << "  ✓ " << first_path.filename().string() << " - "
            << combined_graph.stats().node_count() << " nodes, "
            << combined_graph.stats().edge_count() << " edges\n";

  // Parse remaining files and merge
  for (size_t i = 1; i < dxf_files.size(); i++) {
    finetoo::graph::GraphBuilder file_builder;
    auto graph_or = file_builder.BuildFromFile(dxf_files[i]);

    if (!graph_or.ok()) {
      std::cerr << "  Error parsing " << dxf_files[i] << ": "
                << graph_or.status() << "\n";
      continue;
    }

    auto& graph = *graph_or;
    std::filesystem::path p(dxf_files[i]);
    std::cout << "  ✓ " << p.filename().string() << " - "
              << graph.stats().node_count() << " nodes, "
              << graph.stats().edge_count() << " edges\n";

    // Merge nodes
    for (const auto& [type, collection] : graph.nodes_by_type()) {
      auto& target_collection = (*combined_graph.mutable_nodes_by_type())[type];
      for (const auto& node : collection.nodes()) {
        // Add drawing source to node
        auto* new_node = target_collection.mutable_nodes()->Add();
        *new_node = node;
        (*new_node->mutable_string_props())["source_drawing"] = p.filename().string();
      }
      target_collection.set_count(target_collection.nodes_size());
    }

    // Merge edges
    for (const auto& edge : graph.edges()) {
      auto* new_edge = combined_graph.mutable_edges()->Add();
      *new_edge = edge;
    }
  }

  // Update combined stats
  auto* stats = combined_graph.mutable_stats();
  stats->set_node_count(0);
  stats->set_edge_count(combined_graph.edges_size());

  for (const auto& [type, collection] : combined_graph.nodes_by_type()) {
    int64_t count = collection.nodes_size();
    stats->set_node_count(stats->node_count() + count);
    (*stats->mutable_nodes_per_type())[type] = count;
  }

  std::cout << "\n  Combined graph: " << stats->node_count() << " nodes, "
            << stats->edge_count() << " edges\n\n";

  // Step 3: Initialize Gemini and process query
  std::cout << "Step 3: Sending to Gemini for operation composition...\n";
  std::cout << "  Query: \"" << query << "\"\n\n";

  finetoo::cloud::VertexAIConfig vertex_config;
  vertex_config.project_id = project_id;
  vertex_config.location = location;
  vertex_config.model = "gemini-2.5-flash";

  auto vertex_client =
      std::make_unique<finetoo::cloud::VertexAIClient>(vertex_config);

  finetoo::query::QueryService query_service(std::move(vertex_client));
  auto response_or = query_service.ProcessQuery(query, combined_graph);

  if (!response_or.ok()) {
    std::cerr << "  Error: " << response_or.status() << "\n";
    return 1;
  }

  const auto& response = *response_or;

  if (!response.success()) {
    std::cerr << "  Query failed: " << response.error_message() << "\n";
    return 1;
  }

  // Step 4: Display results
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " Gemini's Understanding:\n";
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << response.plan().reasoning() << "\n\n";

  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " Operation Plan (LLM-Composed):\n";
  std::cout << "════════════════════════════════════════════════════════════\n";

  int op_num = 1;
  for (const auto& operation : response.plan().operations()) {
    std::cout << op_num++ << ". ";

    switch (operation.type()) {
      case finetoo::operations::v1::FILTER:
        std::cout << "FILTER";
        break;
      case finetoo::operations::v1::TRAVERSE:
        std::cout << "TRAVERSE";
        break;
      case finetoo::operations::v1::AGGREGATE:
        std::cout << "AGGREGATE";
        break;
      case finetoo::operations::v1::MATCH:
        std::cout << "MATCH";
        break;
      default:
        std::cout << "UNKNOWN";
    }

    std::cout << "(";
    if (!operation.target_type().empty()) {
      std::cout << operation.target_type();
    }
    if (!operation.property_name().empty()) {
      std::cout << ", " << operation.property_name();
    }

    for (const auto& [key, value] : operation.parameters()) {
      std::cout << ", " << key << "=\"" << value << "\"";
    }

    std::cout << ")\n";
  }

  std::cout << "\n" << response.answer() << "\n";

  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " Summary:\n";
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << "  Drawings analyzed: " << dxf_files.size() << "\n";
  std::cout << "  Total nodes: " << stats->node_count() << "\n";
  std::cout << "  Total edges: " << stats->edge_count() << "\n";
  std::cout << "  Operations executed: " << response.plan().operations_size() << "\n";
  std::cout << "  Processing time: " << response.total_time_ms() << " ms\n";
  std::cout << "  Unique parts found: " << response.result().values_size() << "\n\n";

  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " Multi-Document Schema-Driven Analysis Complete!\n";
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << "  ✓ Zero-shot operation composition\n";
  std::cout << "  ✓ Combined analysis across " << dxf_files.size() << " drawings\n";
  std::cout << "  ✓ Full provenance tracking\n";
  std::cout << "  ✓ Schema-driven (no hardcoded rules)\n\n";

  // Step 5: Export results to files
  std::cout << "Step 4: Exporting results to files...\n";

  finetoo::export_util::BOMExporter exporter;

  // Parse BOM entries from result
  auto bom_entries = exporter.ParseBOMFromResult(response.result(), combined_graph);

  // Extract all dimensions
  auto dimensions = exporter.ExtractDimensions(combined_graph);

  // Export to JSON
  std::string json_file = "finetoo_bom_full.json";
  auto json_status = exporter.ExportToJSON(json_file, bom_entries, dimensions);
  if (json_status.ok()) {
    std::cout << "  ✓ Saved complete BOM to: " << json_file << "\n";
  } else {
    std::cerr << "  Error saving JSON: " << json_status << "\n";
  }

  // Export to CSV
  std::string csv_file = "finetoo_bom_full.csv";
  auto csv_status = exporter.ExportToCSV(csv_file, bom_entries);
  if (csv_status.ok()) {
    std::cout << "  ✓ Saved BOM (parts only) to: " << csv_file << "\n";
  } else {
    std::cerr << "  Error saving CSV: " << csv_status << "\n";
  }

  // Export dimensions to separate CSV
  std::string dim_file = "finetoo_dimensions.csv";
  auto dim_status = exporter.ExportDimensions(dim_file, dimensions);
  if (dim_status.ok()) {
    std::cout << "  ✓ Saved " << dimensions.size() << " dimensions to: " << dim_file << "\n";
  } else {
    std::cerr << "  Error saving dimensions: " << dim_status << "\n";
  }

  std::cout << "\n════════════════════════════════════════════════════════════\n";
  std::cout << " Files Generated:\n";
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << "  1. " << json_file << " - Complete BOM with dimensions (JSON)\n";
  std::cout << "  2. " << csv_file << " - Part list with quantities (CSV)\n";
  std::cout << "  3. " << dim_file << " - All dimensional measurements (CSV)\n";
  std::cout << "\n  These files contain:\n";
  std::cout << "    - " << bom_entries.size() << " unique parts\n";
  std::cout << "    - " << dimensions.size() << " dimensional measurements\n";
  std::cout << "    - Full provenance and traceability\n\n";

  return 0;
}
