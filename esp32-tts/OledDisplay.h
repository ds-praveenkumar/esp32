#pragma once
#include <Arduino.h>

class OledDisplayClass {
public:
    void init();
    void clear();
    void test_fill();
    void show(const char *line1, const char *line2);

private:
    void write_cmd(uint8_t cmd);
    void write_data(const uint8_t *data, size_t len);
    void set_cursor(uint8_t page, uint8_t col);
    void draw_char(uint8_t x, uint8_t page, char c);
    void draw_string(uint8_t x, uint8_t page, const char *str);
};

extern OledDisplayClass OledDisplay;
