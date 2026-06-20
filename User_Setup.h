// See SetupX_Template.h for all options available
#define USER_SETUP_LOADED
#define USER_SETUP_ID 2

#define ST7735_DRIVER


#define TFT_WIDTH  128
#define TFT_HEIGHT 160


#define ST7735_GREENTAB3

// For ST7735, ST7789 and ILI9341 ONLY, define the colour order IF the blue and red are swapped on your display
// Try ONE option at a time to find the correct colour order for your display

// ===== SPI PINS FOR ESP32-S3 SuperMini wiring =====
#define USE_HSPI_PORT
#define TFT_MOSI 12   // GPIO12
#define TFT_SCLK 13   // GPIO13
#define TFT_CS    9   // GPIO9
#define TFT_DC   10   // GPIO10
#define TFT_RST  11   // GPIO11
// Backlight is not controlled by GPIO for this wiring (BLK = -1).
#define TFT_RGB_ORDER TFT_RGB  // 1.8" 128 x RGB x 160 panel
//  #define TFT_RGB_ORDER TFT_BGR  // Colour order Blue-Green-Red

// NodeMCU pin definitions were left here previously and caused macro
// redefinition warnings during build. They are not relevant to the ESP32-S3
// target, so they have been removed to keep compiler output clean.


#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT


// #define SPI_FREQUENCY  20000000
#define SPI_FREQUENCY  27000000
// #define SPI_FREQUENCY  40000000

//#define SPI_TOUCH_FREQUENCY  2500000


// #define SUPPORT_TRANSACTIONS
