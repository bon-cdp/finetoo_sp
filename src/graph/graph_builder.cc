// Copyright 2025 Finetoo
// Graph Builder Implementation (Skeleton)

#include "src/graph/graph_builder.h"

#include "absl/status/status.h"
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

  // TODO: Add entities to graph
  // TODO: Add blocks to graph
  // TODO: Build edges (BELONGS_TO, CONTAINS, REFERENCES)
  // TODO: Compute statistics

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

}  // namespace finetoo::graph
