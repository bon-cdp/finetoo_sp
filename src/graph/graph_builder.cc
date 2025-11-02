// Copyright 2025 Finetoo
// Graph Builder Implementation

#include "src/graph/graph_builder.h"

#include <string>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "src/schema/schema_analyzer.h"

namespace finetoo::graph {

GraphBuilder::GraphBuilder()
    : arena_(std::make_unique<google::protobuf::Arena>()) {}

GraphBuilder::~GraphBuilder() = default;

absl::StatusOr<finetoo::graph::v1::PropertyGraph> GraphBuilder::Build(
    const parser::DXFFile& dxf_file) {
  finetoo::graph::v1::PropertyGraph graph;

  // Create schema with operational metadata
  *graph.mutable_schema() = CreateSchema(dxf_file);

  // Add metadata
  (*graph.mutable_metadata())["dxf_version"] = dxf_file.version;
  (*graph.mutable_metadata())["entity_count"] = std::to_string(dxf_file.entities.size());
  (*graph.mutable_metadata())["block_count"] = std::to_string(dxf_file.blocks.size());

  // Add entities to graph as nodes
  for (const auto& entity : dxf_file.entities) {
    AddEntity(entity, &graph);
  }

  // Add blocks to graph as nodes
  for (const auto& block : dxf_file.blocks) {
    AddBlock(block, &graph);
  }

  // Build REFERENCES edges: INSERT entities → Block nodes
  // For INSERT entities, group code 2 contains the block name
  for (const auto& entity : dxf_file.entities) {
    if (entity.type == "INSERT") {
      // Get block name from group code 2
      auto block_name_or = entity.GetString(2);
      if (block_name_or.ok() && !block_name_or->empty()) {
        const std::string& block_name = *block_name_or;

        auto& edges = *graph.mutable_edges();
        auto* edge = edges.Add();

        edge->set_id("edge_" + entity.handle + "_ref_" + block_name);
        edge->set_type("REFERENCES");
        edge->set_source_node_id(entity.handle);
        edge->set_target_node_id("block_" + block_name);

        (*edge->mutable_properties())["block_name"] = block_name;
      }
    }
  }

  // Compute statistics
  auto* stats = graph.mutable_stats();
  stats->set_node_count(0);
  stats->set_edge_count(graph.edges_size());

  for (const auto& [type, collection] : graph.nodes_by_type()) {
    int64_t count = collection.nodes_size();
    stats->set_node_count(stats->node_count() + count);
    (*stats->mutable_nodes_per_type())[type] = count;
  }

  (*stats->mutable_edges_per_type())["REFERENCES"] = graph.edges_size();

  return graph;
}

absl::StatusOr<finetoo::graph::v1::PropertyGraph> GraphBuilder::BuildFromFile(
    absl::string_view file_path) {
  parser::DXFTextParser parser;
  auto dxf_or = parser.Parse(file_path);
  if (!dxf_or.ok()) return dxf_or.status();

  return Build(*dxf_or);
}

finetoo::graph::v1::Schema GraphBuilder::CreateSchema(
    const parser::DXFFile& dxf_file) {
  // Use SchemaAnalyzer to create DXF schema
  auto schema_or = schema::SchemaAnalyzer::CreateDXFSchema(dxf_file.version);
  if (schema_or.ok()) {
    return *schema_or;
  }
  return finetoo::graph::v1::Schema();  // Fallback
}

absl::string_view GraphBuilder::InternString(absl::string_view str) {
  auto [it, inserted] = string_pool_.insert({std::string(str), str});
  return it->second;
}

void GraphBuilder::AddEntity(const parser::DXFEntity& entity,
                              finetoo::graph::v1::PropertyGraph* graph) {
  // Get or create Entity node collection
  auto& entity_collection = (*graph->mutable_nodes_by_type())["Entity"];
  auto* node = entity_collection.mutable_nodes()->Add();

  // Set node ID and type
  node->set_id(entity.handle);
  node->set_type("Entity");

  // Add basic properties
  (*node->mutable_string_props())["handle"] = InternString(entity.handle);
  (*node->mutable_string_props())["type"] = InternString(entity.type);
  (*node->mutable_string_props())["layer"] = InternString(entity.layer);

  // Store all DXF group codes as properties
  // This is generic - operations will extract semantics later
  for (const auto& pair : entity.data) {
    std::string prop_key = "gc_" + std::to_string(pair.group_code);

    // Try to parse as double for numeric group codes
    if (pair.group_code >= 10 && pair.group_code <= 59) {
      try {
        double numeric_value = std::stod(pair.value);
        (*node->mutable_numeric_props())[prop_key] = numeric_value;
      } catch (...) {
        (*node->mutable_string_props())[prop_key] = InternString(pair.value);
      }
    } else {
      // String property
      (*node->mutable_string_props())[prop_key] = InternString(pair.value);
    }
  }

  // Store reference to node for lookups
  nodes_by_handle_[entity.handle] = node;

  // Update collection count
  entity_collection.set_count(entity_collection.nodes_size());
}

void GraphBuilder::AddBlock(const parser::DXFBlock& block,
                             finetoo::graph::v1::PropertyGraph* graph) {
  // Get or create Block node collection
  auto& block_collection = (*graph->mutable_nodes_by_type())["Block"];
  auto* node = block_collection.mutable_nodes()->Add();

  // Set node ID and type
  node->set_id("block_" + block.name);
  node->set_type("Block");

  // Add basic properties
  (*node->mutable_string_props())["name"] = InternString(block.name);
  (*node->mutable_string_props())["handle"] = InternString(block.handle);

  // Add entity count - this is computed, not from DXF
  (*node->mutable_int_props())["entity_count"] = block.entities.size();

  // TODO: Compute content_hash for divergence detection
  // For now, use a placeholder
  (*node->mutable_string_props())["content_hash"] = "HASH_PLACEHOLDER";

  // Store reference to node for lookups
  nodes_by_handle_["block_" + block.name] = node;

  // Update collection count
  block_collection.set_count(block_collection.nodes_size());
}

}  // namespace finetoo::graph
