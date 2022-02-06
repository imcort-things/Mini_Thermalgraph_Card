/**
 * Copyright (c) 2015 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 * @defgroup tw_sensor_example main.c
 * @{
 * @ingroup nrf_twi_example
 * @brief TWI Sensor Example main file.
 *
 * This file contains the source code for a sample application using TWI.
 *
 */

#include <stdio.h>
#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_drv_clock.h"
#include "app_timer.h"
#include "bsp.h"

#include "transfer_handler.h"
#include "AMG88xx.h"

#include "drv_oled.h"

#include "u8g2.h"
#include "u8x8.h"

APP_TIMER_DEF(get_ir_temp_timer);
static int16_t temp_grid[64];

void disp_temperature(int16_t * temp_grid);

extern u8g2_t u8g2;

void find_num(int16_t* input, int n, int16_t* maxnum, int16_t* minnum, int16_t* avgnum)
{
    *maxnum = input[0];
    *minnum = input[0];
    int32_t avgsum = 0;
    
    for(int i=0;i<n;i++)
    {
        if(input[i] > *maxnum)
            *maxnum = input[i];
        
        if(input[i] < *minnum)
            *minnum = input[i];
        
        avgsum += input[i];
    
    }
    
    *avgnum = avgsum / n;
    
}

static void get_ir_temp_timer_handler(void * p_context)
{
   
	amg88xx_getIRGrid(temp_grid);
    disp_temperature(temp_grid);
    
}

static void lfclk_config(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);
}

void bsp_evt_handler(bsp_event_t evt)
{
    switch (evt)
    {
        case BSP_EVENT_KEY_2:
				
            NRF_LOG_INFO("KEY0");
            drv_oled_on();
            break;

        default:
            NRF_LOG_INFO("KEY?");
            return; // no implementation needed
    }
}

static void bsp_configuration()
{
    uint32_t err_code;
	
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = bsp_init(BSP_INIT_BUTTONS, bsp_evt_handler);
    APP_ERROR_CHECK(err_code);
}

uint8_t xbm[64*16] = {0xff, 0xff};

void showFloyd(int16_t *source_data, uint8_t* xbm, int16_t maxnum, int16_t minnum, int16_t avgnum);

void disp_temperature(int16_t * temp_grid)
{
    
    //u8g2_ClearBuffer(&u8g2);
    u8g2_SetDrawColor(&u8g2,1);
    u8g2_SetBitmapMode(&u8g2,0);
    
    int16_t maxnum,minnum,avgnum;
    find_num(temp_grid, 64, &maxnum, &minnum, &avgnum);
    
    NRF_LOG_INFO("ori max:%d, min:%d", maxnum, minnum);

    memset(xbm, 0, 64*8);

    showFloyd(temp_grid, xbm, maxnum, minnum, avgnum);
    
    

    u8g2_SetFont(&u8g2, u8g2_font_6x13_t_hebrew);
    char outputmax[10];
    char outputmin[10];
    sprintf(outputmax,"Max: %.2f", 0.015625f * maxnum);
    sprintf(outputmin,"Min: %.2f", 0.015625f * minnum);


    u8g2_DrawStr(&u8g2, 64, 10, outputmax);
    u8g2_DrawStr(&u8g2, 64, 20, outputmin);
    u8g2_DrawXBM(&u8g2,0,0,64,64,xbm);

    u8g2_SendBuffer(&u8g2);

	
	
}

/**
 * @brief Function for main application entry.
 */
int main(void)
{
    lfclk_config();
    bsp_configuration();

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    drv_oled_begin();
    amg88xx_begin();
    
    APP_ERROR_CHECK(app_timer_create(&get_ir_temp_timer, APP_TIMER_MODE_REPEATED, get_ir_temp_timer_handler));
    app_timer_start(get_ir_temp_timer, 500000, NULL);

    NRF_LOG_INFO("\r\nTWI sensor example started.");

    while (1)
    {

        NRF_LOG_FLUSH();
        __SEV();
        __WFE();
        
    }
}

/** @} */
