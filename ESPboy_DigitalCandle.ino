#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"
#include "lib/nbSPI.h"
#include <EEPROM.h>

/*
#include "lib/ESPboyTerminalGUI.h"
#include "lib/ESPboyTerminalGUI.cpp"
#include "lib/ESPboyOTA2.h"
#include "lib/ESPboyOTA2.cpp"
*/

ESPboyInit myESPboy;

#define FIRE_WIDTH 128
#define FIRE_HEIGHT 128
#define BLOCK_LINES 16 
#define NUM_PARTICLES 25

#define WICK_X        64
#define WICK_TOP_Y    98   
#define CANDLE_TOP_Y  104  
#define CANDLE_RADIUS 16   

uint8_t fire[FIRE_HEIGHT][FIRE_WIDTH];
uint8_t fuel[FIRE_WIDTH]; 

alignas(4) uint16_t blockBuffer[2][FIRE_WIDTH * BLOCK_LINES];
uint8_t currentBuffer = 0;

#define C16(c) (uint16_t)(((c) >> 8) | ((c) << 8))

const PROGMEM uint16_t palRed[36] = {
  C16(0x0000), C16(0x2800), C16(0x5000), C16(0x7800), C16(0xA000), C16(0xC800),
  C16(0xE200), C16(0xEC00), C16(0xF500), C16(0xFA00), C16(0xFC00), C16(0xFD00),
  C16(0xFE00), C16(0xFEA0), C16(0xFF40), C16(0xFF80), C16(0xFFA0), C16(0xFFC0),
  C16(0xFFE0), C16(0xFFE0), C16(0xFFE0), C16(0xFFE4), C16(0xFFE8), C16(0xFFEC),
  C16(0xFFF0), C16(0xFFF2), C16(0xFFF5), C16(0xFFF8), C16(0xFFFA), C16(0xFFFC),
  C16(0xFFFD), C16(0xFFFE), C16(0xFFFF), C16(0xFFFF), C16(0xFFFF), C16(0xFFFF)
};

const PROGMEM uint16_t palBlue[36] = {
  C16(0x0000), C16(0x0008), C16(0x0012), C16(0x001A), C16(0x009F), C16(0x011F),
  C16(0x021F), C16(0x031F), C16(0x04DF), C16(0x05DF), C16(0x06BF), C16(0x079F),
  C16(0x07DF), C16(0x07FF), C16(0x17FF), C16(0x37FF), C16(0x57FF), C16(0x77FF),
  C16(0x87FF), C16(0x97FF), C16(0xA7FF), C16(0xB7FF), C16(0xC7FF), C16(0xD7FF),
  C16(0xE7FF), C16(0xF7FF), C16(0xF7FF), C16(0xEFFF), C16(0xF7FF), C16(0xFBFF),
  C16(0xFDFF), C16(0xFEFF), C16(0xFFFF), C16(0xFFFF), C16(0xFFFF), C16(0xFFFF)
};

const PROGMEM uint16_t palGreen[36] = {
  C16(0x0000), C16(0x00C0), C16(0x0180), C16(0x0240), C16(0x0320), C16(0x0420),
  C16(0x0540), C16(0x0640), C16(0x0760), C16(0x1780), C16(0x27A0), C16(0x37C0),
  C16(0x47E0), C16(0x67E0), C16(0x87E0), C16(0xA7E0), C16(0xC7E0), C16(0xE7E0),
  C16(0xF7E0), C16(0xF7E4), C16(0xF7E8), C16(0xF7EC), C16(0xF7F0), C16(0xF7F4),
  C16(0xF7F8), C16(0xF7FC), C16(0xF7FE), C16(0xF7FF), C16(0xE7FF), C16(0xEFFF),
  C16(0xF7FF), C16(0xFBFF), C16(0xFDFF), C16(0xFFFF), C16(0xFFFF), C16(0xFFFF)
};

const uint16_t* palettes[3] = { palRed, palBlue, palGreen };
uint8_t currentPaletteIdx = 0;
uint8_t lastKeys = 0;

int flamePower = 4; 
bool ledEnabled = true;

float currentWind = 0.0;
float targetWind = 0.0;

