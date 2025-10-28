// Copyright 2025 Finetoo
// Simple DXF Parser Test Tool
//
// Usage: bazel run //tools:parse_dxf -- <path_to_dxf_file>

#include <iostream>
#include <string>

#include "src/parser/dxf_text_parser.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <dxf_file>\n";
    return 1;
  }

  std::string file_path = argv[1];
  std::cout << "Parsing: " << file_path << "\n\n";

  finetoo::parser::DXFTextParser parser;
  auto result = parser.Parse(file_path);

  if (!result.ok()) {
    std::cerr << "Error parsing DXF: " << result.status() << "\n";
    return 1;
  }

  const auto& file = *result;

  std::cout << "════════════════════════════════════════════════════════════\n";
  std::cout << " DXF File Parsed Successfully\n";
  std::cout << "════════════════════════════════════════════════════════════\n\n";

  std::cout << "DXF Version: " << (file.version.empty() ? "Unknown" : file.version) << "\n\n";

  std::cout << "ENTITIES Section:\n";
  std::cout << "  Total entities: " << file.entities.size() << "\n";

  // Count entity types
  std::map<std::string, int> entity_counts;
  for (const auto& entity : file.entities) {
    entity_counts[entity.type]++;
  }

  std::cout << "  Entity types:\n";
  for (const auto& [type, count] : entity_counts) {
    std::cout << "    " << type << ": " << count << "\n";
  }

  std::cout << "\nBLOCKS Section:\n";
  std::cout << "  Total blocks: " << file.blocks.size() << "\n";

  if (!file.blocks.empty()) {
    std::cout << "  Sample blocks (first 10):\n";
    int shown = 0;
    for (const auto& block : file.blocks) {
      if (shown++ >= 10) break;
      std::cout << "    - " << block.name
                << " (handle: " << block.handle
                << ", entities: " << block.entities.size() << ")\n";
    }
  }

  std::cout << "\nLookup Maps:\n";
  std::cout << "  Entities by handle: " << file.entity_by_handle.size() << "\n";
  std::cout << "  Blocks by name: " << file.block_by_name.size() << "\n";

  std::cout << "\n════════════════════════════════════════════════════════════\n";
  std::cout << " Parser Test: SUCCESS\n";
  std::cout << "════════════════════════════════════════════════════════════\n";

  return 0;
}
