// Copyright 2025 Finetoo
// BOM Exporter Implementation

#include "src/export/bom_exporter.h"

#include <fstream>
#include <sstream>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "nlohmann/json.hpp"

namespace finetoo::export_util {

// Helper to sanitize strings for JSON export
static std::string SanitizeString(const std::string& input) {
  std::string output;
  for (char c : input) {
    // Only include printable ASCII characters
    if (c >= 32 && c <= 126) {
      output += c;
    } else if (c == '\n' || c == '\t') {
      output += ' ';
    }
    // Skip non-printable characters
  }
  return output;
}

std::vector<BOMEntry> BOMExporter::ParseBOMFromResult(
    const finetoo::operations::v1::OperationResult& result,
    const finetoo::graph::v1::PropertyGraph& graph) {

  std::vector<BOMEntry> bom;

  for (const auto& [part_name, count_str] : result.values()) {
    BOMEntry entry;
    entry.part_name = part_name;
    entry.quantity = std::stoi(count_str);

    // Find source drawings for this part
    // Look through Entity nodes to find INSERTs with this block name
    if (graph.nodes_by_type().contains("Entity")) {
      for (const auto& node : graph.nodes_by_type().at("Entity").nodes()) {
        // Check if this is an INSERT for this block
        auto type_it = node.string_props().find("type");
        auto gc2_it = node.string_props().find("gc_2");

        if (type_it != node.string_props().end() && type_it->second == "INSERT" &&
            gc2_it != node.string_props().end() && gc2_it->second == part_name) {

          // Get source drawing if available
          auto source_it = node.string_props().find("source_drawing");
          if (source_it != node.string_props().end()) {
            std::string drawing = source_it->second;
            if (std::find(entry.source_drawings.begin(), entry.source_drawings.end(), drawing)
                == entry.source_drawings.end()) {
              entry.source_drawings.push_back(drawing);
            }
          }
        }
      }
    }

    // Extract any dimensional properties from the block definition
    if (graph.nodes_by_type().contains("Block")) {
      for (const auto& node : graph.nodes_by_type().at("Block").nodes()) {
        auto name_it = node.string_props().find("name");
        if (name_it != node.string_props().end() && name_it->second == part_name) {
          // Add any numeric properties as dimensions
          for (const auto& [key, value] : node.numeric_props()) {
            entry.properties[key] = std::to_string(value);
          }
          break;
        }
      }
    }

    bom.push_back(entry);
  }

  // Sort by quantity descending
  std::sort(bom.begin(), bom.end(),
            [](const BOMEntry& a, const BOMEntry& b) {
              return a.quantity > b.quantity;
            });

  return bom;
}

std::vector<Dimension> BOMExporter::ExtractDimensions(
    const finetoo::graph::v1::PropertyGraph& graph) {

  std::vector<Dimension> dimensions;

  if (!graph.nodes_by_type().contains("Entity")) {
    return dimensions;
  }

  for (const auto& node : graph.nodes_by_type().at("Entity").nodes()) {
    // Check if this is a DIMENSION entity
    auto type_it = node.string_props().find("type");
    if (type_it == node.string_props().end() || type_it->second != "DIMENSION") {
      continue;
    }

    Dimension dim;
    dim.entity_handle = node.id();

    // Get dimension type (group code 70)
    auto gc70_it = node.string_props().find("gc_70");
    if (gc70_it != node.string_props().end()) {
      int dim_type = std::stoi(gc70_it->second);
      if (dim_type == 0) dim.dimension_type = "LINEAR";
      else if (dim_type == 1) dim.dimension_type = "ALIGNED";
      else if (dim_type == 2) dim.dimension_type = "ANGULAR";
      else if (dim_type == 3) dim.dimension_type = "DIAMETER";
      else if (dim_type == 4) dim.dimension_type = "RADIUS";
      else dim.dimension_type = "OTHER";
    }

    // Get measurement value (group code 42)
    auto gc42_it = node.numeric_props().find("gc_42");
    if (gc42_it != node.numeric_props().end()) {
      dim.measurement_value = gc42_it->second;
    } else {
      dim.measurement_value = 0.0;
    }

    // Get text override (group code 1)
    auto gc1_it = node.string_props().find("gc_1");
    if (gc1_it != node.string_props().end()) {
      dim.text_override = gc1_it->second;
    }

    // Get layer
    auto layer_it = node.string_props().find("layer");
    if (layer_it != node.string_props().end()) {
      dim.layer = layer_it->second;
    }

    // Get source drawing
    auto source_it = node.string_props().find("source_drawing");
    if (source_it != node.string_props().end()) {
      dim.source_drawing = source_it->second;
    }

    dimensions.push_back(dim);
  }

  return dimensions;
}

absl::Status BOMExporter::ExportToJSON(
    const std::string& filename,
    const std::vector<BOMEntry>& bom,
    const std::vector<Dimension>& dimensions) {

  nlohmann::json output;
  output["generated_at"] = std::time(nullptr);
  output["total_unique_parts"] = bom.size();

  int total_quantity = 0;
  for (const auto& entry : bom) {
    total_quantity += entry.quantity;
  }
  output["total_instances"] = total_quantity;
  output["total_dimensions"] = dimensions.size();

  // Add BOM entries
  nlohmann::json bom_array = nlohmann::json::array();
  for (const auto& entry : bom) {
    nlohmann::json item;
    item["part_name"] = SanitizeString(entry.part_name);
    item["quantity"] = entry.quantity;

    // Sanitize source drawings
    nlohmann::json drawings_array = nlohmann::json::array();
    for (const auto& drawing : entry.source_drawings) {
      drawings_array.push_back(SanitizeString(drawing));
    }
    item["source_drawings"] = drawings_array;

    // Sanitize properties
    nlohmann::json props_obj;
    for (const auto& [key, value] : entry.properties) {
      props_obj[SanitizeString(key)] = SanitizeString(value);
    }
    item["properties"] = props_obj;

    bom_array.push_back(item);
  }
  output["bom"] = bom_array;

  // Add dimensions
  nlohmann::json dim_array = nlohmann::json::array();
  for (const auto& dim : dimensions) {
    nlohmann::json item;
    item["handle"] = SanitizeString(dim.entity_handle);
    item["type"] = SanitizeString(dim.dimension_type);
    item["value"] = dim.measurement_value;
    item["text"] = SanitizeString(dim.text_override);
    item["layer"] = SanitizeString(dim.layer);
    item["source_drawing"] = SanitizeString(dim.source_drawing);
    dim_array.push_back(item);
  }
  output["dimensions"] = dim_array;

  // Write to file
  std::ofstream file(filename);
  if (!file.is_open()) {
    return absl::InternalError(absl::StrCat("Failed to open file: ", filename));
  }

  file << output.dump(2);  // Pretty print with 2-space indent
  file.close();

  return absl::OkStatus();
}

absl::Status BOMExporter::ExportToCSV(
    const std::string& filename,
    const std::vector<BOMEntry>& bom) {

  std::ofstream file(filename);
  if (!file.is_open()) {
    return absl::InternalError(absl::StrCat("Failed to open file: ", filename));
  }

  // Write header
  file << "Part Name,Quantity,Source Drawings,Properties\n";

  // Write entries
  for (const auto& entry : bom) {
    file << "\"" << entry.part_name << "\",";
    file << entry.quantity << ",";

    // Join source drawings
    file << "\"";
    for (size_t i = 0; i < entry.source_drawings.size(); i++) {
      file << entry.source_drawings[i];
      if (i < entry.source_drawings.size() - 1) file << "; ";
    }
    file << "\",";

    // Join properties
    file << "\"";
    bool first = true;
    for (const auto& [key, value] : entry.properties) {
      if (!first) file << "; ";
      file << key << "=" << value;
      first = false;
    }
    file << "\"\n";
  }

  file.close();
  return absl::OkStatus();
}

absl::Status BOMExporter::ExportDimensions(
    const std::string& filename,
    const std::vector<Dimension>& dimensions) {

  std::ofstream file(filename);
  if (!file.is_open()) {
    return absl::InternalError(absl::StrCat("Failed to open file: ", filename));
  }

  // Write header
  file << "Handle,Type,Measured Value,Display Text,Layer,Source Drawing\n";

  // Write dimensions
  for (const auto& dim : dimensions) {
    file << "\"" << dim.entity_handle << "\",";
    file << "\"" << dim.dimension_type << "\",";
    file << dim.measurement_value << ",";
    file << "\"" << dim.text_override << "\",";
    file << "\"" << dim.layer << "\",";
    file << "\"" << dim.source_drawing << "\"\n";
  }

  file.close();
  return absl::OkStatus();
}

}  // namespace finetoo::export_util
