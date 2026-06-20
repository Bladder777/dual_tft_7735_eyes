

// DMA can be used with RP2040, STM32 and ESP32 processors when the interface
// is SPI, uncomment the next line:
//#define USE_DMA

// Load TFT driver library
#include <SPI.h>
#include <TFT_eSPI.h>
#if EYE_SYNC_MODE != 0
  #include <WiFi.h>
  #include <esp_now.h>
#endif
TFT_eSPI tft;           // A single instance is used for 1 or 2 displays

#ifndef EYE_SYNC_MODE
  #define EYE_SYNC_MODE 0
#endif

#ifndef EYE_SYNC_SIDE
  #define EYE_SYNC_SIDE 0
#endif

#if EYE_SYNC_MODE != 0
struct __attribute__((packed)) EyeSyncPacket {
  uint16_t magic;
  uint8_t version;
  uint8_t side;
  uint32_t seq;
  int16_t scleraX;
  int16_t scleraY;
  uint16_t irisScale;
  uint8_t upperThreshold;
  uint8_t lowerThreshold;
};

static constexpr uint16_t EYE_SYNC_MAGIC = 0xE773;
static constexpr uint8_t EYE_SYNC_VERSION = 1;
static constexpr uint32_t EYE_SYNC_TIMEOUT_MS = 250;
static const uint8_t EYE_SYNC_BROADCAST[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static volatile bool syncPacketPending = false;
static EyeSyncPacket syncLatestPacket = {};
static uint32_t syncLastPacketMs = 0;
static uint32_t syncSequence = 0;

static void onEyeSyncReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  (void)info;
  if (len != sizeof(EyeSyncPacket)) return;

  EyeSyncPacket packet;
  memcpy(&packet, data, sizeof(packet));
  if (packet.magic != EYE_SYNC_MAGIC || packet.version != EYE_SYNC_VERSION) return;

  syncLatestPacket = packet;
  syncLastPacketMs = millis();
  syncPacketPending = true;
}
#endif

// A pixel buffer is used during eye rendering
#define BUFFER_SIZE 1024 // 128 to 1024 seems optimum

#ifdef USE_DMA
  #define BUFFERS 2      // 2 toggle buffers with DMA
#else
  #define BUFFERS 1      // 1 buffer for no DMA
#endif

uint16_t pbuffer[BUFFERS][BUFFER_SIZE]; // Pixel rendering buffer
bool     dmaBuf   = 0;                  // DMA buffer selection

// This struct is populated in config.h
typedef struct {        // Struct is defined before including config.h --
  int8_t  select;       // pin numbers for each eye's screen select line
  int8_t  wink;         // and wink button (or -1 if none) specified there,
  uint8_t rotation;     // also display rotation and the x offset
  int16_t xposition;    // position of eye on the screen
} eyeInfo_t;

#include "../config.h"       // ****** CONFIGURATION IS DONE IN HERE ******

#define SCREEN_X_START 0
#define SCREEN_X_END   SCREEN_WIDTH   // Badly named, actually the "eye" width!
#define SCREEN_Y_START 0
#define SCREEN_Y_END   SCREEN_HEIGHT  // Actually "eye" height

// A simple state machine is used to control eye blinks/winks:
#define NOBLINK 0       // Not currently engaged in a blink
#define ENBLINK 1       // Eyelid is currently closing
#define DEBLINK 2       // Eyelid is currently opening
typedef struct {
  uint8_t  state;       // NOBLINK/ENBLINK/DEBLINK
  uint32_t duration;    // Duration of blink state (micros)
  uint32_t startTime;   // Time (micros) of last state change
} eyeBlink;

struct {                // One-per-eye structure
  int16_t   tft_cs;     // Chip select pin for each display
  eyeBlink  blink;      // Current blink/wink state
  int16_t   xposition;  // x position of eye image
} eye[NUM_EYES];

uint32_t startTime;  // For FPS indicator

extern void user_setup(void); // Functions in the user*.cpp files
extern void user_loop(void);
void split(int16_t startValue, int16_t endValue, uint32_t startTime, int32_t duration, int16_t range);
void setupEyeSync(void);
void syncBroadcastFrame(uint32_t iScale, int16_t eyeX, int16_t eyeY, uint8_t upperThreshold, uint8_t lowerThreshold);
void syncRenderSlaveFrame(void);