struct Particle {
  float x, y, vx, vy;
  int life, maxLife;
  int decay; 
  bool isSpark; // Флаг: искра это или дым
};
Particle particles[NUM_PARTICLES];

static inline void waitSPI() {
  uint32_t t = micros();
  while (nbSPI_isBusy() && (micros() - t < 3000)) { yield(); }
}

uint16_t getCandleWaxPixel(int x, int y, int coreHeat) {
  int dx = abs(x - WICK_X);
  if (dx > CANDLE_RADIUS) return 0;

  int poolRadius = CANDLE_RADIUS - 2; 
  int dip = 0;
  if (dx < poolRadius) {
    dip = (poolRadius - dx) / 2; 
    if (dip > 4) dip = 4;        
  }
  
  int topY = CANDLE_TOP_Y + dip;
  if (y < topY) return 0; 

  int shade = CANDLE_RADIUS - dx; 
  
  uint8_t rW = constrain(80 + shade * 6 + (coreHeat / 2), 0, 255);
  uint8_t gW = constrain(50 + shade * 5 + (coreHeat / 3), 0, 255);
  uint8_t bW = constrain(15 + shade * 2, 0, 200);

  if (dx < poolRadius && y <= topY + 2) {
    rW = constrain(rW + 20, 0, 255);
    gW = constrain(gW + 15, 0, 255);
    bW = constrain(bW + 5,  0, 255);
  }
  
  if (currentPaletteIdx == 1) { rW /= 2; gW = gW * 0.8; bW = constrain(bW * 1.5, 0, 255); }
  if (currentPaletteIdx == 2) { rW /= 2; gW = constrain(gW * 1.3, 0, 255); bW *= 0.7; }
  
  return C16(myESPboy.tft.color565(rW, gW, bW));
}

void setup() {
  myESPboy.begin("Digital Candle");
/*
  //Check OTA2
  if (myESPboy.getKeys()&PAD_ACT || myESPboy.getKeys()&PAD_ESC) { 
     ESPboyTerminalGUI *terminalGUIobj = new ESPboyTerminalGUI(&myESPboy.tft, &myESPboy.mcp);
     ESPboyOTA2 *OTA2obj = new ESPboyOTA2(terminalGUIobj);
  }
*/  
  EEPROM.begin(4);
  currentPaletteIdx = EEPROM.read(0);
  if (currentPaletteIdx > 2) currentPaletteIdx = 0;
  
  for (int i = 0; i < NUM_PARTICLES; i++) particles[i].life = 0;
  for (int x = 0; x < FIRE_WIDTH; x++) {
    fuel[x] = 0;
    for (int y = 0; y < FIRE_HEIGHT; y++) fire[y][x] = 0;
  }
}

