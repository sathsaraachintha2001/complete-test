// NORVI X1 (X-CPU-ESPS3-X1 V2, N16R2) - Display + Touch test
// Display: ST7789 240x320 via LovyanGFX (shared SPI bus with SD + Ethernet)
// Touch:   CST816S on I2C (SDA 8, SCL 9, RST 47, IRQ 48)

#include <Wire.h>
#include <SPI.h>
#include <CST816S.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// ---------------- Pins ----------------
#define PIN_SCLK   12
#define PIN_MOSI   11
#define PIN_MISO   13
#define PIN_TFT_CS 45
#define PIN_TFT_DC 46
#define PIN_TFT_RST 47   // shared with touch reset
#define PIN_SD_CS  42
#define PIN_ETH_CS 1
#define PIN_SDA    8
#define PIN_SCL    9
#define PIN_TP_RST 47
#define PIN_TP_IRQ 48

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI      _bus;
public:
  LGFX(void) {
    {
      auto c = _bus.config();
      c.spi_host    = SPI2_HOST;
      c.spi_mode    = 0;
      c.freq_write  = 27000000;
      c.freq_read   = 16000000;
      c.spi_3wire   = false;
      c.use_lock    = true;
      c.dma_channel = SPI_DMA_CH_AUTO;
      c.pin_sclk    = PIN_SCLK;
      c.pin_mosi    = PIN_MOSI;
      c.pin_miso    = PIN_MISO;
      c.pin_dc      = PIN_TFT_DC;
      _bus.config(c);
      _panel.setBus(&_bus);
    }
    {
      auto c = _panel.config();
      c.pin_cs   = PIN_TFT_CS;
      c.pin_rst  = PIN_TFT_RST;
      c.pin_busy = -1;
      c.panel_width  = 240;
      c.panel_height = 320;
      c.offset_x = 0;
      c.offset_y = 0;
      c.offset_rotation = 0;
      c.readable   = false;
      c.invert     = true;
      c.rgb_order  = false;
      c.dlen_16bit = false;
      c.bus_shared = true;
      _panel.config(c);
    }
    setPanel(&_panel);
  }
};

LGFX tft;
CST816S touch(PIN_SDA, PIN_SCL, PIN_TP_RST, PIN_TP_IRQ);

// ---------------- UI helpers ----------------
const int BTN_W = 100, BTN_H = 36;
int btnX, btnY;
int touchCount = 0;

void showStatus(const char *line1, uint16_t color = TFT_WHITE) {
  tft.fillRect(0, 0, tft.width(), 52, TFT_NAVY);
  tft.setTextColor(color, TFT_NAVY);
  tft.setFont(&fonts::Font2);
  tft.setTextSize(1);
  tft.setCursor(6, 6);
  tft.print(line1);
}

