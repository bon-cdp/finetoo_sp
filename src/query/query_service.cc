// Copyright 2025 Finetoo
// Query Service Implementation

#include "src/query/query_service.h"

#include <chrono>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "nlohmann/json.hpp"
#include "src/operations/operation_executor.h"

namespace finetoo::query {

QueryService::QueryService(std::unique_ptr<cloud::VertexAIClient> vertex_client)
    : vertex_client_(std::move(vertex_client)) {}

std::string QueryService::BuildPrompt(
    const std::string& query, const finetoo::graph::v1::Schema& schema) {
  // This is THE KEY: Schema-driven operation discovery prompt
  std::string prompt = R"(You are an operation composer for engineering document analysis using a property graph database.

AVAILABLE SCHEMA:

Node Types:
)";

  // Add node types from schema
  for (const auto& node_type : schema.node_types()) {
    prompt += absl::StrCat("- ", node_type.name(), ": properties [");
    for (const auto& prop : node_type.properties()) {
      prompt += prop.name();
      if (prop.unique()) prompt += " (unique)";
      if (prop.indexed()) prompt += " (indexed)";
      if (prop.comparable()) prompt += " (comparable)";
      if (prop.aggregable()) prompt += " (aggregable)";
      prompt += ", ";
    }
    prompt += "]\n";
  }

  prompt += "\nEdge Types:\n";
  for (const auto& edge_type : schema.edge_types()) {
    prompt += absl::StrCat("- ", edge_type.name(), ": ", edge_type.source_type(),
                           " → ", edge_type.target_type(), "\n");
  }

  prompt += R"(
AVAILABLE OPERATIONS (discovered from schema metadata):

1. FILTER - Select nodes by criteria
   {
     "type": "FILTER",
     "target_type": "Entity",
     "property_name": "type",
     "parameters": {"operator": "EQUALS", "value": "INSERT"}
   }
   Example: Find all INSERT entities (each INSERT is a part instance in the drawing)

2. TRAVERSE - Follow edges to find connected nodes
   {
     "type": "TRAVERSE",
     "parameters": {"edge_type": "REFERENCES", "start_node_ids": "comma-separated-ids"}
   }
   Example: Follow REFERENCES edges from INSERT entities to find which Block each references

3. AGGREGATE - Count/sum/group nodes
   {
     "type": "AGGREGATE",
     "target_type": "Entity",
     "property_name": "gc_2",
     "parameters": {"function": "COUNT", "group_by": "gc_2"}
   }
   Example: Count INSERT entities grouped by gc_2 (which contains the block name they reference)

IMPORTANT FOR BOM GENERATION:
- Each INSERT entity represents ONE instance of a part in the drawing
- The block name is stored in the INSERT entity's property "gc_2" (group code 2 in DXF)
- To get part quantities, you must AGGREGATE the INSERT entities (instances), NOT the Block definitions
- Block definitions are just templates - there's only 1 of each
- INSERT instances can appear many times - these are the actual parts used

USER QUERY: ")";

  prompt += query;

  prompt += R"(

Compose an operation plan to answer this query. Return ONLY valid JSON (no markdown, no code blocks):

{
  "query": "the original query",
  "reasoning": "brief explanation of your approach",
  "operations": [
    {operation objects as shown above}
  ]
}
)";

  return prompt;
}

