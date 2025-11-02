// Copyright 2025 Finetoo
// Vertex AI Client for Gemini API

#pragma once

#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace finetoo::cloud {

// Configuration for Vertex AI client
struct VertexAIConfig {
  std::string project_id;
  std::string location = "us-central1";
  std::string model = "gemini-1.5-pro";

  // Authentication (either credentials_path OR use ADC)
  std::string credentials_path;  // Path to service account JSON
};

// Vertex AI client for calling Gemini API
class VertexAIClient {
 public:
  explicit VertexAIClient(const VertexAIConfig& config);

  // Generate content from prompt using Gemini
  absl::StatusOr<std::string> GenerateContent(absl::string_view prompt);

  // Get OAuth token for authentication
  absl::StatusOr<std::string> GetAccessToken();

 private:
  VertexAIConfig config_;
  std::string cached_token_;

  // Build API endpoint URL
  std::string BuildEndpoint() const;

  // Execute curl command and return output
  absl::StatusOr<std::string> ExecuteCurl(const std::vector<std::string>& args);
};

}  // namespace finetoo::cloud
