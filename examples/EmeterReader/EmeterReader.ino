/**
 * SMA Energy Meter Reader Example for ESP32
 *
 * This example demonstrates how to receive and parse energy meter data
 * from SMA Speedwire energy meters. It displays real-time power consumption,
 * grid feed-in, and energy counters.
 *
 * Hardware: ESP32 (any variant)
 *
 * Setup:
 * 1. Update WiFi credentials below
 * 2. Optionally configure NTP for timestamp synchronization
 * 3. Make sure your ESP32 is on the same network as your SMA energy meter
 * 4. Upload and open Serial Monitor at 115200 baud
 */

#include <WiFi.h>
#include <time.h>
#include <LocalHost.hpp>
#include <SpeedwireSocket.hpp>
#include <SpeedwireSocketFactory.hpp>
#include <SpeedwireReceiveDispatcher.hpp>
#include <SpeedwireHeader.hpp>
#include <SpeedwireEmeterProtocol.hpp>
#include <ObisData.hpp>
#include <Logger.hpp>

using namespace libspeedwire;

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// NTP server for time synchronization
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;      // Adjust for your timezone
const int daylightOffset_sec = 0;  // Adjust for daylight saving

// Speedwire objects
LocalHost* localhost = nullptr;
SpeedwireSocketFactory* factory = nullptr;
std::vector<SpeedwireSocket> sockets;
SpeedwireReceiveDispatcher* dispatcher = nullptr;

// Custom receiver for emeter packets
class EmeterReceiver : public SpeedwireReceiveDispatcher::IReceiver {
public:
  EmeterReceiver() : protocolID(sma_emeter_protocol_id) {}

  virtual void receive(SpeedwireHeader& speedwire_packet, struct sockaddr& src) override {
    // Parse emeter packet
    SpeedwireEmeterProtocol emeter(speedwire_packet);

    Serial.println("\n=== Energy Meter Data ===");
    Serial.printf("Time: %s\n", LocalHost::unixEpochTimeInMsToString(
      LocalHost::getUnixEpochTimeInMs()).c_str());
    Serial.printf("Serial: %u\n", emeter.getSerialNumber());
    Serial.printf("Time: %u\n", emeter.getTime());

    // Get all OBIS data
    std::vector<ObisData> obis_data = emeter.getObisData();

    // Display power values
    Serial.println("\nPower (W):");
    for (const auto& data : obis_data) {
      if (data.measurementType.name.find("positive active power") != std::string::npos) {
        Serial.printf("  %s: %.1f W\n",
          data.measurementType.name.c_str(),
          data.value);
      }
      if (data.measurementType.name.find("negative active power") != std::string::npos) {
        Serial.printf("  %s: %.1f W\n",
          data.measurementType.name.c_str(),
          data.value);
      }
    }

    // Display energy counters
    Serial.println("\nEnergy (Wh):");
    for (const auto& data : obis_data) {
      if (data.measurementType.name.find("positive active energy") != std::string::npos) {
        Serial.printf("  %s: %.1f Wh\n",
          data.measurementType.name.c_str(),
          data.value);
      }
      if (data.measurementType.name.find("negative active energy") != std::string::npos) {
        Serial.printf("  %s: %.1f Wh\n",
          data.measurementType.name.c_str(),
          data.value);
      }
    }

    Serial.println("========================\n");
  }

  const unsigned long protocolID;
};

EmeterReceiver* emeterReceiver = nullptr;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\nSMA Energy Meter Reader Example");
  Serial.println("=================================\n");

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
  Serial.println();

  // Configure NTP time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Waiting for NTP time sync...");
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.println("Time synchronized");
  }

  // Initialize LocalHost singleton
  localhost = &LocalHost::getInstance();

  // Update LocalHost with current WiFi info
  localhost->cacheHostname(std::string(WiFi.getHostname()));
  localhost->cacheLocalIPAddresses(LocalHost::queryLocalIPAddresses());
  localhost->cacheLocalInterfaceInfos(LocalHost::queryLocalInterfaceInfos());

  // Create socket factory and open multicast socket
  factory = new SpeedwireSocketFactory(*localhost);
  const std::vector<std::string>& localIPs = localhost->getLocalIPv4Addresses();

  if (localIPs.empty()) {
    Serial.println("ERROR: No local IP address found!");
    return;
  }

  Serial.printf("Opening socket on interface: %s\n", localIPs[0].c_str());
  sockets = factory->getRecvSockets(localIPs, SpeedwireSocketFactory::MULTICAST);

  if (sockets.empty()) {
    Serial.println("ERROR: Failed to open socket!");
    return;
  }

  Serial.printf("Socket opened successfully (fd: %d)\n", sockets[0].getSocketFd());

  // Create dispatcher and register emeter receiver
  dispatcher = new SpeedwireReceiveDispatcher(*localhost);
  emeterReceiver = new EmeterReceiver();
  dispatcher->registerReceiver(*emeterReceiver);

  Serial.println("\nListening for energy meter packets...\n");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  if (sockets.empty()) {
    Serial.println("No sockets available!");
    delay(1000);
    return;
  }

  // Dispatch incoming packets (timeout: 1000ms)
  int result = dispatcher->dispatch(sockets, 1000);

  if (result < 0) {
    Serial.println("Error receiving data");
    delay(1000);
  }
  // If result == 0, it's just a timeout, continue listening
}
