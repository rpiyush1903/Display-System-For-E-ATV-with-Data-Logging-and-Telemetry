/*
  TFT_eSPI Library Test for ST7789 (320x240)

  This sketch will run all tests in landscape mode.
  It relies on your User_Setup.h file being
  correctly configured.
*/
#include <SPI.h>
#include <TFT_eSPI.h> // Include the configured library

// Create a TFT_eSPI object
TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(9600);
  Serial.println("TFT_eSPI ST7789 Test (Landscape)");

  tft.init(); // Initialize the library
  
  // --- LANDSCAPE MODE ---
  // setRotation(1) or setRotation(3) will give 320x240.
  // If this is upside-down, change 1 to 3.
  tft.setRotation(1); 

  // --- USER REQUEST: Set background to BLACK ---
  // This should now work with TFT_INVERSION_OFF
  tft.fillScreen(TFT_BLACK); 
  Serial.println("Screen set to black.");
  delay(500);

  // --- Start Test Sequence ---

  // Test 1: Text
  testText();
  delay(2000);

  // Test 2: Lines
  testLines(TFT_CYAN);
  delay(1000);

  // Test 3: Rectangles
  testRects(TFT_GREEN);
  delay(1000);

  // Test 4: Circles
  testCircles(TFT_MAGENTA);
  delay(1000);

  // Test 5: Screen Fills (R, G, B)
  testFillScreen();
  delay(1000);

  // --- Test Complete ---
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(40, 100); // Centered for 320x240
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(3);
  
  // This text will now print correctly on one line
  tft.println("TEST COMPLETE"); 
  Serial.println("Test complete.");
}

void loop() {
  // The test runs once in setup(), so loop is empty.
}

// --- Helper Test Functions ---
// All functions now use tft.width() = 320 and tft.height() = 240

void testText() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.println("This is ST7789 landscape.");

  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(3);
  tft.println(1234.56);

  tft.setTextColor(TFT_RED);
  tft.setTextSize(4);
  tft.println("TEST!");

  tft.setTextColor(TFT_BLUE);
  tft.setTextSize(1);
  tft.println("Testing complete.");
}

void testLines(uint16_t color) {
  tft.fillScreen(TFT_BLACK);
  // tft.width() is 320
  // tft.height() is 240
  for (int16_t x = 0; x < tft.width(); x += 8) {
    tft.drawLine(0, 0, x, tft.height() - 1, color);
  }
  for (int16_t y = 0; y < tft.height(); y += 8) {
    tft.drawLine(0, 0, tft.width() - 1, y, color);
  }
}

void testRects(uint16_t color) {
  tft.fillScreen(TFT_BLACK);
  tft.drawRect(tft.width() / 4, tft.height() / 4, tft.width() / 2, tft.height() / 2, color);
  delay(500);
  tft.fillRect(tft.width() / 3, tft.height() / 3, tft.width() / 3, tft.height() / 3, color);
}

void testCircles(uint16_t color) {
  tft.fillScreen(TFT_BLACK);
  tft.drawCircle(tft.width() / 2, tft.height() / 2, 50, color);
  delay(500);
  tft.fillCircle(tft.width() / 2, tft.height() / 2, 25, color);
}

void testFillScreen() {
  tft.fillScreen(TFT_RED);
  Serial.println("RED");
  delay(500);
  tft.fillScreen(TFT_GREEN);
  Serial.println("GREEN");
  delay(500);
  tft.fillScreen(TFT_BLUE);
  Serial.println("BLUE");
  delay(500);
  tft.fillScreen(TFT_BLACK);
  Serial.println("BLACK");
  delay(500);
}

