#include "openxr_minimal.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <vector>
#include <iomanip>
#include <exception>

// ============================================================================
// CONFIGURATION
// ============================================================================
// Set the target Manus Core IP address here
const std::string TARGET_MANUS_CORE_IP = "172.16.25.99";

// Optional: Set a custom name for the target (for display purposes only)
const std::string TARGET_MANUS_CORE_NAME = "Target Manus Core";
// ============================================================================

// Simple hand tracking only application
class HandTrackingOnlyApp {
public:
    bool Initialize() {
        std::cout << "Hand Tracking Only Application" << std::endl;
        std::cout << "===============================" << std::endl;
        std::cout << "Target Manus Core IP: " << TARGET_MANUS_CORE_IP << std::endl;
        std::cout << "===============================" << std::endl;
        
        // Create OpenXR instance
        if (!CreateInstance()) {
            return false;
        }
        
        // Get system
        if (!GetSystem()) {
            return false;
        }
        
        // Initialize hand tracking (includes Manus connection)
        if (!InitializeHandTracking()) {
            return false;
        }
        
        // Create session for hand tracking
        if (!CreateSimpleSession()) {
            return false;
        }
        
        // Create hand trackers
        if (!CreateHandTrackers()) {
            return false;
        }
        
        // Now document the Manus connections we can see
        std::cout << "=== MANUS CORE CONNECTION STATUS ===" << std::endl;
        std::cout << "Target IP: " << TARGET_MANUS_CORE_IP << " (" << TARGET_MANUS_CORE_NAME << ")" << std::endl;
        std::cout << "Remote connection function: " << (m_xrConnectToRemoteMANUSCore ? "Available" : "Not available") << std::endl;
        std::cout << "Note: Manus SDK appears to be auto-discovering and connecting to available cores" << std::endl;
        std::cout << "Check the logs above for 'Client) Got availability response' messages" << std::endl;
        std::cout << "====================================" << std::endl;
        
        // Skip the direct function call for now to avoid crashes
        // TODO: Investigate correct function signature and calling context
        /*
        if (m_xrConnectToRemoteMANUSCore != nullptr) {
            std::cout << "Attempting to connect to specific Manus Core at " << TARGET_MANUS_CORE_IP << "..." << std::endl;
            
            try {
                XrResult result = m_xrConnectToRemoteMANUSCore(TARGET_MANUS_CORE_IP);
                if (XR_SUCCEEDED(result)) {
                    std::cout << "✅ Successfully connected to remote Manus Core at " << TARGET_MANUS_CORE_IP << "!" << std::endl;
                } else {
                    std::cout << "⚠️  Failed to connect to remote Manus Core at " << TARGET_MANUS_CORE_IP << " (result: " << result << ")" << std::endl;
                    std::cout << "   Continuing with default/auto-discovery connection..." << std::endl;
                }
            } catch (...) {
                std::cout << "⚠️  Exception occurred while connecting to remote Manus Core" << std::endl;
                std::cout << "   Continuing with default connection..." << std::endl;
            }
        } else {
            std::cout << "Using default Manus Core connection (auto-discovery)" << std::endl;
        }
        */
        
        return true;
    }
    
    void Run() {
        std::cout << "Starting continuous hand tracking loop..." << std::endl;
        std::cout << "Press Ctrl+C to exit" << std::endl;
        std::cout << "======================================" << std::endl;
        
        int frameCount = 0;
        auto lastLogTime = std::chrono::steady_clock::now();
        int lastFrameCount = 0;
        bool manusConnectionLogged = false;
        bool remoteConnectionAttempted = false;
        
        while (true) {  // Run continuously
            // Poll events
            XrEventDataBuffer eventData{XR_TYPE_EVENT_DATA_BUFFER};
            while (xrPollEvent(m_instance, &eventData) == XR_SUCCESS) {
                if (eventData.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
                    XrEventDataSessionStateChanged* stateEvent = 
                        reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
                    std::cout << "[Frame " << frameCount << "] Session state changed to: " << stateEvent->state << std::endl;
                }
            }
            
            // Log Manus connection details once after a few frames
            if (!manusConnectionLogged && frameCount == 50) {
                std::cout << "\n=== MANUS CONNECTION ANALYSIS ===" << std::endl;
                std::cout << "Based on the logs above, check for:" << std::endl;
                std::cout << "- Lines containing 'CoreSdkWrapper_UnrecognizedClient_XXXXX is connecting to'" << std::endl;
                std::cout << "- Look for target IP " << TARGET_MANUS_CORE_IP << " in the connection logs" << std::endl;
                std::cout << "- The 'Connected:' message shows successful connection details" << std::endl;
                std::cout << "=================================\n" << std::endl;
                manusConnectionLogged = true;
            }
            
            // Get hand tracking data
            if (m_handTrackingSupported) {
                UpdateHandTracking(frameCount);
            }
            
            frameCount++;
            
            // Log frame rate every 5 seconds
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastLogTime);
            if (elapsed.count() >= 5) {
                double intervalFrames = frameCount - lastFrameCount;
                double fps = intervalFrames / elapsed.count();
                std::cout << "[Status] Frame " << frameCount << " - Running at ~" << fps << " FPS" << std::endl;
                lastLogTime = currentTime;
                lastFrameCount = frameCount;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));  // 20 FPS
        }
    }
    
    void Shutdown() {
        if (m_handTrackers[0] != XR_NULL_HANDLE) {
            m_xrDestroyHandTrackerEXT(m_handTrackers[0]);
        }
        if (m_handTrackers[1] != XR_NULL_HANDLE) {
            m_xrDestroyHandTrackerEXT(m_handTrackers[1]);
        }
        if (m_session != XR_NULL_HANDLE) {
            xrDestroySession(m_session);
        }
        if (m_instance != XR_NULL_HANDLE) {
            xrDestroyInstance(m_instance);
        }
    }

