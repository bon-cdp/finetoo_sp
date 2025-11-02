// Copyright 2025 Finetoo
// Query Service - Natural Language to BOM via LLM Operation Composition

#pragma once

#include <memory>
#include <string>

#include "absl/status/statusor.h"
#include "proto/graph.pb.h"
#include "proto/operations.pb.h"
#include "src/cloud/vertex_ai_client.h"

namespace finetoo::query {

// Query Service orchestrates natural language queries
// Schema → Prompt → LLM → Operations → Results
class QueryService {
 public:
  explicit QueryService(std::unique_ptr<cloud::VertexAIClient> vertex_client);

  // Process natural language query and return BOM
  absl::StatusOr<finetoo::operations::v1::QueryResponse> ProcessQuery(
      const std::string& query, const finetoo::graph::v1::PropertyGraph& graph);

 private:
  std::unique_ptr<cloud::VertexAIClient> vertex_client_;

  // Generate prompt from schema and query
  std::string BuildPrompt(const std::string& query,
                          const finetoo::graph::v1::Schema& schema);

  // Parse LLM JSON response → OperationPlan
  absl::StatusOr<finetoo::operations::v1::OperationPlan> ParseOperationPlan(
      const std::string& llm_response);

  // Format operation results as BOM
  std::string FormatBOM(const finetoo::operations::v1::OperationResult& result,
                        const finetoo::graph::v1::PropertyGraph& graph);
};

}  // namespace finetoo::query
