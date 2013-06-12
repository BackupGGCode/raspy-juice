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

/**
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
 * 
 * Note: This driver only currently operates the ADC
 *       at resolutions 12 - 16-bits.
 * Note: This driver only supports positive output codes.
 * 
 * Example use case:
 * - Call mcp342x_init() to set the i2c device to slave and
 *   read the configuration register.
 * - Call mcp342x_set_config() to set the desired PGA gain,
 *   resolution and conversion mode.
 * - Call mcp342x_request_conversion() to commit the configuration
 *   register to the device and invoke a conversion.
 * - Call mcp342x_read() after the minimum sampling period
 *   has passed for the desired channel.
 */

#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <sys/ioctl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h> 
#include <unistd.h> 
#include <linux/i2c-dev.h> 

#include "mcp342x.h"

/** 
 * i2c Address of MCP342x device
 */
#define MCP342X_ADDR (0x69)
#define MCP342X_I2CBUS "/dev/i2c-1"

/**
 *  Configuration Register Bits 
 */
#define MCP342X_CONFIG_RDY      (7)     /**< RDY Bit */
#define MCP342X_CONFIG_CSB      (5)     /**< Channel Selection Bits */
#define MCP342X_CONFIG_CMB      (4)     /**< Conversion Mode Bit */
#define MCP342X_CONFIG_SRSB     (2)     /**< Sample Rate Selection Bit */
#define MCP342X_CONFIG_GSB      (0)     /**< PGA Gain Selection Bits */

/**
 *  Minimum sampling periods (ms) for this ADC.
 */
#define MCP342X_SAMPLE_PERIOD_12    5       /**< 240 SPS */
#define MCP342X_SAMPLE_PERIOD_14    17      /**< 60 SPS */
#define MCP342X_SAMPLE_PERIOD_16    67      /**< 15 SPS */
#define MCP342X_SAMPLE_PERIOD_18    267     /**< 3.75 SPS */

/**
 * LSB values for each resolution setting (uV).
 */
#define MCP342X_LSB_12      1000
#define MCP342X_LSB_14      250
#define MCP342X_LSB_16      62.5
#define MCP342X_LSB_18      15.625

/**
 * External calibration gain.
 * gain = ((cal_gan + 1) / 2)
 */
#define MCP342X_EXT_GAIN    1.001734972

/** File handle to i2c-dev */
int fd;

/** Configuration register */
static uint8_t config_register;

/** 4 byte i2c buffer */
uint8_t i2c_buffer[4] = {0}; 

/** Cached ADC values + timers */
static float mcp342x_values[4];

/** Sampling period */
static int16_t sample_period;

/**
 * Initialise MCP342X ADC
 */
void mcp342x_init()
{
    fd = open(MCP342X_I2CBUS, O_RDWR); 

    if (fd < 0)
    { 
        fprintf(stderr, "Fatal error: can't open i2c bus.\n"); 
        exit(1); 
    } 
}

/**
 * Read the output codes from the chip.
 * @return The number of bytes read.
 */
uint8_t mcp342x_read_output_codes()
{
    /* 
     * Set i2c device in Slave mode.
     */ 
    if (ioctl(fd, I2C_SLAVE, MCP342X_ADDR) < 0) { 
        fprintf(stderr, "Error during i2c-dev IOCTL\n"); 
        exit(1); 
    }


    uint8_t bytes = 0;
    if(mcp342x_adc_getres() == MCP342X_ADC_RES_18)
    {
        /* Read 4 bytes from i2c device */
        bytes = read(fd, i2c_buffer, 4);
        config_register = i2c_buffer[3];
    }
    else
    {
        /* Read 3 bytes from i2c device */
        bytes = read(fd, i2c_buffer, 3);
        config_register = i2c_buffer[2];
    }
    
    //printf("Read Output code: %02x", i2c_buffer[0]);
    //printf("%02x", i2c_buffer[1]);
    //printf("%02x", i2c_buffer[2]);
    //printf("%02x\n", i2c_buffer[3]);
    return bytes;
}

/**
 * Read ADC output from stored value. 
 * Only call this after you've requested a conversion for 
 * your channel!
 * @param channel The channel to read from.
 * @return A float representing measured input (uV).
 */
