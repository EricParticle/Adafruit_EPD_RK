#include "Adafruit_EPD.h"
#include "Adafruit_IL0373.h"

#define BUSY_WAIT 500

#ifdef USE_EXTERNAL_SRAM

/**************************************************************************/
/*!
    @brief constructor if using external SRAM chip and software SPI
    @param width the width of the display in pixels
    @param height the height of the display in pixels
    @param SID the SID pin to use
    @param SCLK the SCLK pin to use
    @param DC the data/command pin to use
    @param RST the reset pin to use
    @param CS the chip select pin to use
    @param SRCS the SRAM chip select pin to use
    @param MISO the MISO pin to use
    @param BUSY the busy pin to use
*/
/**************************************************************************/
Adafruit_IL0373::Adafruit_IL0373(int width, int height, int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS, int8_t SRCS, int8_t spi_miso, int8_t BUSY) : Adafruit_EPD(width, height, SID, SCLK, DC, RST, CS, SRCS, spi_miso, BUSY){
#else

/**************************************************************************/
/*!
    @brief constructor if using on-chip RAM and software SPI
    @param width the width of the display in pixels
    @param height the height of the display in pixels
    @param SID the SID pin to use
    @param SCLK the SCLK pin to use
    @param DC the data/command pin to use
    @param RST the reset pin to use
    @param CS the chip select pin to use
    @param BUSY the busy pin to use
*/
/**************************************************************************/
Adafruit_IL0373::Adafruit_IL0373(int width, int height, int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY) : Adafruit_EPD(width, height, SID, SCLK, DC, RST, CS, BUSY) {
  bw_buf = (uint8_t *)malloc(((uint32_t)width * (uint32_t)height) / 8);
  red_buf = (uint8_t *)malloc(((uint32_t)width * (uint32_t)height) / 8);
#endif
  bw_bufsize = ((uint32_t)width * (uint32_t)height) / 8;
  red_bufsize = bw_bufsize;
}

// constructor for hardware SPI - we indicate DataCommand, ChipSelect, Reset
#ifdef USE_EXTERNAL_SRAM

/**************************************************************************/
/*!
    @brief constructor if using on-chip RAM and hardware SPI
    @param width the width of the display in pixels
    @param height the height of the display in pixels
    @param DC the data/command pin to use
    @param RST the reset pin to use
    @param CS the chip select pin to use
    @param SRCS the SRAM chip select pin to use
    @param BUSY the busy pin to use
*/
/**************************************************************************/
Adafruit_IL0373::Adafruit_IL0373(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t SRCS, int8_t BUSY) : Adafruit_EPD(width, height, DC, RST, CS, SRCS, BUSY) {
#else

/**************************************************************************/
/*!
    @brief constructor if using on-chip RAM and hardware SPI
    @param width the width of the display in pixels
    @param height the height of the display in pixels
    @param DC the data/command pin to use
    @param RST the reset pin to use
    @param CS the chip select pin to use
    @param BUSY the busy pin to use
*/
/**************************************************************************/
Adafruit_IL0373::Adafruit_IL0373(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY) : Adafruit_EPD(width, height, DC, RST, CS, BUSY) {
  bw_buf = (uint8_t *)malloc(((uint32_t)width * (uint32_t)height) / 8);
  red_buf = (uint8_t *)malloc(((uint32_t)width * (uint32_t)height) / 8);
#endif
  bw_bufsize = ((uint32_t)width * (uint32_t)height) / 8;
  red_bufsize = bw_bufsize;
}

/**************************************************************************/
/*!
    @brief wait for busy signal to end
*/
/**************************************************************************/
void Adafruit_IL0373::busy_wait(void)
{
  if (busy >= 0) {
    while(!digitalRead(busy)) {
      delay(1); //wait for busy high
    }
  } else {
    delay(BUSY_WAIT);
  }
}

/**************************************************************************/
/*!
    @brief begin communication with and set up the display.
    @param reset if true the reset pin will be toggled.
*/
/**************************************************************************/
void Adafruit_IL0373::begin(bool reset)
{
  uint8_t buf[5];
  Adafruit_EPD::begin(reset);
  invertColorLogic(0, true);  // black defaults to inverted
  invertColorLogic(1, true);  // red defaults to inverted
  
  buf[0] = 0x03;
  buf[1] = 0x00;
  buf[2] = 0x2b;
  buf[3] = 0x2b;
  buf[4] = 0x09;
  EPD_command(IL0373_POWER_SETTING, buf, 5);
  
  buf[0] = 0x17;
  buf[1] = 0x17;
  buf[2] = 0x17;
  EPD_command(IL0373_BOOSTER_SOFT_START, buf, 3);
}

/**************************************************************************/
/*!
    @brief signal the display to update
*/
/**************************************************************************/
void Adafruit_IL0373::update()
{
  EPD_command(IL0373_DISPLAY_REFRESH);
	
  busy_wait();
  
  //power off
  uint8_t buf[4];
  
  buf[0] = 0x17;
  EPD_command(IL0373_CDI, buf, 1);
  
  buf[0] = 0x00;
  EPD_command(IL0373_VCM_DC_SETTING, buf, 0);
  
  EPD_command(IL0373_POWER_OFF);
  
  delay(2000);
}

/**************************************************************************/
/*!
    @brief start up the display
*/
/**************************************************************************/
void Adafruit_IL0373::powerUp()
{
	uint8_t buf[4];
	 
	EPD_command(IL0373_POWER_ON);
	busy_wait();
	delay(200);
	
	buf[0] = 0xCF;
	EPD_command(IL0373_PANEL_SETTING, buf, 1);
	
	buf[0] = 0x37;
	EPD_command(IL0373_CDI, buf, 1);
	
	buf[0] = 0x29;
	EPD_command(IL0373_PLL, buf, 1);

	buf[0] = HEIGHT & 0xFF;
	buf[1] = (WIDTH >> 8) & 0xFF;
	buf[2] = WIDTH & 0xFF;
	EPD_command(IL0373_RESOLUTION, buf, 3);
			
	buf[0] = 0x0A;
	EPD_command(IL0373_VCM_DC_SETTING, buf, 1);
	delay(20);
}

/**************************************************************************/
/*!
    @brief show the data stored in the buffer on the display
*/
/**************************************************************************/
void Adafruit_IL0373::display()
{
	powerUp();
	
#ifdef USE_EXTERNAL_SRAM
	uint8_t c;
	
	sram.csLow();
	//send read command
	fastSPIwrite(MCPSRAM_READ);
	
	//send address
	fastSPIwrite(0x00);
	fastSPIwrite(0x00);
	
	//first data byte from SRAM will be transfered in at the same time as the EPD command is transferred out
	c = EPD_command(EPD_RAM_BW, false);
	
	dcHigh();
	for(uint16_t i=0; i<bw_bufsize; i++){
		c = fastSPIwrite(c);
	}
	csHigh();
	sram.csHigh();
	
	delay(2);
	
	sram.csLow();
	//send write command
	fastSPIwrite(MCPSRAM_READ);
	
	uint8_t b[2];
	b[0] = (bw_bufsize >> 8);
	b[1] = (bw_bufsize & 0xFF);
	//send address
	fastSPIwrite(b[0]);
	fastSPIwrite(b[1]);
	
	//first data byte from SRAM will be transfered in at the same time as the EPD command is transferred out
	c = EPD_command(EPD_RAM_RED, false);
	
	dcHigh();
	
	for(uint16_t i=0; i<red_bufsize; i++){
		c = fastSPIwrite(c);
	}
	csHigh();
	sram.csHigh();
#else
	//write image
	EPD_command(EPD_RAM_BW, false);
	dcHigh();

	for(uint16_t i=0; i<bw_bufsize; i++){
		fastSPIwrite(bw_buf[i]);
	}
	csHigh();
	
	EPD_command(EPD_RAM_RED, false);
	dcHigh();
		
	for(uint16_t i=0; i<red_bufsize; i++){
		fastSPIwrite(red_buf[i]);
	}
	csHigh();

#endif
	update();
}

/**************************************************************************/
/*!
    @brief draw a single pixel on the screen
	@param x the x axis position
	@param y the y axis position
	@param color the color of the pixel
*/
/**************************************************************************/
void Adafruit_IL0373::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;
	
  uint8_t *pBuf;
  
  // check rotation, move pixel around if necessary
  switch (getRotation()) {
  case 1:
    EPD_swap(x, y);
    x = WIDTH - x - 1;
    break;
  case 2:
    x = WIDTH - x - 1;
    y = HEIGHT - y - 1;
    break;
  case 3:
    EPD_swap(x, y);
    y = HEIGHT - y - 1;
    break;
  }
  //make our buffer happy
  x = (x == 0 ? 1 : x);
  
  uint16_t addr = ( (uint32_t)(WIDTH - x) * (uint32_t)HEIGHT + y)/8;
  
#ifdef USE_EXTERNAL_SRAM
  if ((color == EPD_RED) || (color == EPD_GRAY)) {
    addr = addr + bw_bufsize;    //red is written after bw
  }
  uint8_t c = sram.read8(addr);
  pBuf = &c;
#else
  if((color == EPD_RED) || (color == EPD_GRAY)) {
    pBuf = red_buf + addr;
  } else {
    pBuf = bw_buf + addr;
  }
#endif
  if (((color == EPD_RED || color == EPD_GRAY) && redInverted) || 
      ((color == EPD_BLACK) && blackInverted)) {
    *pBuf &= ~(1 << (7 - y%8));
  } else if (((color == EPD_RED || color == EPD_GRAY) && !redInverted) || 
	     ((color == EPD_BLACK) && !blackInverted)) {
    *pBuf |= (1 << (7 - y%8));
  } else if (color == EPD_INVERSE) {
    *pBuf ^= (1 << (7 - y%8));
  }
#ifdef USE_EXTERNAL_SRAM
  sram.write8(addr, *pBuf);
#endif
}

/**************************************************************************/
/*!
    @brief clear all data buffers
*/
/**************************************************************************/
void Adafruit_IL0373::clearBuffer()
{
#ifdef USE_EXTERNAL_SRAM
  if (blackInverted) {
    sram.erase(0x00, bw_bufsize, 0xFF);
  } else {
    sram.erase(0x00, bw_bufsize, 0x00);
  }
  if (redInverted) {
    sram.erase(bw_bufsize, red_bufsize, 0xFF);
  } else {
    sram.erase(bw_bufsize, red_bufsize, 0x00);
  }
#else
  if (blackInverted) {
    memset(bw_buf, 0xFF, bw_bufsize);
  } else {
    memset(bw_buf, 0x00, bw_bufsize);
  }
  if (redInverted) {
    memset(red_buf, 0xFF, red_bufsize);
  } else {
    memset(red_buf, 0x00, red_bufsize);
  }
#endif
}

/**************************************************************************/
/*!
    @brief clear the display twice to remove any spooky ghost images
*/
/**************************************************************************/
void Adafruit_IL0373::clearDisplay() {
  clearBuffer();
  display();
  delay(100);
  display();
}
