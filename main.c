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
{drv_oled_on();
    switch (evt)
    {
        case BSP_EVENT_KEY_2:
				
						NRF_LOG_INFO("KEY0");
						
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

//void Bubble_sort(int16_t *arr, int len)
//{
//	
//    int16_t tmp = 0;
//    bool swap = false;
//    for (int i = 0; i < len -1; i++)
//    {
//        for (int j = 0; j < len - 1 - i; j++)
//        {
//            if (arr[j] > arr[j + 1])
//            {
//                tmp = arr[j];
//                arr[j] = arr[j + 1];
//                arr[j + 1] = tmp;
//                swap = true;
//            }
//        }
//        if (!swap)
//        {
//            return;
//        }
//    }
//}

extern u8g2_t u8g2;

void disp_temperature(int16_t * temp_grid)
{
	
	float temp_raw[64];
	static char out[10];
	
	for(int i=0;i<64;i++)
		temp_raw[i] = temp_grid[i] * 0.015625f;
//			for(int j=0;j<8;j++){
//				sprintf(out,"%f,%f,%f,%f,%f,%f,%f,%f\r\n", temp_raw[j*8], temp_raw[j*8+1], temp_raw[j*8+2], temp_raw[j*8+3], temp_raw[j*8+4], temp_raw[j*8+5], temp_raw[j*8+6], temp_raw[j*8+7]);
//				NRF_LOG_INFO("%s", out);
//				NRF_LOG_FLUSH();
//			}

  u8g2_FirstPage(&u8g2);
  do {
			u8g2_SetFontPosTop(&u8g2);
			u8g2_SetFont(&u8g2, u8g2_font_u8glib_4_tf );
		
			for(int i=0;i<8;i++) //y
			{
					for(int j=0;j<8;j++)  //x
					{
							sprintf(out, "%.1f", temp_raw[i+j]);
							u8g2_DrawStr(&u8g2, j*16, i*8, out);
					
					}
			}
		
			
			
//			u8g2_SetDrawColor(&u8g2, 1);
//			u8g2_DrawRBox(&u8g2, 0, 0, 62, 12, 1);
//			u8g2_SetDrawColor(&u8g2, 0);
//			u8g2_DrawStr(&u8g2, 10, 0, "Console");
//			
//			u8g2_SetDrawColor(&u8g2, 1);
//			u8g2_DrawRBox(&u8g2, 65, 0, 62, 12, 1);
//			u8g2_SetDrawColor(&u8g2, 0);
//			u8g2_DrawStr(&u8g2, 80, 0, "Cards");
//		
//			u8g2_SetFont(&u8g2, u8g2_font_bubble_tn);
//			u8g2_SetDrawColor(&u8g2, 1);
//			sprintf(strtemp,"%d",slot);
//			u8g2_DrawStr(&u8g2, 80, 13, strtemp);
//					
//			u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);
//			u8g2_SetDrawColor(&u8g2, 1);
//			u8g2_DrawStr(&u8g2, 72, 33, "Ultralight");
//		
//			u8g2_SetFont(&u8g2, u8g2_font_t0_11b_tf);
//			u8g2_SetDrawColor(&u8g2, 1);
//			sprintf(strtemp,"%.2x%.2x %.2x%.2x",uid[0],uid[1],uid[2],uid[3]);
//			u8g2_DrawStr(&u8g2, 70, 43, strtemp);
//			sprintf(strtemp,"%.2x%.2x %.2x",uid[4],uid[5],uid[6]);
//			u8g2_DrawStr(&u8g2, 70, 54, strtemp); 
//		
//			u8g2_SetDrawColor(&u8g2, 1);
//			u8g2_DrawVLine(&u8g2, 63, 13, 52);
//			u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);		// set the font for the terminal window
//			u8g2_DrawLog(&u8g2, 2, 13, &u8log);		// draw the terminal window on the display
  } while (u8g2_NextPage(&u8g2));

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
	
		nrf_gpio_cfg_output(26);
		nrf_gpio_pin_set(26);
	
		iic_init();
		amg88xx_begin();
		drv_oled_on();
	
		APP_ERROR_CHECK(app_timer_create(&get_ir_temp_timer, APP_TIMER_MODE_REPEATED, get_ir_temp_timer_handler));
		app_timer_start(get_ir_temp_timer, 1000, NULL);
	
		NRF_LOG_INFO("\r\nTWI sensor example started.");
	
	

		while (1)
    {

				NRF_LOG_FLUSH();
        __SEV();
        __WFE();
        __WFE();
    }
}

/** @} */
