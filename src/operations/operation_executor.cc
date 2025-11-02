// Copyright 2025 Finetoo
// Operation Executor Implementation (Skeleton)

#include "src/operations/operation_executor.h"

#include "absl/status/status.h"

namespace finetoo::operations {

OperationExecutor::OperationExecutor(finetoo::graph::v1::PropertyGraph* graph)
    : graph_(graph) {}

absl::StatusOr<finetoo::operations::v1::OperationResult>
OperationExecutor::Execute(const finetoo::operations::v1::Operation& operation) {
  switch (operation.type()) {
    case finetoo::operations::v1::MATCH:
      return Match(operation);
    case finetoo::operations::v1::FILTER:
      return Filter(operation);
    case finetoo::operations::v1::COMPARE:
      return Compare(operation);
    case finetoo::operations::v1::TRAVERSE:
      return Traverse(operation);
    case finetoo::operations::v1::AGGREGATE:
      return Aggregate(operation);
    case finetoo::operations::v1::GROUP_BY:
      return GroupBy(operation);
    case finetoo::operations::v1::PROJECT:
      return Project(operation);
    case finetoo::operations::v1::JOIN:
      return Join(operation);
    default:
      return absl::InvalidArgumentError("Unknown operation type");
  }
}

absl::StatusOr<finetoo::operations::v1::OperationResult>
OperationExecutor::ExecutePlan(
    const finetoo::operations::v1::OperationPlan& plan) {
  // TODO: Execute operations in sequence
  // TODO: Pass results from one operation to the next
  // TODO: Return final result with full provenance

  return absl::UnimplementedError("ExecutePlan not yet implemented");
}

// Operation implementations (skeletons)

absl::StatusOr<finetoo::operations::v1::OperationResult>
OperationExecutor::Match(const finetoo::operations::v1::Operation& op) {
  finetoo::operations::v1::OperationResult result;

  // Extract parameters
  const std::string& target_type = op.target_type();
  const std::string& property_name = op.property_name();

  auto it_value = op.parameters().find("value");
  if (it_value == op.parameters().end()) {
    return absl::InvalidArgumentError("Match operation requires 'value' parameter");
  }

  const std::string& value = it_value->second;

  // Get nodes of target type
  const auto& nodes_by_type = graph_->nodes_by_type();
  auto type_it = nodes_by_type.find(target_type);

  if (type_it == nodes_by_type.end()) {
    return result;  // No nodes of this type
  }

  // Find matching node
  for (const auto& node : type_it->second.nodes()) {
    // Check string properties
    auto str_it = node.string_props().find(property_name);
    if (str_it != node.string_props().end() && str_it->second == value) {
      result.add_node_ids(node.id());
      result.add_provenance(node.id());
      (*result.mutable_values())[property_name] = value;
      result.set_nodes_processed(1);
      return result;  // Return first match for unique property
    }
  }

  result.set_nodes_processed(type_it->second.nodes_size());
  return result;
}

absl::StatusOr<finetoo::operations::v1::OperationResult>
OperationExecutor::Filter(const finetoo::operations::v1::Operation& op) {
  finetoo::operations::v1::OperationResult result;

  // Extract parameters
  const std::string& target_type = op.target_type();
  const std::string& property_name = op.property_name();

  auto it_operator = op.parameters().find("operator");
  auto it_value = op.parameters().find("value");

  if (it_value == op.parameters().end()) {
    return absl::InvalidArgumentError("Filter operation requires 'value' parameter");
  }

  const std::string& value = it_value->second;
  const std::string op_str = (it_operator != op.parameters().end()) ? it_operator->second : "EQUALS";

  // Get nodes of target type
  const auto& nodes_by_type = graph_->nodes_by_type();
  auto type_it = nodes_by_type.find(target_type);

  if (type_it == nodes_by_type.end()) {
    return result;  // No nodes of this type
  }

  // Filter nodes
  int64_t processed = 0;
  for (const auto& node : type_it->second.nodes()) {
    processed++;

    bool matches = false;

    // Check string properties
    auto str_it = node.string_props().find(property_name);
    if (str_it != node.string_props().end()) {
      if (op_str == "EQUALS") {
        matches = (str_it->second == value);
      } else if (op_str == "CONTAINS") {
        matches = (str_it->second.find(value) != std::string::npos);
      }
    }

    // Check numeric properties
    auto num_it = node.numeric_props().find(property_name);
    if (num_it != node.numeric_props().end()) {
      try {
        double target_value = std::stod(value);
        if (op_str == "EQUALS") {
          matches = (num_it->second == target_value);
        } else if (op_str == "GREATER_THAN") {
          matches = (num_it->second > target_value);
        } else if (op_str == "LESS_THAN") {
          matches = (num_it->second < target_value);
        }
      } catch (...) {
        // Ignore parse errors
      }
    }

    if (matches) {
      result.add_node_ids(node.id());
      result.add_provenance(node.id());
    }
  }

  result.set_nodes_processed(processed);
  return result;
}

absl::StatusOr<finetoo::operations::v1::OperationResult>
OperationExecutor::Compare(const finetoo::operations::v1::Operation& op) {
  // TODO: Compare property values between entities
  return absl::UnimplementedError("Compare operation not yet implemented");
}

absl::StatusOr<finetoo::operations::v1::OperationResult>
OperationExecutor::Traverse(const finetoo::operations::v1::Operation& op) {
  finetoo::operations::v1::OperationResult result;

  // Extract parameters
  auto it_edge_type = op.parameters().find("edge_type");
  auto it_start_nodes = op.parameters().find("start_node_ids");

  if (it_edge_type == op.parameters().end()) {
    return absl::InvalidArgumentError("Traverse operation requires 'edge_type' parameter");
  }

  const std::string& edge_type = it_edge_type->second;

  // Get start nodes from previous operation result or filter
  std::vector<std::string> start_nodes;
  if (it_start_nodes != op.parameters().end()) {
    // Parse comma-separated node IDs
    const std::string& node_ids_str = it_start_nodes->second;
    size_t start = 0;
    size_t end = node_ids_str.find(',');
    while (end != std::string::npos) {
      start_nodes.push_back(node_ids_str.substr(start, end - start));
      start = end + 1;
      end = node_ids_str.find(',', start);
    }
    start_nodes.push_back(node_ids_str.substr(start));
  }

  // Traverse edges
  int64_t processed = 0;
  for (const auto& edge : graph_->edges()) {
    if (edge.type() == edge_type) {
      processed++;

      // Check if this edge starts from one of our start nodes
      bool should_traverse = start_nodes.empty();
      if (!should_traverse) {
        for (const auto& start_id : start_nodes) {
          if (edge.source_node_id() == start_id) {
            should_traverse = true;
            break;
          }
        }
      }

      if (should_traverse) {
        result.add_node_ids(edge.target_node_id());
        result.add_provenance(edge.source_node_id() + " -> " + edge.target_node_id());

        // Add edge properties to values
        for (const auto& [key, value] : edge.properties()) {
          (*result.mutable_values())[edge.target_node_id() + "." + key] = value;
        }
      }
    }
  }

  result.set_nodes_processed(processed);
  return result;
}

absl::StatusOr<finetoo::operations::v1::OperationResult>
OperationExecutor::Aggregate(const finetoo::operations::v1::Operation& op) {
  finetoo::operations::v1::OperationResult result;

  // Extract parameters
  auto it_function = op.parameters().find("function");
  auto it_group_by = op.parameters().find("group_by");
  auto it_node_ids = op.parameters().find("node_ids");

  if (it_function == op.parameters().end()) {
    return absl::InvalidArgumentError("Aggregate operation requires 'function' parameter");
  }

  const std::string& function = it_function->second;
  const std::string& target_type = op.target_type();
  const std::string& property_name = op.property_name();

  // Get nodes to aggregate
  std::vector<const finetoo::graph::v1::Node*> nodes_to_aggregate;

  // If node_ids specified, use those
  if (it_node_ids != op.parameters().end()) {
    // Parse comma-separated node IDs and find nodes
    const std::string& node_ids_str = it_node_ids->second;
    // For simplicity, aggregate all nodes of target type
  }

  // Get all nodes of target type
  const auto& nodes_by_type = graph_->nodes_by_type();
  auto type_it = nodes_by_type.find(target_type);
  if (type_it == nodes_by_type.end()) {
    return result;
  }

  // Group by if specified
  if (it_group_by != op.parameters().end()) {
    const std::string& group_by_prop = it_group_by->second;
    std::map<std::string, int64_t> counts;

    for (const auto& node : type_it->second.nodes()) {
      // Get grouping key
      std::string group_key = "unknown";

      auto str_it = node.string_props().find(group_by_prop);
      if (str_it != node.string_props().end()) {
        group_key = str_it->second;
      }

      counts[group_key]++;
      result.add_provenance(node.id());
    }

    // Add results
    for (const auto& [key, count] : counts) {
      (*result.mutable_values())[key] = std::to_string(count);
    }

    result.set_nodes_processed(type_it->second.nodes_size());
    return result;
  }

  // Simple aggregation without grouping
  if (function == "COUNT") {
    int64_t count = type_it->second.nodes_size();
    (*result.mutable_values())["count"] = std::to_string(count);
    result.set_nodes_processed(count);
  } else if (function == "SUM" || function == "AVG") {
    double sum = 0.0;
    int64_t count = 0;

    for (const auto& node : type_it->second.nodes()) {
      auto num_it = node.numeric_props().find(property_name);
      if (num_it != node.numeric_props().end()) {
        sum += num_it->second;
        count++;
      }
    }

    if (function == "SUM") {
      (*result.mutable_values())["sum"] = std::to_string(sum);
    } else {
      double avg = (count > 0) ? (sum / count) : 0.0;
      (*result.mutable_values())["avg"] = std::to_string(avg);
    }

    result.set_nodes_processed(count);
  }

  return result;
}

absl::StatusOr<finetoo::operations::v1::OperationResult>
OperationExecutor::GroupBy(const finetoo::operations::v1::Operation& op) {
  // TODO: Group entities by property
  return absl::UnimplementedError("GroupBy operation not yet implemented");
}

absl::StatusOr<finetoo::operations::v1::OperationResult>
OperationExecutor::Project(const finetoo::operations::v1::Operation& op) {
  // TODO: Extract specific properties
  return absl::UnimplementedError("Project operation not yet implemented");
}

absl::StatusOr<finetoo::operations::v1::OperationResult>
OperationExecutor::Join(const finetoo::operations::v1::Operation& op) {
  // TODO: Join results by relationship
  return absl::UnimplementedError("Join operation not yet implemented");
}

}  // namespace finetoo::operations