void drawButton() {
  tft.fillRoundRect(btnX, btnY, BTN_W, BTN_H, 6, TFT_DARKGREY);
  tft.drawRoundRect(btnX, btnY, BTN_W, BTN_H, 6, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setFont(&fonts::Font2);
  tft.setTextDatum(lgfx::textdatum_t::middle_center);
  tft.drawString("CLEAR", btnX + BTN_W / 2, btnY + BTN_H / 2);
  tft.setTextDatum(lgfx::textdatum_t::top_left);
}

void drawTouchScreen() {
  tft.fillScreen(TFT_BLACK);
  showStatus("TOUCH TEST - touch the screen");
  tft.setTextColor(TFT_CYAN, TFT_NAVY);
  tft.setCursor(6, 28);
  tft.print("X:--- Y:--- Gesture: none");
  drawButton();
}

// ---------------- Display tests ----------------
void testColors() {
  const uint16_t cols[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_WHITE, TFT_BLACK};
  const char *names[]   = {"RED", "GREEN", "BLUE", "WHITE", "BLACK"};
  for (int i = 0; i < 5; i++) {
    tft.fillScreen(cols[i]);
    tft.setFont(&fonts::Font4);
    tft.setTextColor(cols[i] == TFT_WHITE ? TFT_BLACK : TFT_WHITE, cols[i]);
    tft.setTextDatum(lgfx::textdatum_t::middle_center);
    tft.drawString(names[i], tft.width() / 2, tft.height() / 2);
    Serial.printf("Display: fill %s\n", names[i]);
    delay(700);
  }
  tft.setTextDatum(lgfx::textdatum_t::top_left);
}

void testBars() {
  const uint16_t cols[] = {TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_BLUE, TFT_MAGENTA, TFT_WHITE};
  int w = tft.width(), h = tft.height();
  int barH = h / 8;
  for (int i = 0; i < 8; i++) tft.fillRect(0, i * barH, w, barH, cols[i]);
  Serial.println("Display: colour bars");
  delay(1500);
}

void testShapes() {
  tft.fillScreen(TFT_BLACK);
  int w = tft.width(), h = tft.height();
  tft.drawRect(0, 0, w, h, TFT_WHITE);                       // border shows full-panel coverage
  tft.drawLine(0, 0, w - 1, h - 1, TFT_YELLOW);
  tft.drawLine(w - 1, 0, 0, h - 1, TFT_YELLOW);
  tft.drawCircle(w / 2, h / 2, 60, TFT_GREEN);
  tft.fillCircle(w / 2, h / 2, 30, TFT_RED);
  tft.fillTriangle(20, 80, 80, 80, 50, 30, TFT_CYAN);
  tft.fillRoundRect(w - 90, 30, 70, 50, 8, TFT_MAGENTA);
  tft.setFont(&fonts::Font2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, h - 40);
  tft.printf("%d x %d", w, h);
  Serial.printf("Display: shapes, panel %d x %d\n", w, h);
  delay(2000);
}

void testText() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 10);
  tft.setTextColor(TFT_WHITE);  tft.setFont(&fonts::Font2); tft.println("NORVI X1 display test");
  tft.setTextColor(TFT_YELLOW); tft.setFont(&fonts::Font4); tft.println("Font 4 text");
  tft.setTextColor(TFT_GREEN);  tft.setFont(&fonts::Font6); tft.println("12:34");
  tft.setTextColor(TFT_CYAN);   tft.setFont(&fonts::Font2); tft.println("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  tft.println("abcdefghijklmnopqrstuvwxyz");
  tft.println("0123456789 !@#$%^&*()");
  Serial.println("Display: text");
  delay(2500);
}

// ---------------- Setup / loop ----------------
void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000) delay(10);
  Serial.println("\n=== NORVI X1 display + touch test ===");

  // Keep other devices on the shared SPI bus deselected
  pinMode(PIN_SD_CS, OUTPUT);  digitalWrite(PIN_SD_CS, HIGH);
  pinMode(PIN_ETH_CS, OUTPUT); digitalWrite(PIN_ETH_CS, HIGH);

  // Touch first: its begin() pulses the shared reset line, which would
  // blank an already-initialised display. Display init (below) resets again.
  touch.begin();
  delay(50);

  tft.init();
  tft.setRotation(0);
  tft.setBrightness(255);
  delay(150);
  Serial.printf("Display initialised: %d x %d\n", tft.width(), tft.height());

  btnX = (tft.width() - BTN_W) / 2;
  btnY = tft.height() - BTN_H - 8;

  testColors();
  testBars();
  testShapes();
  testText();

  drawTouchScreen();
  Serial.println("Touch test running - touch the screen (CLEAR button resets).");
}

void loop() {
  if (touch.available()) {
    int x = touch.data.x;
    int y = touch.data.y;
    String g = touch.gesture();
    touchCount++;

    Serial.printf("Touch #%d  x=%d  y=%d  gesture=%s\n", touchCount, x, y, g.c_str());

    // CLEAR button
    if (x >= btnX && x <= btnX + BTN_W && y >= btnY && y <= btnY + BTN_H) {
      drawTouchScreen();
      touchCount = 0;
      Serial.println("Screen cleared");
      return;
    }

    // Status line
    tft.fillRect(0, 26, tft.width(), 22, TFT_NAVY);
    tft.setFont(&fonts::Font2);
    tft.setTextColor(TFT_CYAN, TFT_NAVY);
    tft.setCursor(6, 28);
    tft.printf("X:%d Y:%d  %s", x, y, g.c_str());

    // Touch point marker
    if (y > 52 && y < btnY - 2) {
      tft.drawFastHLine(x - 10, y, 21, TFT_YELLOW);
      tft.drawFastVLine(x, y - 10, 21, TFT_YELLOW);
      tft.fillCircle(x, y, 4, TFT_GREEN);
    }
  }
  delay(5);
}
