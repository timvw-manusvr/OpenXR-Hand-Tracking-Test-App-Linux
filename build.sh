#!/bin/bash

echo "Building Manus Hand Tracking Test..."
echo "===================================="

# Build the handtracking test executable
g++ -std=c++17 -o test_handtracking_only test_handtracking_only.cpp -L/usr/local/lib -lopenxr_loader -ldl

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "To run the test:"
    echo "  ./test_handtracking_only.sh"
    echo ""
    echo "Or run directly:"
    echo "  ./test_handtracking_only"
else
    echo "❌ Build failed!"
    exit 1
fi