absl::StatusOr<finetoo::operations::v1::OperationPlan>
QueryService::ParseOperationPlan(const std::string& llm_response) {
  try {
    // Strip markdown code blocks if present
    std::string json_str = llm_response;

    // Remove ```json and ``` markers
    size_t start = json_str.find("```json");
    if (start != std::string::npos) {
      json_str = json_str.substr(start + 7);  // Skip ```json
    } else {
      start = json_str.find("```");
      if (start != std::string::npos) {
        json_str = json_str.substr(start + 3);  // Skip ```
      }
    }

    size_t end = json_str.find("```");
    if (end != std::string::npos) {
      json_str = json_str.substr(0, end);
    }

    // Trim whitespace
    size_t first = json_str.find_first_not_of(" \n\r\t");
    size_t last = json_str.find_last_not_of(" \n\r\t");
    if (first != std::string::npos && last != std::string::npos) {
      json_str = json_str.substr(first, last - first + 1);
    }

    // Parse JSON response
    auto json_response = nlohmann::json::parse(json_str);

    finetoo::operations::v1::OperationPlan plan;
    plan.set_query(json_response["query"].get<std::string>());
    plan.set_reasoning(json_response["reasoning"].get<std::string>());

    // Parse operations
    for (const auto& op_json : json_response["operations"]) {
      auto* op = plan.add_operations();

      std::string type_str = op_json["type"].get<std::string>();
      if (type_str == "FILTER") {
        op->set_type(finetoo::operations::v1::FILTER);
      } else if (type_str == "TRAVERSE") {
        op->set_type(finetoo::operations::v1::TRAVERSE);
      } else if (type_str == "AGGREGATE") {
        op->set_type(finetoo::operations::v1::AGGREGATE);
      } else if (type_str == "MATCH") {
        op->set_type(finetoo::operations::v1::MATCH);
      }

      if (op_json.contains("target_type")) {
        op->set_target_type(op_json["target_type"].get<std::string>());
      }

      if (op_json.contains("property_name")) {
        op->set_property_name(op_json["property_name"].get<std::string>());
      }

      if (op_json.contains("parameters")) {
        for (const auto& [key, value] : op_json["parameters"].items()) {
          (*op->mutable_parameters())[key] = value.get<std::string>();
        }
      }
    }

    return plan;

  } catch (const nlohmann::json::exception& e) {
    return absl::InvalidArgumentError(
        absl::StrCat("Failed to parse operation plan: ", e.what(),
                     "\nLLM Response: ", llm_response));
  }
}

std::string QueryService::FormatBOM(
    const finetoo::operations::v1::OperationResult& result,
    const finetoo::graph::v1::PropertyGraph& graph) {
  std::string output = "\nBill of Materials:\n";
  output += "════════════════════════════════════════════════════════════\n";

  if (result.values().empty()) {
    output += "No results\n";
    return output;
  }

  output += absl::StrCat("Block Name", std::string(30, ' '), "| Quantity\n");
  output += "────────────────────────────────────────────────────────────\n";

  for (const auto& [block_name, count_str] : result.values()) {
    // Format: pad block name to 40 chars
    std::string padded_name = block_name;
    if (padded_name.length() > 40) {
      padded_name = padded_name.substr(0, 37) + "...";
    } else {
      padded_name += std::string(40 - padded_name.length(), ' ');
    }

    output += absl::StrCat(padded_name, "| ", count_str, "\n");
  }

  output += "════════════════════════════════════════════════════════════\n";
  return output;
}

absl::StatusOr<finetoo::operations::v1::QueryResponse>
QueryService::ProcessQuery(const std::string& query,
                            const finetoo::graph::v1::PropertyGraph& graph) {
  auto start_time = std::chrono::steady_clock::now();

  finetoo::operations::v1::QueryResponse response;
  response.set_success(false);

  // Step 1: Build prompt from schema
  std::string prompt = BuildPrompt(query, graph.schema());

  // Step 2: Send to Gemini
  auto llm_response_or = vertex_client_->GenerateContent(prompt);
  if (!llm_response_or.ok()) {
    response.set_error_message(std::string(llm_response_or.status().message()));
    return response;
  }

  const std::string& llm_response = *llm_response_or;

  // Step 3: Parse response → OperationPlan
  auto plan_or = ParseOperationPlan(llm_response);
  if (!plan_or.ok()) {
    response.set_error_message(std::string(plan_or.status().message()));
    return response;
  }

  auto& plan = *plan_or;
  *response.mutable_plan() = plan;

  // Step 4: Execute operations
  operations::OperationExecutor executor(
      const_cast<finetoo::graph::v1::PropertyGraph*>(&graph));

  finetoo::operations::v1::OperationResult final_result;

  for (const auto& operation : plan.operations()) {
    auto result_or = executor.Execute(operation);
    if (!result_or.ok()) {
      response.set_error_message(std::string(result_or.status().message()));
      return response;
    }

    final_result = *result_or;
  }

  *response.mutable_result() = final_result;

  // Step 5: Format BOM
  std::string bom_output = FormatBOM(final_result, graph);
  response.set_answer(bom_output);
  response.set_success(true);

  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                      end_time - start_time)
                      .count();
  response.set_total_time_ms(duration);

  return response;
}

}  // namespace finetoo::query
