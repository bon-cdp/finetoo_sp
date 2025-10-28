// Copyright 2025 Finetoo
// Schema Analysis and Operational Metadata Extraction

#pragma once

#include <memory>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "proto/graph.pb.h"

namespace finetoo::schema {

// SchemaAnalyzer extracts operational metadata from document schemas.
// This is the foundation of the finetoo approach: schemas tell us what
// operations are possible, not hardcoded logic.
class SchemaAnalyzer {
 public:
  SchemaAnalyzer() = default;

  // Non-copyable, movable
  SchemaAnalyzer(const SchemaAnalyzer&) = delete;
  SchemaAnalyzer& operator=(const SchemaAnalyzer&) = delete;
  SchemaAnalyzer(SchemaAnalyzer&&) = default;
  SchemaAnalyzer& operator=(SchemaAnalyzer&&) = default;

  // Create a schema for DXF documents with operational metadata
  // This demonstrates how we encode CAD-specific operations in schema
  static absl::StatusOr<finetoo::graph::v1::Schema> CreateDXFSchema(
      absl::string_view version);

  // Analyze a schema to find unique properties (enable match operations)
  std::vector<std::string> FindUniqueProperties(
      const finetoo::graph::v1::Schema& schema,
      absl::string_view node_type) const;

  // Analyze a schema to find comparable properties (enable compare operations)
  std::vector<std::string> FindComparableProperties(
      const finetoo::graph::v1::Schema& schema,
      absl::string_view node_type) const;

  // Analyze a schema to find indexed properties (enable filter operations)
  std::vector<std::string> FindIndexedProperties(
      const finetoo::graph::v1::Schema& schema,
      absl::string_view node_type) const;

  // Analyze a schema to find aggregable properties (enable aggregate operations)
  std::vector<std::string> FindAggregableProperties(
      const finetoo::graph::v1::Schema& schema,
      absl::string_view node_type) const;

  // Get all edge types that enable traversal operations
  std::vector<std::string> GetTraversableEdgeTypes(
      const finetoo::graph::v1::Schema& schema) const;

  // Validate that a schema is well-formed
  absl::Status ValidateSchema(const finetoo::graph::v1::Schema& schema) const;
};

}  // namespace finetoo::schema
