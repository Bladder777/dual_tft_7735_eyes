#include <Arduino.h>
#include <ESP32Servo.h>

#ifndef SERVO_UART_BAUD
#define SERVO_UART_BAUD 9600
#endif

#ifndef SERVO_UART_RX
#define SERVO_UART_RX 16
#endif

#ifndef SERVO_UART_TX
#define SERVO_UART_TX 17
#endif

#ifndef SERVO_OUTPUT_ENABLE
#define SERVO_OUTPUT_ENABLE 0
#endif

#ifndef PAN_SERVO_PIN
#define PAN_SERVO_PIN 25
#endif

#ifndef TILT_SERVO_PIN
#define TILT_SERVO_PIN 26
#endif

#ifndef PAN_HOME_DEG
#define PAN_HOME_DEG 90
#endif

#ifndef TILT_HOME_DEG
#define TILT_HOME_DEG 90
#endif

#ifndef PAN_MIN_DEG
#define PAN_MIN_DEG 20
#endif

#ifndef PAN_MAX_DEG
#define PAN_MAX_DEG 160
#endif

#ifndef TILT_MIN_DEG
#define TILT_MIN_DEG 35
#endif

#ifndef TILT_MAX_DEG
#define TILT_MAX_DEG 145
#endif

static HardwareSerial BrainUart(2);
static Servo panServo;
static Servo tiltServo;

static bool outputEnabled = SERVO_OUTPUT_ENABLE != 0;
static bool servosAttached = false;
static int panDeg = PAN_HOME_DEG;
static int tiltDeg = TILT_HOME_DEG;

static char brainBuf[96];
static size_t brainLen = 0;
static char usbBuf[96];
static size_t usbLen = 0;

static int clampInt(int value, int low, int high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}

static void attachServosIfNeeded() {
  if (!outputEnabled || servosAttached) return;

  panServo.setPeriodHertz(50);
  tiltServo.setPeriodHertz(50);
  panServo.attach(PAN_SERVO_PIN, 500, 2500);
  tiltServo.attach(TILT_SERVO_PIN, 500, 2500);
  servosAttached = true;
}

static void detachServos() {
  if (!servosAttached) return;

  panServo.detach();
  tiltServo.detach();
  servosAttached = false;
}

static void writeServos() {
  if (!outputEnabled) {
    detachServos();
    return;
  }

  attachServosIfNeeded();
  panServo.write(panDeg);
  tiltServo.write(tiltDeg);
}

static void reply(Stream &out, const char *message) {
  out.println(message);
  Serial.print("[reply] ");
  Serial.println(message);
}

static void replyStatus(Stream &out) {
  out.printf("STATUS pan=%d tilt=%d output=%s attached=%s\n",
             panDeg, tiltDeg,
             outputEnabled ? "on" : "off",
             servosAttached ? "yes" : "no");
}

static char *trim(char *s) {
  while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;

  char *end = s + strlen(s);
  while (end > s && (end[-1] == ' ' || end[-1] == '\t' ||
                     end[-1] == '\r' || end[-1] == '\n')) {
    *--end = '\0';
  }
  return s;
}

static bool parseOneInt(char *args, int *a) {
  char *end = nullptr;
  long first = strtol(args, &end, 10);
  if (end == args) return false;
  *a = (int)first;
  return true;
}

static bool parseTwoInts(char *args, int *a, int *b) {
  char *end = nullptr;
  long first = strtol(args, &end, 10);
  if (end == args) return false;

  while (*end == ' ' || *end == '\t' || *end == ',') end++;

  char *end2 = nullptr;
  long second = strtol(end, &end2, 10);
  if (end2 == end) return false;

  *a = (int)first;
  *b = (int)second;
  return true;
}

