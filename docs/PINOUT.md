# JAF-R Pinout

This is the current hardware pin source of truth. If code and this document
disagree, treat that as a bug and fix both together.

## Eye ESP32-S3 SuperMini Boards

Each eye has its own ESP32-S3 SuperMini and one ST7735 1.8 inch TFT.

| Signal | ESP32-S3 GPIO | Notes |
| --- | --- | --- |
| TFT MOSI / SDA | GPIO12 | `TFT_MOSI` in `User_Setup.h` |
| TFT SCLK / SCK | GPIO13 | `TFT_SCLK` in `User_Setup.h` |
| TFT DC / A0 | GPIO10 | `TFT_DC` in `User_Setup.h` |
| TFT CS | GPIO9 | `TFT_CS` in `User_Setup.h` |
| TFT RST / RESET | GPIO11 | `TFT_RST` in `User_Setup.h` |
| TFT BLK / LED | not controlled | Firmware uses `BLK=-1` |
| TFT VCC | 3.3 V | Do not use 5 V logic |
| TFT GND | GND | Shared with board |

Important display settings:

```cpp
#define ST7735_DRIVER
#define TFT_WIDTH  128
#define TFT_HEIGHT 160
#define ST7735_GREENTAB3
#define USE_HSPI_PORT
#define TFT_RGB_ORDER TFT_RGB
#define SPI_FREQUENCY 27000000
```

`USE_HSPI_PORT` is required for this ESP32-S3 SuperMini setup.

## Brain ESP32-S3 SuperMini To ESP32D UART

| Signal | Brain ESP32-S3 | ESP32D | Notes |
| --- | --- | --- | --- |
| UART TX from brain | GPIO1 | GPIO16 / RX2 | Brain sends commands |
| UART RX to brain | GPIO2 | GPIO17 / TX2 | Brain receives replies |
| Ground | GND | GND | Required shared ground |

Do not use ESP32-S3 GPIO20 for this UART bridge. It is tied to native USB and
must remain available for flashing/monitoring.

## ESP32D Servo Bridge

| Function | ESP32D GPIO | Build flag |
| --- | --- | --- |
| UART RX from brain | GPIO16 | `SERVO_UART_RX=16` |
| UART TX to brain | GPIO17 | `SERVO_UART_TX=17` |
| Pan servo signal | GPIO25 | `PAN_SERVO_PIN=25` |
| Tilt servo signal | GPIO26 | `TILT_SERVO_PIN=26` |

Servo power should come from an external supply sized for the servos. Grounds
must be common between servo supply, ESP32D, and brain.

## Reserved / Avoid

| Board | Pin | Reason |
| --- | --- | --- |
| ESP32-S3 SuperMini | GPIO20 | Native USB, keep free |
| Eye boards | GPIO1/GPIO2 | Keep eye boards display-only; UART bridge belongs on brain |

