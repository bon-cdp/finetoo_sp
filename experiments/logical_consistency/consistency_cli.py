#!/usr/bin/env python3
"""
CLI Tool for Logical Consistency Scoring

A command-line interface that accepts text input and outputs a cohomological
obstruction score quantifying logical consistency.

Usage:
    python consistency_cli.py [options]
    cat statements.txt | python consistency_cli.py
    echo "dog is animal\ncat is animal\ndog is cat" | python consistency_cli.py

Options:
    -f, --file FILE     Read statements from file
    -v, --verbose       Show detailed analysis
    -j, --json         Output in JSON format
    -h, --help         Show this help message
"""

import sys
import argparse
import json
from typing import Optional
from logical_consistency_scorer import LogicalConsistencyScorer


class ConsistencyCLI:
    """Command-line interface for logical consistency scoring."""

    def __init__(self):
        self.scorer = LogicalConsistencyScorer(verbose=False)

    def print_banner(self):
        """Print the CLI banner."""
        print("=" * 80)
        print(" Logical Consistency Scorer - Powered by Wreath-Sheaf Cohomology ".center(80))
        print("=" * 80)
        print()

    def print_score_bar(self, score: float, width: int = 50):
        """Print a visual score bar."""
        filled = int(score / 100 * width)
        bar = "█" * filled + "░" * (width - filled)

        if score >= 80:
            color = "\033[92m"  # Green
        elif score >= 50:
            color = "\033[93m"  # Yellow
        else:
            color = "\033[91m"  # Red

        reset = "\033[0m"

        print(f"Consistency: {color}[{bar}] {score:.1f}%{reset}")

    def format_result(self, result: dict, verbose: bool = False, json_output: bool = False) -> str:
        """Format the result for display."""

        if json_output:
            return json.dumps(result, indent=2)

        output = []

        # Basic score
        output.append("\n" + "─" * 80)
        output.append("CONSISTENCY ANALYSIS RESULTS")
        output.append("─" * 80)

        # Visual score bar
        score_bar = self._create_score_bar(result['score'])
        output.append(f"\n{score_bar}")

        # Key metrics
        output.append(f"\nLogical Consistency Score: {result['score']:.1f}%")
        output.append(f"Cohomological Obstruction: {result['residual']:.4e}")

        # Contradictions
        if result['contradictions']:
            output.append(f"\n⚠️  Found {len(result['contradictions'])} contradiction(s):")
            for c in result['contradictions']:
                output.append(f"   ✗ \"{c[0]}\" contradicts \"{c[1]}\"")
        else:
            output.append("\n✓ No direct contradictions detected")

        # Analysis
        output.append(f"\n📊 Analysis: {result['analysis']}")

        # Detailed metrics if verbose
        if verbose:
            output.append("\n" + "─" * 40)
            output.append("DETAILED METRICS")
            output.append("─" * 40)
            output.append(f"Statements analyzed: {result.get('n_statements', 'N/A')}")
            output.append(f"Entities identified: {result.get('n_entities', 'N/A')}")
            output.append(f"Sheaf patches: {result.get('n_patches', 'N/A')}")
            output.append(f"Gluing constraints: {result.get('n_gluings', 'N/A')}")

        # Interpretation
        output.append("\n" + "─" * 80)
        output.append(self._interpret_score(result['score'], result['residual']))
        output.append("─" * 80)

        return "\n".join(output)

    def _create_score_bar(self, score: float, width: int = 50) -> str:
        """Create a visual score bar string."""
        filled = int(score / 100 * width)
        bar = "█" * filled + "░" * (width - filled)

        if score >= 80:
            color_code = "🟢"
            label = "HIGH"
        elif score >= 50:
            color_code = "🟡"
            label = "MEDIUM"
        else:
            color_code = "🔴"
            label = "LOW"

        return f"Consistency: [{bar}] {score:.1f}% {color_code} {label}"

    def _interpret_score(self, score: float, residual: float) -> str:
        """Provide interpretation of the score."""
        if score >= 95:
            return ("💡 INTERPRETATION: Excellent logical consistency! The statements form a\n"
                   "   coherent knowledge structure with minimal or no contradictions.")
        elif score >= 80:
            return ("💡 INTERPRETATION: Good logical consistency. Minor tensions exist but\n"
                   "   the overall structure is sound. Consider reviewing edge cases.")
        elif score >= 50:
            return ("💡 INTERPRETATION: Moderate consistency. Several logical issues detected.\n"
                   "   Recommend reviewing and clarifying ambiguous statements.")
        else:
            return ("💡 INTERPRETATION: Poor logical consistency. Significant contradictions\n"
                   "   present. The knowledge structure needs substantial revision.")

    def read_input(self, file_path: Optional[str] = None) -> str:
        """Read input from file or stdin."""
        if file_path:
            try:
                with open(file_path, 'r') as f:
                    return f.read()
            except FileNotFoundError:
                print(f"Error: File '{file_path}' not found.")
                sys.exit(1)
            except Exception as e:
                print(f"Error reading file: {e}")
                sys.exit(1)
        else:
            # Read from stdin
            if sys.stdin.isatty():
                print("Enter logical statements (one per line).")
                print("Press Ctrl+D (Unix) or Ctrl+Z (Windows) when done:")
                print()

            try:
                return sys.stdin.read()
            except KeyboardInterrupt:
                print("\n\nInterrupted by user.")
                sys.exit(0)

    def run(self):
        """Run the CLI."""
        parser = argparse.ArgumentParser(
            description='Score logical consistency of statements using wreath-sheaf cohomology',
            formatter_class=argparse.RawDescriptionHelpFormatter
        )

        parser.add_argument('-f', '--file',
                          help='Read statements from file')
        parser.add_argument('-v', '--verbose',
                          action='store_true',
                          help='Show detailed analysis')
        parser.add_argument('-j', '--json',
                          action='store_true',
                          help='Output in JSON format')
        parser.add_argument('--no-banner',
                          action='store_true',
                          help='Skip the banner')

        args = parser.parse_args()

        # Print banner unless suppressed
        if not args.json and not args.no_banner:
            self.print_banner()

        # Read input
        text = self.read_input(args.file)

        if not text.strip():
            print("Error: No input provided.")
            sys.exit(1)

        # Score consistency
        if not args.json:
            print("Analyzing logical consistency...")

        result = self.scorer.compute_consistency_score(text)

        # Format and print result
        formatted = self.format_result(result, args.verbose, args.json)
        print(formatted)

        # Exit with non-zero code if consistency is very low
        if result['score'] < 30:
            sys.exit(2)


def main():
    """Main entry point."""
    cli = ConsistencyCLI()
    cli.run()


if __name__ == "__main__":
    main()