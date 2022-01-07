#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "transfer_handler.h"
 
#define SOURCE_X 8
#define SOURCE_Y 8
#define SCALE 8
 
#define CLAMP(v, min, max) if(v < min) { v = min; } else if(v > max) { v = max; }

static int16_t destination_data[SOURCE_X * SOURCE_Y * SCALE * SCALE];

static float cubic_hermite(float A, float B, float C, float D, float t) {
 
    float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
    float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
    float c = -A / 2.0f + C / 2.0f;
    float d = B;
 
    return a*t*t*t + b*t*t + c*t + d;
}
 
void get_pixel_clamped(int16_t *source_data, int x, int y, uint8_t *temp)  {
 
    CLAMP(x, 0, SOURCE_X - 1);
    CLAMP(y, 0, SOURCE_Y - 1);
    //Debug("get source_image pixel x=%d y=%d",x,y);
    
    *temp = source_data[x+(SOURCE_X*y)];
    
}
 
void sample_bicubic(int16_t *source_data, float u, float v, uint8_t sample[]) {
 
    float x = (u * SOURCE_X) - 0.5f;
    int xint = (int)x;
    float xfract = x-floor(x);
 
    float y = (v * SOURCE_Y) - 0.5f;
    int yint = (int)y;
    float yfract = y - floor(y);
    
    int i;
 
    uint8_t p00[1];
    uint8_t p10[1];
    uint8_t p20[1];
    uint8_t p30[1];
    
    uint8_t p01[1];
    uint8_t p11[1];
    uint8_t p21[1];
    uint8_t p31[1];
 
    uint8_t p02[1];
    uint8_t p12[1];
    uint8_t p22[1];
    uint8_t p32[1];
 
    uint8_t p03[1];
    uint8_t p13[1];
    uint8_t p23[1];
    uint8_t p33[1];
    
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
    for (i = 0; i < 1; i++) {
 
        float col0 = cubic_hermite(p00[i], p10[i], p20[i], p30[i], xfract);
        float col1 = cubic_hermite(p01[i], p11[i], p21[i], p31[i], xfract);
        float col2 = cubic_hermite(p02[i], p12[i], p22[i], p32[i], xfract);
        float col3 = cubic_hermite(p03[i], p13[i], p23[i], p33[i], xfract);
 
        float value = cubic_hermite(col0, col1, col2, col3, yfract);
 
        CLAMP(value, 0.0f, 255.0f);
 
        sample[i] = (uint8_t)value;
        
        //Debug("sample[%d]=%d\n",i,sample[i]);      
        
    }
    
}
 
