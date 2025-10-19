#define LINE_LEN  16 + 1 //+EOL

class MyLcd {
public:
    void init(void);
    void setCursor(uint8_t x, uint8_t y);
    void print(const char* text);
    void print(const uint32_t digit);
    void clear(void);

    void handle(void);

private:
    //char currentTxt[2][LINE_LEN];
    char currentTxt1[LINE_LEN*2];
    char currentTxt2[LINE_LEN*2];
    uint8_t m_x;
    uint8_t m_y;
};

extern MyLcd myLcd;