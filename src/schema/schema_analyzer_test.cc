// Copyright 2025 Finetoo
// SchemaAnalyzer Tests

#include "src/schema/schema_analyzer.h"

#include <gtest/gtest.h>

namespace finetoo::schema {
namespace {

class SchemaAnalyzerTest : public ::testing::Test {
 protected:
  SchemaAnalyzer analyzer_;
};

TEST_F(SchemaAnalyzerTest, CreateDXFSchemaSucceeds) {
  auto schema_or = SchemaAnalyzer::CreateDXFSchema("AC1027");
  ASSERT_TRUE(schema_or.ok()) << schema_or.status();

  const auto& schema = *schema_or;
  EXPECT_EQ(schema.source_format(), "DXF");
  EXPECT_EQ(schema.format_version(), "AC1027");
  EXPECT_EQ(schema.schema_version(), "1.0.0");

  // Should have Entity, Block, and Layer node types
  EXPECT_GE(schema.node_types_size(), 3);

  // Should have edge types for traversal
  EXPECT_GE(schema.edge_types_size(), 3);
}

TEST_F(SchemaAnalyzerTest, FindsUniqueProperties) {
  auto schema_or = SchemaAnalyzer::CreateDXFSchema("AC1027");
  ASSERT_TRUE(schema_or.ok());

  // Entity type should have 'handle' as unique property
  auto unique_props = analyzer_.FindUniqueProperties(*schema_or, "Entity");
  EXPECT_FALSE(unique_props.empty());
  EXPECT_EQ(unique_props[0], "handle");
}

TEST_F(SchemaAnalyzerTest, FindsComparableProperties) {
  auto schema_or = SchemaAnalyzer::CreateDXFSchema("AC1027");
  ASSERT_TRUE(schema_or.ok());

  // Entity type should have 'x' and 'y' as comparable properties
  auto comparable_props = analyzer_.FindComparableProperties(*schema_or, "Entity");
  EXPECT_GE(comparable_props.size(), 2);

  // Block type should have 'content_hash' as comparable (for divergence detection!)
  auto block_comparable = analyzer_.FindComparableProperties(*schema_or, "Block");
  EXPECT_FALSE(block_comparable.empty());
}

TEST_F(SchemaAnalyzerTest, FindsIndexedProperties) {
  auto schema_or = SchemaAnalyzer::CreateDXFSchema("AC1027");
  ASSERT_TRUE(schema_or.ok());

  // Entity type should have indexed properties for filtering
  auto indexed_props = analyzer_.FindIndexedProperties(*schema_or, "Entity");
  EXPECT_FALSE(indexed_props.empty());

  // Should include 'type' and 'layer'
  bool has_type = false;
  bool has_layer = false;
  for (const auto& prop : indexed_props) {
    if (prop == "type") has_type = true;
    if (prop == "layer") has_layer = true;
  }
  EXPECT_TRUE(has_type);
  EXPECT_TRUE(has_layer);
}

TEST_F(SchemaAnalyzerTest, FindsAggregableProperties) {
  auto schema_or = SchemaAnalyzer::CreateDXFSchema("AC1027");
  ASSERT_TRUE(schema_or.ok());

  // Coordinates should be aggregable (for calculating extents, etc.)
  auto aggregable_props = analyzer_.FindAggregableProperties(*schema_or, "Entity");
  EXPECT_GE(aggregable_props.size(), 2);  // x and y
}

TEST_F(SchemaAnalyzerTest, FindsTraversableEdgeTypes) {
  auto schema_or = SchemaAnalyzer::CreateDXFSchema("AC1027");
  ASSERT_TRUE(schema_or.ok());

  auto edge_types = analyzer_.GetTraversableEdgeTypes(*schema_or);
  EXPECT_GE(edge_types.size(), 3);

  // Should have BELONGS_TO, CONTAINS, and REFERENCES
  bool has_belongs_to = false;
  bool has_contains = false;
  bool has_references = false;

  for (const auto& edge : edge_types) {
    if (edge == "BELONGS_TO") has_belongs_to = true;
    if (edge == "CONTAINS") has_contains = true;
    if (edge == "REFERENCES") has_references = true;
  }

  EXPECT_TRUE(has_belongs_to);
  EXPECT_TRUE(has_contains);
  EXPECT_TRUE(has_references);
}

TEST_F(SchemaAnalyzerTest, ValidatesSchema) {
  auto schema_or = SchemaAnalyzer::CreateDXFSchema("AC1027");
  ASSERT_TRUE(schema_or.ok());

  auto status = analyzer_.ValidateSchema(*schema_or);
  EXPECT_TRUE(status.ok()) << status;
}

// This test demonstrates the core finetoo insight:
// The schema tells us what operations are possible!
TEST_F(SchemaAnalyzerTest, SchemaEnablesOperationDiscovery) {
  auto schema_or = SchemaAnalyzer::CreateDXFSchema("AC1027");
  ASSERT_TRUE(schema_or.ok());
  const auto& schema = *schema_or;

  // Because 'handle' is unique, we can do match operations
  auto unique_props = analyzer_.FindUniqueProperties(schema, "Entity");
  ASSERT_FALSE(unique_props.empty());
  EXPECT_EQ(unique_props[0], "handle");
  // This enables: match_by_handle(v1_entity, v2_entity)

  // Because 'x', 'y' are comparable, we can do compare operations
  auto comparable_props = analyzer_.FindComparableProperties(schema, "Entity");
  ASSERT_GE(comparable_props.size(), 2);
  // This enables: compare(v1.x, v2.x), compare(v1.y, v2.y)

  // Because 'layer', 'type' are indexed, we can do filter operations
  auto indexed_props = analyzer_.FindIndexedProperties(schema, "Entity");
  ASSERT_GE(indexed_props.size(), 2);
  // This enables: filter(type == "POLYLINE"), filter(layer == "EMS_REV")

  // Because 'x', 'y' are aggregable, we can do aggregate operations
  auto aggregable_props = analyzer_.FindAggregableProperties(schema, "Entity");
  ASSERT_GE(aggregable_props.size(), 2);
  // This enables: avg(y), min(x), max(x), etc.

  // Because edge types exist, we can do traverse operations
  auto edge_types = analyzer_.GetTraversableEdgeTypes(schema);
  ASSERT_GE(edge_types.size(), 3);
  // This enables: traverse(entity, BELONGS_TO, layer)

  // ALL OF THESE OPERATIONS ARE DISCOVERED FROM SCHEMA,
  // NOT HARDCODED IN THE APPLICATION!
}

}  // namespace
}  // namespace finetoo::schema
