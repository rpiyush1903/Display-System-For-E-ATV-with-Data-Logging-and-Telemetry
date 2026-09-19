/*
  Aveon Racing - JBD BMS Dashboard (2.8" CENTERED VERSION)
  
  - Controller: ATmega328P (Arduino Uno)
  - Display: ST7789 / ILI9341 (320x240 Landscape)
  - CAN Bus: MCP2515 (CS Pin 7)
  - Safety: Relay on Pin 5 (Active LOW)
  
  UPDATES:
  1. Centered Logo for 320px wide screens.
  2. Colors: TEAM/RACING = White, AVEON = Dark Green.
  3. Dashboard layout expanded to fill 2.8" screen.
*/

#include <SPI.h>
#include <TFT_eSPI.h>
#include <mcp_can.h>
#include <avr/pgmspace.h>

// --- Load Custom Font ---
#define RACING_FONT &FreeSansBold12pt7b 

// --- Pin Definitions ---
const int MCP_CS_PIN = 7;
const int RELAY_PIN  = 5; // Safety Relay (Active LOW)

// --- CAN IDs ---
#define CAN_ID_VOLT_CURR 0x100 
#define CAN_ID_SOC       0x101
#define CAN_ID_TEMP      0x105

// --- Global Objects ---
TFT_eSPI tft = TFT_eSPI();
MCP_CAN mcp2515(MCP_CS_PIN);
SPISettings mcpSPISettings(10000000, MSBFIRST, SPI_MODE0);

// --- Data Variables ---
float rawVoltage = 0.0;
float smoothedVoltage = 0.0;
float current = 0.0;
int   soc = 0;
float tempNTC1 = 0.0;
bool  isCharging = false;
bool  newDataBMS = false;

// --- Screen Constants ---
// 2.8" Screens are usually 320x240 in Landscape
const int SCREEN_W = 320;
const int SCREEN_H = 240;
int centerX = SCREEN_W / 2; // Will be calculated in setup

// --- Layout Constants ---
const int BORDER_COLOR = TFT_CYAN;
const int TEXT_COLOR   = TFT_WHITE;
const int ROW_VOLTS = 50; 
const int ROW_SOC   = 110;
const int ROW_TEMP  = 170;

// Adjusted columns for wider 320px screen
const int COL_LABEL = 20;  // Labels slightly indented
const int COL_DATA  = 160; // Data numbers moved to the right side

// --- Timing ---
unsigned long lastCANRequest = 0;
unsigned long lastDisplayUpdate = 0;
const long canRequestInterval = 1000;   
const long displayUpdateInterval = 250; 

// ==========================================
//   HELPER FUNCTIONS 
// ==========================================
void drawStaticInterface();
void updateDisplayValues();
void parseBMSData(long id, unsigned char data[8]);
void updateSafetyRelay();

// ==========================================
//   MAIN SETUP
// ==========================================
void setup() {
  // 1. Init Relay (Fail-Safe: HIGH)
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); 

  // 2. Init TFT
  tft.init();
  tft.setRotation(1); // Landscape
  tft.fillScreen(TFT_BLACK);
  
  // Calculate true center based on actual screen width
  centerX = tft.width() / 2;

  // =========================================
  //   STARTUP SEQUENCE (CENTERED)
  // =========================================
  
  tft.setTextDatum(MC_DATUM); // Set alignment to Middle Center
  
  // Draw "TEAM" (Small, White)
  tft.setTextSize(1);
  tft.setFreeFont(RACING_FONT);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.drawString("TEAM", centerX, 60);

  // Draw "AVEON" (Large, Dark Green)
  tft.setTextSize(2); // Double size
  tft.setTextColor(TFT_DARKGREEN, TFT_BLACK);
  tft.drawString("AVEON", centerX, 100);

  // Draw "RACING" (Small, White)
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("RACING", centerX, 140);
  
  // Draw Init Status
  tft.setFreeFont(NULL); // Standard font
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Initializing CAN Bus...", centerX, 180);

  // 3. Init CAN
  SPI.beginTransaction(mcpSPISettings);
  while (mcp2515.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) != CAN_OK) {
    SPI.endTransaction();
    
    // Update status (Centered)
    tft.fillRect(0, 170, tft.width(), 30, TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("CAN FAIL. Retrying...", centerX, 180);
    
    delay(500);
    SPI.beginTransaction(mcpSPISettings);
  }
  SPI.endTransaction();
  
  SPI.beginTransaction(mcpSPISettings);
  mcp2515.setMode(MCP_NORMAL);
  SPI.endTransaction();

  // Show Success
  tft.fillRect(0, 170, tft.width(), 30, TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("CAN CONNECTED!", centerX, 180);
  delay(1500); 
  
  // =========================================
  //   START DASHBOARD
  // =========================================
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM); // Reset alignment to Top-Left for dashboard
  drawStaticInterface();
}