static void handleCommand(char *line, Stream &out) {
  char *cmd = trim(line);
  if (*cmd == '\0') return;

  Serial.print("[cmd] ");
  Serial.println(cmd);

  char *args = strchr(cmd, ' ');
  if (args) {
    *args++ = '\0';
    args = trim(args);
  } else {
    args = cmd + strlen(cmd);
  }

  for (char *p = cmd; *p; p++) *p = (char)toupper((unsigned char)*p);

  if (strcmp(cmd, "PING") == 0) {
    reply(out, "PONG");
  } else if (strcmp(cmd, "STATUS") == 0) {
    replyStatus(out);
  } else if (strcmp(cmd, "HOME") == 0) {
    panDeg = clampInt(PAN_HOME_DEG, PAN_MIN_DEG, PAN_MAX_DEG);
    tiltDeg = clampInt(TILT_HOME_DEG, TILT_MIN_DEG, TILT_MAX_DEG);
    writeServos();
    out.printf("OK HOME pan=%d tilt=%d\n", panDeg, tiltDeg);
  } else if (strcmp(cmd, "PAN") == 0) {
    int value = 0;
    if (!parseOneInt(args, &value)) {
      reply(out, "ERR PAN needs degrees");
      return;
    }
    panDeg = clampInt(value, PAN_MIN_DEG, PAN_MAX_DEG);
    writeServos();
    out.printf("OK PAN %d\n", panDeg);
  } else if (strcmp(cmd, "TILT") == 0) {
    int value = 0;
    if (!parseOneInt(args, &value)) {
      reply(out, "ERR TILT needs degrees");
      return;
    }
    tiltDeg = clampInt(value, TILT_MIN_DEG, TILT_MAX_DEG);
    writeServos();
    out.printf("OK TILT %d\n", tiltDeg);
  } else if (strcmp(cmd, "MOVE") == 0) {
    int pan = 0;
    int tilt = 0;
    if (!parseTwoInts(args, &pan, &tilt)) {
      reply(out, "ERR MOVE needs pan tilt");
      return;
    }
    panDeg = clampInt(pan, PAN_MIN_DEG, PAN_MAX_DEG);
    tiltDeg = clampInt(tilt, TILT_MIN_DEG, TILT_MAX_DEG);
    writeServos();
    out.printf("OK MOVE pan=%d tilt=%d\n", panDeg, tiltDeg);
  } else if (strcmp(cmd, "ENABLE") == 0) {
#if SERVO_OUTPUT_ENABLE
    outputEnabled = true;
    writeServos();
    reply(out, "OK OUTPUT ON");
#else
    reply(out, "ERR output disabled at build time");
#endif
  } else if (strcmp(cmd, "DISABLE") == 0) {
    outputEnabled = false;
    detachServos();
    reply(out, "OK OUTPUT OFF");
  } else if (strcmp(cmd, "HELP") == 0) {
    reply(out, "CMDS PING STATUS HOME PAN n TILT n MOVE pan tilt ENABLE DISABLE");
  } else {
    out.printf("ERR unknown command: %s\n", cmd);
  }
}

static void pollLine(Stream &in, Stream &out, char *buf, size_t *len, const char *tag) {
  while (in.available()) {
    char c = (char)in.read();
    if (c == '\n' || c == '\r') {
      if (*len == 0) continue;
      buf[*len] = '\0';
      Serial.print(tag);
      Serial.print(" ");
      Serial.println(buf);
      handleCommand(buf, out);
      *len = 0;
    } else if (*len < 95) {
      buf[(*len)++] = c;
    } else {
      *len = 0;
      reply(out, "ERR line too long");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  BrainUart.begin(SERVO_UART_BAUD, SERIAL_8N1, SERVO_UART_RX, SERVO_UART_TX);

  panDeg = clampInt(PAN_HOME_DEG, PAN_MIN_DEG, PAN_MAX_DEG);
  tiltDeg = clampInt(TILT_HOME_DEG, TILT_MIN_DEG, TILT_MAX_DEG);
  writeServos();

  Serial.println();
  Serial.println("JAFR ESP32D Servo Bridge");
  Serial.printf("UART2 RX GPIO%d TX GPIO%d baud %d\n",
                SERVO_UART_RX, SERVO_UART_TX, SERVO_UART_BAUD);
  Serial.printf("Servo output build=%s runtime=%s panPin=%d tiltPin=%d\n",
                SERVO_OUTPUT_ENABLE ? "on" : "off",
                outputEnabled ? "on" : "off",
                PAN_SERVO_PIN, TILT_SERVO_PIN);
  Serial.println("Ready. Send HELP, PING, STATUS, HOME, PAN n, TILT n, MOVE pan tilt.");

  BrainUart.println("BOOT JAFR-ESP32D-SERVO output=off");
}

void loop() {
  pollLine(BrainUart, BrainUart, brainBuf, &brainLen, "[uart]");
  pollLine(Serial, Serial, usbBuf, &usbLen, "[usb]");
}

