#ifndef __KURIA_UTILS_H__
#define __KURIA_UTILS_H__

#include <stdint.h>
#include "x4driver.h"


#define HIGH 1
#define LOW 0

#define KURIA_TRUE 1
#define KURIA_FALSE 0

typedef float float32_t;

typedef struct {
    uint16_t dac_min;
    uint16_t dac_max;
    uint8_t iterations;
    uint16_t pps; // pulses per step
    uint8_t downconversion_en;
    uint32_t fps; //frames per second
    xtx4_tx_center_frequency_t tx_center_freq; 
} hlo_x4_config_t;

typedef enum {
    CONFIG_STR_DAC_MIN, // MIN
    CONFIG_STR_DAC_MAX, // MAX
    CONFIF_STR_ITR,     // ITR
    CONFIG_STR_PPS,     // PPS
    CONFIG_STR_DWN,     // DWN
    CONFIG_STR_FPS,     // FPS
    CONFIG_STR_TXF,     // TXF
    // add new configuration here
    CONFIG_STR_MAX
} config_str_t;



void pabort(const char *s);
int32_t hlo_x4_read_config_from_file (char* filename, hlo_x4_config_t* config); 
void hlo_delay_us (uint32_t delay_us); 

#endif
