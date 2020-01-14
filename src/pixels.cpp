#include <pixels.h>

#ifdef RGBW
    // 4 bytes * pixel
    NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt0Ws2812xMethod> strip(PIXELCOUNT, pixelpin[0]);
#else
    // RGB 3 bytes * pixel
    NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> strip(PIXELCHUNK, pixelpin[0]);
  #ifdef PIXELCHUNK
    NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt1Ws2812xMethod> strip1(PIXELCHUNK, pixelpin[1]);
  #endif
#endif

PIXELS::PIXELS(){} // I'll do something with this, I swear.

void PIXELS::init(){
    // Starts the LEDs and blanks them out
    strip.Begin();
    strip.Show();
    #ifdef PIXELCHUNK
    strip1.Begin();
    strip1.Show();
    #endif
}

bool PIXELS::receive(uint8_t *pyld, unsigned length){
    uint16_t pixCnt = 0;
    uint16_t pixChunk = 0;
    pixel *pattern = unmarshal(pyld, length, &pixCnt, &pixChunk);
    if(pixCnt==0){
        strip.Show();
        Serial.println("Clearing strand");
        return true;
    }
    /*
    for(uint i=0; i<pixCnt; i++){
        Serial.print("Got LED value RGB(");
        Serial.print(pattern[i].R);
        Serial.print(",");
        Serial.print(pattern[i].G);
        Serial.print(",");
        Serial.print(pattern[i].B);
        Serial.println(")");
    }
    */
    this->show(pattern, pixCnt, pixChunk);
    
    return true;
}

void PIXELS::write(unsigned location, uint8_t R, uint8_t G, uint8_t B, uint8_t W){
    #ifdef RGBW
    strip.SetPixelColor(location, RgbwColor(R,G,B,W));
    #else
    strip.SetPixelColor(location, RgbColor(R,G,B));
    #endif
}

void PIXELS::show(){
    strip.Show();
}

void PIXELS::show(pixel *pixels, unsigned cnt, unsigned chunk){
    int firstChunk = cnt;
    if (chunk) {
        firstChunk = chunk;
    }
    for(unsigned i = 0; i<firstChunk; i++){
        strip.SetPixelColor(i, pixels[i]);
    }
    strip.Show();

    #ifdef PIXELCHUNK
        int pixelIndex = 0;
        for(unsigned i = chunk; i<(chunk*2); i++){
            strip1.SetPixelColor(pixelIndex, pixels[i]);
            pixelIndex++;
        }
        strip1.Show(); 
    #endif
}

pixel *PIXELS::unmarshal(uint8_t *pyld, unsigned len, uint16_t *pixCnt, uint16_t *pixChunk, uint8_t *channel){
    uint8_t payloadByteStart = 6;
    // Number of chunks:
    uint16_t chunk = pyld[2] | pyld[3]<<8;
    // Decode number of pixels, we don't have to send the entire strip if we don't want to
    uint16_t cnt = pyld[4] | pyld[5]<<8;
    *pixCnt = cnt;
    *pixChunk = chunk;

    if(cnt>PIXELCOUNT){
        Serial.printf("Max PIXELCOUNT %d got %d pixels\n", PIXELCOUNT, cnt);
        *pixCnt = 0;
        return NULL;
    }
    if (cnt == 0){
        return false;
    }

    
    // Protocol P or R 565
    switch (pyld[0])
    {
    case 0x50:
        /* pixels */
        {
        #ifdef RGBW
          return (RgbwColor*)(pyld+payloadByteStart);
        #else
          return (RgbColor*)(pyld+payloadByteStart);
        #endif
        break;
        }
        break;
        // TODO: Not considering RGBW for 565 so far
    case 0x52:
    {
        /* 565 */
        pixel *result = new pixel[cnt];
        for(uint16_t i = 0; i<cnt; i++){
            uint16_t data16 = (uint16_t) pyld[payloadByteStart+(i*2)] << 8 | pyld[payloadByteStart+(i*2+1)];
            result[i].R = ((((data16 >> 11) & 0x1F) * 527) + 23) >> 6;
            result[i].G = ((((data16 >> 5) & 0x3F) * 259) + 33) >> 6;
            result[i].B = (((data16 & 0x1F) * 527) + 23) >> 6;
        }
        RgbColor* pixels = result;
        delete result;
        return pixels;
          break;
    }

    default:
        Serial.println("Missing checkvalue");
        // Set pixCnt to zero as we have not decoded any pixels and return NULL
        *pixCnt = 0;
        return NULL;
        break;
    }
}

void PIXELS::all_off(){
    #ifdef PIXELCHUNK
    for(unsigned i = 0; i<PIXELCHUNK; i++){
        #ifdef RGBW
        strip.SetPixelColor(i, RgbwColor(0,0,0,0));
        #else
        strip.SetPixelColor(i, RgbColor(0,0,0));
        #endif
    }
    for(unsigned i = PIXELCHUNK; i<(PIXELCHUNK*2); i++){
        #ifdef RGBW
        strip1.SetPixelColor(i, RgbwColor(0,0,0,0));
        #else
        strip1.SetPixelColor(i, RgbColor(0,0,0));
        #endif
    }
    strip.Show();
    strip1.Show();
    #else
    for(unsigned i = 0; i<PIXELCOUNT; i++){
        #ifdef RGBW
        strip.SetPixelColor(i, RgbwColor(0,0,0,0));
        #else
        strip.SetPixelColor(i, RgbColor(0,0,0));
        #endif
    }
    strip.Show();
    #endif
}