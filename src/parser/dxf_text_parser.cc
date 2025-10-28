// Copyright 2025 Finetoo
// DXF Text Parser Implementation

#include "src/parser/dxf_text_parser.h"

#include <fstream>
#include <sstream>

#include "absl/status/status.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_format.h"
#include "absl/strings/strip.h"

namespace finetoo::parser {

// DXFEntity convenience accessors
absl::StatusOr<std::string> DXFEntity::GetString(int group_code) const {
  for (const auto& pair : data) {
    if (pair.group_code == group_code) {
      return pair.value;
    }
  }
  return absl::NotFoundError(
      absl::StrFormat("Group code %d not found in entity %s", group_code, type));
}

absl::StatusOr<double> DXFEntity::GetDouble(int group_code) const {
  auto value_or = GetString(group_code);
  if (!value_or.ok()) return value_or.status();

  double result;
  if (!absl::SimpleAtod(*value_or, &result)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("Cannot convert '%s' to double", *value_or));
  }
  return result;
}

absl::StatusOr<int> DXFEntity::GetInt(int group_code) const {
  auto value_or = GetString(group_code);
  if (!value_or.ok()) return value_or.status();

  int result;
  if (!absl::SimpleAtoi(*value_or, &result)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("Cannot convert '%s' to int", *value_or));
  }
  return result;
}

// DXFTextParser implementation

absl::StatusOr<DXFFile> DXFTextParser::Parse(absl::string_view file_path) {
  std::ifstream input{std::string{file_path}};  // Use braces to avoid "most vexing parse"
  if (!input.is_open()) {
    return absl::NotFoundError(absl::StrFormat("Cannot open file: %s", file_path));
  }

  return Parse(input);
}

absl::StatusOr<DXFFile> DXFTextParser::Parse(std::istream& input) {
  DXFFile file;
  line_number_ = 0;

  // Parse sections
  while (input.good()) {
    auto pair_or = ReadPair(input);
    if (!pair_or.ok()) {
      if (absl::IsOutOfRange(pair_or.status())) {
        break;  // EOF
      }
      return pair_or.status();
    }

    const auto& pair = *pair_or;

    // Section marker
    if (pair.group_code == 0 && pair.value == "SECTION") {
      // Read section name
      auto name_pair_or = ReadPair(input);
      if (!name_pair_or.ok()) return name_pair_or.status();

      if (name_pair_or->group_code != 2) {
        return absl::InvalidArgumentError(
            absl::StrFormat("Expected group code 2 after SECTION, got %d",
                           name_pair_or->group_code));
      }

      const std::string& section_name = name_pair_or->value;

      if (section_name == "HEADER") {
        auto status = ParseHeader(input, file);
        if (!status.ok()) return status;
      } else if (section_name == "BLOCKS") {
        auto status = ParseBlocks(input, file);
        if (!status.ok()) return status;
      } else if (section_name == "ENTITIES") {
        auto status = ParseEntities(input, file);
        if (!status.ok()) return status;
      } else {
        // Skip unknown sections
        while (input.good()) {
          auto skip_pair_or = ReadPair(input);
          if (!skip_pair_or.ok()) break;
          if (skip_pair_or->group_code == 0 && skip_pair_or->value == "ENDSEC") {
            break;
          }
        }
      }
    }

    if (pair.group_code == 0 && pair.value == "EOF") {
      break;
    }
  }

  // Build lookup maps
  BuildLookups(file);

  return file;
}

absl::StatusOr<DXFPair> DXFTextParser::ReadPair(std::istream& input) {
  std::string group_code_str;
  std::string value;

  // Read group code
  if (!std::getline(input, group_code_str)) {
    if (input.eof()) {
      return absl::OutOfRangeError("End of file");
    }
    return absl::DataLossError("Failed to read group code");
  }
  line_number_++;

  // Read value
  if (!std::getline(input, value)) {
    return absl::DataLossError(
        absl::StrFormat("Failed to read value at line %d", line_number_));
  }
  line_number_++;

  // Parse group code
  int group_code;
  std::string trimmed = std::string(absl::StripAsciiWhitespace(group_code_str));
  if (!absl::SimpleAtoi(trimmed, &group_code)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("Invalid group code '%s' at line %d", trimmed, line_number_ - 2));
  }

  // Trim value
  value = std::string(absl::StripAsciiWhitespace(value));

  return DXFPair{group_code, value};
}

