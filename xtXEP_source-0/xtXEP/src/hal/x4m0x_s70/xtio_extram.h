/**
 * @file
 *
 * @brief Local header file for xtio functions.
 */

#ifndef XTIO_EXTRAM_H
#define  XTIO_EXTRAM_H

#include <stddef.h>
#include "xep_hal.h"

/**  Board SDRAM size for MT48LC16M16A2 */
#define BOARD_SDRAM_SIZE        (8 * 1024 * 1024)

/**  SDRAM pins definitions */
#define SDRAM_BA0_PIO        PIO_PA20_IDX
#define SDRAM_BA1_PIO        PIO_PA0_IDX
#define SDRAM_SDCK_PIO       PIO_PD23_IDX
#define SDRAM_SDCKE_PIO      PIO_PD14_IDX
#define SDRAM_SDCS_PIO       PIO_PD18_IDX
#define SDRAM_RAS_PIO        PIO_PD16_IDX
#define SDRAM_CAS_PIO        PIO_PD17_IDX
#define SDRAM_SDWE_PIO       PIO_PD29_IDX
#define SDRAM_NBS0_PIO       PIO_PC18_IDX
#define SDRAM_NBS1_PIO       PIO_PD15_IDX
#define SDRAM_A2_PIO         PIO_PC20_IDX
#define SDRAM_A3_PIO         PIO_PC21_IDX
#define SDRAM_A4_PIO         PIO_PC22_IDX
#define SDRAM_A5_PIO         PIO_PC23_IDX
#define SDRAM_A6_PIO         PIO_PC24_IDX
#define SDRAM_A7_PIO         PIO_PC25_IDX
#define SDRAM_A8_PIO         PIO_PC26_IDX
#define SDRAM_A9_PIO         PIO_PC27_IDX
#define SDRAM_A10_PIO        PIO_PC28_IDX
#define SDRAM_A11_PIO        PIO_PC29_IDX
#define SDRAM_A13_PIO        PIO_PC31_IDX
#define SDRAM_SDA10_PIO      PIO_PD13_IDX
#define SDRAM_D0_PIO         PIO_PC0_IDX
#define SDRAM_D1_PIO         PIO_PC1_IDX
#define SDRAM_D2_PIO         PIO_PC2_IDX
#define SDRAM_D3_PIO         PIO_PC3_IDX
#define SDRAM_D4_PIO         PIO_PC4_IDX
#define SDRAM_D5_PIO         PIO_PC5_IDX
#define SDRAM_D6_PIO         PIO_PC6_IDX
#define SDRAM_D7_PIO         PIO_PC7_IDX
#define SDRAM_D8_PIO         PIO_PE0_IDX
#define SDRAM_D9_PIO         PIO_PE1_IDX
#define SDRAM_D10_PIO        PIO_PE2_IDX
#define SDRAM_D11_PIO        PIO_PE3_IDX
#define SDRAM_D12_PIO        PIO_PE4_IDX
#define SDRAM_D13_PIO        PIO_PE5_IDX
#define SDRAM_D14_PIO        PIO_PA15_IDX
#define SDRAM_D15_PIO        PIO_PA16_IDX

#define SDRAM_BA0_FLAGS      PIO_PERIPH_C
#define SDRAM_BA1_FLAGS      PIO_PERIPH_C
#define SDRAM_SDCK_FLAGS     PIO_PERIPH_C
#define SDRAM_SDCKE_FLAGS    PIO_PERIPH_C
#define SDRAM_SDCS_FLAGS     PIO_PERIPH_A
#define SDRAM_RAS_FLAGS      PIO_PERIPH_C
#define SDRAM_CAS_FLAGS      PIO_PERIPH_C
#define SDRAM_SDWE_FLAGS     PIO_PERIPH_C
#define SDRAM_NBS0_FLAGS     PIO_PERIPH_A
#define SDRAM_NBS1_FLAGS     PIO_PERIPH_C
#define SDRAM_A_FLAGS        PIO_PERIPH_A
#define SDRAM_SDA10_FLAGS    PIO_PERIPH_C
#define SDRAM_D_FLAGS        PIO_PERIPH_A

extern const int SDRAM_PINS[];
extern const int SDRAM_PIN_COUNT;

#ifdef __cplusplus
extern "C" {
#endif

int xtio_extram_init(void);

#ifdef __cplusplus
}
#endif

#endif //  XTIO_EXTRAM_H
