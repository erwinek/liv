/*
 * AdvancedDemo - Zaawansowany przykład użycia biblioteki LEDMatrix
 * 
 * Ten przykład demonstruje:
 * - Różne rozmiary czcionek
 * - Różne pozycje tekstu
 * - Ładowanie animacji GIF
 * - Dynamiczną zmianę jasności
 * - Animację tekstu
 */

#include "LEDMatrix.h"

LEDMatrix matrix(Serial1);

// Stany programu
enum State {
    STATE_TEXT,
    STATE_GIF,
    STATE_BRIGHTNESS,
    STATE_ANIMATION
};

State currentState = STATE_TEXT;
unsigned long stateStartTime = 0;
unsigned long lastUpdate = 0;
uint8_t brightness = 80;
bool brightnessDirection = true;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("Advanced LED Matrix Demo");
    Serial.println("========================================\n");
    
    // Inicjalizacja
    matrix.begin(1000000);
    delay(500);
    matrix.clearScreen();
    delay(500);
    matrix.setBrightness(brightness);
    
    Serial.println("Starting demo sequence...\n");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Zmiana stanu co 5 sekund
    if (currentTime - stateStartTime > 5000) {
        stateStartTime = currentTime;
        nextState();
    }
    
    // Aktualizacja w zależności od stanu
    switch (currentState) {
        case STATE_TEXT:
            updateTextDemo(currentTime);
            break;
            
        case STATE_GIF:
            updateGifDemo(currentTime);
            break;
            
        case STATE_BRIGHTNESS:
            updateBrightnessDemo(currentTime);
            break;
            
        case STATE_ANIMATION:
            updateAnimationDemo(currentTime);
            break;
    }
    
    delay(10);
}

void nextState() {
    currentState = (State)((currentState + 1) % 4);
    
    switch (currentState) {
        case STATE_TEXT:
            Serial.println("State: Text Demo");
            break;
        case STATE_GIF:
            Serial.println("State: GIF Demo");
            break;
        case STATE_BRIGHTNESS:
            Serial.println("State: Brightness Demo");
            break;
        case STATE_ANIMATION:
            Serial.println("State: Animation Demo");
            break;
    }
}

void updateTextDemo(unsigned long currentTime) {
    static uint8_t textIndex = 0;
    static unsigned long lastTextUpdate = 0;
    
    if (currentTime - lastTextUpdate > 1000) {
        lastTextUpdate = currentTime;
        
        const char* texts[] = {"SMALL", "MEDIUM", "LARGE"};
        uint8_t sizes[] = {1, 2, 3};
        uint8_t colors[][3] = {
            {255, 0, 0},    // Czerwony
            {0, 255, 0},    // Zielony
            {0, 0, 255}     // Niebieski
        };
        
        matrix.displayText(
            texts[textIndex],
            0, 0,
            sizes[textIndex],
            colors[textIndex][0],
            colors[textIndex][1],
            colors[textIndex][2]
        );
        
        textIndex = (textIndex + 1) % 3;
    }
}

void updateGifDemo(unsigned long currentTime) {
    static bool gifLoaded = false;
    
    if (!gifLoaded) {
        Serial.println("Loading GIF: anim/1.gif");
        matrix.loadGif("anim/1.gif", 0, 0, 96, 96);
        gifLoaded = true;
    }
}

void updateBrightnessDemo(unsigned long currentTime) {
    static unsigned long lastBrightnessUpdate = 0;
    
    if (currentTime - lastBrightnessUpdate > 100) {
        lastBrightnessUpdate = currentTime;
        
        // Zmiana jasności w tę i z powrotem
        if (brightnessDirection) {
            brightness++;
            if (brightness >= 100) {
                brightnessDirection = false;
            }
        } else {
            brightness--;
            if (brightness <= 20) {
                brightnessDirection = true;
            }
        }
        
        matrix.setBrightness(brightness);
        matrix.displayText("BRIGHTNESS", 0, 0, 2, 255, 255, 0);
        
        Serial.print("Brightness: ");
        Serial.println(brightness);
    }
}

void updateAnimationDemo(unsigned long currentTime) {
    static unsigned long lastAnimUpdate = 0;
    static uint8_t animStep = 0;
    
    if (currentTime - lastAnimUpdate > 200) {
        lastAnimUpdate = currentTime;
        
        // Animacja tekstu poruszającego się po ekranie
        uint8_t x = (animStep * 10) % 192;
        uint8_t y = (animStep * 5) % 192;
        
        // Oblicz kolor
        uint8_t r = 255 * (sin(animStep * PI / 30) + 1) / 2;
        uint8_t g = 255 * (sin((animStep + 40) * PI / 30) + 1) / 2;
        uint8_t b = 255 * (sin((animStep + 80) * PI / 30) + 1) / 2;
        
        matrix.displayText("ANIMATE", x, y, 2, r, g, b);
        
        animStep++;
    }
}

