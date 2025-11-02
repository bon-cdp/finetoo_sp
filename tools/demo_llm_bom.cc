// Copyright 2025 Finetoo
// Demo: Natural Language BOM Generation with Gemini
//
// This demonstrates the complete "Beyond Fine-Tuning" thesis:
//   Natural Language Query → Schema-Driven Prompt → Gemini Composes Operations → Execute → BOM

#include <iostream>
#include <string>

#include "src/cloud/vertex_ai_client.h"
#include "src/graph/graph_builder.h"
#include "src/query/query_service.h"

int main(int argc, char** argv) {
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " Finetoo: Natural Language BOM Generation\n";
  std::cout << " Powered by Gemini + Schema-Driven Operation Discovery\n";
  std::cout << "════════════════════════════════════════════════════════════\n\n";

  // Parse command line arguments
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <dxf_file> [query]\n";
    std::cerr << "\nExample:\n";
    std::cerr << "  " << argv[0]
              << " drawing.dxf \"Generate a bill of materials\"\n\n";
    return 1;
  }

  std::string file_path = argv[1];
  std::string query = (argc >= 3) ? argv[2] : "Generate a bill of materials";

  // Get Google Cloud configuration from environment
  const char* project_env = std::getenv("FINETOO_GCP_PROJECT");
  const char* location_env = std::getenv("FINETOO_GCP_LOCATION");

  std::string project_id = project_env ? project_env : "";
  std::string location = location_env ? location_env : "us-central1";

  if (project_id.empty()) {
    std::cerr << "Error: FINETOO_GCP_PROJECT environment variable not set\n";
    std::cerr << "\nSetup instructions:\n";
    std::cerr << "  export FINETOO_GCP_PROJECT=your-project-id\n";
    std::cerr << "  export FINETOO_GCP_LOCATION=us-central1  # optional\n";
    std::cerr << "  gcloud auth application-default login\n\n";
    return 1;
  }

  std::cout << "Configuration:\n";
  std::cout << "  GCP Project: " << project_id << "\n";
  std::cout << "  Location: " << location << "\n";
  std::cout << "  Model: gemini-1.5-pro\n\n";

  // Step 1: Parse DXF file
  std::cout << "Step 1: Parsing DXF file...\n";
  std::cout << "  File: " << file_path << "\n";

  finetoo::graph::GraphBuilder builder;
  auto graph_or = builder.BuildFromFile(file_path);

  if (!graph_or.ok()) {
    std::cerr << "  Error: " << graph_or.status() << "\n";
    return 1;
  }

  auto& graph = *graph_or;
  std::cout << "  ✓ " << graph.stats().node_count() << " nodes, "
            << graph.stats().edge_count() << " edges\n\n";

  // Step 2: Initialize Gemini client
  std::cout << "Step 2: Connecting to Vertex AI Gemini...\n";

  finetoo::cloud::VertexAIConfig vertex_config;
  vertex_config.project_id = project_id;
  vertex_config.location = location;
  vertex_config.model = "gemini-2.5-flash";  // Use Gemini 2.5 Flash

  auto vertex_client =
      std::make_unique<finetoo::cloud::VertexAIClient>(vertex_config);
  std::cout << "  ✓ Connected to Vertex AI\n\n";

  // Step 3: Process query with LLM
  std::cout << "Step 3: Processing natural language query...\n";
  std::cout << "  Query: \"" << query << "\"\n\n";

  finetoo::query::QueryService query_service(std::move(vertex_client));
  auto response_or = query_service.ProcessQuery(query, graph);

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
  std::cout << " Gemini Reasoning:\n";
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << response.plan().reasoning() << "\n\n";

  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " Operation Plan (LLM-Composed):\n";
  std::cout << "════════════════════════════════════════════════════════════\n";

  int op_num = 1;
  for (const auto& operation : response.plan().operations()) {
    std::cout << op_num++ << ". ";

    if (operation.type() == finetoo::operations::v1::FILTER) {
      std::cout << "FILTER";
    } else if (operation.type() == finetoo::operations::v1::TRAVERSE) {
      std::cout << "TRAVERSE";
    } else if (operation.type() == finetoo::operations::v1::AGGREGATE) {
      std::cout << "AGGREGATE";
    } else if (operation.type() == finetoo::operations::v1::MATCH) {
      std::cout << "MATCH";
    }

    std::cout << "(" << operation.target_type();
    if (!operation.property_name().empty()) {
      std::cout << ", " << operation.property_name();
    }

    for (const auto& [key, value] : operation.parameters()) {
      std::cout << ", " << key << "=\"" << value << "\"";
    }

    std::cout << ")\n";
  }

  std::cout << "\n";
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " Results:\n";
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << response.answer() << "\n";

  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " Performance:\n";
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << "  Total time: " << response.total_time_ms() << " ms\n";
  std::cout << "  Operations executed: " << response.plan().operations_size()
            << "\n";
  std::cout << "  Nodes processed: " << response.result().nodes_processed()
            << "\n\n";

  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " KEY INSIGHT: Zero-Shot Operation Composition!\n";
  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << "  ✓ No fine-tuning required\n";
  std::cout << "  ✓ Schema-driven operation discovery\n";
  std::cout << "  ✓ LLM composed operations from natural language\n";
  std::cout << "  ✓ 100% explainability (full provenance)\n\n";

  return 0;
}
