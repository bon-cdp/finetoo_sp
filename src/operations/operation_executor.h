// Copyright 2025 Finetoo
// Operation Executor - Execute generic operations on property graphs

#pragma once

#include "absl/status/statusor.h"
#include "proto/graph.pb.h"
#include "proto/operations.pb.h"

namespace finetoo::operations {

// OperationExecutor executes generic operations discovered from schema
class OperationExecutor {
 public:
  explicit OperationExecutor(finetoo::graph::v1::PropertyGraph* graph);

  // Execute a single operation
  absl::StatusOr<finetoo::operations::v1::OperationResult> Execute(
      const finetoo::operations::v1::Operation& operation);

  // Execute an operation plan (sequence of operations)
  absl::StatusOr<finetoo::operations::v1::OperationResult> ExecutePlan(
      const finetoo::operations::v1::OperationPlan& plan);

 private:
  finetoo::graph::v1::PropertyGraph* graph_;

  // 8 Generic Operation Primitives:

  // 1. Match - Find entities by unique property
  absl::StatusOr<finetoo::operations::v1::OperationResult> Match(
      const finetoo::operations::v1::Operation& op);

  // 2. Filter - Select entities by criteria
  absl::StatusOr<finetoo::operations::v1::OperationResult> Filter(
      const finetoo::operations::v1::Operation& op);

  // 3. Compare - Compare property values
  absl::StatusOr<finetoo::operations::v1::OperationResult> Compare(
      const finetoo::operations::v1::Operation& op);

  // 4. Traverse - Follow edges/relationships
  absl::StatusOr<finetoo::operations::v1::OperationResult> Traverse(
      const finetoo::operations::v1::Operation& op);

  // 5. Aggregate - Compute aggregate values
  absl::StatusOr<finetoo::operations::v1::OperationResult> Aggregate(
      const finetoo::operations::v1::Operation& op);

  // 6. GroupBy - Group entities by property
  absl::StatusOr<finetoo::operations::v1::OperationResult> GroupBy(
      const finetoo::operations::v1::Operation& op);

  // 7. Project - Extract specific properties
  absl::StatusOr<finetoo::operations::v1::OperationResult> Project(
      const finetoo::operations::v1::Operation& op);

  // 8. Join - Combine results by relationship
  absl::StatusOr<finetoo::operations::v1::OperationResult> Join(
      const finetoo::operations::v1::Operation& op);
};

}  // namespace finetoo::operations
