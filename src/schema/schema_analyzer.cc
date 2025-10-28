// Copyright 2025 Finetoo
// Schema Analysis Implementation

#include "src/schema/schema_analyzer.h"

#include "absl/status/status.h"
#include "absl/strings/str_format.h"

namespace finetoo::schema {

using ::finetoo::graph::v1::PropertyMetadata;
using ::finetoo::graph::v1::Schema;

absl::StatusOr<Schema> SchemaAnalyzer::CreateDXFSchema(
    absl::string_view version) {
  Schema schema;
  schema.set_source_format("DXF");
  schema.set_format_version(std::string(version));
  schema.set_schema_version("1.0.0");

  // Entity NodeType (represents LINE, CIRCLE, POLYLINE, etc.)
  auto* entity_type = schema.add_node_types();
  entity_type->set_name("Entity");

  // handle property: UNIQUE (enables match operations across document versions)
  auto* handle_prop = entity_type->add_properties();
  handle_prop->set_name("handle");
  handle_prop->set_type(PropertyMetadata::STRING);
  handle_prop->set_unique(true);      // KEY: Enables match operations!
  handle_prop->set_indexed(true);     // Also enables fast lookups

  // type property: INDEXED (enables filter by entity type)
  auto* type_prop = entity_type->add_properties();
  type_prop->set_name("type");
  type_prop->set_type(PropertyMetadata::STRING);
  type_prop->set_indexed(true);       // Enables: filter(type == "LINE")

  // layer property: INDEXED (enables filter by layer)
  auto* layer_prop = entity_type->add_properties();
  layer_prop->set_name("layer");
  layer_prop->set_type(PropertyMetadata::STRING);
  layer_prop->set_indexed(true);      // Enables: filter(layer == "EMS_REV")

  // x, y coordinates: COMPARABLE and AGGREGABLE
  auto* x_prop = entity_type->add_properties();
  x_prop->set_name("x");
  x_prop->set_type(PropertyMetadata::DOUBLE);
  x_prop->set_comparable(true);       // Enables: compare(v1.x, v2.x)
  x_prop->set_aggregable(true);       // Enables: avg(x), min(x), max(x)

  auto* y_prop = entity_type->add_properties();
  y_prop->set_name("y");
  y_prop->set_type(PropertyMetadata::DOUBLE);
  y_prop->set_comparable(true);
  y_prop->set_aggregable(true);

  // Block NodeType (represents block definitions)
  auto* block_type = schema.add_node_types();
  block_type->set_name("Block");

  auto* block_name_prop = block_type->add_properties();
  block_name_prop->set_name("name");
  block_name_prop->set_type(PropertyMetadata::STRING);
  block_name_prop->set_unique(true);
  block_name_prop->set_indexed(true);

  auto* block_hash_prop = block_type->add_properties();
  block_hash_prop->set_name("content_hash");
  block_hash_prop->set_type(PropertyMetadata::STRING);
  block_hash_prop->set_comparable(true);  // Enables divergence detection!

  // Layer NodeType
  auto* layer_type = schema.add_node_types();
  layer_type->set_name("Layer");

  auto* layer_name_prop = layer_type->add_properties();
  layer_name_prop->set_name("name");
  layer_name_prop->set_type(PropertyMetadata::STRING);
  layer_name_prop->set_unique(true);
  layer_name_prop->set_indexed(true);

  // EdgeType: Entity BELONGS_TO Layer (enables traversal)
  auto* belongs_to_edge = schema.add_edge_types();
  belongs_to_edge->set_name("BELONGS_TO");
  belongs_to_edge->set_source_type("Entity");
  belongs_to_edge->set_target_type("Layer");

  // EdgeType: Block CONTAINS Entity (enables traversal)
  auto* contains_edge = schema.add_edge_types();
  contains_edge->set_name("CONTAINS");
  contains_edge->set_source_type("Block");
  contains_edge->set_target_type("Entity");

  // EdgeType: Entity REFERENCES Block (enables traversal for INSERTs)
  auto* references_edge = schema.add_edge_types();
  references_edge->set_name("REFERENCES");
  references_edge->set_source_type("Entity");
  references_edge->set_target_type("Block");

  return schema;
}

std::vector<std::string> SchemaAnalyzer::FindUniqueProperties(
    const Schema& schema, absl::string_view node_type) const {
  std::vector<std::string> unique_props;

  for (const auto& nt : schema.node_types()) {
    if (nt.name() == node_type) {
      for (const auto& prop : nt.properties()) {
        if (prop.unique()) {
          unique_props.push_back(prop.name());
        }
      }
      break;
    }
  }

  return unique_props;
}

std::vector<std::string> SchemaAnalyzer::FindComparableProperties(
    const Schema& schema, absl::string_view node_type) const {
  std::vector<std::string> comparable_props;

  for (const auto& nt : schema.node_types()) {
    if (nt.name() == node_type) {
      for (const auto& prop : nt.properties()) {
        if (prop.comparable()) {
          comparable_props.push_back(prop.name());
        }
      }
      break;
    }
  }

  return comparable_props;
}

std::vector<std::string> SchemaAnalyzer::FindIndexedProperties(
    const Schema& schema, absl::string_view node_type) const {
  std::vector<std::string> indexed_props;

  for (const auto& nt : schema.node_types()) {
    if (nt.name() == node_type) {
      for (const auto& prop : nt.properties()) {
        if (prop.indexed()) {
          indexed_props.push_back(prop.name());
        }
      }
      break;
    }
  }

  return indexed_props;
}

std::vector<std::string> SchemaAnalyzer::FindAggregableProperties(
    const Schema& schema, absl::string_view node_type) const {
  std::vector<std::string> aggregable_props;

  for (const auto& nt : schema.node_types()) {
    if (nt.name() == node_type) {
      for (const auto& prop : nt.properties()) {
        if (prop.aggregable()) {
          aggregable_props.push_back(prop.name());
        }
      }
      break;
    }
  }

  return aggregable_props;
}

std::vector<std::string> SchemaAnalyzer::GetTraversableEdgeTypes(
    const Schema& schema) const {
  std::vector<std::string> edge_types;

  for (const auto& et : schema.edge_types()) {
    edge_types.push_back(et.name());
  }

  return edge_types;
}

absl::Status SchemaAnalyzer::ValidateSchema(const Schema& schema) const {
  if (schema.node_types().empty()) {
    return absl::InvalidArgumentError("Schema must have at least one node type");
  }

  if (schema.source_format().empty()) {
    return absl::InvalidArgumentError("Schema must specify source_format");
  }

  // Validate that edge types reference valid node types
  std::set<std::string> node_type_names;
  for (const auto& nt : schema.node_types()) {
    node_type_names.insert(nt.name());
  }

  for (const auto& et : schema.edge_types()) {
    if (!node_type_names.contains(et.source_type())) {
      return absl::InvalidArgumentError(
          absl::StrFormat("Edge type '%s' references unknown source type '%s'",
                         et.name(), et.source_type()));
    }
    if (!node_type_names.contains(et.target_type())) {
      return absl::InvalidArgumentError(
          absl::StrFormat("Edge type '%s' references unknown target type '%s'",
                         et.name(), et.target_type()));
    }
  }

  return absl::OkStatus();
}

}  // namespace finetoo::schema