void resize_image(int16_t *source_data) {
 
    uint8_t sample[1] = {0xff};
    int y, x;
    
    for (y = 0; y < (SOURCE_Y * SCALE); y++) {
 
        float v = (float)y / (float)((SOURCE_Y * SCALE) - 1);
        for (x = 0; x < (SOURCE_X * SCALE); x++) {
 
            float u = (float)x / (float)((SOURCE_X * SCALE) - 1);
            
            sample_bicubic(source_data, u, v, sample);
            
            destination_data[x+((SOURCE_X * SCALE)*y)] = sample[0];
            //Debug("set destination_image pixel x=%d y=%d, memory%d",x,y, x+((SOURCE_X * SCALE)*y));
            
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



void showFloyd(int16_t *source_data, uint8_t* xbm)
{
    
    int x,y;
    
    int16_t maxnum = max(source_data, 64);
    int16_t minnum = min(source_data, 64);

    resize_image(source_data);
    Debug("Max %d, Min %d",maxnum,minnum);
    
    for (y = 0; y < (SOURCE_Y * SCALE); y++) {
 
        for (x = 0; x < (SOURCE_X * SCALE); x++) {
 
            int16_t pixel = destination_data[x+((SOURCE_X * SCALE)*y)];
            int16_t err;
            
            if(pixel < ((maxnum - minnum)/2))
            {
                //draw_dot(x,y,0);
                err = pixel - minnum;
                Debug("BLACK x %d, y %d",x,y);
            }
            
            else
            {
                //draw_dot(x,y,1);
                xbm[(y*(SOURCE_X * SCALE)+x)/8] |= 1<<((y*(SOURCE_X * SCALE)+x)%8);
                err = pixel - maxnum;
                Debug("WHITE x %d, y %d",x,y);
            }
            
            if(y+1<(SOURCE_Y * SCALE))
                destination_data[x+((SOURCE_X * SCALE)*(y+1))] = destination_data[x+((SOURCE_X * SCALE)*(y+1))] +7/16*err;
                //tmp(x,y+1)=tmp(x,y+1)+7/16*err;
            if(x+1<(SOURCE_X * SCALE) && y>2)
                destination_data[(x+1)+((SOURCE_X * SCALE)*(y-1))] = destination_data[(x+1)+((SOURCE_X * SCALE)*(y-1))] +3/16*err;
                //tmp(x+1,y-1)=tmp(x+1,y-1)+3/16*err;
            if(x+1<(SOURCE_X * SCALE))
                destination_data[(x+1)+((SOURCE_X * SCALE)*y)] = destination_data[(x+1)+((SOURCE_X * SCALE)*y)] +5/16*err;
                //tmp(x+1,y)=tmp(x+1,y)+5/16*err;
            if(x+1<(SOURCE_X * SCALE) && y+1<(SOURCE_Y * SCALE))
                destination_data[(x+1)+((SOURCE_X * SCALE)*(y+1))] = destination_data[(x+1)+((SOURCE_X * SCALE)*(y+1))] +1/16*err;
                //tmp(x+1,y+1)=tmp(x+1,y+1)+1/16*err;
            
        }
    }
    
//    for(int x=0; x<m; x++)
//    {
//        for(int y=0; y<n; y++)
//        {
//            int16_t pixel = destination_image->data[x+((destination_image->x)*y)];
//            int16_t err;
//            if(pixel < ((maxnum - minnum)/2))
//            {
//                //draw_dot(x,y,0);
//                err = pixel - minnum;
//                Debug("BLACK x %d, y %d",x,y);
//            }
//                
//            else
//            {
//                //draw_dot(x,y,1);
//                xbm[(y+m*x)/8] |= 1<<((y+m*x)%8);
//                err = pixel - maxnum;
//                Debug("WHITE x %d, y %d",x,y);
//            }
//            
//            if(y+1<m)
//                destination_image->data[x+((destination_image->x)*(y+1))] = destination_image->data[x+((destination_image->x)*(y+1))] +7/16*err;
//                //tmp(x,y+1)=tmp(x,y+1)+7/16*err;
//            if(x+1<m && y>2)
//                destination_image->data[(x+1)+((destination_image->x)*(y-1))] = destination_image->data[(x+1)+((destination_image->x)*(y-1))] +3/16*err;
//                //tmp(x+1,y-1)=tmp(x+1,y-1)+3/16*err;
//            if(x+1<m)
//                destination_image->data[(x+1)+((destination_image->x)*y)] = destination_image->data[(x+1)+((destination_image->x)*y)] +5/16*err;
//                //tmp(x+1,y)=tmp(x+1,y)+5/16*err;
//            if(x+1<m && y+1<m)
//                destination_image->data[(x+1)+((destination_image->x)*(y+1))] = destination_image->data[(x+1)+((destination_image->x)*(y+1))] +1/16*err;
//                //tmp(x+1,y+1)=tmp(x+1,y+1)+1/16*err;
//                
//        }
//    
//    }
    
}
 
//int main() {
//    
//    float scale = 2.0f; 
//    PPMImage *source_image;
//    PPMImage *destination_image;
//   
//    printf("starting...\n\n");
//    
//    if(remove(OUT_PATH) == 0)
//        printf("Deleting old image...\n\n");
// 
//    source_image = readPPM(INP_PATH);
//    destination_image = init_destination_image(scale);
//    
//    resize_image(source_image, destination_image, scale);
// 
//    writePPM(OUT_PATH, destination_image);
// 
//}
