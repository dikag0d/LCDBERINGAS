#include "lcd_beringas.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"

unsigned long lastInputTime = 0;
uint8_t charIndex = 0;
char lastGroup[6] = "";
// Constructors
lcd_beringas::lcd_beringas(uint8_t rs, uint8_t rw, uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
			     uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  init(0, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

lcd_beringas::lcd_beringas(uint8_t rs, uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
			     uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  init(0, rs, 255, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

lcd_beringas::lcd_beringas(uint8_t rs, uint8_t rw, uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  init(1, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

lcd_beringas::lcd_beringas(uint8_t rs,  uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  init(1, rs, 255, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

void lcd_beringas::init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
			 uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
			 uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  _rs_pin = rs;
  _rw_pin = rw;
  _enable_pin = enable;
  
  _data_pins[0] = d0;
  _data_pins[1] = d1;
  _data_pins[2] = d2;
  _data_pins[3] = d3; 
  _data_pins[4] = d4;
  _data_pins[5] = d5;
  _data_pins[6] = d6;
  _data_pins[7] = d7; 

  if (fourbitmode)
    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  else 
    _displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
  
  begin(16, 1);  
}

void lcd_beringas::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);  

  if ((dotsize != LCD_5x8DOTS) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }

  pinMode(_rs_pin, OUTPUT);
  if (_rw_pin != 255) { 
    pinMode(_rw_pin, OUTPUT);
  }
  pinMode(_enable_pin, OUTPUT);
  
  for (int i=0; i<((_displayfunction & LCD_8BITMODE) ? 8 : 4); ++i) {
    pinMode(_data_pins[i], OUTPUT);
   } 

  delayMicroseconds(50000); 
  digitalWrite(_rs_pin, LOW);
  digitalWrite(_enable_pin, LOW);
  if (_rw_pin != 255) { 
    digitalWrite(_rw_pin, LOW);
  }
  
  if (! (_displayfunction & LCD_8BITMODE)) {
    write4bits(0x03);
    delayMicroseconds(4500);

    write4bits(0x03);
    delayMicroseconds(4500);
    
    write4bits(0x03); 
    delayMicroseconds(150);

    write4bits(0x02); 
  } else {
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(4500);
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(150);
    command(LCD_FUNCTIONSET | _displayfunction);
  }

  command(LCD_FUNCTIONSET | _displayfunction);  

  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  clear();

  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

void lcd_beringas::setRowOffsets(int row0, int row1, int row2, int row3)
{
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

void lcd_beringas::clear()
{
  command(LCD_CLEARDISPLAY);
  delayMicroseconds(2000);
}

void lcd_beringas::home()
{
  command(LCD_RETURNHOME);
  delayMicroseconds(2000);
}

void lcd_beringas::setCursor(uint8_t col, uint8_t row)
{
  const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
  if ( row >= max_lines ) row = max_lines - 1;
  if ( row >= _numlines ) row = _numlines - 1;
  
  command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

void lcd_beringas::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void lcd_beringas::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_beringas::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void lcd_beringas::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_beringas::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void lcd_beringas::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void lcd_beringas::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void lcd_beringas::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void lcd_beringas::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

void lcd_beringas::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

void lcd_beringas::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

void lcd_beringas::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

void lcd_beringas::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7;
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}
void lcd_beringas::nyalakanDisplay() {
  _displaycontrol |= 0x04;         // Bit untuk DISPLAY_ON
  command(0x08 | _displaycontrol); // 0x08 = Display Control command
}

void lcd_beringas::matikanDisplay() {
  _displaycontrol &= ~0x04;        // Clear bit DISPLAY_ON
  command(0x08 | _displaycontrol); // Kirim command
}

void lcd_beringas::runningTextLoop(const char* text, uint8_t row, uint16_t delayTime) {
  int textLength = strlen(text);
  int lcdWidth = 16; // bisa diganti pakai variabel internal kalau kamu simpan

  if (textLength <= lcdWidth) {
    setCursor(0, row);
    print(text);
    return;
  }

  char buffer[17];
  int i = 0;

  while (true) {
    if (i > textLength - lcdWidth) i = 0;

    strncpy(buffer, text + i, lcdWidth);
    buffer[lcdWidth] = '\0';

    setCursor(0, row);
    print(buffer);
    delay(delayTime);

    i++;
  }
}
void lcd_beringas::kedipTeks(const char* text, uint8_t col, uint8_t row, uint8_t jumlahKedip, uint16_t interval) {
  for (uint8_t i = 0; i < jumlahKedip; i++) {
    setCursor(col, row);
    print(text);
    delay(interval);
    setCursor(col, row);
    for (size_t j = 0; j < strlen(text); j++) print(" ");
    delay(interval);
  }
}
void lcd_beringas::efekKetik(const char* text, uint8_t col, uint8_t row, uint16_t kecepatan) {
  setCursor(col, row);
  for (size_t i = 0; i < strlen(text); i++) {
    print(text[i]);
    delay(kecepatan);
  }
}
void lcd_beringas::jamRealTime(uint8_t col, uint8_t row) {
  static unsigned long previousMillis = 0;
  static uint8_t jam = 12, menit = 0, detik = 0;

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    detik++;
    if (detik >= 60) {
      detik = 0;
      menit++;
      if (menit >= 60) {
        menit = 0;
        jam = (jam + 1) % 24;
      }
    }

    char buffer[9]; // Format: 12:34:56
    sprintf(buffer, "%02d:%02d:%02d", jam, menit, detik);
    setCursor(col, row);
    print(buffer);
  }
}
void lcd_beringas::inputPS3Nokia(const char* group, bool pindahKursor) {
  unsigned long now = millis();

  if (pindahKursor) {
    // Geser kursor ke kanan
    if (_x < 15) {
      _x++;
    } else if (_y == 0) {
      _x = 0;
      _y = 1;
    }
    setCursor(_x, _y);
    return;
  }

  // Jika group kosong, keluar
  if (group == nullptr || strlen(group) == 0) return;

  // Cek apakah tombol berbeda atau lebih dari 1 detik sejak input terakhir
  if (strcmp(group, lastGroup) != 0 || now - lastInputTime > 1000) {
    charIndex = 0;
  }

  // Simpan info terakhir
  strcpy(lastGroup, group);
  lastInputTime = now;

  // Ambil dan tampilkan karakter
  char ch = group[charIndex];
  setCursor(_x, _y);
  print(ch);
  setCursor(_x, _y); // tetap di posisi untuk re-tap

  charIndex++;
  if (charIndex >= strlen(group)) {
    charIndex = 0;
  }

  delay(200); // Hindari spam input
}
void lcd_beringas::hapusKarakter() {
  if (_x > 0) {
    _x--;
  } else if (_y == 1 && _x == 0) {
    _y = 0;
    _x = 15;
  } else {
    return;
  }
  setCursor(_x, _y);
  print(' ');
  setCursor(_x, _y);
}

inline void lcd_beringas::command(uint8_t value) {
  send(value, LOW);
}

inline size_t lcd_beringas::write(uint8_t value) {
  send(value, HIGH);
  return 1;
}

void lcd_beringas::send(uint8_t value, uint8_t mode) {
  digitalWrite(_rs_pin, mode);
  if (_rw_pin != 255) { 
    digitalWrite(_rw_pin, LOW);
  }
  
  if (_displayfunction & LCD_8BITMODE) {
    write8bits(value); 
  } else {
    write4bits(value>>4);
    write4bits(value);
  }
}

void lcd_beringas::pulseEnable(void) {
  digitalWrite(_enable_pin, LOW);
  delayMicroseconds(1);    
  digitalWrite(_enable_pin, HIGH);
  delayMicroseconds(1);    
  digitalWrite(_enable_pin, LOW);
  delayMicroseconds(100);   
}

void lcd_beringas::write4bits(uint8_t value) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(_data_pins[i], (value >> i) & 0x01);
  }
  pulseEnable();
}

void lcd_beringas::write8bits(uint8_t value) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(_data_pins[i], (value >> i) & 0x01);
  }
  pulseEnable();
}
