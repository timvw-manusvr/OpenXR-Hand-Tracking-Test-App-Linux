# Manus Hand Tracking Test

A standalone OpenXR hand tracking application that works with Manus gloves without requiring SteamVR room setup.

## Features

- **Headless Mode**: Runs without VR headset or room setup
- **Manus Integration**: Uses Manus OpenXR API layer for hand tracking
- **Network Discovery**: Automatically discovers Manus Core instances on the network
- **Real-time Tracking**: Provides continuous hand joint data at 20 FPS
- **Multiple Cores**: Supports connecting to specific Manus Core IPs

## Files

- `test_handtracking_only.cpp` - Main application source code
- `test_handtracking_only.sh` - Script to run with proper environment variables
- `build.sh` - Build script to compile the application
- `APILAYER/` - Manus OpenXR API layer libraries
  - `libXR_APILAYER_MANUS_handtracking.so` - Main API layer
  - `libManusSDK.so` - Manus SDK library
  - `XR_APILAYER_MANUS_handtracking.json` - API layer manifest

## Requirements

- OpenXR runtime (SteamVR)
- OpenXR loader library
- Manus SDK (included in APILAYER folder)
- Linux x86_64 system

## Building

```bash
./build.sh
```

## Running

```bash
./test_handtracking_only.sh
```

Or run directly:
```bash
./test_handtracking_only
```

## Configuration

The application uses a configurable target IP address for connecting to a specific Manus Core. To change the target IP, edit the configuration section at the top of `test_handtracking_only.cpp`:

```cpp
// ============================================================================
// CONFIGURATION
// ============================================================================
// Set the target Manus Core IP address here
const std::string TARGET_MANUS_CORE_IP = "172.16.25.99";

// Optional: Set a custom name for the target (for display purposes only)
const std::string TARGET_MANUS_CORE_NAME = "Target Manus Core";
// ============================================================================
```

After changing the configuration, rebuild the application:
```bash
./build.sh
```

## Output

The application will:
1. Discover available Manus Cores on the network
2. Attempt to connect to the target IP
3. Fall back to auto-discovery if target connection fails
4. Provide real-time hand tracking data
5. Log hand joint positions and orientations

## Environment Variables

The test script sets up the following environment variables:
- `XR_API_LAYER_PATH` - Points to the APILAYER folder
- `XR_ENABLE_API_LAYERS` - Enables the Manus hand tracking layer
- `LD_LIBRARY_PATH` - Includes the APILAYER folder for library loading
- `XR_RUNTIME_JSON` - Points to SteamVR runtime

## Troubleshooting

### No Manus Cores Found
- Ensure Manus Core is running on the network
- Check firewall settings
- Verify network connectivity

### Build Errors
- Ensure OpenXR development packages are installed
- Check that `/usr/local/lib/libopenxr_loader.so` exists
- Install missing dependencies with your package manager

### Runtime Errors
- Ensure SteamVR is installed (even if not running)
- Check that the APILAYER libraries have execute permissions
- Verify OpenXR runtime environment variables are set

## Package Contents

This folder contains everything needed to run the Manus hand tracking test on any compatible Linux system. Simply copy the entire `handtracking_test` folder to the target system and run `./build.sh` followed by `./test_handtracking_only.sh`.
