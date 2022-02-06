#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "transfer_handler.h"
 
#define SOURCE_X 8
#define SOURCE_Y 8
#define SCALE 8

#define DEST_X (SOURCE_X * SCALE)
#define DEST_Y (SOURCE_Y * SCALE)
 
#define CLAMP(v, min, max) if(v < min) { v = min; } else if(v > max) { v = max; }

static int16_t destination_data[DEST_X * DEST_Y];

static float cubic_hermite(float A, float B, float C, float D, float t) {
 
    float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
    float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
    float c = -A / 2.0f + C / 2.0f;
    float d = B;
 
    return a*t*t*t + b*t*t + c*t + d;
}
 
void get_pixel_clamped(int16_t *source_data, int x, int y, int16_t *temp)  {
 
    CLAMP(x, 0, SOURCE_X - 1);
    CLAMP(y, 0, SOURCE_Y - 1);
    
    *temp = source_data[x+(SOURCE_X*y)];
    
}
 
void sample_bicubic(int16_t *source_data, float u, float v, int16_t sample[]) {
 
    float x = (u * SOURCE_X) - 0.5f;
    int xint = (int)x;
    float xfract = x-floor(x);
 
    float y = (v * SOURCE_Y) - 0.5f;
    int yint = (int)y;
    float yfract = y - floor(y);
    
    int i;
 
    int16_t p00[1];
    int16_t p10[1];
    int16_t p20[1];
    int16_t p30[1];
    
    int16_t p01[1];
    int16_t p11[1];
    int16_t p21[1];
    int16_t p31[1];
 
    int16_t p02[1];
    int16_t p12[1];
    int16_t p22[1];
    int16_t p32[1];
 
    int16_t p03[1];
    int16_t p13[1];
    int16_t p23[1];
    int16_t p33[1];
    
    // 1st row
    get_pixel_clamped(source_data, xint - 1, yint - 1, p00);   
    get_pixel_clamped(source_data, xint + 0, yint - 1, p10);
    get_pixel_clamped(source_data, xint + 1, yint - 1, p20);
    get_pixel_clamped(source_data, xint + 2, yint - 1, p30);
    
    // 2nd row
    get_pixel_clamped(source_data, xint - 1, yint + 0, p01);
    get_pixel_clamped(source_data, xint + 0, yint + 0, p11);
    get_pixel_clamped(source_data, xint + 1, yint + 0, p21);
    get_pixel_clamped(source_data, xint + 2, yint + 0, p31);
 
    // 3rd row
    get_pixel_clamped(source_data, xint - 1, yint + 1, p02);
    get_pixel_clamped(source_data, xint + 0, yint + 1, p12);
    get_pixel_clamped(source_data, xint + 1, yint + 1, p22);
    get_pixel_clamped(source_data, xint + 2, yint + 1, p32);
 
    // 4th row
    get_pixel_clamped(source_data, xint - 1, yint + 2, p03);
    get_pixel_clamped(source_data, xint + 0, yint + 2, p13);
    get_pixel_clamped(source_data, xint + 1, yint + 2, p23);
    get_pixel_clamped(source_data, xint + 2, yint + 2, p33);
    
    // interpolate bi-cubically!
    i = 0;
 
    float col0 = cubic_hermite(p00[i], p10[i], p20[i], p30[i], xfract);
    float col1 = cubic_hermite(p01[i], p11[i], p21[i], p31[i], xfract);
    float col2 = cubic_hermite(p02[i], p12[i], p22[i], p32[i], xfract);
    float col3 = cubic_hermite(p03[i], p13[i], p23[i], p33[i], xfract);

    float value = cubic_hermite(col0, col1, col2, col3, yfract);

    CLAMP(value, -32768.0f, 32767.0f);

    sample[i] = (int16_t)value;
   
}
 
void resize_image(int16_t *source_data) {
 
    int16_t sample[1];
    int y, x;
    
    for (y = 0; y < DEST_Y; y++) {
 
        float v = (float)y / (float)(DEST_Y - 1);
        for (x = 0; x < DEST_X; x++) {
 
            float u = (float)x / (float)(DEST_X - 1);
            
            sample_bicubic(source_data, u, v, sample);
            
            destination_data[x+(DEST_X*y)] = sample[0];
            
        }
    }
}

int16_t max(int16_t* a, int n)
{
    int16_t maxnum = a[0];
    
    for(int i=0;i<n;i++)
    {
        if(a[i] > maxnum)
            maxnum = a[i];
    
    }
    return maxnum;
}

int16_t min(int16_t* a, int n)
{
    int16_t minnum = a[0];
    
    for(int i=0;i<n;i++)
    {
        if(a[i] < minnum)
            minnum = a[i];
    
    }
    return minnum;
}

int16_t get_dest(int x, int y){

    return destination_data[x+(DEST_X*y)];

}

void set_dest(int x, int y, int16_t val){

    destination_data[x+(DEST_X*y)] = val;

}

void set_xbm(uint8_t* xbm, int x, int y){

    xbm[(x+(DEST_X*y))/8] |= ( 1 << ( ( x + (DEST_X * y) ) % 8 ) );

}


void showFloyd(int16_t *source_data, uint8_t* xbm, int16_t maxnum, int16_t minnum, int16_t avgnum)
{
    
    int x,y;

    resize_image(source_data);
    
    for (y = 0; y < DEST_Y; y++) {
 
        for (x = 0; x < DEST_X; x++) {
 
            int16_t pixel = get_dest(x, y);
            int16_t err;
            
            if(pixel < avgnum)
            {
                err = pixel - minnum;
            }
            
            else
            {
                set_xbm(xbm, y, 63-x);
                err = pixel - maxnum;
            }
            
            if(y+1<=(DEST_Y-1)){
                set_dest(x, y+1, get_dest(x, y+1) + (err * 7 / 16));
            }
                
            
            if(x+1<=(DEST_X-1) && y>=1){
                set_dest(x+1, y-1, get_dest(x+1, y-1) + (err * 3 / 16));
            }
                
            
            if(x+1<=(DEST_X-1)){
                set_dest(x+1, y, get_dest(x+1, y) + (err * 5 / 16));
            }
                
            
            if(x+1<=(DEST_X-1) && y+1<=(DEST_Y-1))
            {
                set_dest(x+1, y+1, get_dest(x+1, y+1) + (err * 1 / 16));
            }
                
            
        }
    }
    
}


