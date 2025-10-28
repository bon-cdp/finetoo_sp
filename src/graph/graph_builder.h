// Copyright 2025 Finetoo
// Graph Builder - Converts DXF to Property Graph with Arena Allocation

#pragma once

#include <memory>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "google/protobuf/arena.h"
#include "proto/graph.pb.h"
#include "src/parser/dxf_text_parser.h"

namespace finetoo::graph {

// GraphBuilder converts DXF files to property graphs with operational metadata
// Uses Protocol Buffer arena allocation for memory efficiency
class GraphBuilder {
 public:
  GraphBuilder();
  ~GraphBuilder();

  // Non-copyable, movable
  GraphBuilder(const GraphBuilder&) = delete;
  GraphBuilder& operator=(const GraphBuilder&) = delete;
  GraphBuilder(GraphBuilder&&) = default;
  GraphBuilder& operator=(GraphBuilder&&) = default;

  // Build property graph from parsed DXF file
  absl::StatusOr<finetoo::graph::v1::PropertyGraph> Build(
      const parser::DXFFile& dxf_file);

  // Build property graph directly from DXF file path
  absl::StatusOr<finetoo::graph::v1::PropertyGraph> BuildFromFile(
      absl::string_view file_path);

 private:
  // Arena for efficient protobuf allocation
  std::unique_ptr<google::protobuf::Arena> arena_;

  // String interning for deduplication
  absl::flat_hash_map<std::string, absl::string_view> string_pool_;

  // Node lookup by handle
  absl::flat_hash_map<std::string, finetoo::graph::v1::Node*> nodes_by_handle_;

  // Add entity to graph
  void AddEntity(const parser::DXFEntity& entity,
                 finetoo::graph::v1::PropertyGraph* graph);

  // Add block to graph
  void AddBlock(const parser::DXFBlock& block,
                finetoo::graph::v1::PropertyGraph* graph);

  // Intern string (deduplicate)
  absl::string_view InternString(absl::string_view str);

  // Create schema for DXF graph
  finetoo::graph::v1::Schema CreateSchema(const parser::DXFFile& dxf_file);
};

}  // namespace finetoo::graph
