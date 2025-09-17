#!/bin/bash

echo "=================================="
echo "Hand Tracking Only Test"
echo "=================================="

# Set environment variables for Manus API layer (using relative paths)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Disable setting these variables to use system-wide implicit layer installation
#export LD_LIBRARY_PATH="$SCRIPT_DIR/APILAYER:$LD_LIBRARY_PATH"
#export XR_API_LAYER_PATH="$SCRIPT_DIR/APILAYER"
#export XR_ENABLE_API_LAYERS="XR_APILAYER_MANUS_handtracking"
export XR_RUNTIME_JSON="/home/manus/.local/share/Steam/steamapps/common/SteamVR/steamxr_linux64.json"

echo "Environment variables set:"
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
echo "XR_API_LAYER_PATH: $XR_API_LAYER_PATH"
echo "XR_ENABLE_API_LAYERS: $XR_ENABLE_API_LAYERS"
echo "XR_RUNTIME_JSON: $XR_RUNTIME_JSON"
echo ""

echo "Running hand tracking only test..."
./test_handtracking_only

echo "Test completed!"