private:
    XrInstance m_instance = XR_NULL_HANDLE;
    XrSystemId m_systemId = XR_NULL_SYSTEM_ID;
    XrSession m_session = XR_NULL_HANDLE;
    XrSpace m_appSpace = XR_NULL_HANDLE;
    
    bool m_handTrackingSupported = false;
    XrHandTrackerEXT m_handTrackers[2] = {XR_NULL_HANDLE, XR_NULL_HANDLE};
    
    // Manus-specific function pointers
    PFN_xrCreateHandTrackerEXT m_xrCreateHandTrackerEXT = nullptr;
    PFN_xrDestroyHandTrackerEXT m_xrDestroyHandTrackerEXT = nullptr;
    PFN_xrLocateHandJointsEXT m_xrLocateHandJointsEXT = nullptr;
    
    // Manus remote connection function pointer
    typedef XrResult (*PFN_xrConnectToRemoteMANUSCore)(std::string);
    
    PFN_xrConnectToRemoteMANUSCore m_xrConnectToRemoteMANUSCore = nullptr;
    
    bool CreateInstance() {
        // Enumerate extensions
        uint32_t extensionCount;
        xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr);
        std::vector<XrExtensionProperties> availableExtensions(extensionCount,
                                                              {XR_TYPE_EXTENSION_PROPERTIES});
        xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount,
                                             availableExtensions.data());

        // Check for required extensions
        bool hasHandTracking = false;
        bool hasHeadless = false;
        
        for (const auto& ext : availableExtensions) {
            if (strcmp(ext.extensionName, XR_EXT_HAND_TRACKING_EXTENSION_NAME) == 0) {
                hasHandTracking = true;
            }
            if (strcmp(ext.extensionName, "XR_MND_headless") == 0) {
                hasHeadless = true;
            }
        }
        
        std::vector<const char*> extensions;
        if (hasHeadless) {
            extensions.push_back("XR_MND_headless");
            std::cout << "Using headless mode" << std::endl;
        }
        if (hasHandTracking) {
            extensions.push_back(XR_EXT_HAND_TRACKING_EXTENSION_NAME);
            m_handTrackingSupported = true;
            std::cout << "Hand tracking available" << std::endl;
        }
        
        XrInstanceCreateInfo createInfo{XR_TYPE_INSTANCE_CREATE_INFO};
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.enabledExtensionNames = extensions.data();
        strcpy(createInfo.applicationInfo.applicationName, "Hand Tracking Only");
        createInfo.applicationInfo.applicationVersion = 1;
        strcpy(createInfo.applicationInfo.engineName, "Simple Engine");
        createInfo.applicationInfo.engineVersion = 1;
        createInfo.applicationInfo.apiVersion = XR_MAKE_VERSION(1, 0, 0);

        XrResult result = xrCreateInstance(&createInfo, &m_instance);
        if (XR_FAILED(result)) {
            std::cerr << "Failed to create OpenXR instance" << std::endl;
            return false;
        }
        
        std::cout << "OpenXR instance created successfully" << std::endl;
        return true;
    }
    
    bool GetSystem() {
        XrSystemGetInfo systemInfo{XR_TYPE_SYSTEM_GET_INFO};
        systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

        XrResult result = xrGetSystem(m_instance, &systemInfo, &m_systemId);
        if (XR_FAILED(result)) {
            std::cerr << "Failed to get system" << std::endl;
            return false;
        }
        
        std::cout << "OpenXR system obtained" << std::endl;
        return true;
    }
    
    bool InitializeHandTracking() {
        if (!m_handTrackingSupported) {
            std::cout << "Hand tracking not supported" << std::endl;
            return true;
        }
        
        // Get standard OpenXR hand tracking function pointers
        XrResult result = xrGetInstanceProcAddr(m_instance, "xrCreateHandTrackerEXT",
                                              (PFN_xrVoidFunction*)&m_xrCreateHandTrackerEXT);
        if (XR_FAILED(result)) {
            std::cerr << "Failed to get xrCreateHandTrackerEXT" << std::endl;
            return false;
        }

        result = xrGetInstanceProcAddr(m_instance, "xrDestroyHandTrackerEXT",
                                     (PFN_xrVoidFunction*)&m_xrDestroyHandTrackerEXT);
        if (XR_FAILED(result)) {
            std::cerr << "Failed to get xrDestroyHandTrackerEXT" << std::endl;
            return false;
        }

        result = xrGetInstanceProcAddr(m_instance, "xrLocateHandJointsEXT",
                                     (PFN_xrVoidFunction*)&m_xrLocateHandJointsEXT);
        if (XR_FAILED(result)) {
            std::cerr << "Failed to get xrLocateHandJointsEXT" << std::endl;
            return false;
        }
        
        // Get Manus-specific remote connection function pointer
        result = xrGetInstanceProcAddr(m_instance, "xrConnectToRemoteMANUSCore",
                                     (PFN_xrVoidFunction*)&m_xrConnectToRemoteMANUSCore);
        if (XR_FAILED(result)) {
            std::cout << "Warning: Failed to get xrConnectToRemoteMANUSCore function - using default connection" << std::endl;
            m_xrConnectToRemoteMANUSCore = nullptr;
        } else {
            std::cout << "Manus remote connection function obtained - attempting immediate connection" << std::endl;
            
            // Try to connect to specific IP right after getting the function pointer
            std::cout << "\n=== ATTEMPTING REMOTE CONNECTION (EARLY) ===" << std::endl;
            std::cout << "Connecting to target IP: " << TARGET_MANUS_CORE_IP << " (" << TARGET_MANUS_CORE_NAME << ")" << std::endl;
            
            try {
                XrResult connectResult = m_xrConnectToRemoteMANUSCore(TARGET_MANUS_CORE_IP);
                if (XR_SUCCEEDED(connectResult)) {
                    std::cout << "✅ Successfully connected to remote Manus Core at " << TARGET_MANUS_CORE_IP << "!" << std::endl;
                } else {
                    std::cout << "⚠️ Remote connection returned: " << connectResult << " (0x" << std::hex << connectResult << std::dec << ")" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "❌ Exception during remote connection: " << e.what() << std::endl;
            } catch (...) {
                std::cout << "❌ Unknown exception during remote connection" << std::endl;
            }
            
            std::cout << "=== REMOTE CONNECTION ATTEMPT COMPLETED ===" << std::endl;
        }
        
        std::cout << "Hand tracking function pointers obtained" << std::endl;
        return true;
    }
    
    bool CreateSimpleSession() {
        XrSessionCreateInfo sessionInfo{XR_TYPE_SESSION_CREATE_INFO};
        sessionInfo.systemId = m_systemId;
        // No graphics binding for headless mode

        XrResult result = xrCreateSession(m_instance, &sessionInfo, &m_session);
        if (XR_FAILED(result)) {
            std::cerr << "Failed to create session" << std::endl;
            return false;
        }

        // Create reference space
        XrReferenceSpaceCreateInfo spaceInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
        spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
        spaceInfo.poseInReferenceSpace = {{0, 0, 0, 1}, {0, 0, 0}};

        result = xrCreateReferenceSpace(m_session, &spaceInfo, &m_appSpace);
        if (XR_FAILED(result)) {
            std::cerr << "Failed to create reference space" << std::endl;
            return false;
        }
        
        std::cout << "Session and reference space created" << std::endl;
        return true;
    }
    
    bool CreateHandTrackers() {
        if (!m_handTrackingSupported) {
            return true;
        }
        
        // Create hand trackers for left and right hands
        for (int hand = 0; hand < 2; ++hand) {
            XrHandTrackerCreateInfoEXT createInfo{XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT};
            createInfo.hand = (hand == 0) ? XR_HAND_LEFT_EXT : XR_HAND_RIGHT_EXT;
            createInfo.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;

            XrResult result = m_xrCreateHandTrackerEXT(m_session, &createInfo, &m_handTrackers[hand]);
            if (XR_FAILED(result)) {
                std::cerr << "Failed to create hand tracker for " 
                          << (hand == 0 ? "left" : "right") << " hand" << std::endl;
                return false;
            }
        }
        
        std::cout << "Hand trackers created for both hands" << std::endl;

        return true;
    }
    
    void UpdateHandTracking(int frameCount) {
        static bool lastHandActive[2] = {false, false};
        static auto lastDataTime = std::chrono::steady_clock::now();
        
        for (int hand = 0; hand < 2; ++hand) {
            if (m_handTrackers[hand] == XR_NULL_HANDLE) continue;
            
            XrHandJointLocationEXT jointLocations[XR_HAND_JOINT_COUNT_EXT];
            XrHandJointLocationsEXT locations{XR_TYPE_HAND_JOINT_LOCATIONS_EXT};
            locations.jointCount = XR_HAND_JOINT_COUNT_EXT;
            locations.jointLocations = jointLocations;

            XrHandJointsLocateInfoEXT locateInfo{XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT};
            locateInfo.baseSpace = m_appSpace;
            locateInfo.time = 0;  // Use current time

            XrResult result = m_xrLocateHandJointsEXT(m_handTrackers[hand], &locateInfo, &locations);
            
            const char* handName = (hand == 0) ? "LEFT" : "RIGHT";
            
            if (XR_SUCCEEDED(result)) {
                if (locations.isActive) {
                    // Log when hand becomes active
                    if (!lastHandActive[hand]) {
                        std::cout << "[Frame " << frameCount << "] " << handName << " hand became ACTIVE" << std::endl;
                        lastHandActive[hand] = true;
                    }
                    
                    // Log detailed joint data every 40 frames (~2 seconds at 20fps)
                    if (frameCount % 40 == 0) {
                        std::cout << "\n[Frame " << frameCount << "] " << handName << " HAND TRACKING DATA:" << std::endl;
                        std::cout << "  Active: YES, Joint Count: " << locations.jointCount << std::endl;
                        
                        // Log key joints
                        const struct {
                            XrHandJointEXT joint;
                            const char* name;
                        } keyJoints[] = {
                            {XR_HAND_JOINT_WRIST_EXT, "Wrist"},
                            {XR_HAND_JOINT_PALM_EXT, "Palm"},
                            {XR_HAND_JOINT_THUMB_TIP_EXT, "Thumb Tip"},
                            {XR_HAND_JOINT_INDEX_TIP_EXT, "Index Tip"},
                            {XR_HAND_JOINT_MIDDLE_TIP_EXT, "Middle Tip"},
                            {XR_HAND_JOINT_RING_TIP_EXT, "Ring Tip"},
                            {XR_HAND_JOINT_LITTLE_TIP_EXT, "Little Tip"}
                        };
                        
                        for (const auto& keyJoint : keyJoints) {
                            if (keyJoint.joint < locations.jointCount) {
                                auto& joint = jointLocations[keyJoint.joint];
                                std::cout << "    " << keyJoint.name << ": ";
                                
                                if (joint.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) {
                                    std::cout << "pos(" 
                                              << std::fixed << std::setprecision(3)
                                              << joint.pose.position.x << ", "
                                              << joint.pose.position.y << ", "
                                              << joint.pose.position.z << ") ";
                                } else {
                                    std::cout << "pos(INVALID) ";
                                }
                                
                                if (joint.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) {
                                    std::cout << "rot(" 
                                              << std::fixed << std::setprecision(3)
                                              << joint.pose.orientation.x << ", "
                                              << joint.pose.orientation.y << ", "
                                              << joint.pose.orientation.z << ", "
                                              << joint.pose.orientation.w << ")";
                                } else {
                                    std::cout << "rot(INVALID)";
                                }
                                
                                std::cout << " radius(" << joint.radius << ")" << std::endl;
                            }
                        }
                        std::cout << std::endl;
                    }
                    
                    // Quick status every 10 frames
                    else if (frameCount % 10 == 0) {
                        if (locations.jointCount > XR_HAND_JOINT_PALM_EXT) {
                            auto& palm = jointLocations[XR_HAND_JOINT_PALM_EXT];
                            if (palm.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) {
                                std::cout << "[Frame " << frameCount << "] " << handName 
                                          << " palm: (" << std::fixed << std::setprecision(3)
                                          << palm.pose.position.x << ", "
                                          << palm.pose.position.y << ", "
                                          << palm.pose.position.z << ")" << std::endl;
                            }
                        }
                    }
                } else {
                    // Log when hand becomes inactive
                    if (lastHandActive[hand]) {
                        std::cout << "[Frame " << frameCount << "] " << handName << " hand became INACTIVE" << std::endl;
                        lastHandActive[hand] = false;
                    }
                }
            } else {
                std::cout << "[Frame " << frameCount << "] " << handName << " hand tracking FAILED with result: " << result << std::endl;
                lastHandActive[hand] = false;
            }
        }
    }
};

int main() {
    HandTrackingOnlyApp app;
    
    if (!app.Initialize()) {
        std::cerr << "Failed to initialize hand tracking app" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Shutdown();
    
    return 0;
}
