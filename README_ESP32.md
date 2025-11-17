# Speedwire Library for ESP32

Port của thư viện SMA Speedwire cho ESP32/Arduino. Thư viện này cho phép ESP32 giao tiếp với các thiết bị SMA Speedwire như inverter năng lượng mặt trời và đồng hồ điện thông qua giao thức UDP multicast.

## Tính năng

- ✅ Hỗ trợ ESP32 với Arduino framework
- ✅ Phát hiện thiết bị SMA Speedwire tự động
- ✅ Đọc dữ liệu năng lượng thời gian thực từ energy meter
- ✅ Truy vấn dữ liệu từ solar inverter
- ✅ Hỗ trợ IPv4 và IPv6
- ✅ Logging qua Serial Monitor
- ✅ Xử lý gói tin OBIS (Object Identification System)

## Yêu cầu phần cứng

- ESP32 (bất kỳ biến thể nào: ESP32, ESP32-S2, ESP32-S3, ESP32-C3)
- Kết nối WiFi
- Thiết bị SMA Speedwire trên cùng mạng LAN

## Cài đặt

### Cách 1: Cài đặt từ thư mục local (khuyến nghị cho development)

1. Clone repository này:
```bash
git clone https://github.com/lthquy/libspeedwire-arduino.git
```

2. Copy thư mục vào Arduino libraries:
```bash
# Windows
xcopy /E /I libspeedwire-arduino "%USERPROFILE%\Documents\Arduino\libraries\Speedwire"

# macOS/Linux
cp -r libspeedwire-arduino ~/Arduino/libraries/Speedwire
```

3. Khởi động lại Arduino IDE

### Cách 2: Cài đặt qua Arduino Library Manager (sau khi publish)

1. Mở Arduino IDE
2. Vào **Sketch** → **Include Library** → **Manage Libraries**
3. Tìm kiếm "Speedwire"
4. Click **Install**

## Cấu hình Arduino IDE

### Board Settings

1. Cài đặt ESP32 board support:
   - File → Preferences
   - Thêm URL vào "Additional Boards Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Tools → Board → Boards Manager
   - Tìm "esp32" và cài đặt

2. Chọn board ESP32 của bạn:
   - Tools → Board → ESP32 Arduino → (chọn board của bạn)

3. Cấu hình Partition Scheme (nếu gặp lỗi memory):
   - Tools → Partition Scheme → "Minimal SPIFFS (1.9MB APP with OTA)"

## Ví dụ sử dụng

### 1. Device Discovery (Phát hiện thiết bị)

```cpp
#include <WiFi.h>
#include <LocalHost.hpp>
#include <SpeedwireDiscovery.hpp>

using namespace libspeedwire;

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  LocalHost& localhost = LocalHost::getInstance();
  SpeedwireDiscovery discovery(localhost);

  const std::vector<SpeedwireInfo>& devices = discovery.discoverDevices();

  for (const auto& device : devices) {
    Serial.printf("Found device: %s (Serial: %u)\n",
                  device.deviceAddress.toString().c_str(),
                  device.serialNumber);
  }
}

void loop() {}
```

### 2. Energy Meter Reader (Đọc dữ liệu năng lượng)

Xem file `examples/EmeterReader/EmeterReader.ino` để biết ví dụ đầy đủ.

## API Reference

### LocalHost

Quản lý thông tin về network interface local:

```cpp
LocalHost& localhost = LocalHost::getInstance();
localhost.getLocalIPAddresses();      // Lấy danh sách IP
localhost.getMacAddress(ip);          // Lấy MAC address
localhost.getHostname();               // Lấy hostname
LocalHost::sleep(1000);                // Sleep 1 giây
LocalHost::getTickCountInMs();         // Lấy uptime (ms)
LocalHost::getUnixEpochTimeInMs();     // Lấy thời gian Unix (ms)
```

### SpeedwireSocket

Quản lý UDP socket cho giao tiếp Speedwire:

```cpp
SpeedwireSocket socket(localhost);
socket.openSocket(local_ip, true);     // true = multicast
socket.send(buffer, size);             // Gửi broadcast
socket.sendto(buffer, size, dest_ip);  // Gửi unicast
socket.recvfrom(buffer, size, src);    // Nhận dữ liệu
socket.closeSocket();
```

### SpeedwireDiscovery

Phát hiện thiết bị Speedwire:

```cpp
SpeedwireDiscovery discovery(localhost);
const std::vector<SpeedwireInfo>& devices = discovery.discoverDevices();

for (const auto& device : devices) {
  uint32_t serial = device.serialNumber;
  uint16_t susyID = device.susyID;
  std::string ip = device.peer.toString();
}
```

### SpeedwireEmeterProtocol

Parse dữ liệu energy meter:

```cpp
SpeedwireEmeterProtocol emeter(speedwire_packet);
uint32_t serial = emeter.getSerialNumber();
uint32_t time = emeter.getTime();
std::vector<ObisData> data = emeter.getObisData();

for (const auto& obis : data) {
  Serial.printf("%s: %.2f %s\n",
                obis.measurementType.name.c_str(),
                obis.value,
                obis.measurementType.unit.c_str());
}
```

