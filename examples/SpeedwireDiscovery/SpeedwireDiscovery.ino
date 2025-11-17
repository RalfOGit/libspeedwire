/**
 * SMA Speedwire Device Discovery Example for ESP32
 *
 * This example demonstrates how to discover SMA Speedwire devices
 * (solar inverters and energy meters) on your local network.
 *
 * Hardware: ESP32 (any variant)
 *
 * Setup:
 * 1. Update WiFi credentials below
 * 2. Make sure your ESP32 is on the same network as your SMA devices
 * 3. Upload and open Serial Monitor at 115200 baud
 */

#include <WiFi.h>
#include <LocalHost.hpp>
#include <SpeedwireDiscovery.hpp>
#include <SpeedwireSocket.hpp>
#include <SpeedwireDevice.hpp>
#include <Logger.hpp>

using namespace libspeedwire;

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Speedwire objects
LocalHost* localhost = nullptr;
SpeedwireDiscovery* discovery = nullptr;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\nSMA Speedwire Discovery Example");
  Serial.println("================================\n");

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.println();

  // Initialize LocalHost singleton
  localhost = &LocalHost::getInstance();

  // Update LocalHost with current WiFi info
  localhost->cacheHostname(std::string(WiFi.getHostname()));
  localhost->cacheLocalIPAddresses(LocalHost::queryLocalIPAddresses());
  localhost->cacheLocalInterfaceInfos(LocalHost::queryLocalInterfaceInfos());

  // Create discovery instance
  discovery = new SpeedwireDiscovery(*localhost);

  Serial.println("Starting device discovery...\n");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  // Perform discovery
  Serial.println("Discovering Speedwire devices...");
  const std::vector<SpeedwireInfo>& devices = discovery->discoverDevices();

  if (devices.empty()) {
    Serial.println("No Speedwire devices found.");
  } else {
    Serial.printf("Found %d device(s):\n\n", devices.size());

    for (const auto& device : devices) {
      Serial.println("--------------------------------");
      Serial.printf("Device: %s\n", device.deviceAddress.toString().c_str());
      Serial.printf("  Serial: %u\n", device.serialNumber);
      Serial.printf("  SusyID: %u\n", device.susyID);
      Serial.printf("  IP: %s\n", device.peer.toString().c_str());

      // Decode device class and type if available
      uint32_t deviceClass = (device.deviceClass >> 24) & 0xFF;
      Serial.printf("  Class: 0x%02X ", deviceClass);
      switch (deviceClass) {
        case 0x00: Serial.println("(Unknown)"); break;
        case 0x01: Serial.println("(Solar Inverter)"); break;
        case 0x02: Serial.println("(Energy Meter)"); break;
        default: Serial.println("(Other)"); break;
      }

      Serial.println();
    }
  }

  // Wait 30 seconds before next discovery
  Serial.println("Waiting 30 seconds before next scan...\n");
  delay(30000);
}
