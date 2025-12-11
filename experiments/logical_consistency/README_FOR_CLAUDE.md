# 🧠 Logical Consistency Scorer - For Claude Code & Humans

## What This Tool Does

This tool measures the **logical consistency** of natural language statements using **wreath-sheaf cohomological obstruction**. Think of it as a mathematical lie detector for knowledge bases!

### The Big Idea
- **Wreath-Sheaf** isn't great at generating human-like text
- But it's AMAZING at detecting logical contradictions
- The "cohomological obstruction" (fancy math term) = how inconsistent your statements are
- Score of 0 = perfect consistency, higher = more contradictions

## Quick Start for Claude Code

### 1. Basic Usage
```bash
# Check if statements are logically consistent
echo "dog is animal
cat is animal
dog is cat
dog is not cat" | python3 consistency_cli.py

# Or from a file
python3 consistency_cli.py -f examples/contradictions.txt
```

### 2. What You'll Get Back
```
Consistency Score: 50.0%  (0-100%, higher is better)
Cohomological Obstruction: 0.5000  (0 = perfect, >1 = major issues)
Contradictions Found:
  - "dog is cat" vs "dog is not cat"
```

## How It Works (Simple Version)

1. **Parse** natural language into logical statements
2. **Build** a knowledge graph of entities and relationships
3. **Detect** contradictions (A is B + A is not B = problem!)
4. **Create** patches for each relation type (what wreath-sheaf is good at)
5. **Solve** one big linear system (no machine learning needed!)
6. **Score** based on the mathematical residual

## How It Works (For Math Nerds)

The tool uses:
- **Wreath Products**: Position-dependent transformations on cyclic groups
- **Sheaf Theory**: Local patches with global consistency constraints
- **Cohomology**: The obstruction to finding a global solution

The residual from solving the unified linear system IS the cohomological obstruction, which directly quantifies logical inconsistency.

## File Structure

```
logical_consistency/
├── README_FOR_CLAUDE.md        # You are here!
├── consistency_cli.py           # Main CLI interface
├── logical_consistency_scorer.py # Core scoring logic
├── examples/                    # Test files
│   ├── consistent.txt          # Logically consistent statements
│   ├── contradictions.txt      # Contains contradictions
│   └── complex.txt            # Complex relationships
├── requirements.txt            # Python dependencies
└── run_demo.sh                # One-click demo
```

## Supported Statement Patterns

The tool understands:
- `X is Y` - Type/identity relationships
- `X is not Y` - Negative relationships
- `X has Y` - Properties/attributes
- `X lacks Y` - Absence of properties
- `X and Y have Z` - Shared properties

## Examples for Testing

### Example 1: Perfect Consistency
```
mammal is animal
dog is mammal
cat is mammal
dog has fur
cat has fur
dog is not cat
```
**Result**: 100% consistent ✅

### Example 2: Direct Contradiction
```
bird has wings
bird lacks wings
```
**Result**: 50% consistent ⚠️

### Example 3: Complex Knowledge
```
consciousness is phenomenon
thought is process
memory is storage
consciousness has thought
thought has memory
process is not storage
```
**Result**: Score depends on logical structure

## For Your Wharton Friend's Digital Brain

This tool is PERFECT for:
1. **Checking thought consistency** before adding to knowledge base
2. **Finding contradictions** in existing beliefs
3. **Scoring new inputs** for logical coherence
4. **Building robust schemas** that won't break

The key insight: Use this as a **pre-filter** before feeding thoughts to language models!

## Advanced Usage

### JSON Output (for integration)
```bash
python3 consistency_cli.py -f input.txt --json
```

### Verbose Mode (see the math!)
```bash
python3 consistency_cli.py -f input.txt --verbose
```

### Convert to Finetoo Schema
```python
from thought_to_finetoo import ThoughtToFinetoo

converter = ThoughtToFinetoo()
schema = converter.convert_to_schema(text)
# Now you have a structured schema with semantic flags!
```

## Installation

```bash
# Install dependencies (numpy and scipy)
pip install -r requirements.txt

# Run the demo
./run_demo.sh
```

## The Math Magic ✨

The cohomological obstruction formula:
```
Obstruction = ||A_sheaf @ w* - b_sheaf||² + base_penalty
```

Where:
- `A_sheaf` = Block matrix of local patches + gluing constraints
- `w*` = Optimal weights from least-squares solve
- `base_penalty` = 0.5 × number of contradictions

## Why This Matters

Traditional approaches:
- Use LLMs (expensive, slow, unpredictable)
- Hard-code rules (brittle, doesn't scale)
- Ignore consistency (leads to confused AI)

This approach:
- Pure linear algebra (fast, deterministic)
- No training needed (works instantly)
- FHE-compatible (privacy-preserving)
- Exact mathematical guarantee

## Tips for Claude Code

1. **Start simple** - Test with 2-3 statements first
2. **Look for contradictions** - The tool will find them!
3. **Use the score** - Below 50% = needs human review
4. **Check the residual** - It's the "true" mathematical inconsistency
5. **Combine with Finetoo** - Convert to schemas for structured processing

## Common Issues & Solutions

**Issue**: Score is 0% but no contradictions shown
**Solution**: Check for subtle logical conflicts (A→B, B→C, A≠C)

**Issue**: Perfect consistency but seems wrong
**Solution**: The tool only catches logical contradictions, not factual errors

**Issue**: Import errors
**Solution**: Make sure you're in the right directory with fhe_transformer available

## The Secret Sauce 🍝

This tool demonstrates that:
- **Algebra > Machine Learning** for logical consistency
- **Wreath-sheaf cohomology** can solve real problems
- **Mathematical structure** beats brute force
- **One linear solve** > thousands of gradient steps

## Next Steps

1. Try the examples in the `examples/` folder
2. Test your own statements
3. Integrate with your knowledge management system
4. Use the consistency score to filter inputs
5. Build more robust AI systems!

## Credits

Built on the wreath-sheaf framework from the paper:
"An Algebraic Theory of Learnability" by bon-cdp

The key insight: Problems with position-dependent structure and local-to-global consistency are perfectly suited for wreath-sheaf analysis.

---

**Remember**: This tool doesn't judge if statements are TRUE, only if they're CONSISTENT with each other. "Dogs can fly" is consistent with "Flying dogs exist" even though both are false!

Happy consistency checking! 🎯