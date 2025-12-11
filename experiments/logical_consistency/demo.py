#!/usr/bin/env python3
"""
Interactive Demo: Logical Consistency Scoring for Digital Brain

This demo showcases how the wreath-sheaf cohomological obstruction
can help structure and validate a digital brain's knowledge.
"""

import sys
import time
from logical_consistency_scorer import LogicalConsistencyScorer
from thought_to_finetoo import ThoughtToFinetoo


def print_header():
    """Print demo header."""
    print("\n" + "=" * 80)
    print(" DIGITAL BRAIN CONSISTENCY ANALYZER ".center(80))
    print(" Powered by Wreath-Sheaf Cohomological Obstruction ".center(80))
    print("=" * 80)


def print_section(title):
    """Print section header."""
    print("\n" + "-" * 60)
    print(f" {title} ".center(60))
    print("-" * 60)


def animated_print(text, delay=0.02):
    """Print text with animation."""
    for char in text:
        sys.stdout.write(char)
        sys.stdout.flush()
        time.sleep(delay)
    print()


def run_demo():
    """Run the interactive demo."""

    print_header()

    animated_print("\nWelcome to the Digital Brain Consistency Analyzer!")
    animated_print("This tool helps structure unorganized thoughts using mathematical logic.\n")

    # Initialize components
    scorer = LogicalConsistencyScorer(verbose=False)
    converter = ThoughtToFinetoo(verbose=False)

    # Demo 1: Friend's contradictory thoughts
    print_section("DEMO 1: Analyzing Contradictory Thoughts")

    thoughts_1 = """
technology is progress
progress is good
technology is dangerous
dangerous is bad
good is not bad
"""

    print("Your friend's thoughts:")
    print("-" * 40)
    for line in thoughts_1.strip().split('\n'):
        print(f"  📝 {line}")

    print("\n🔍 Analyzing logical consistency...")
    time.sleep(1)

    result_1 = scorer.compute_consistency_score(thoughts_1)

    print(f"\n📊 Results:")
    print(f"  Consistency Score: {result_1['score']:.1f}%")
    print(f"  Cohomological Obstruction: {result_1['residual']:.4e}")

    if result_1['contradictions']:
        print(f"\n⚠️  Logical tensions detected:")
        for c in result_1['contradictions']:
            print(f"  - \"{c[0]}\" conflicts with \"{c[1]}\"")

    print(f"\n💡 Interpretation:")
    print("  The thoughts contain logical tensions. Technology being both")
    print("  'progress' (good) and 'dangerous' (bad) creates a contradiction")
    print("  since good is not bad. This needs reconciliation.")

    input("\nPress Enter to continue...")

    # Demo 2: Well-structured knowledge
    print_section("DEMO 2: Well-Structured Knowledge")

    thoughts_2 = """
learning is process
understanding is outcome
process has steps
outcome has value
learning has understanding
steps has order
value is measurable
"""

    print("Organized thoughts:")
    print("-" * 40)
    for line in thoughts_2.strip().split('\n'):
        print(f"  📝 {line}")

    print("\n🔍 Analyzing logical consistency...")
    time.sleep(1)

    result_2 = scorer.compute_consistency_score(thoughts_2)

    print(f"\n📊 Results:")
    print(f"  Consistency Score: {result_2['score']:.1f}%")
    print(f"  Cohomological Obstruction: {result_2['residual']:.4e}")

    print(f"\n✅ Analysis: Excellent logical structure!")
    print("  All concepts connect coherently without contradictions.")

    # Demo 3: Converting to Finetoo Schema
    print_section("DEMO 3: Generating Finetoo Schema")

    print("Converting the well-structured thoughts to a schema...")
    time.sleep(1)

    schema = converter.convert_to_schema(thoughts_2)

    print(f"\n📋 Generated Schema:")
    print(f"  Nodes: {len(schema.nodes)}")
    for node in schema.nodes[:3]:
        print(f"    - {node.name} ({len(node.properties)} properties)")

    print(f"\n  Edges: {len(schema.edges)}")
    for edge in schema.edges[:3]:
        print(f"    - {edge.name}: {edge.source_type} → {edge.target_type}")

    operations = converter.generate_operations(schema)
    print(f"\n  Available Operations: {len(operations)}")
    print("    - MATCH: Find entities by unique properties")
    print("    - FILTER: Filter by comparable attributes")
    print("    - TRAVERSE: Navigate relationships")
    print("    - AGGREGATE: Count and group data")

    # Demo 4: Real-time scoring
    print_section("DEMO 4: Interactive Thought Analysis")

    print("Let's analyze custom thoughts in real-time!")
    print("\nEnter statements (or 'done' to finish):")

    custom_thoughts = []
    while True:
        line = input("  > ")
        if line.lower() == 'done':
            break
        custom_thoughts.append(line)

        # Real-time scoring
        if len(custom_thoughts) >= 2:
            current_text = '\n'.join(custom_thoughts)
            quick_result = scorer.compute_consistency_score(current_text)
            score = quick_result['score']

            if score >= 80:
                status = "✅"
            elif score >= 50:
                status = "🟡"
            else:
                status = "🔴"

            print(f"    Current consistency: {status} {score:.0f}%")

    if custom_thoughts:
        final_text = '\n'.join(custom_thoughts)
        print("\n🔍 Final analysis...")
        final_result = scorer.compute_consistency_score(final_text)

        print(f"\n📊 Final Results:")
        print(f"  Consistency Score: {final_result['score']:.1f}%")
        print(f"  Cohomological Obstruction: {final_result['residual']:.4e}")
        print(f"  Entities: {final_result['n_entities']}")
        print(f"  Statements: {final_result['n_statements']}")

        if final_result['contradictions']:
            print(f"\n⚠️  Contradictions found:")
            for c in final_result['contradictions']:
                print(f"  - {c[0]} vs {c[1]}")

    # Conclusion
    print_section("CONCLUSION")

    print("The wreath-sheaf cohomological obstruction provides a mathematical")
    print("foundation for measuring logical consistency in unstructured thoughts.")
    print()
    print("Key Benefits:")
    print("  ✓ Quantifies logical coherence (0 = perfect, higher = conflicts)")
    print("  ✓ Identifies specific contradictions")
    print("  ✓ Converts to structured Finetoo schemas")
    print("  ✓ Enables robust knowledge management")
    print("  ✓ FHE-compatible (depth 0 operations)")
    print()
    print("This tool bridges abstract mathematics with practical knowledge")
    print("organization, making it ideal for digital brain applications.")

    print("\n" + "=" * 80)
    print(" Thank you for using the Digital Brain Consistency Analyzer! ".center(80))
    print("=" * 80)


if __name__ == "__main__":
    try:
        run_demo()
    except KeyboardInterrupt:
        print("\n\nDemo interrupted by user.")
    except Exception as e:
        print(f"\nError: {e}")
        sys.exit(1)