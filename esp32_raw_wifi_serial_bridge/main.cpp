// src/main.cpp - ESP32 Optimized AP-mode RAW TCP <-> UART bridge
// - Core 1 is dedicated to high-priority serial bridging.
// - Wi-Fi Sleep is disabled for ultra-low jitter.
// - Uses Serial2 on safe pins (16/17).
// - Serial debug is DISABLED.

#include <WiFi.h>
#include <HardwareSerial.h>
#include <freertos/task.h> // Required for FreeRTOS functions like xTaskCreatePinnedToCore

// CONFIG - **UPDATE THESE**
#define TCP_PORT      8888
// --- Access Point Details ---
#define AP_SSID       "3D_Printer"  // <-- CHOOSE YOUR AP NAME
#define AP_PASS       "CNCTinker" // <-- CHOOSE YOUR AP PASSWORD
#define SERIAL_BAUD   500000   // Highly recommended speed now!

// --- Static IP Configuration ---
IPAddress localIP(192, 168, 4, 1);       
IPAddress subnet(255, 255, 255, 0);

// --- UART PINS (MUST match your wiring: GPIO 16/17) ---
#define UART_RX_PIN 16 
#define UART_TX_PIN 17 

// Pin the bridging task to Core 1 (App CPU)
#define CORE_BRIDGE 1
#define PRIORITY_BRIDGE 3 // Higher priority ensures rapid execution

// server/client and buffers
WiFiServer server(TCP_PORT);
WiFiClient client;
HardwareSerial& MCU_Serial = Serial2; // Use Serial2

static const size_t TCP_BUF_SZ = 256;
static const size_t UART_BUF_SZ = 256;
uint8_t tcpBuf[TCP_BUF_SZ];
uint8_t uartBuf[UART_BUF_SZ];

// --- Core Bridge Logic (Function Definition) ---

void acceptClientIfNeeded() {
  if (!client || !client.connected()) {
    WiFiClient newClient = server.available();
    if (newClient) {
      if (client) client.stop();
      client = newClient;
      // Disable Nagle's algorithm for low latency
      client.setNoDelay(true);
    }
  }
}

void bridgeOnce() {
  // TCP -> UART
  if (client && client.connected()) {
    size_t n = client.available();
    if (n > 0) {
      if (n > TCP_BUF_SZ) n = TCP_BUF_SZ;
      int r = client.read(tcpBuf, n);
      if (r > 0) {
        MCU_Serial.write(tcpBuf, r);
      }
    }
  }

  // UART -> TCP
  if (client && client.connected()) {
    size_t n = MCU_Serial.available();
    if (n > 0) {
      if (n > UART_BUF_SZ) n = UART_BUF_SZ;
      int r = MCU_Serial.readBytes(uartBuf, n);
      if (r > 0) {
        client.write(uartBuf, r);
      }
    }
  }
}

void checkClientDisconnected() {
  if (client && !client.connected()) {
    client.stop();
  }
}

// Dedicated FreeRTOS Task for Bridging - PINNED TO CORE 1
void bridge_task(void *pvParameters) {
  for (;;) {
    // Run bridge in short bursts
    for (int i = 0; i < 6; ++i) {
      bridgeOnce();
      taskYIELD(); // Yield to other tasks on the same core
    }
    
    // Check for new/disconnected client
    acceptClientIfNeeded();
    checkClientDisconnected();
    
    // Brief sleep to yield time to other system tasks
    vTaskDelay(pdMS_TO_TICKS(1)); 
  }
}

// --- Main Setup and Loop (Wi-Fi setup on Core 0) ---

void setup() {
  // 1. Init UART2 on specific pins
  MCU_Serial.begin(SERIAL_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN); 

  // 2. Disable Wi-Fi sleep mode for ultra-low jitter (Optimization!)
  WiFi.setSleep(false); 

  // 3. Configure and start AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(localIP, localIP, subnet); 
  WiFi.softAP(AP_SSID, AP_PASS);

  // 4. Start TCP server
  server.begin();
  
  // 5. Create and start the dedicated bridging task on Core 1 (Optimization!)
  xTaskCreatePinnedToCore(
    bridge_task,    // Task function
    "BridgeTask",   // Task name
    8192,           // Stack size (bytes)
    NULL,           // Parameter
    PRIORITY_BRIDGE,// Priority (2 is higher than default 1)
    NULL,           // Task handle 
    CORE_BRIDGE     // Core to run on (Core 1 / App CPU)
  );
}

// The main loop is now minimal, as Core 1 handles the heavy lifting
void loop() {
  // The core task of accepting clients is moved to the high-priority task,
  // but we keep a minimal loop. In a perfect setup, loop() should be empty.
  // We'll let the bridge_task handle all connections and data.
  delay(10); 
}
