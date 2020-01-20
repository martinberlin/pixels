/**
 * PIXELS is a super simple LED application layer to allow for RGB and RGBW to be transported between any data transport layer. PIXELS was
 * written by Samuel Archibald (@IoTPanic) for the UDPX project found at https://github.com/martinberlin/udpx.
 */

#ifndef neopixelbus_h
#include <NeoPixelBus.h>
#endif

#ifndef pixels_h
#define pixels_h
//#define RGBW //Removing the comment will enable RGBW instead of RGB

#define PIXELCOUNT 1000
// Uncomment if we sent per chunks in different channels
#define PIXELCHUNK 500 

const char pixelpin[] = { 19, 18 };

#define USECRC false

#ifdef RGBW
    typedef RgbwColor pixel;
#else
    typedef RgbColor pixel;
#endif

class PIXELS
{
    public:
    PIXELS();

    void init();
    // receive requires a pointer to a uint8_t array and the length of the array from a callback function
    bool receive(uint8_t *pyld, unsigned len);
    // sync will return the sync byte as a uint8_t to ensure the library and controller are on the same page
    uint8_t sync();
    // Write sets the LED at the location to R,G,B to the values provided for the next show()
    void write(unsigned location, uint8_t R, uint8_t G, uint8_t B, uint8_t W = 0);
    // Show writes the previously made write() calls to the array of LEDs
    void show();

    void all_off();
    private:
       bool unmarshal(uint8_t *pyld, unsigned len, uint16_t *pixCnt, uint16_t *pixChunk, uint8_t *channel=NULL);

    uint8_t syncWord = 0x0;

    // We want to inform our lib if RGB or RGBW was selected
    #ifdef RGBW
    const bool RGBWE = true;
    #else
    const bool RGBWE = false;
    #endif
};
#endif
