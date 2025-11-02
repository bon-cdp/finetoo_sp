// Copyright 2025 Finetoo
// BOM Exporter - Generate BOM files with dimensional data

#pragma once

#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "proto/graph.pb.h"
#include "proto/operations.pb.h"

namespace finetoo::export_util {

// Part entry in BOM
struct BOMEntry {
  std::string part_name;
  int quantity;
  std::vector<std::string> source_drawings;
  std::map<std::string, std::string> properties;  // Dimensions, materials, etc.
};

// Dimensional measurement from drawing
struct Dimension {
  std::string entity_handle;
  std::string dimension_type;  // LINEAR, ANGULAR, RADIAL, etc.
  double measurement_value;
  std::string text_override;  // What's displayed (may include tolerances)
  std::string layer;
  std::string source_drawing;
};

// BOM Exporter
class BOMExporter {
 public:
  BOMExporter() = default;

  // Export BOM to JSON file
  absl::Status ExportToJSON(
      const std::string& filename,
      const std::vector<BOMEntry>& bom,
      const std::vector<Dimension>& dimensions);

  // Export BOM to CSV file
  absl::Status ExportToCSV(
      const std::string& filename,
      const std::vector<BOMEntry>& bom);

  // Export dimensional analysis to separate file
  absl::Status ExportDimensions(
      const std::string& filename,
      const std::vector<Dimension>& dimensions);

  // Parse operation result into BOM entries
  static std::vector<BOMEntry> ParseBOMFromResult(
      const finetoo::operations::v1::OperationResult& result,
      const finetoo::graph::v1::PropertyGraph& graph);

  // Extract all dimensions from property graph
  static std::vector<Dimension> ExtractDimensions(
      const finetoo::graph::v1::PropertyGraph& graph);
};

}  // namespace finetoo::export_util
