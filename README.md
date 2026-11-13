# ESP32-IoT-Air-Quality-Monitoring-System
This folder contains the firmware extracted from Appendix C of the supplied
"IoT-Enabled Real-Time Air Quality Monitoring System" PDF, arranged as an
Arduino IDE project for an ESP32 development board.
Board note: The source in the PDF is already ESP32 code. It uses
WiFi.h, WebServer.h, GPIO 4 and GPIO 34. It is not ESP8266 firmware.

Features
- DHT11 temperature and relative-humidity readings
- MQ-135 analogue sensor reading (raw ADC count and voltage estimate)
- ESP32 Wi-Fi connection in station mode
- Web dashboard hosted by the ESP32
- Temperature, humidity and gas-reading graphs
- JSON endpoint at /data
- Dashboard refresh every 2 seconds; up to 30 points per graph
- Serial output at 115200 baud
Files
- ESP32_Air_Quality_Monitoring.ino — complete ESP32 Arduino sketch
- README.md — this guide
Hardware connections
Sensor/module	Signal	ESP32 connection
DHT11	DATA	GPIO 4
DHT11	GND	GND
DHT11	VCC	Supply appropriate for your exact DHT11 module
MQ-135	AO	GPIO 34, through suitable voltage conditioning
MQ-135	GND	GND
MQ-135	VCC	Supply appropriate for your exact MQ-135 module


All modules must share a common ground.
Important electrical warning
Do not connect an MQ-135 analogue output directly to the ESP32 if it can
exceed the ESP32 GPIO input-voltage limit. Many MQ-135 modules are powered from
5 V and their AO output may exceed the safe ESP32 input range. Measure the AO
voltage and use a correctly designed voltage divider or level-conditioning
circuit before connecting it to GPIO 34. Confirm the board and sensor-module
datasheets. GPIO 34 is input-only on common ESP32 boards.
Software requirements
1. Install Arduino IDE.
2. Install the ESP32 board support package using Boards Manager.
3. Install DHT sensor library by Adafruit using Library Manager.
4. Install Adafruit Unified Sensor if Arduino IDE prompts for the dependency.
The WiFi.h and WebServer.h libraries are included with the ESP32 Arduino
core; they are not the ESP8266 equivalents.
Configure Wi-Fi
Open ESP32_Air_Quality_Monitoring.ino and replace:
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
with the Wi-Fi name and password. Use a trusted local network or phone hotspot.
Do not publish your real Wi-Fi credentials in a public repository.
Upload and run
1. Connect the ESP32 to the computer using a USB data cable.
2. Open ESP32_Air_Quality_Monitoring.ino in Arduino IDE.
3. Select the matching ESP32 board under Tools > Board.
4. Select the correct serial port under Tools > Port.
5. Verify the wiring and voltage conditioning before powering the sensors.
6. Click Upload.
7. Open Tools > Serial Monitor and set the baud rate to 115200.
8. Reset the ESP32 if required. Wait for the Wi-Fi connection attempt.
9. When connected, the Serial Monitor prints Open dashboard: http:// followed
   by the ESP32's local IP address.
10. Connect your phone or computer to the same network and open that IP address
    in a web browser.
If Wi-Fi does not connect, check the SSID/password and ensure the network is
available to the ESP32.
Dashboard and data interpretation
The dashboard displays temperature in °C, relative humidity in %, MQ-135 raw
ADC count (0–4095), and an estimated input voltage. It requests /data every
two seconds. The ESP32 returns JSON containing:
- temperature
- humidity
- mqRaw
- mqVoltage
- status
If the DHT11 read fails, temperature and humidity are returned as null and
shown as --.

The MQ-135 value is only a relative sensor indication. The status labels
(Lower, Moderate, Higher) are simple thresholds applied to raw ADC counts.
They are not a calibrated gas concentration in ppm and are not an official
Air Quality Index (AQI). MQ-135 readings depend on warm-up, calibration,
humidity, temperature, gas mixture and the individual sensor. Do not use this
prototype as a certified safety or air-quality instrument.
Troubleshooting
- DHT11 shows --: Check DATA on GPIO 4, power, ground and the module's
  pull-up arrangement.
- MQ-135 stays at zero or behaves unexpectedly: Check module power, AO
  wiring, common ground and voltage conditioning. Allow the sensor to warm up
  according to its datasheet.
- Dashboard does not open: Confirm the ESP32 connected to Wi-Fi, use the
  printed IP address, and keep the client device on the same reachable network.
- Upload fails: Check the selected board, COM port, USB cable and USB-UART
  driver.
Security
This example uses plain HTTP and has no login, authentication or encryption.
Use it only on a trusted private network. Do not expose the dashboard directly
to the public internet.
Source note
The firmware structure and dashboard behaviour are based on Appendix C of the
user-provided PDF (pages 28–33). Wi-Fi credentials have intentionally been
replaced with placeholders. Formatting and HTML whitespace have been cleaned
for readability; the ESP32 pin assignments and core behaviour are retained.
