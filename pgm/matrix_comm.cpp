#include "matrix_comm.h"

// Queue dla komend (do 20 komend w kolejce)
static QueueHandle_t matrixCommandQueue = NULL;
static LEDMatrix* matrixPtr = NULL;
TaskHandle_t MatrixCommTaskHandle = NULL;

// High-priority task obsługujący komunikację Serial z matrycą
void MatrixCommTask(void* parameter) {
    MatrixCommand cmd;
    
    while(1) {
        // Czekaj na komendę z kolejki
        if (xQueueReceive(matrixCommandQueue, &cmd, portMAX_DELAY) == pdTRUE) {
            // Przetwórz komendę
            switch(cmd.type) {
                case MATRIX_CMD_LOAD_GIF:
                    matrixPtr->loadGif(cmd.gif.filename, cmd.gif.x, cmd.gif.y,
                                      cmd.gif.width, cmd.gif.height, cmd.gif.elementId);
                    break;
                    
                case MATRIX_CMD_DISPLAY_TEXT:
                    matrixPtr->displayText(cmd.text.text, cmd.text.x, cmd.text.y,
                                          cmd.text.fontSize, cmd.text.r, cmd.text.g, cmd.text.b,
                                          cmd.text.fontName, cmd.text.elementId, 
                                          cmd.text.blinkInterval);
                    break;
                    
                case MATRIX_CMD_CLEAR_SCREEN:
                    matrixPtr->clearScreen();
                    break;
                    
                case MATRIX_CMD_CLEAR_TEXT:
                    matrixPtr->clearText();
                    break;
                    
                case MATRIX_CMD_DELETE_ELEMENT:
                    matrixPtr->deleteElement(cmd.deleteElement.elementId);
                    break;
                    
                case MATRIX_CMD_SET_BRIGHTNESS:
                    matrixPtr->setBrightness(cmd.brightness.brightness);
                    break;
            }
        }
    }
}

void MatrixCommInit(LEDMatrix* matrix) {
    matrixPtr = matrix;
    
    // Utwórz kolejkę na 20 komend
    matrixCommandQueue = xQueueCreate(20, sizeof(MatrixCommand));
    
    if (matrixCommandQueue == NULL) {
        Serial.println("ERROR: Failed to create matrix command queue!");
        return;
    }
    
    // Utwórz high-priority task
    // Priority: configMAX_PRIORITIES - 2 (niższy tylko od TaskPcf)
    // Core: 0 (oddzielny od loop() który jest na core 1)
    xTaskCreatePinnedToCore(
        MatrixCommTask,           // Task function
        "MatrixComm",             // Task name
        8192,                     // Stack size (8KB)
        NULL,                     // Parameters
        configMAX_PRIORITIES - 2, // High priority (niższy tylko od TaskPcf)
        &MatrixCommTaskHandle,    // Task handle
        0                         // Core 0 (loop() jest na core 1)
    );
    
    Serial.println("MatrixComm task started with high priority on Core 0");
}

// Funkcje pomocnicze - dodają komendy do kolejki (non-blocking)
void MatrixComm_LoadGif(const char* filename, uint16_t x, uint16_t y, 
                        uint16_t width, uint16_t height, uint8_t elementId) {
    MatrixCommand cmd;
    cmd.type = MATRIX_CMD_LOAD_GIF;
    strncpy(cmd.gif.filename, filename, 63);
    cmd.gif.filename[63] = '\0';
    cmd.gif.x = x;
    cmd.gif.y = y;
    cmd.gif.width = width;
    cmd.gif.height = height;
    cmd.gif.elementId = elementId;
    
    xQueueSend(matrixCommandQueue, &cmd, 0);  // 0 = nie czekaj jeśli kolejka pełna
}

void MatrixComm_DisplayText(const char* text, uint16_t x, uint16_t y,
                            uint8_t fontSize, uint8_t r, uint8_t g, uint8_t b,
                            const char* fontName, uint8_t elementId, 
                            uint16_t blinkInterval) {
    MatrixCommand cmd;
    cmd.type = MATRIX_CMD_DISPLAY_TEXT;
    strncpy(cmd.text.text, text, 31);
    cmd.text.text[31] = '\0';
    cmd.text.x = x;
    cmd.text.y = y;
    cmd.text.fontSize = fontSize;
    cmd.text.r = r;
    cmd.text.g = g;
    cmd.text.b = b;
    strncpy(cmd.text.fontName, fontName, 31);
    cmd.text.fontName[31] = '\0';
    cmd.text.elementId = elementId;
    cmd.text.blinkInterval = blinkInterval;
    
    xQueueSend(matrixCommandQueue, &cmd, 0);
}

void MatrixComm_ClearScreen() {
    MatrixCommand cmd;
    cmd.type = MATRIX_CMD_CLEAR_SCREEN;
    xQueueSend(matrixCommandQueue, &cmd, 0);
}

void MatrixComm_ClearText() {
    MatrixCommand cmd;
    cmd.type = MATRIX_CMD_CLEAR_TEXT;
    xQueueSend(matrixCommandQueue, &cmd, 0);
}

void MatrixComm_DeleteElement(uint8_t elementId) {
    MatrixCommand cmd;
    cmd.type = MATRIX_CMD_DELETE_ELEMENT;
    cmd.deleteElement.elementId = elementId;
    xQueueSend(matrixCommandQueue, &cmd, 0);
}

void MatrixComm_SetBrightness(uint8_t brightness) {
    MatrixCommand cmd;
    cmd.type = MATRIX_CMD_SET_BRIGHTNESS;
    cmd.brightness.brightness = brightness;
    xQueueSend(matrixCommandQueue, &cmd, 0);
}

