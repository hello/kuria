/**
 * @file
 *
 * @brief Implementent support for external RAM on boards with Atmel SAM S70 MCU.
 *
 * See @ref X4M0x_SAMS70/xtio_extram.h and @ref xt_XEP_HAL.h for more documentation.
 */

#include <pio.h>
#include "sdramc.h"
#include "board.h"
#include "pmc.h"
#include "sysclk.h"
#include "xep_hal.h"
#include "xtio_extram.h"
#include <string.h>

/*
 * Define SDRAM properties
 */

const sdramc_memory_dev_t SDRAM_WINBOND_W9864G6JT_6I = {
	22,                // Block1 is at the bit 22, 1(M0)+8(Col)+12(Row)+1(BK1).
    0b0000000110000,   // Set SDRAMC to normal mode, CAS = 3
    /*
         * This configures the SDRAM with the following parameters in the
         * mode register:
         * - bits 0 to 2: burst length: 1 (000b);
         * - bit 3: burst type: sequential (0b);
         * - bits 4 to 6: CAS latency (3);
         * - bits 7 to 8: operating mode: standard operation (00b);
         * - bit 9: write burst mode: programmed burst length (0b);
         * - all other bits: reserved: 0b.
         */
	{
		SDRAMC_CR_NC_COL8      |
		SDRAMC_CR_NR_ROW12     |
		SDRAMC_CR_NB_BANK4     |
		SDRAMC_CR_CAS_LATENCY3 |
		SDRAMC_CR_DBW          |
        SDRAMC_CR_TWR(2)       | //5  //2
        SDRAMC_CR_TRC_TRFC(12)  | //15  //12
        SDRAMC_CR_TRP(3)       | //5  //3
        SDRAMC_CR_TRCD(3)      | //5  //3
        SDRAMC_CR_TRAS(7)      | //9  //7
        SDRAMC_CR_TXSR(12U)      //15u //12u
	},
};

/*
 * Pins used for SDRAM
 */
const int SDRAM_PINS[] = {
    SDRAM_A2_PIO,
    SDRAM_A3_PIO,
    SDRAM_A4_PIO,
    SDRAM_A5_PIO,
    SDRAM_A6_PIO,
    SDRAM_A7_PIO,
    SDRAM_A8_PIO,
    SDRAM_A9_PIO,
    SDRAM_A10_PIO,
    SDRAM_A11_PIO,
    SDRAM_SDA10_PIO,
    SDRAM_A13_PIO,
    SDRAM_D0_PIO,
    SDRAM_D1_PIO,
    SDRAM_D2_PIO,
    SDRAM_D3_PIO,
    SDRAM_D4_PIO,
    SDRAM_D5_PIO,
    SDRAM_D6_PIO,
    SDRAM_D7_PIO,
    SDRAM_D8_PIO,
    SDRAM_D9_PIO,
    SDRAM_D10_PIO,
    SDRAM_D11_PIO,
    SDRAM_D12_PIO,
    SDRAM_D13_PIO,
    SDRAM_D14_PIO,
    SDRAM_D15_PIO,
    SDRAM_BA0_PIO,
    SDRAM_BA1_PIO,
    SDRAM_RAS_PIO,
    SDRAM_CAS_PIO,
    SDRAM_NBS0_PIO,
    SDRAM_NBS1_PIO,
    SDRAM_SDCK_PIO,
    SDRAM_SDCKE_PIO,
    SDRAM_SDCS_PIO,
    SDRAM_SDWE_PIO
};
const int SDRAM_PIN_COUNT = sizeof(SDRAM_PINS)/sizeof(int);

int xtio_extram_init(void)
{
    // Initialize used IO pins and set proper pin mode
	pio_configure_pin(SDRAM_BA0_PIO, SDRAM_BA0_FLAGS);
	pio_configure_pin(SDRAM_BA1_PIO, SDRAM_BA1_FLAGS);
	pio_configure_pin(SDRAM_SDCK_PIO, SDRAM_SDCK_FLAGS);
	pio_configure_pin(SDRAM_SDCKE_PIO, SDRAM_SDCKE_FLAGS);
	pio_configure_pin(SDRAM_SDCS_PIO, SDRAM_SDCS_FLAGS);
	pio_configure_pin(SDRAM_RAS_PIO, SDRAM_RAS_FLAGS);
	pio_configure_pin(SDRAM_CAS_PIO, SDRAM_CAS_FLAGS);
	pio_configure_pin(SDRAM_SDWE_PIO, SDRAM_SDWE_FLAGS);
	pio_configure_pin(SDRAM_NBS0_PIO, SDRAM_NBS0_FLAGS);
	pio_configure_pin(SDRAM_NBS1_PIO, SDRAM_NBS1_FLAGS);
	pio_configure_pin(SDRAM_A2_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A3_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A4_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A5_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A6_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A7_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A8_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A9_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A10_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A11_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_A13_PIO, SDRAM_A_FLAGS);
	pio_configure_pin(SDRAM_SDA10_PIO, SDRAM_SDA10_FLAGS);
	pio_configure_pin(SDRAM_D0_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D1_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D2_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D3_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D4_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D5_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D6_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D7_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D8_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D9_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D10_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D11_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D12_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D13_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D14_PIO, SDRAM_D_FLAGS);
	pio_configure_pin(SDRAM_D15_PIO, SDRAM_D_FLAGS);

    // Initialize SDRAM Controller
	pmc_enable_periph_clk(ID_SDRAMC);

	MATRIX->CCFG_SMCNFCS = CCFG_SMCNFCS_SDRAMEN;

	sdramc_init((sdramc_memory_dev_t *)&SDRAM_WINBOND_W9864G6JT_6I,
           sysclk_get_cpu_hz());

	//## SDRAMC->SDRAMC_IER = 1;

	/*
	// Test SDRAM
    uint8_t * p8 = (uint8_t *)0x70000000;
    uint16_t * p16 = (uint16_t *)0x70000000;
    uint32_t * p32 = (uint32_t *)0x70000000;

    for (int j = 0; j < 100; j++)
    {
        memset(p8, 0xaa, BOARD_SDRAM_SIZE);
        for (int i = 0; i < BOARD_SDRAM_SIZE; i++)
        {
            if (*p8 != 0xaa)
            {
                volatile int test = *p8;
                (void)test;
            }
            p8++;
        }

		p8 = (uint8_t *)0x70000000;
		p16 = (uint16_t *)0x70000000;
		p32 = (uint32_t *)0x70000000;
        for (uint32_t i = 0; i < (BOARD_SDRAM_SIZE / sizeof(uint32_t)); i++)
        {
            *p32++ = i;
        }

		p8 = (uint8_t *)0x70000000;
		p16 = (uint16_t *)0x70000000;
		p32 = (uint32_t *)0x70000000;
        for (uint32_t i = 0; i < (BOARD_SDRAM_SIZE / sizeof(uint32_t)); i++)
        {

            if (*p32 != i)
            {
                volatile int test = *p32;
                (void)test;
            }
            p32++;
        }
    }

    p8 = (uint8_t *)0x70000000;
    memset(p8, 0x00, 2); // BOARD_SDRAM_SIZE);
	*/



    return XT_SUCCESS;
}
