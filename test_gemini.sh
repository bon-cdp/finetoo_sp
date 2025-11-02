#!/bin/bash
# Quick test script for Gemini BOM generation

export FINETOO_GCP_PROJECT=finetoo-475619
export FINETOO_GCP_LOCATION=us-central1

echo "════════════════════════════════════════════════════════════"
echo " Testing Finetoo + Gemini Integration"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "Project: $FINETOO_GCP_PROJECT"
echo "Location: $FINETOO_GCP_LOCATION"
echo ""

# Run the demo
bazel run //tools:demo_llm_bom -- \
    /home/shakil/Documents/finetoo_sp/binder_test_no_refs/18066-G-300.dxf \
    "Generate a bill of materials"
