// Copyright 2025 Finetoo
// Block Analyzer - Divergence Detection and Comparison

#pragma once

#include <vector>

#include "absl/status/statusor.h"
#include "proto/graph.pb.h"

namespace finetoo::analysis {

// BlockAnalyzer detects and analyzes block divergence across drawings
class BlockAnalyzer {
 public:
  BlockAnalyzer() = default;

  // Parse multiple drawings and detect divergence
  absl::StatusOr<finetoo::graph::v1::BlockDivergenceReport> AnalyzeDrawings(
      const std::vector<std::string>& file_paths);

  // Compute SHA-256 hash of block content
  std::string ComputeBlockHash(const finetoo::graph::v1::Node& block_node);

  // Compare two versions of a block in detail
  absl::StatusOr<finetoo::graph::v1::BlockComparison> CompareBlockVersions(
      const std::string& block_name,
      const std::vector<finetoo::graph::v1::PropertyGraph*>& graphs);

  // Find all blocks shared across multiple drawings
  std::vector<std::string> FindSharedBlocks(
      const std::vector<finetoo::graph::v1::PropertyGraph>& graphs);

 private:
  // Helper: Extract all blocks from a graph
  std::vector<const finetoo::graph::v1::Node*> ExtractBlocks(
      const finetoo::graph::v1::PropertyGraph& graph);
};

}  // namespace finetoo::analysis