// ==========================================
//   MAIN LOOP
// ==========================================
void loop() {
  unsigned long now = millis();

  // --- TASK 1: BMS Request ---
  if (now - lastCANRequest >= canRequestInterval) {
    lastCANRequest = now;
    byte msg[1] = {0x5A};
    SPI.beginTransaction(mcpSPISettings);
    mcp2515.sendMsgBuf(CAN_ID_VOLT_CURR, 0, 1, msg);
    delay(5); 
    mcp2515.sendMsgBuf(CAN_ID_SOC, 0, 1, msg);
    delay(5);
    mcp2515.sendMsgBuf(CAN_ID_TEMP, 0, 1, msg);
    SPI.endTransaction();
  }

  // --- TASK 2: Receive CAN ---
  SPI.beginTransaction(mcpSPISettings);
  if (mcp2515.checkReceive() == CAN_MSGAVAIL) {
    long canId;
    unsigned char len;
    unsigned char buf[8];
    mcp2515.readMsgBuf(&canId, &len, buf);
    parseBMSData(canId, buf);
    newDataBMS = true;
  }
  SPI.endTransaction();

  // --- TASK 3: Safety Logic ---
  updateSafetyRelay();

  // --- TASK 4: Update Display ---
  if (now - lastDisplayUpdate >= displayUpdateInterval) {
    lastDisplayUpdate = now;
    
    if(rawVoltage > 1.0) { 
        smoothedVoltage = (smoothedVoltage * 0.8) + (rawVoltage * 0.2);
    }
    
    updateDisplayValues();
  }
}

// ==========================================
//   FUNCTIONS
// ==========================================

void updateSafetyRelay() {
  if (isCharging) {
    digitalWrite(RELAY_PIN, LOW); 
  } else {
    digitalWrite(RELAY_PIN, HIGH);
  }
}

void parseBMSData(long id, unsigned char data[8]) {
  switch (id) {
    case CAN_ID_VOLT_CURR:
      rawVoltage = ((data[0] << 8) | data[1]) * 0.01;
      current = (int16_t)((data[2] << 8) | data[3]) * 0.01;
      
      if (current > 0.5) {
        isCharging = true;
      } else {
        isCharging = false;
      }
      break;

    case CAN_ID_SOC:
      soc = (data[4] << 8) | data[5];
      break;
      
    case CAN_ID_TEMP:
      float tempK = ((data[0] << 8) | data[1]) * 0.1;
      tempNTC1 = tempK - 273.15;
      break;
  }
}

void drawStaticInterface() {
  // Draw Borders (Dynamic Width)
  int w = tft.width();
  int h = tft.height();
  
  tft.drawRect(0, 0, w, h, BORDER_COLOR);
  tft.drawRect(1, 1, w-2, h-2, BORDER_COLOR); 

  // Draw Separator Lines
  tft.drawLine(10, 65, w-10, 65, TFT_DARKGREY);  
  tft.drawLine(10, 125, w-10, 125, TFT_DARKGREY); 
  tft.drawLine(10, 185, w-10, 185, TFT_DARKGREY); 

  // Draw Labels
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK); 
  tft.setFreeFont(RACING_FONT); 
  tft.setTextSize(1);

  tft.setCursor(COL_LABEL, ROW_VOLTS);
  tft.print("VOLT");
  tft.setCursor(COL_LABEL, ROW_SOC);
  tft.print("SOC");
  tft.setCursor(COL_LABEL, ROW_TEMP);
  tft.print("TEMP");
}

void updateDisplayValues() {
  tft.setFreeFont(RACING_FONT); 
  tft.setTextColor(TEXT_COLOR, TFT_BLACK);
  tft.setTextSize(1);

  // Update Voltage
  // Erase box is wider now for 320px screen
  tft.fillRect(COL_DATA, ROW_VOLTS - 25, 140, 30, TFT_BLACK);
  tft.setCursor(COL_DATA, ROW_VOLTS);
  tft.print(smoothedVoltage, 1);
  tft.print(" V");

  // Update SOC
  tft.fillRect(COL_DATA, ROW_SOC - 25, 140, 30, TFT_BLACK);
  if (soc < 20) tft.setTextColor(TFT_RED, TFT_BLACK);
  else tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(COL_DATA, ROW_SOC);
  tft.print(soc);
  tft.print(" %");

  // Update Temp
  tft.fillRect(COL_DATA, ROW_TEMP - 25, 140, 30, TFT_BLACK);
  if (tempNTC1 > 55) tft.setTextColor(TFT_RED, TFT_BLACK);
  else tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(COL_DATA, ROW_TEMP);
  tft.print(tempNTC1, 0);
  tft.print(" C");

  // Update Status
  tft.fillRect(10, 195, tft.width()-20, 40, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);  
  
  if (isCharging) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("CHARGING", centerX, 215); // Centered
  } else {
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.drawString("DISCHARGING", centerX, 215); // Centered
  }
  tft.setTextDatum(TL_DATUM); 
}