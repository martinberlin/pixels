#include <pixels.h>

#ifdef RGBW
    // 4 bytes * pixel
    NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt0Ws2812xMethod> strip(PIXELCOUNT, PIXELPIN);
#else
    // RGB 3 bytes * pixel
    NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> strip(PIXELCHUNK, PIXELPIN);
  #ifdef PIXELCHUNK
    NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt1Ws2812xMethod> strip1(PIXELCHUNK, PIXELPIN+1);
  #endif
#endif

PIXELS::PIXELS(){} // I'll do something with this, I swear.

void PIXELS::init(){
    // Starts the LEDs and blanks them out
    strip.Begin();
    strip.Show();
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
    #ifdef PIXELCHUNK
        for(unsigned i = 0; i<chunk; i++){
            strip.SetPixelColor(i, pixels[i]);
        }
        for(unsigned i = chunk; i<(chunk*2); i++){
            strip1.SetPixelColor(i, pixels[i]);
        }
        strip.Show();
        strip1.Show();
    #else
        for(unsigned i = 0; i<cnt; i++){
            strip.SetPixelColor(i, pixels[i]);
        }
        strip.Show();
    #endif
}

pixel *PIXELS::unmarshal(uint8_t *pyld, unsigned len, uint16_t *pixCnt, uint16_t *pixChunk, uint8_t *channel){
    if(pyld[0]!=0x50){
        Serial.println("Missing checkvalue");
        // Set pixCnt to zero as we have not decoded any pixels and return NULL
        *pixCnt = 0;
        return NULL;
    }
    //if (pyld[1]!=syncWord||syncWord==0x0){
    //    *pixCnt = 0;
    //    return NULL;
    //} TODO UNCOMMENT REMOVED FOR EASY NETCAT USAGE QUICKLY
    if(channel!=NULL){
        *channel = pyld[2];
    }
    // Number of chunks:
    uint16_t chunk = pyld[1] | pyld[2]<<8;

    // Decode number of pixels, we don't have to send the entire strip if we don't want to
    uint16_t cnt = pyld[3] | pyld[4]<<8;

    if(cnt>PIXELCOUNT){
        Serial.printf("Max PIXELCOUNT %d got %d pixels\n", PIXELCOUNT, cnt);
        *pixCnt = 0;
        return NULL;
    }
    if (cnt ==0){
        return false;
    }
    // TODO Add logic to return if len is impossibly large or small
    // TODO Add CRC check before setting pixCnt
    *pixCnt = cnt;

    #ifdef RGBW
      return (RgbwColor*)(pyld+5);
    #else
      return (RgbColor*)(pyld+5);
    #endif
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