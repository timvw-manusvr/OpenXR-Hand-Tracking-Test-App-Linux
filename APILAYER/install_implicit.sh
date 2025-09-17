#!/bin/bash

echo "=================================="
echo "Installing Manus API Layer as Implicit"
echo "=================================="

# Create system API layer directory if it doesn't exist
sudo mkdir -p /usr/share/openxr/1/api_layers/implicit.d/

# Copy the API layer manifest to system location
sudo cp XR_APILAYER_MANUS_handtracking.json /usr/share/openxr/1/api_layers/implicit.d/

# Copy the library files to system location  
sudo mkdir -p /usr/lib/openxr/
sudo cp libXR_APILAYER_MANUS_handtracking.so /usr/lib/openxr/
sudo cp libManusSDK.so /usr/lib/openxr/

# Update the manifest to point to absolute paths
sudo sed -i 's|"library_path": "./libXR_APILAYER_MANUS_handtracking.so"|"library_path": "/usr/lib/openxr/libXR_APILAYER_MANUS_handtracking.so"|' /usr/share/openxr/1/api_layers/implicit.d/XR_APILAYER_MANUS_handtracking_Linux.json

echo ""
echo "✅ Manus API layer installed as implicit!"
echo "Now the layer will be automatically loaded by all OpenXR applications."
echo ""
echo "To uninstall:"
echo "sudo rm /usr/share/openxr/1/api_layers/implicit.d/XR_APILAYER_MANUS_handtracking_Linux.json"
echo "sudo rm /usr/lib/openxr/libXR_APILAYER_MANUS_handtracking.so"
echo "sudo rm /usr/lib/openxr/libManusSDK.so"
