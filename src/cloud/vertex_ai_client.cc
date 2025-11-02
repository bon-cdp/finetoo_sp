// Copyright 2025 Finetoo
// Vertex AI Client Implementation

#include "src/cloud/vertex_ai_client.h"

#include <array>
#include <cstdio>
#include <memory>
#include <sstream>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "nlohmann/json.hpp"

namespace finetoo::cloud {

VertexAIClient::VertexAIClient(const VertexAIConfig& config)
    : config_(config) {}

std::string VertexAIClient::BuildEndpoint() const {
  return absl::StrFormat(
      "https://%s-aiplatform.googleapis.com/v1/projects/%s/locations/%s/"
      "publishers/google/models/%s:generateContent",
      config_.location, config_.project_id, config_.location, config_.model);
}

absl::StatusOr<std::string> VertexAIClient::ExecuteCurl(
    const std::vector<std::string>& args) {
  // Build command
  std::string cmd = "curl";
  for (const auto& arg : args) {
    cmd += " " + arg;
  }

  // Execute command
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                  pclose);

  if (!pipe) {
    return absl::InternalError("Failed to execute curl command");
  }

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }

  return result;
}

absl::StatusOr<std::string> VertexAIClient::GetAccessToken() {
  if (!cached_token_.empty()) {
    return cached_token_;
  }

  // Try application-default credentials first
  std::array<char, 256> buffer;
  std::string token;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(
      popen("gcloud auth application-default print-access-token 2>/dev/null",
            "r"),
      pclose);

  if (pipe) {
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      token += buffer.data();
    }
  }

  // If ADC didn't work, fall back to user credentials
  if (token.empty()) {
    std::unique_ptr<FILE, decltype(&pclose)> user_pipe(
        popen("gcloud auth print-access-token 2>/dev/null", "r"), pclose);

    if (!user_pipe) {
      return absl::UnauthenticatedError(
          "Failed to get access token. Run: gcloud auth login");
    }

    while (fgets(buffer.data(), buffer.size(), user_pipe.get()) != nullptr) {
      token += buffer.data();
    }
  }

  // Remove trailing newline
  if (!token.empty() && token.back() == '\n') {
    token.pop_back();
  }

  if (token.empty()) {
    return absl::UnauthenticatedError(
        "No access token. Run: gcloud auth login");
  }

  cached_token_ = token;
  return token;
}

absl::StatusOr<std::string> VertexAIClient::GenerateContent(
    absl::string_view prompt) {
  // Get access token
  auto token_or = GetAccessToken();
  if (!token_or.ok()) {
    return token_or.status();
  }

  const std::string& token = *token_or;
  std::string endpoint = BuildEndpoint();

  // Build request body
  nlohmann::json request_body = {
      {"contents",
       {{{"role", "user"}, {"parts", {{{"text", std::string(prompt)}}}}}}}};

  std::string json_str = request_body.dump();

  // Create temporary file for request body
  std::string temp_file = "/tmp/finetoo_request.json";
  FILE* f = fopen(temp_file.c_str(), "w");
  if (!f) {
    return absl::InternalError("Failed to create temp file");
  }
  fputs(json_str.c_str(), f);
  fclose(f);

  // Execute curl request
  std::string cmd = absl::StrFormat(
      "curl -s -X POST '%s' "
      "-H 'Authorization: Bearer %s' "
      "-H 'Content-Type: application/json' "
      "-d @%s",
      endpoint, token, temp_file);

  std::array<char, 1024> buffer;
  std::string response;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                  pclose);

  if (!pipe) {
    return absl::InternalError("Failed to execute curl");
  }

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    response += buffer.data();
  }

  // Parse response
  try {
    auto json_response = nlohmann::json::parse(response);

    // Extract text from response
    if (json_response.contains("candidates") &&
        json_response["candidates"].is_array() &&
        !json_response["candidates"].empty()) {
      auto& candidate = json_response["candidates"][0];
      if (candidate.contains("content") &&
          candidate["content"].contains("parts") &&
          candidate["content"]["parts"].is_array() &&
          !candidate["content"]["parts"].empty()) {
        auto& part = candidate["content"]["parts"][0];
        if (part.contains("text")) {
          return part["text"].get<std::string>();
        }
      }
    }

    // Check for error
    if (json_response.contains("error")) {
      std::string error_msg = json_response["error"].dump();
      return absl::InternalError(
          absl::StrCat("Vertex AI error: ", error_msg));
    }

    return absl::InternalError(
        absl::StrCat("Unexpected response format: ", response));

  } catch (const nlohmann::json::exception& e) {
    return absl::InternalError(
        absl::StrCat("JSON parse error: ", e.what(), "\nResponse: ", response));
  }
}

}  // namespace finetoo::cloud