absl::Status DXFTextParser::ParseHeader(std::istream& input, DXFFile& file) {
  while (input.good()) {
    auto pair_or = ReadPair(input);
    if (!pair_or.ok()) return pair_or.status();

    const auto& pair = *pair_or;

    // End of section
    if (pair.group_code == 0 && pair.value == "ENDSEC") {
      break;
    }

    // Extract version
    if (pair.group_code == 9 && pair.value == "$ACADVER") {
      auto version_pair_or = ReadPair(input);
      if (version_pair_or.ok()) {
        file.version = version_pair_or->value;
      }
    }
  }

  return absl::OkStatus();
}

absl::Status DXFTextParser::ParseBlocks(std::istream& input, DXFFile& file) {
  while (input.good()) {
    auto pair_or = ReadPair(input);
    if (!pair_or.ok()) return pair_or.status();

    const auto& pair = *pair_or;

    // End of section
    if (pair.group_code == 0 && pair.value == "ENDSEC") {
      break;
    }

    // Start of block
    if (pair.group_code == 0 && pair.value == "BLOCK") {
      DXFBlock block;

      // Read block properties
      while (input.good()) {
        auto block_pair_or = ReadPair(input);
        if (!block_pair_or.ok()) return block_pair_or.status();

        const auto& block_pair = *block_pair_or;

        if (block_pair.group_code == 2) {
          block.name = block_pair.value;
        } else if (block_pair.group_code == 5) {
          block.handle = block_pair.value;
        } else if (block_pair.group_code == 0) {
          // Start of entity within block or ENDBLK
          if (block_pair.value == "ENDBLK") {
            break;
          } else {
            // Parse entity within block
            auto entity_or = ParseEntity(input, block_pair.value);
            if (entity_or.ok()) {
              block.entities.push_back(*entity_or);
            }
          }
        }
      }

      file.blocks.push_back(std::move(block));
    }
  }

  return absl::OkStatus();
}

absl::Status DXFTextParser::ParseEntities(std::istream& input, DXFFile& file) {
  while (input.good()) {
    auto pair_or = ReadPair(input);
    if (!pair_or.ok()) return pair_or.status();

    const auto& pair = *pair_or;

    // End of section
    if (pair.group_code == 0 && pair.value == "ENDSEC") {
      break;
    }

    // Start of entity
    if (pair.group_code == 0) {
      auto entity_or = ParseEntity(input, pair.value);
      if (entity_or.ok()) {
        file.entities.push_back(*entity_or);
      }
    }
  }

  return absl::OkStatus();
}

absl::StatusOr<DXFEntity> DXFTextParser::ParseEntity(std::istream& input,
                                                       const std::string& entity_type) {
  DXFEntity entity;
  entity.type = entity_type;

  // Read entity data until next 0 code
  while (input.good()) {
    // Peek to see if next is group code 0 (next entity)
    auto pos = input.tellg();
    auto pair_or = ReadPair(input);

    if (!pair_or.ok()) {
      input.seekg(pos);  // Reset position
      break;
    }

    const auto& pair = *pair_or;

    if (pair.group_code == 0) {
      // Next entity - put it back
      input.seekg(pos);
      break;
    }

    // Store pair
    entity.data.push_back(pair);

    // Extract common fields
    if (pair.group_code == 5) {
      entity.handle = pair.value;
    } else if (pair.group_code == 8) {
      entity.layer = pair.value;
    }
  }

  return entity;
}

void DXFTextParser::BuildLookups(DXFFile& file) {
  // Build entity lookup by handle
  for (const auto& entity : file.entities) {
    if (!entity.handle.empty()) {
      file.entity_by_handle[entity.handle] = &entity;
    }
  }

  // Build block lookup by name
  for (const auto& block : file.blocks) {
    if (!block.name.empty()) {
      file.block_by_name[block.name] = &block;
    }
  }

  // Also add block entities to entity lookup
  for (const auto& block : file.blocks) {
    for (const auto& entity : block.entities) {
      if (!entity.handle.empty()) {
        file.entity_by_handle[entity.handle] = &entity;
      }
    }
  }
}

}  // namespace finetoo::parser