void loop() {
  // --- 1. ОБРАБОТКА ВВОДА И ВЕТЕР ---
  uint8_t keys = myESPboy.getKeys();
  
  if ((keys & PAD_ACT) && !(lastKeys & PAD_ACT)) {
    currentPaletteIdx = (currentPaletteIdx + 1) % 3;
    EEPROM.write(0, currentPaletteIdx);
    EEPROM.commit();
  }
  
  if ((keys & PAD_ESC) && !(lastKeys & PAD_ESC)) {
    ledEnabled = !ledEnabled;
  }
  
  if ((keys & PAD_UP) && !(lastKeys & PAD_UP)) {
    if (flamePower < 5) flamePower++;
  }
  if ((keys & PAD_DOWN) && !(lastKeys & PAD_DOWN)) {
    if (flamePower > 1) flamePower--;
  }

  if(!targetWind)
   if (rand() % 100 < 5) {
     targetWind = (rand() % 100 - 50) / 200.0; 
   }
   
  if (rand() % 100 < 1) {
    targetWind = 0.0;
  }
  
  currentWind += (targetWind - currentWind) * 0.0050;
  float activeWind = currentWind;
  
  if (keys & PAD_LEFT)  activeWind = -1; 
  if (keys & PAD_RIGHT) activeWind = 1;
  
  lastKeys = keys;

  // --- 2. ЖАР ОТ ФИТИЛЯ ---
  for (int x = 0; x < FIRE_WIDTH; x++) {
    if (x == WICK_X) fuel[x] = 35;
    else if (x >= WICK_X - 2 && x <= WICK_X + 2) fuel[x] = 35; 
    else fuel[x] = 0; 
    
    fire[WICK_TOP_Y][x] = fuel[x];
  }

  // --- 3. ПЛАВНАЯ ФИЗИКА ОГНЯ (PULL MODEL) ---
  int intDrift = (int)activeWind;
  float fracDrift = fabs(activeWind - (float)intDrift);
  
  if ((rand() % 100) < (fracDrift * 100.0)) {
      intDrift += (activeWind > 0) ? 1 : -1;
  }

  for (int y = 1; y <= WICK_TOP_Y; y++) {
    int h = WICK_TOP_Y - y; 
    int expectedX = WICK_X + round((activeWind * h) / 4.0);
    
    int maxR = 3 + flamePower;
    if (h > 5) maxR += (h - 5) / 4;
    if (h > 12 + flamePower * 2) maxR -= (h - (12 + flamePower * 2)) / 3;
    if (maxR < 0) maxR = 0;

    for (int x = 1; x < FIRE_WIDTH - 1; x++) {
      int srcX = x - intDrift;
      if (srcX < 1) srcX = 1;
      if (srcX > FIRE_WIDTH - 2) srcX = FIRE_WIDTH - 2;

      int sum = fire[y + 1][srcX - 1] + fire[y + 1][srcX] * 2 + fire[y + 1][srcX + 1];
      int avg = sum / 4;

      if (avg > 0) {
        int dist = abs(x - expectedX);
        int cool = 0;
        
        if (dist > maxR) {
          cool = (dist - maxR) + (rand() % 2); 
        } else {
          int noiseChance = 5 + (h / 3);
          if (rand() % 100 < noiseChance) cool = 1;
        }

        int newHeat = avg - cool;
        
        if (newHeat > 0) {
          if (flamePower == 1) newHeat -= (rand() % 2);
          else if (flamePower == 2 && (rand() % 4 == 0)) newHeat--;
          else if (flamePower == 4 && (rand() % 4 == 0)) newHeat++;
          else if (flamePower == 5 && (rand() % 2 == 0)) newHeat++;
        }

        if (newHeat < 0) newHeat = 0;
        if (newHeat > 35) newHeat = 35;
        
        fire[y][x] = newHeat;
      } else {
        fire[y][x] = 0;
      }
    }
  }

  // --- 4. ЯРКИЕ ДЫМКИ И ИСКРЫ ---
  for (int i = 0; i < NUM_PARTICLES; i++) {
    if (particles[i].life <= 0) {
      if (rand() % 100 < 5) {
        particles[i].x = WICK_X + (rand() % 5 - 2);
        particles[i].y = WICK_TOP_Y - (rand() % 20 + 5);
        particles[i].vx = (rand() % 7 - 3) / 15.0;       
        particles[i].vy = -((rand() % 8 + 12) / 10.0);    
        particles[i].maxLife = rand() % 30 + 30;           
        particles[i].life = particles[i].maxLife;
        particles[i].decay = (rand() % 2) + 1;
        
        // 30% частиц рождаются яркими искрами, остальные - дымом
        particles[i].isSpark = (rand() % 100 < 30);
      }
    } else {
      particles[i].y += particles[i].vy;
      particles[i].x += particles[i].vx + (activeWind * 0.5);         
      particles[i].life -= particles[i].decay;
      
      // Искры со временем остывают и превращаются в дым
      if (particles[i].isSpark && particles[i].life < (particles[i].maxLife / 2)) {
        particles[i].isSpark = false;
      }
    }
  }

  int coreHeat = fire[WICK_TOP_Y - 5][WICK_X]; 

  // --- 5. ОТРИСОВКА ---
  const uint16_t* activePal = palettes[currentPaletteIdx];
  
  for (int blockY = 0; blockY < FIRE_HEIGHT; blockY += BLOCK_LINES) {
    int ptr = 0;
    
    for (int y = 0; y < BLOCK_LINES; y++) {
      int screenY = blockY + y;
      
      for (int x = 0; x < FIRE_WIDTH; x++) {
        uint16_t pixelColor = 0;
        
        if (screenY <= WICK_TOP_Y) {
          pixelColor = activePal[fire[screenY][x]];
        } 
        else if (screenY < CANDLE_TOP_Y) {
          if (x == WICK_X || x == WICK_X + 1) {
            if (screenY <= WICK_TOP_Y + 2) {
              uint8_t rEmber = 180 + (rand() % 75);
              uint8_t gEmber = 40 + (rand() % 40);
              pixelColor = C16(myESPboy.tft.color565(rEmber, gEmber, 0));
            } else {
              pixelColor = C16(0x2104);
            }
          }
        } 
        else {
          pixelColor = getCandleWaxPixel(x, screenY, coreHeat);
        }
        
        blockBuffer[currentBuffer][ptr++] = pixelColor;
      }
    }

    // Отрисовка ярких частиц и дыма
    for (int i = 0; i < NUM_PARTICLES; i++) {
      if (particles[i].life > 0) {
        int py = (int)particles[i].y;
        if (py >= blockY && py < blockY + BLOCK_LINES) { 
          int px = (int)particles[i].x;
          if (px >= 0 && px < FIRE_WIDTH) {
            if (particles[i].isSpark) {
              // Яркая горячая искра
              uint8_t rS = map(particles[i].life, 0, particles[i].maxLife, 150, 255);
              uint8_t gS = map(particles[i].life, 0, particles[i].maxLife, 50, 180);
              blockBuffer[currentBuffer][(py - blockY) * FIRE_WIDTH + px] = C16(myESPboy.tft.color565(rS, gS, 0));
            } else {
              // Заметный серый дым
              uint8_t gray = map(particles[i].life, 0, particles[i].maxLife, 25, 110);
              blockBuffer[currentBuffer][(py - blockY) * FIRE_WIDTH + px] = C16(myESPboy.tft.color565(gray, gray, gray));
            }
          }
        }
      }
    }

    waitSPI();
    myESPboy.tft.setWindow(0, blockY, FIRE_WIDTH - 1, blockY + BLOCK_LINES - 1);
    nbSPI_writeBytes((uint8_t*)blockBuffer[currentBuffer], FIRE_WIDTH * BLOCK_LINES * 2);
    currentBuffer = 1 - currentBuffer;
  }

  waitSPI();

  // --- 6. ПРАВИЛЬНЫЙ ЦВЕТ СВЕТОДИОДА ---
  if (!ledEnabled) {
    myESPboy.myLED.setRGB(0, 0, 0); 
  } else {
    int ledVal = map(coreHeat, 0, 35, 0, 5);
    ledVal = constrain(ledVal, 0, 5);
    
    if (currentPaletteIdx == 0) {
      int gVal = (ledVal > 0) ? constrain(ledVal - 1, 1, 5) : 0;
      myESPboy.myLED.setRGB(ledVal, gVal, 0);
    } 
    else if (currentPaletteIdx == 1) {
      int gVal = (ledVal > 1) ? ledVal - 1 : 0;
      myESPboy.myLED.setRGB(0, gVal, ledVal); 
    } 
    else if (currentPaletteIdx == 2) {
      int rVal = (ledVal > 2) ? 1 : 0;
      myESPboy.myLED.setRGB(rVal, ledVal, 0); 
    }
  }

  // --- 7. ГЛУБОКОЕ МЕРЦАНИЕ ПОДСВЕТКИ В ЗАВИСИМОСТИ ОТ ВЕТРА ---
  int baseDac = 3400; 
  
  // Чем сильнее дует ветер (activeWind), тем глубже просаживается напряжение DAC
  int windDip = abs(activeWind * 800); 
  
  // Редкое случайное микро-мерцание самого пламени
  int flicker = (rand() % 100 < 30) ? (rand() % 600 - 300) : 0;
  
  int targetDac = constrain(baseDac - windDip + flicker, 500, 4095);
  

  static uint16_t currentDac = 3400;
  currentDac = (currentDac * 3 + targetDac) / 4; 
  
  myESPboy.mcp.writeDAC(currentDac, false);
}
