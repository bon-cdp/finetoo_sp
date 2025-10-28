# Contributing to Finetoo

Thank you for your interest in contributing to Finetoo! We welcome contributions from the community.

## Table of Contents

- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Code Style](#code-style)
- [Making Changes](#making-changes)
- [Testing](#testing)
- [Submitting Changes](#submitting-changes)
- [Priority Areas](#priority-areas)
- [Documentation](#documentation)

---

## Getting Started

Finetoo is a production-grade C++ system for schema-driven understanding of engineering documents. Before contributing, please:

1. Read the [README.md](README.md) to understand the project goals
2. Review [docs/USER_JOURNEYS.md](docs/USER_JOURNEYS.md) for the target use cases
3. Read [docs/SEMANTIC_UNDERSTANDING.md](docs/SEMANTIC_UNDERSTANDING.md) for the architectural approach
4. Familiarize yourself with the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)

## Development Setup

### Prerequisites

- **Bazel 8.0+** - Build system ([Installation guide](https://bazel.build/install))
- **GCC 13.3+** or **Clang 17+** - C++20 compiler
- **Git** - Version control
- **Linux or macOS** - Tested on Ubuntu 24.04

### Clone and Build

```bash
# Clone the repository
git clone https://github.com/bon-cdp/finetoo_sp.git
cd finetoo_sp

# Build all targets
bazel build //...

# Run tests
bazel test //...

# Try the demo tools
bazel run //tools:demo_schema_discovery
bazel run //tools:parse_dxf -- binder_test_no_refs/18066-G-300.dxf
```

### Project Structure

```
finetoo_sp/
├── proto/           # Protocol Buffer schemas
├── src/
│   ├── parser/      # Format parsers (DXF, future: STEP, STL)
│   ├── schema/      # Schema analysis and operation discovery
│   ├── graph/       # Property graph representation
│   ├── operations/  # Generic operation primitives
│   ├── semantic/    # Part identification, dimension extraction
│   ├── export/      # BOM generator, DXF writer
│   ├── analysis/    # Block analyzer, divergence detection
│   ├── cli/         # Interactive CLI
│   ├── query/       # LLM query service (planned)
│   └── cloud/       # Google Cloud integration (planned)
├── tools/           # Command-line utilities
├── docs/            # Documentation
└── binder_test_no_refs/  # Test data (C-loop drawings)
```

---

## Code Style

Finetoo strictly follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

### Key Guidelines

**Naming Conventions:**
```cpp
// Classes: PascalCase
class DXFTextParser { ... };

// Functions/methods: PascalCase
void ParseHeader();

// Variables: snake_case
std::string file_path;
int line_number_;  // trailing underscore for private members

// Constants: kPascalCase
constexpr int kMaxBufferSize = 1024;

// Files: snake_case
dxf_text_parser.h
dxf_text_parser.cc
```

**Error Handling:**
```cpp
// ✅ Use absl::StatusOr<T> (no exceptions!)
absl::StatusOr<DXFFile> Parse(absl::string_view file_path);

// ✅ Check status before using value
auto result = parser.Parse("file.dxf");
if (!result.ok()) {
  return result.status();  // Propagate error
}
const auto& file = *result;

// ❌ Never throw exceptions
// throw std::runtime_error("Parse failed");  // DON'T DO THIS
```

**Memory Management:**
```cpp
// ✅ Use smart pointers for ownership
std::unique_ptr<GraphBuilder> builder = std::make_unique<GraphBuilder>();

// ✅ Use raw pointers for observation only
const DXFEntity* entity = file.entity_by_handle["3F2"];

// ✅ Use const references for parameters
void ProcessEntity(const DXFEntity& entity);

// ❌ Don't use manual memory management
// DXFFile* file = new DXFFile();  // DON'T DO THIS
```

**Const Correctness:**
```cpp
// ✅ Mark const methods
class Parser {
  int GetLineCount() const;  // Doesn't modify object
};

// ✅ Use const references for input
void Analyze(const PropertyGraph& graph);

// ✅ Use string_view for string parameters
void SetName(absl::string_view name);
```

### Code Formatting

```bash
# Format your code before committing (if you have clang-format)
clang-format -i src/path/to/your_file.cc

# Or follow the style in existing files
```

---

## Making Changes

### Branch Strategy

1. **Fork** the repository to your GitHub account
2. **Create a feature branch** from `main`:
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make your changes** following the code style guidelines
4. **Test thoroughly** (see Testing section)
5. **Commit** with clear, descriptive messages (see below)

### Commit Message Format

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `build`: Build system changes
- `perf`: Performance improvements

**Examples:**

```bash
# Feature
git commit -m "feat(parser): Add STEP file parser for 3D assemblies

Implements text-based STEP parser following same pattern as DXF parser.
Extracts PRODUCT entities, assembly structure, and material properties."

# Bug fix
git commit -m "fix(graph): Correct arena allocation size calculation

Fixed buffer overflow in GraphBuilder when processing large files."

# Documentation
git commit -m "docs: Add STEP format examples to SEMANTIC_UNDERSTANDING.md"
```

---

## Testing

### Running Tests

```bash
# Run all tests
bazel test //...

# Run specific test
bazel test //src/schema:schema_analyzer_test

# Run tests with verbose output
bazel test //src/schema:schema_analyzer_test --test_output=all
```

### Writing Tests

**Use GoogleTest:**

```cpp
// my_component_test.cc
#include "src/path/my_component.h"
#include <gtest/gtest.h>

namespace finetoo::mymodule {
namespace {

class MyComponentTest : public ::testing::Test {
 protected:
  MyComponent component_;
};

TEST_F(MyComponentTest, DescriptiveTestName) {
  auto result = component_.DoSomething();
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(*result, expected_value);
}

}  // namespace
}  // namespace finetoo::mymodule
```

**Test Requirements:**
- ✅ Every new feature must include tests
- ✅ Aim for >80% code coverage
- ✅ Test both success and error paths
- ✅ Use descriptive test names (not `Test1`, `Test2`)

### BUILD.bazel for Tests

```python
cc_test(
    name = "my_component_test",
    srcs = ["my_component_test.cc"],
    deps = [
        ":my_component",
        "@com_google_googletest//:gtest_main",
    ],
)
```

---

## Submitting Changes

### Pull Request Process

1. **Ensure your code builds:**
   ```bash
   bazel build //...
   ```

2. **Ensure all tests pass:**
   ```bash
   bazel test //...
   ```

3. **Update documentation** if needed (README, docs/, code comments)

4. **Push your branch** to your fork:
   ```bash
   git push origin feature/your-feature-name
   ```

5. **Create a Pull Request** on GitHub:
   - Use a clear, descriptive title
   - Reference any related issues
   - Describe what changes you made and why
   - Include examples or test output if relevant

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Unit tests added/updated
- [ ] All tests pass locally
- [ ] Tested on sample DXF files

## Checklist
- [ ] Code follows Google C++ style guide
- [ ] Comments added for complex logic
- [ ] Documentation updated
- [ ] No compiler warnings introduced
```

---

## Priority Areas

We're currently in **Phase 2: BOM Generation**. High-priority contributions:

### 1. GraphBuilder Implementation (HIGH PRIORITY)

**File:** `src/graph/graph_builder.cc`

**What's needed:**
- Implement `Build()` method to convert `DXFFile` → `PropertyGraph`
- Add entities as nodes with property maps
- Create INSERT→Block REFERENCES edges
- Implement string interning for memory efficiency

**Example:**
```cpp
absl::StatusOr<PropertyGraph> GraphBuilder::Build(const parser::DXFFile& dxf_file) {
  PropertyGraph graph;

  // Add schema
  *graph.mutable_schema() = CreateSchema(dxf_file);

  // Convert entities to nodes
  for (const auto& entity : dxf_file.entities) {
    AddEntity(entity, &graph);
  }

  // Convert blocks to nodes
  for (const auto& block : dxf_file.blocks) {
    AddBlock(block, &graph);
  }

  // Build INSERT→Block edges
  // TODO: Your implementation here

  return graph;
}
```

### 2. Operation Primitives (HIGH PRIORITY)

**File:** `src/operations/operation_executor.cc`

**What's needed:**
- Implement MATCH operation (find by unique property)
- Implement FILTER operation (select by criteria)
- Implement TRAVERSE operation (follow edges - key for BOM!)
- Implement AGGREGATE operation (count quantities)

**Example:**
```cpp
absl::StatusOr<OperationResult> OperationExecutor::Traverse(
    const Operation& op) {
  // Extract parameters
  const std::string& edge_type = op.parameters().at("edge_type");
  const std::string& target_type = op.parameters().at("target_type");

  // Follow edges in property graph
  // TODO: Your implementation here

  return result;
}
```

### 3. PartIdentifier (MEDIUM PRIORITY)

**File:** `src/semantic/part_identifier.{h,cc}` (NEW FILE)

**What's needed:**
- Schema-driven part classification (not hardcoded rules!)
- Identify which blocks are parts vs. annotations
- Extract part attributes (part numbers, descriptions)

### 4. BlockAnalyzer SHA-256 Hashing (MEDIUM PRIORITY)

**File:** `src/analysis/block_analyzer.cc`

**What's needed:**
- Implement `ComputeBlockHash()` using SHA-256
- Sort entities by handle for stable ordering
- Detect divergence (same name, different hash)

### 5. BOMGenerator (MEDIUM PRIORITY)

**File:** `src/export/bom_generator.{h,cc}` (NEW FILE)

**What's needed:**
- Generate JSON/CSV BOM from property graph
- Include quantities, provenance, divergence warnings

### 6. Additional Parsers (FUTURE)

**Files:** `src/parser/step_parser.{h,cc}`, `src/parser/stl_parser.{h,cc}`

**What's needed:**
- STEP text parser (EXPRESS entities)
- STL ASCII parser (triangle meshes)
- Follow same pattern as `DXFTextParser`

---

## Documentation

### Code Comments

**Use clear, concise comments:**

```cpp
// ✅ Good: Explains why, not what
// Sort entities by handle to ensure stable hash computation across runs
std::sort(entities.begin(), entities.end(),
          [](const auto& a, const auto& b) { return a.handle < b.handle; });

// ❌ Bad: Restates the code
// Sort the entities
std::sort(entities.begin(), entities.end());
```

**Document complex algorithms:**

```cpp
// Compute SHA-256 hash of block content for divergence detection.
//
// Process:
// 1. Sort entities by handle (stable ordering)
// 2. Concatenate entity data (type + properties)
// 3. Hash with SHA-256
// 4. Return hex-encoded hash
//
// This enables cross-drawing comparison: same block name + different hash
// indicates divergent content requiring user attention.
std::string ComputeBlockHash(const Block& block) {
  // Implementation...
}
```

### Updating Documentation

If your changes affect:
- **User workflows** → Update `docs/USER_JOURNEYS.md`
- **Architecture** → Update `README.md` Architecture section
- **Semantic understanding** → Update `docs/SEMANTIC_UNDERSTANDING.md`
- **New features** → Update `README.md` Roadmap

---

## Questions or Issues?

- **Questions**: Open a [GitHub Discussion](https://github.com/bon-cdp/finetoo_sp/discussions)
- **Bug Reports**: Open a [GitHub Issue](https://github.com/bon-cdp/finetoo_sp/issues)
- **Security Issues**: Email sj@sf-labs.co (do not open public issues)

---

## Code of Conduct

We follow the [Contributor Covenant Code of Conduct](https://www.contributor-covenant.org/version/2/1/code_of_conduct/).

### Our Pledge

We pledge to make participation in our project a harassment-free experience for everyone, regardless of age, body size, disability, ethnicity, gender identity and expression, level of experience, nationality, personal appearance, race, religion, or sexual identity and orientation.

### Expected Behavior

- Be respectful and professional
- Focus on technical merit
- Accept constructive criticism gracefully
- Help others learn and grow

---

## License

By contributing to Finetoo, you agree that your contributions will be licensed under the MIT License.

---

## Acknowledgments

Thank you for contributing to Finetoo! Every contribution, no matter how small, helps advance schema-driven document understanding. 🚀