float mcp342x_read_output(mcp342x_adc_ch_t channel)
{
    mcp342x_read_output_codes();

    /* Only process conversion if previous has completed. */
    mcp342x_rdy_t rdy = mcp342x_read_rdy();
    switch(rdy)
    {
        /* Not updated from last request, return cached value */
        case MCP342X_RDY_1:
        {
            break;
        }
        /* Updated since last request. Save new value. */
        case MCP342X_RDY_0:
        {
            int32_t result = i2c_buffer[0];

            if(mcp342x_adc_getres() == MCP342X_ADC_RES_18)
            {
                /* Sign extend, append 2nd and 3rd data byte. */
                result <<= 24;
                result >>= 16;
                result |= i2c_buffer[1];
                result <<= 8;
                result |= i2c_buffer[2];
            }
            else
            {
                /* Sign extend, append 2nd data byte. */
                result <<= 24;
                result >>= 16;
                result |= i2c_buffer[1]; 
            }

            /* Convert digital output code to uV */
            float store = result * (mcp342x_get_lsb(mcp342x_adc_getres()) / mcp342x_convert_gsb(mcp342x_read_gs()));

            /* Save result with gain correction */
            mcp342x_values[channel] = store * MCP342X_EXT_GAIN;
        }
        default:
        {
            break;
        }
    }
    return mcp342x_values[channel];
}

/**
 * Configure the ADC with required settings.
 * Applied at the next conversion request.
 * gain, resolution, mode
 * Currently, only 12 - 16-bits resolution is supported.
 * @param gain          The desired gain setting.
 * @param res           The desired resolution.
 * @param mode          The conversion mode.
 */
void mcp342x_set_config(mcp342x_adc_ch_t channel, mcp342x_gs_t gain, mcp342x_adc_resolution_t res, mcp342x_oc_t mode)
{

    /* 
     * Set i2c device in Slave mode.
     */ 
    if (ioctl(fd, I2C_SLAVE, MCP342X_ADDR) < 0) { 
        fprintf(stderr, "Error during i2c-dev IOCTL\n"); 
        exit(1); 
    }

    // Channel in configuration register
    uint8_t config_register_tmp = config_register & ~(3 << MCP342X_CONFIG_CSB);
    config_register_tmp |= (channel << MCP342X_CONFIG_CSB);
    config_register = config_register_tmp;
    
    // Gain selection
    config_register_tmp = config_register & ~(3 << MCP342X_CONFIG_GSB);
    config_register_tmp |= (gain << MCP342X_CONFIG_GSB);
    config_register = config_register_tmp;

    // Resolution
    config_register_tmp = config_register & ~(3 << MCP342X_CONFIG_SRSB);
    config_register_tmp |= (res << MCP342X_CONFIG_SRSB);
    config_register = config_register_tmp;

    // Conversion mode
    config_register_tmp = config_register & ~(1 << MCP342X_CONFIG_CMB);
    config_register_tmp |= (mode << MCP342X_CONFIG_CMB);
    config_register = config_register_tmp;

    // Write to device
    //printf("Writing config: %02x\n", config_register);
    write(fd, &config_register, sizeof(config_register));
}

#if 1
/**
 * Request an ADC conversion.
 * Returns true if successful, false if
 * a previous conversion is in progress.
 * @return Success result
 */
bool mcp342x_request_conversion()
{
    
}
#else
/**
 * This piece of code was found in a another SVN revision.
 * Will use it to update the above "do nothing" but working code.
 */
bool mcp342x_request_conversion(mcp342x_adc_ch_t channel)
{
    mcp342x_read_config();
    mcp342x_rdy_t rdy = mcp324x_read_rdy();

    // Only request conversion if previous has completed.
    switch(rdy)
    {
        // Not updated since since last request
        case MCP342X_RDY_1:
            {
                return false;
            }
            // Updated since last request. Perform new request.
        case MCP342X_RDY_0:
            {

                // Set channel in configuration register
                uint8_t config_register_tmp = config_register & ~(3 << MCP342X_CONFIG_CSB);
                config_register_tmp |= (channel << MCP342X_CONFIG_CSB);
                config_register = config_register_tmp;

                i2c_smbus_write_byte_data(fd, 0, config_register);

                return true;
            }
        default:
            {
                break;
            }
    }
    return false;
}
#endif



