# 🚀 QUICK START - Logical Consistency Scorer

## Install & Run in 30 Seconds

```bash
# 1. Go to the tool directory
cd experiments/logical_consistency/

# 2. Run the demo (auto-installs dependencies)
./run_demo.sh
```

## Test Your Own Statements

```bash
# Option 1: Interactive mode
python3 consistency_cli.py

# Option 2: From a file
python3 consistency_cli.py -f your_statements.txt

# Option 3: Pipe input
echo "dog is cat
dog is not cat" | python3 consistency_cli.py
```

## What the Score Means

- **100%** = Perfect logical consistency ✅
- **80-99%** = Good consistency, minor tensions 🟢
- **50-79%** = Moderate issues, review needed 🟡
- **0-49%** = Major contradictions detected 🔴

## The Math Behind It

The tool uses **wreath-sheaf cohomological obstruction** to measure logical inconsistency:
- Zero obstruction = perfect consistency
- Higher obstruction = more contradictions

## For Your Digital Brain Project

Use this as a **pre-filter** before adding thoughts to your knowledge base:

```python
from logical_consistency_scorer import LogicalConsistencyScorer

scorer = LogicalConsistencyScorer()
result = scorer.compute_consistency_score(new_thoughts)

if result['score'] < 50:
    print("⚠️ These thoughts contradict existing knowledge!")
    print(f"Contradictions: {result['contradictions']}")
else:
    print("✅ Safe to add to knowledge base")
```

## Key Files

- `consistency_cli.py` - Main CLI interface
- `logical_consistency_scorer.py` - Core logic (use the fixed version)
- `examples/` - Test files to try
- `README_FOR_CLAUDE.md` - Full documentation

## Why This Matters

Traditional LLMs can generate plausible-sounding but logically inconsistent text. This tool provides a **mathematical guarantee** of logical consistency using pure linear algebra - no machine learning needed!

---

Built with ❤️ using wreath-sheaf theory from "An Algebraic Theory of Learnability"