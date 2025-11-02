# Gemini Integration Setup Guide

This guide shows you how to set up Google Cloud for the Finetoo natural language BOM generation demo.

## Prerequisites

- Google Cloud account
- `gcloud` CLI installed ([installation guide](https://cloud.google.com/sdk/docs/install))
- A Google Cloud project

## Step 1: Set Your Project

```bash
# Set your active project
gcloud config set project YOUR_PROJECT_ID

# Or create a new project
gcloud projects create finetoo-demo --name="Finetoo Demo"
gcloud config set project finetoo-demo
```

## Step 2: Enable Vertex AI API

```bash
# Enable the Vertex AI API
gcloud services enable aiplatform.googleapis.com
```

This may take a few minutes. You can check the status:

```bash
gcloud services list --enabled | grep aiplatform
```

## Step 3: Authenticate

### Option A: Application Default Credentials (Recommended for Demo)

This is the simplest approach for local development:

```bash
gcloud auth application-default login
```

This will open a browser window for you to authenticate. Once complete, your credentials will be stored locally.

### Option B: Service Account (For Production)

If you prefer to use a service account:

```bash
# Create service account
gcloud iam service-accounts create finetoo-vertex-ai \
    --display-name="Finetoo Vertex AI Client"

# Grant Vertex AI User role
gcloud projects add-iam-policy-binding YOUR_PROJECT_ID \
    --member="serviceAccount:finetoo-vertex-ai@YOUR_PROJECT_ID.iam.gserviceaccount.com" \
    --role="roles/aiplatform.user"

# Create and download credentials
gcloud iam service-accounts keys create ~/finetoo-credentials.json \
    --iam-account=finetoo-vertex-ai@YOUR_PROJECT_ID.iam.gserviceaccount.com

# Set environment variable
export GOOGLE_APPLICATION_CREDENTIALS=~/finetoo-credentials.json
```

## Step 4: Set Environment Variables

```bash
# Required: Your GCP project ID
export FINETOO_GCP_PROJECT=YOUR_PROJECT_ID

# Optional: Location (defaults to us-central1)
export FINETOO_GCP_LOCATION=us-central1

# Add to your ~/.bashrc or ~/.zshrc to make permanent
echo 'export FINETOO_GCP_PROJECT=YOUR_PROJECT_ID' >> ~/.bashrc
echo 'export FINETOO_GCP_LOCATION=us-central1' >> ~/.bashrc
```

## Step 5: Verify Setup

Test that authentication works:

```bash
gcloud auth application-default print-access-token
```

You should see a long access token printed. If you get an error, re-run:

```bash
gcloud auth application-default login
```

## Step 6: Run the Demo!

```bash
# Build the demo
bazel build //tools:demo_llm_bom

# Run with a DXF file
bazel run //tools:demo_llm_bom -- \
    /home/shakil/Documents/finetoo_sp/binder_test_no_refs/18066-G-300.dxf \
    "Generate a bill of materials"
```

## Troubleshooting

### Error: "No access token"

**Solution:** Run `gcloud auth application-default login`

### Error: "Permission denied" or "403 Forbidden"

**Solution:** Make sure Vertex AI API is enabled:
```bash
gcloud services enable aiplatform.googleapis.com
```

### Error: "FINETOO_GCP_PROJECT environment variable not set"

**Solution:** Set the environment variable:
```bash
export FINETOO_GCP_PROJECT=your-project-id
```

### Error: "Project not found"

**Solution:** Verify your project exists:
```bash
gcloud projects list
gcloud config set project YOUR_PROJECT_ID
```

## Cost Considerations

**Gemini 1.5 Pro Pricing** (as of January 2025):
- Input: $0.00125 per 1,000 characters
- Output: $0.005 per 1,000 characters

**Estimated cost per BOM query:**
- Prompt size: ~2,000 characters = $0.0025
- Response size: ~500 characters = $0.0025
- **Total per query: ~$0.005 (half a cent)**

For this demo with ~10 test queries: **< $0.05 total**

## Security Best Practices

1. **Never commit credentials** to git
2. **Use service accounts** for production (not your personal account)
3. **Limit IAM roles** to only what's needed (`roles/aiplatform.user`)
4. **Rotate keys regularly** if using service account keys
5. **Use Workload Identity** if deploying to GKE/Cloud Run

## Next Steps

Once setup is complete, you're ready to demonstrate:

**"Beyond Fine-Tuning"** - Natural language → Schema-driven operation discovery → Gemini composes operations → BOM output!

See [../README.md](../README.md) for the full project documentation.