/**
 * Get resolution of ADC.
 * @return The currently configured resolution.
 */
mcp342x_adc_resolution_t mcp342x_adc_getres()
{
    uint8_t res = config_register >> MCP342X_CONFIG_SRSB;
    res &= 3;
    return res;
}


/**
 * Update stored sampling period.
 * @param res  The resolution used.
 */
void mcp342x_update_sampling_period(mcp342x_adc_resolution_t res)
{
    switch(res)
    {   
        case MCP342X_ADC_RES_12:
            sample_period = MCP342X_SAMPLE_PERIOD_12;
            break;
        case MCP342X_ADC_RES_14:
            sample_period = MCP342X_SAMPLE_PERIOD_14;
            break;
        case MCP342X_ADC_RES_16:
            sample_period = MCP342X_SAMPLE_PERIOD_16;
            break;
        case MCP342X_ADC_RES_18:
            sample_period = MCP342X_SAMPLE_PERIOD_18;
            break;
        default:
            break;
    }
}

/**
 * Get sampling period for the ADC - this depends
 * on the resolution.
 * @return The current minimum sampling period.
 */ 
uint16_t mcp342x_get_sampling_period()
{
    return sample_period;
}

/**
 * Get value of RDY bit. Indicates if the output
 * register has been updated with a latest conversion
 * result.</p>
 * 1 = Not updated <br/>
 * 0 = Updated 
 * @return The value of the RDY bit.
 */
mcp342x_rdy_t mcp342x_read_rdy()
{
    uint8_t rdy = config_register >> MCP342X_CONFIG_RDY;
    rdy &= 1;
    return rdy;
}

/**
 * Get value of Channel Selection bits. 
 * Indicates if the output register has been
 * updated with a latest conversion result.</p>
 * 00 = Channel 1<br/>
 * 01 = Channel 2<br/>
 * 10 = Channel 3<br/>
 * 11 = Channel 4
 * @return The value of the Channel Selection bits.
 */
mcp342x_adc_ch_t mcp342x_read_csb()
{
    uint8_t csb = config_register >> MCP342X_CONFIG_CSB;
    csb &= 3;
    return csb;
}


/**
 * Get value of O/C bit. Indicates the conversion
 * mode of the ADC.
 * @return The value of the O/C bit.
 */
mcp342x_oc_t mcp342x_read_oc()
{
    uint8_t oc = config_register >> MCP342X_CONFIG_CMB;
    oc &= 1;
    return oc;
}

/**
 * Get value of PGA Gain selection bits.
 * Indicates the current gain mode of the
 * programmable gain amplifier.
 * @return The value of the PGA Gain selection bits.
 */
mcp342x_gs_t mcp342x_read_gs()
{
    uint8_t gs = config_register >> MCP342X_CONFIG_GSB;
    gs &= 3;
    return gs;
}

/**
 * Get the LSB value for a given resolution
 * setting.
 * @param res The resolution
 * @return The LSB value corresponding to the resolution.
 */
float mcp342x_get_lsb(mcp342x_adc_resolution_t res)
{
    switch(res)
    {   
        case MCP342X_ADC_RES_12:
            return MCP342X_LSB_12;
        case MCP342X_ADC_RES_14:
            return MCP342X_LSB_14;
        case MCP342X_ADC_RES_16:
            return MCP342X_LSB_16;
        case MCP342X_ADC_RES_18:
            return MCP342X_LSB_18;
        default:
            break;
    }
    return 0;
}

/**
 * Convert Gain setting enum to their
 * numerical counterparts.
 * @param gsb The PGA gain setting.
 * @return The numerical value of the gain setting.
 */
uint8_t mcp342x_convert_gsb(mcp342x_gs_t gsb)
{
    switch(gsb)
    {   
        case MCP342X_GS_X1:
            return 1;
        case MCP342X_GS_X2:
            return 2;
        case MCP342X_GS_X4:
            return 4;
        case MCP342X_GS_X8:
            return 8;
        default:
            break;
    }
    return 0;
}