#include "../eye_functions.ino" // Eye rendering and animation helpers
#include "../user.cpp"          // User hooks (compiled in same TU)

void setupEyeSync(void) {
#if EYE_SYNC_MODE != 0
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

#if EYE_SYNC_MODE == 1
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, EYE_SYNC_BROADCAST, sizeof(EYE_SYNC_BROADCAST));
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("ESP-NOW broadcast peer failed");
    return;
  }
  Serial.print("ESP-NOW master MAC: ");
  Serial.println(WiFi.macAddress());
#elif EYE_SYNC_MODE == 2
  esp_now_register_recv_cb(onEyeSyncReceive);
  Serial.print("ESP-NOW slave MAC: ");
  Serial.println(WiFi.macAddress());
#endif
#endif
}

void syncBroadcastFrame(uint32_t iScale, int16_t eyeX, int16_t eyeY, uint8_t upperThreshold, uint8_t lowerThreshold) {
#if EYE_SYNC_MODE == 1
  EyeSyncPacket packet = {
    EYE_SYNC_MAGIC,
    EYE_SYNC_VERSION,
    EYE_SYNC_SIDE,
    ++syncSequence,
    eyeX,
    eyeY,
    (uint16_t)iScale,
    upperThreshold,
    lowerThreshold
  };
  esp_now_send(EYE_SYNC_BROADCAST, (const uint8_t *)&packet, sizeof(packet));
#else
  (void)iScale;
  (void)eyeX;
  (void)eyeY;
  (void)upperThreshold;
  (void)lowerThreshold;
#endif
}

void syncRenderSlaveFrame(void) {
#if EYE_SYNC_MODE == 2
  static uint32_t frames = 0;
  static bool reportedWaiting = false;
  if (!(++frames & 255)) {
    float elapsed = (millis() - startTime) / 1000.0;
    if (elapsed) Serial.println((uint16_t)(frames / elapsed));
  }

  EyeSyncPacket packet = syncLatestPacket;
  bool fresh = syncPacketPending && ((millis() - syncLastPacketMs) <= EYE_SYNC_TIMEOUT_MS);
  if (fresh) {
    reportedWaiting = false;
    drawEye(0, packet.irisScale, packet.scleraX, packet.scleraY,
            packet.upperThreshold, packet.lowerThreshold);
  } else {
    if (!reportedWaiting) {
      Serial.println("ESP-NOW slave waiting for master packets");
      reportedWaiting = true;
    }
    const int16_t centerX = (SCLERA_WIDTH - 128) / 2;
    const int16_t centerY = (SCLERA_HEIGHT - 128) / 2;
    const uint16_t irisScale = (IRIS_MIN + IRIS_MAX) / 2;
    drawEye(0, irisScale, centerX, centerY, 128, 126);
  }
  user_loop();
#endif
}

// INITIALIZATION -- runs once at startup ----------------------------------
void setup(void) {
  Serial.begin(115200);
  //while (!Serial);
  Serial.println("Starting");

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  // Enable backlight pin, initially off
  Serial.println("Backlight turned off");
  pinMode(DISPLAY_BACKLIGHT, OUTPUT);
  digitalWrite(DISPLAY_BACKLIGHT, LOW);
#endif

  // User call for additional features
  user_setup();
  setupEyeSync();

  // Initialise the eye(s), this will set all chip selects low for the tft.init()
  initEyes();

  // Initialise TFT
  Serial.println("Initialising displays");
  tft.init();

#ifdef USE_DMA
  tft.initDMA();
#endif

  // Raise chip select(s) so that displays can be individually configured
  digitalWrite(eye[0].tft_cs, HIGH);
  if (NUM_EYES > 1) digitalWrite(eye[1].tft_cs, HIGH);

  for (uint8_t e = 0; e < NUM_EYES; e++) {
    digitalWrite(eye[e].tft_cs, LOW);
    tft.setRotation(eyeInfo[e].rotation);
    tft.fillScreen(TFT_BLACK);
    digitalWrite(eye[e].tft_cs, HIGH);
  }

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)
  Serial.println("Backlight now on!");
  analogWrite(DISPLAY_BACKLIGHT, BACKLIGHT_MAX);
#endif

  startTime = millis(); // For frame-rate calculation
}

// MAIN LOOP -- runs continuously after setup() ----------------------------
void loop() {
  updateEye();
}
