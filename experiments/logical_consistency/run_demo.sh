#!/bin/bash

# Logical Consistency Scorer Demo
# One-click demo to show the tool in action

echo "=================================================="
echo "    LOGICAL CONSISTENCY SCORER DEMO"
echo "    Powered by Wreath-Sheaf Cohomology"
echo "=================================================="
echo ""

# Check if Python 3 is installed
if ! command -v python3 &> /dev/null; then
    echo "Error: Python 3 is required but not installed."
    echo "Please install Python 3 and try again."
    exit 1
fi

# Check if required packages are installed
echo "Checking dependencies..."
python3 -c "import numpy, scipy" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "Installing required packages..."
    pip3 install -r requirements.txt
fi

echo ""
echo "Running demos..."
echo ""

# Demo 1: Consistent statements
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "DEMO 1: Perfectly Consistent Knowledge"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
cat examples/consistent.txt | grep -v "^#" | head -5
echo "..."
echo ""
python3 consistency_cli.py -f examples/consistent.txt --no-banner
echo ""

# Demo 2: Contradictory statements
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "DEMO 2: Statements with Contradictions"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
cat examples/contradictions.txt | grep -v "^#" | head -5
echo "..."
echo ""
python3 consistency_cli.py -f examples/contradictions.txt --no-banner
echo ""

# Demo 3: Complex relationships
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "DEMO 3: Complex Knowledge Structure"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
cat examples/complex.txt | grep -v "^#" | head -5
echo "..."
echo ""
python3 consistency_cli.py -f examples/complex.txt --no-banner
echo ""

# Demo 4: Digital brain simulation
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "DEMO 4: Digital Brain Thoughts"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
cat examples/digital_brain.txt | grep -v "^#" | head -5
echo "..."
echo ""
python3 consistency_cli.py -f examples/digital_brain.txt --no-banner
echo ""

# Interactive demo
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "INTERACTIVE: Try Your Own Statements"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Enter logical statements (one per line)."
echo "Press Ctrl+D when done:"
echo ""
python3 consistency_cli.py --no-banner

echo ""
echo "=================================================="
echo "Demo complete! The cohomological obstruction"
echo "quantifies logical inconsistency mathematically."
echo ""
echo "Try: python3 consistency_cli.py --help"
echo "=================================================="