// Copyright 2025 Finetoo
// Simple DXF Text Parser
//
// Parses DXF files (text format) without external dependencies.
// DXF format: alternating group code / value pairs

#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace finetoo::parser {

// DXF group code / value pair
struct DXFPair {
  int group_code;
  std::string value;
};

// Parsed DXF entity
struct DXFEntity {
  std::string type;     // "LINE", "CIRCLE", "DIMENSION", etc.
  std::string handle;   // Unique identifier (group code 5)
  std::string layer;    // Layer name (group code 8)

  // All group code/value pairs for this entity
  std::vector<DXFPair> data;

  // Convenience accessors
  absl::StatusOr<std::string> GetString(int group_code) const;
  absl::StatusOr<double> GetDouble(int group_code) const;
  absl::StatusOr<int> GetInt(int group_code) const;
};

// Parsed DXF block definition
struct DXFBlock {
  std::string name;     // Block name (group code 2)
  std::string handle;   // Block handle
  std::vector<DXFEntity> entities;  // Entities within the block
};

// Complete parsed DXF file
struct DXFFile {
  std::string version;  // DXF version (e.g., "AC1027")
  std::vector<DXFEntity> entities;  // All entities in ENTITIES section
  std::vector<DXFBlock> blocks;     // All blocks in BLOCKS section

  // Entity lookup by handle
  absl::flat_hash_map<std::string, const DXFEntity*> entity_by_handle;

  // Block lookup by name
  absl::flat_hash_map<std::string, const DXFBlock*> block_by_name;
};

// Simple DXF text parser
class DXFTextParser {
 public:
  DXFTextParser() = default;

  // Non-copyable, movable
  DXFTextParser(const DXFTextParser&) = delete;
  DXFTextParser& operator=(const DXFTextParser&) = delete;
  DXFTextParser(DXFTextParser&&) = default;
  DXFTextParser& operator=(DXFTextParser&&) = default;

  // Parse DXF file from path
  absl::StatusOr<DXFFile> Parse(absl::string_view file_path);

  // Parse DXF from input stream
  absl::StatusOr<DXFFile> Parse(std::istream& input);

 private:
  // Read next group code / value pair
  absl::StatusOr<DXFPair> ReadPair(std::istream& input);

  // Parse HEADER section
  absl::Status ParseHeader(std::istream& input, DXFFile& file);

  // Parse BLOCKS section
  absl::Status ParseBlocks(std::istream& input, DXFFile& file);

  // Parse ENTITIES section
  absl::Status ParseEntities(std::istream& input, DXFFile& file);

  // Parse a single entity
  absl::StatusOr<DXFEntity> ParseEntity(std::istream& input,
                                         const std::string& entity_type);

  // Build lookup maps
  void BuildLookups(DXFFile& file);

  // Current line number (for error reporting)
  int line_number_ = 0;
};

}  // namespace finetoo::parser