## Các vấn đề thường gặp

### 1. Không tìm thấy thiết bị

**Nguyên nhân:**
- ESP32 và SMA device không cùng subnet
- Firewall chặn UDP port 9522
- Router không forward multicast packets

**Giải pháp:**
- Kiểm tra cả hai thiết bị đều có IP trong cùng dải (VD: 192.168.1.x)
- Tắt firewall tạm thời để test
- Kiểm tra router settings cho IGMP/multicast

### 2. Lỗi compile "cannot convert"

**Nguyên nhân:**
- C++ standard library version mismatch

**Giải pháp:**
Thêm vào `platform.txt` hoặc `boards.txt`:
```
compiler.cpp.extra_flags=-std=gnu++11
```

### 3. Stack overflow / Watchdog reset

**Nguyên nhân:**
- Buffer quá lớn trên stack
- Loop blocking quá lâu

**Giải pháp:**
- Tăng stack size trong menuconfig
- Thêm `delay(1)` hoặc `yield()` trong loop
- Sử dụng FreeRTOS task riêng

### 4. Memory issues (Heap/Stack)

**Giải pháp:**
- Chọn Partition Scheme phù hợp trong Arduino IDE
- Giảm buffer size nếu cần
- Sử dụng PSRAM nếu có (ESP32-WROVER)

### 5. Không nhận được packets

**Kiểm tra:**
```cpp
if (socket.getSocketFd() < 0) {
  Serial.println("Socket open failed!");
}

// Kiểm tra multicast membership
Serial.printf("Listening on: %s:%d\n",
              local_ip.c_str(),
              SpeedwireSocket::speedwire_port_9522);
```

## Cấu trúc thư mục

```
libspeedwire-arduino/
├── examples/
│   ├── SpeedwireDiscovery/    # Ví dụ phát hiện thiết bị
│   └── EmeterReader/           # Ví dụ đọc energy meter
├── include/                    # Header files
│   ├── SpeedwireSocket.hpp
│   ├── LocalHost.hpp
│   ├── SpeedwireEmeterProtocol.hpp
│   └── ...
├── src/                        # Source files
│   ├── SpeedwireSocket.cpp
│   ├── LocalHost.cpp
│   └── ...
├── library.properties          # Arduino library metadata
├── keywords.txt               # IDE syntax highlighting
├── README.md                  # Original README
└── README_ESP32.md           # ESP32-specific README (file này)
```

## Platform-specific Notes

### ESP32 vs Original Library

| Tính năng | Original (Linux/Win) | ESP32 Port |
|-----------|---------------------|------------|
| Sockets | BSD sockets | lwip sockets |
| Network | getifaddrs() | WiFi.localIP() |
| Multicast | Native | lwip IGMP |
| Time | std::chrono | millis()/gettimeofday() |
| Logging | fprintf(stderr) | Serial.print() |
| Threading | std::thread | FreeRTOS tasks |

### Preprocessor Defines

Code ESP32-specific được bao bọc bởi:
```cpp
#ifdef ARDUINO
  // ESP32/Arduino code
#else
  // Original platform code
#endif
```

## Performance

### Memory Usage

- **Flash**: ~200-300KB (tùy thuộc vào features sử dụng)
- **Heap**: ~20-50KB runtime
- **Stack**: ~8-16KB per task

### Network Performance

- **Discovery**: ~2-5 giây
- **Emeter polling**: ~1-2 packets/giây
- **Latency**: <100ms typical

## Contributing

Contributions được chào đón! Vui lòng:

1. Fork repository
2. Tạo feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open Pull Request

## Changelog

### v1.0.0 (2024-11-17)
- ✅ Initial ESP32 port
- ✅ LocalHost adapted for WiFi
- ✅ Socket operations using lwip
- ✅ poll() replaced with lwip_poll()
- ✅ Logging via Serial
- ✅ Example sketches added
- ✅ Documentation in Vietnamese

## License

Giống với thư viện gốc - xem file [LICENSE](LICENSE)

## Credits

- **Original Library**: [RalfOGit/libspeedwire](https://github.com/RalfOGit/libspeedwire)
- **ESP32 Port**: lthquy
- **SMA Protocol**: SMA Solar Technology AG

## Liên kết hữu ích

- [SMA Speedwire Protocol Documentation](https://developer.sma.de)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [lwIP Documentation](https://www.nongnu.org/lwip/)

## Support

Nếu gặp vấn đề:
1. Kiểm tra phần "Các vấn đề thường gặp" ở trên
2. Xem ví dụ trong `examples/`
3. Mở issue trên GitHub với:
   - Board ESP32 đang dùng
   - Arduino IDE version
   - Log đầy đủ từ Serial Monitor
   - Mô tả chi tiết vấn đề

---

**Happy coding! 🚀**
