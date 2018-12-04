/*
 *  Based on Adafruit_SSD1306
 */

#ifndef _SSD1306_H_
#define _SSD1306_H_

#include "mbed.h"
#include "Adafruit_GFX.h"

#include <vector>
#include <algorithm>

// A DigitalOut sub-class that provides a constructed default state
class DigitalOut2 : public DigitalOut
{
public:
	DigitalOut2(PinName pin, bool active = false) : DigitalOut(pin) { write(active); };
	DigitalOut2& operator= (int value) { write(value); return *this; };
	DigitalOut2& operator= (DigitalOut2& rhs) { write(rhs.read()); return *this; };
	operator int() { return read(); };
};

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

class SSD1306 : public Adafruit_GFX
{
public:
	SSD1306(PinName RST, uint8_t rawHeight = 32, uint8_t rawWidth = 128)
		: Adafruit_GFX(rawWidth, rawHeight)
		, rst(RST,false)
	{
		buffer.resize(rawHeight * rawWidth / 8);
	};

	void begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC);
	
	// These must be implemented in the derived transport driver
	virtual void command(uint8_t c) = 0;
	virtual void data(uint8_t c) = 0;
	virtual void drawPixel(int16_t x, int16_t y, uint16_t color);

	/// Clear the display buffer    
	void clearDisplay(void);
	virtual void invertDisplay(bool i);

	/// Cause the display to be updated with the buffer content.
	void display();
	/// Fill the buffer with the AdaFruit splash screen.
	virtual void splash();
    
protected:
	virtual void sendDisplayBuffer() = 0;
	DigitalOut2 rst;

	// the memory buffer for the LCD
	std::vector<uint8_t> buffer;
};

class SSD1306OverI2C : public SSD1306
{
public:
	static const uint8_t DefaultI2CAddress = 0x78;
	SSD1306OverI2C(I2C &i2c, PinName RST, uint8_t i2cAddress = DefaultI2CAddress, uint8_t rawHeight = 64, uint8_t rawWidth = 128);

	virtual void command(uint8_t c);
	virtual void data(uint8_t c);

protected:
	virtual void sendDisplayBuffer();

	I2C &mi2c;
	uint8_t mi2cAddress;
};

#endif