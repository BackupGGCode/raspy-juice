/*
 Copyright (c) 2012, ';DROP TABLE teams;, University of Adelaide.
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met: 

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer. 
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution. 

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies, 
 either expressed or implied, of the University of Adelaide.
 */

/*
 * Microchip MCP3422/3/4 ADC userspace driver
 * for Linux i2c-dev.
 * Implemented acccording to specs of:
 * http://ww1.microchip.com/downloads/en/DeviceDoc/22088c.pdf
 *
 * Default startup settings:
 * Conversion bit resolution: 12 bits (240 sps)
 * Input channel: Channel 1
 * PGA gain setting: x1
 * Continuous conversion
 */

#include <stdint.h>
#include <stdbool.h>

/**
 * Support ADC resolutions of the MCP342x (S1-S0) 
 */
typedef enum 
{
    MCP342X_ADC_RES_12,         /**< 12-bit "00" */
    MCP342X_ADC_RES_14,         /**< 14-bit "01" */
    MCP342X_ADC_RES_16,         /**< 16-bit "10" */
    MCP342X_ADC_RES_18          /**< 18-bit "11" */
} mcp342x_adc_resolution_t;

/**
 * Channel selection bits (C1-C0)
 */
typedef enum 
{
    MCP342X_ADC_CH1,            /**< "00" */
    MCP342X_ADC_CH2,            /**< "01" */
    MCP342X_ADC_CH3,            /**< "10" (Treated as "00" by MCP3422/MCP3423) */
    MCP342X_ADC_CH4             /**< "11" (Treated as "01" by MCP3422/MCP3423) */
} mcp342x_adc_ch_t;

/**
 * RDY bit
 */
typedef enum
{
    MCP342X_RDY_0,              /**< "0" (Updated) */
    MCP342X_RDY_1               /**< "1" (Not updated) */
} mcp342x_rdy_t;

/**
 * Conversion mode bit (O/C)
 */
typedef enum
{
    MCP342X_OC_ONESHOT,         /**< "0" (One-shot conversion mode) */
    MCP342X_OC_CONTINUOUS       /**< "1" (Continuous conversion mode) */
} mcp342x_oc_t;

/**
 * PGA Gain selection bits (G1-G0)
 */
typedef enum
{
    MCP342X_GS_X1,              /**< "00" (x1) */
    MCP342X_GS_X2,              /**< "01" (x2) */
    MCP342X_GS_X4,              /**< "10" (x4) */
    MCP342X_GS_X8,              /**< "11" (x8) */
} mcp342x_gs_t;

void mcp342x_init();

float mcp342x_read_output(mcp342x_adc_ch_t channel);

uint8_t mcp342x_read_output_codes();

void mcp342x_set_config(mcp342x_adc_ch_t channel, mcp342x_gs_t gain, mcp342x_adc_resolution_t res, mcp342x_oc_t mode);

bool mcp342x_request_conversion();

mcp342x_adc_resolution_t mcp342x_adc_getres();

void mcp342x_update_sampling_period(mcp342x_adc_resolution_t res);

mcp342x_adc_ch_t mcp342x_read_csb();

mcp342x_rdy_t mcp342x_read_rdy();

mcp342x_oc_t mcp342x_read_oc();

mcp342x_gs_t mcp342x_read_gs();

float mcp342x_get_lsb(mcp342x_adc_resolution_t res);

uint8_t mcp342x_convert_gsb(mcp342x_gs_t gsb);


