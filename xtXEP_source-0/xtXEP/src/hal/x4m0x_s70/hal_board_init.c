/**
 * \file
 *
 * \brief User board initialization template
 *
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>
#include <ioport.h>
#include <pio.h>
#include "compiler.h"
#include "freertos_xdma.h"
#include "hal_board_init.h"
#include "xep_hal.h"
#include "xtio_extram.h"
#include "xtio_gpio.h"
#include "xt_selftest.h"

#include "string.h" //##KIH

#include <mpu.h>
#include <fpu.h>

/**
 * \brief Set peripheral mode for one single IOPORT pin.
 * It will configure port mode and disable pin mode (but enable peripheral).
 * \param pin IOPORT pin to configure
 * \param mode Mode masks to configure for the specified pin (\ref ioport_modes)
 */
#define ioport_set_pin_peripheral_mode(pin, mode) \
	do {\
		ioport_set_pin_mode(pin, mode);\
		ioport_disable_pin(pin);\
	} while (0)

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
/* Default memory map
   Address range          Memory region          Memory type      Shareability   Cache policy
   0x00000000- 0x1FFFFFFF Code                   Normal           Non-shareable  WT
   0x20000000- 0x3FFFFFFF SRAM                   Normal           Non-shareable  WBWA
   0x40000000- 0x5FFFFFFF Peripheral             Device           Non-shareable  -
   0x60000000- 0x7FFFFFFF RAM                    Normal           Non-shareable  WBWA
   0x80000000- 0x9FFFFFFF RAM                    Normal           Non-shareable  WT
   0xA0000000- 0xBFFFFFFF Device                 Device           Shareable
   0xC0000000- 0xDFFFFFFF Device                 Device           Non Shareable
   0xE0000000- 0xFFFFFFFF System                  -                     -
   */

/**
 * \brief Setup a memory region.
 */

// Optimization will not work when calling before main
__attribute__ ((optimize("-O0"))) void _SetupMemoryRegion( void )
{

    uint32_t dwRegionBaseAddr;
    uint32_t dwRegionAttr;

    __DMB();

/***************************************************
    ITCM memory region --- Normal
    START_Addr:-  0x00000000UL
    END_Addr:-    0x00400000UL
****************************************************/
    dwRegionBaseAddr =
        ITCM_START_ADDRESS |
        MPU_REGION_VALID |
        MPU_DEFAULT_ITCM_REGION;        // 1

    dwRegionAttr =
        MPU_AP_PRIVILEGED_READ_WRITE |
        mpu_cal_mpu_region_size(ITCM_END_ADDRESS - ITCM_START_ADDRESS) |
        MPU_REGION_ENABLE;

    mpu_set_region( dwRegionBaseAddr, dwRegionAttr);

/****************************************************
    Internal flash memory region --- Normal read-only (update to Strongly ordered in write accesses)
    START_Addr:-  0x00400000UL
    END_Addr:-    0x00600000UL
******************************************************/

    dwRegionBaseAddr =
        IFLASH_START_ADDRESS |
        MPU_REGION_VALID |
        MPU_DEFAULT_IFLASH_REGION;      //2

    dwRegionAttr =
        MPU_AP_READONLY |
        INNER_NORMAL_WB_NWA_TYPE( NON_SHAREABLE ) |
        mpu_cal_mpu_region_size(IFLASH_END_ADDRESS - IFLASH_START_ADDRESS) |
        MPU_REGION_ENABLE;

    mpu_set_region( dwRegionBaseAddr, dwRegionAttr);

/****************************************************
    DTCM memory region --- Normal
    START_Addr:-  0x20000000L
    END_Addr:-    0x20400000UL
******************************************************/

    /* DTCM memory region */
    dwRegionBaseAddr =
        DTCM_START_ADDRESS |
        MPU_REGION_VALID |
        MPU_DEFAULT_DTCM_REGION;         //3

    dwRegionAttr =
        MPU_AP_PRIVILEGED_READ_WRITE |
        mpu_cal_mpu_region_size(DTCM_END_ADDRESS - DTCM_START_ADDRESS) |
        MPU_REGION_ENABLE;

    mpu_set_region( dwRegionBaseAddr, dwRegionAttr);

/****************************************************
    SRAM Cacheable memory region --- Normal
    START_Addr:-  0x20400000UL
    END_Addr:-    0x2043FFFFUL
******************************************************/
    /* SRAM memory  region */
    dwRegionBaseAddr =
        SRAM_FIRST_START_ADDRESS |
        MPU_REGION_VALID |
        MPU_DEFAULT_SRAM_REGION_1;         //4

    dwRegionAttr =
        MPU_AP_FULL_ACCESS    |
        INNER_NORMAL_WB_NWA_TYPE( NON_SHAREABLE ) |
        mpu_cal_mpu_region_size(SRAM_FIRST_END_ADDRESS - SRAM_FIRST_START_ADDRESS) |
        MPU_REGION_ENABLE;

    mpu_set_region( dwRegionBaseAddr, dwRegionAttr);


/****************************************************
    Internal SRAM second partition memory region --- Normal
    START_Addr:-  0x20440000UL
    END_Addr:-    0x2045FFFFUL
******************************************************/
    /* SRAM memory region */
    dwRegionBaseAddr =
        SRAM_SECOND_START_ADDRESS |
        MPU_REGION_VALID |
        MPU_DEFAULT_SRAM_REGION_2;         //5

    dwRegionAttr =
        MPU_AP_FULL_ACCESS    |
        INNER_NORMAL_WB_NWA_TYPE( NON_SHAREABLE ) |
        mpu_cal_mpu_region_size(SRAM_SECOND_END_ADDRESS - SRAM_SECOND_START_ADDRESS) |
        MPU_REGION_ENABLE;

    mpu_set_region( dwRegionBaseAddr, dwRegionAttr);

/****************************************************
    Peripheral memory region --- DEVICE Shareable
    START_Addr:-  0x40000000UL
    END_Addr:-    0x5FFFFFFFUL
******************************************************/
    dwRegionBaseAddr =
        PERIPHERALS_START_ADDRESS |
        MPU_REGION_VALID |
        MPU_PERIPHERALS_REGION;          //6

    dwRegionAttr = MPU_AP_FULL_ACCESS |
        MPU_REGION_EXECUTE_NEVER |
        SHAREABLE_DEVICE_TYPE |
        mpu_cal_mpu_region_size(PERIPHERALS_END_ADDRESS - PERIPHERALS_START_ADDRESS) |
        MPU_REGION_ENABLE;

    mpu_set_region( dwRegionBaseAddr, dwRegionAttr);


/****************************************************
    External EBI memory  memory region --- Strongly Ordered
    START_Addr:-  0x60000000UL
    END_Addr:-    0x6FFFFFFFUL
******************************************************/
    dwRegionBaseAddr =
        EXT_EBI_START_ADDRESS |
        MPU_REGION_VALID |
        MPU_EXT_EBI_REGION;

    dwRegionAttr =
        MPU_AP_FULL_ACCESS |
        /* External memory Must be defined with 'Device' or 'Strongly Ordered' attribute for write accesses (AXI) */
        STRONGLY_ORDERED_SHAREABLE_TYPE |
        mpu_cal_mpu_region_size(EXT_EBI_END_ADDRESS - EXT_EBI_START_ADDRESS) |
        MPU_REGION_ENABLE;

    mpu_set_region( dwRegionBaseAddr, dwRegionAttr);

/****************************************************
    SDRAM Cacheable memory region --- Normal
    START_Addr:-  0x70000000UL
    END_Addr:-    0x7FFFFFFFUL
******************************************************/
    dwRegionBaseAddr =
        SDRAM_START_ADDRESS |
        MPU_REGION_VALID |
        MPU_DEFAULT_SDRAM_REGION;        //7

    dwRegionAttr =
        MPU_AP_FULL_ACCESS    |
        INNER_NORMAL_WB_RWA_TYPE( SHAREABLE ) |
        mpu_cal_mpu_region_size(SDRAM_END_ADDRESS - SDRAM_START_ADDRESS) |
        MPU_REGION_ENABLE;

    mpu_set_region( dwRegionBaseAddr, dwRegionAttr);

/****************************************************
    QSPI memory region --- Strongly ordered
    START_Addr:-  0x80000000UL
    END_Addr:-    0x9FFFFFFFUL
******************************************************/
    dwRegionBaseAddr =
        QSPI_START_ADDRESS |
        MPU_REGION_VALID |
        MPU_QSPIMEM_REGION;              //8

    dwRegionAttr =
        MPU_AP_FULL_ACCESS |
        STRONGLY_ORDERED_SHAREABLE_TYPE |
        mpu_cal_mpu_region_size(QSPI_END_ADDRESS - QSPI_START_ADDRESS) |
        MPU_REGION_ENABLE;

    mpu_set_region( dwRegionBaseAddr, dwRegionAttr);


/****************************************************
    USB RAM Memory region --- Device
    START_Addr:-  0xA0100000UL
    END_Addr:-    0xA01FFFFFUL
******************************************************/
    dwRegionBaseAddr =
        USBHSRAM_START_ADDRESS |
        MPU_REGION_VALID |
        MPU_USBHSRAM_REGION;              //9

    dwRegionAttr =
        MPU_AP_FULL_ACCESS |
        MPU_REGION_EXECUTE_NEVER |
        SHAREABLE_DEVICE_TYPE |
        mpu_cal_mpu_region_size(USBHSRAM_END_ADDRESS - USBHSRAM_START_ADDRESS) |
        MPU_REGION_ENABLE;

    mpu_set_region( dwRegionBaseAddr, dwRegionAttr);


    /* Enable the memory management fault , Bus Fault, Usage Fault exception */
    SCB->SHCSR |= (SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_USGFAULTENA_Msk);

    /* Enable the MPU region */
    mpu_enable( MPU_ENABLE | MPU_PRIVDEFENA);

    __DMB();
}

// Optimization will not work when calling xt_board_init()
__attribute__ ((optimize("-O0"))) int xt_board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */

    int status;
	(void)status;

    sysclk_init(); // ASF function to setup clocking.

    _SetupMemoryRegion();

//#if defined (DEBUG)
    WDT->WDT_MR = WDT_MR_WDDIS; // Disable the watchdog
//#endif

    fpu_enable();

	ioport_init(); // Initialize IOPORTs

	// Set PWR_ENABLE output as IO (was ERASE)              //##
	REG_CCFG_SYSIO = REG_CCFG_SYSIO | CCFG_SYSIO_SYSIO12;   //##

    status = xtio_led_init(XTIO_LED_RED, XTIO_LED_ONOFF, 0);
    status = xtio_led_init(XTIO_LED_GREEN, XTIO_LED_ONOFF, 0);
    status = xtio_led_init(XTIO_LED_BLUE, XTIO_LED_ONOFF, 0);


/*##
	ioport_set_pin_dir(NVA_PWREN_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(NVA_PWREN_GPIO, false); // First turn radar power off.



	ioport_set_pin_dir(XPIN_PCB_REV0,         IOPORT_DIR_INPUT);
	ioport_set_pin_dir(XPIN_PCB_REV1,         IOPORT_DIR_INPUT);

	ioport_set_pin_mode(XPIN_PCB_REV0, IOPORT_MODE_PULLDOWN);
	ioport_set_pin_mode(XPIN_PCB_REV1, IOPORT_MODE_PULLDOWN);
	uint8_t rev0 = ioport_get_pin_level(XPIN_PCB_REV0);
	uint8_t rev1 = ioport_get_pin_level(XPIN_PCB_REV1);
	uint32_t ismcu01 = 1;
	if(rev0 == 1 && rev1 == 1)
	{
		ismcu01 = 1;
	}
	else
	{
		ismcu01 = 0;
	}



	if(ismcu01)
	{
		redpin = XPIN_LED_RED;
		bluepin = XPIN_LED_BLUE;
		greenpin = XPIN_LED_GREEN;
	}
	else
	{
		redpin = XPIN_LED_RED_MCU02;
		bluepin = XPIN_LED_BLUE_MCU02;
		greenpin = XPIN_LED_GREEN_MCU02;
	}
## */
	/* Configure LED pins */
//##	ioport_set_pin_dir(redpin,     IOPORT_DIR_OUTPUT);
//##	ioport_set_pin_dir(greenpin,   IOPORT_DIR_OUTPUT);
//##	ioport_set_pin_dir(bluepin,    IOPORT_DIR_OUTPUT);

//##	ioport_set_pin_level(redpin, !XPIN_LEVEL_LED_ACTIVE);
//##	ioport_set_pin_level(greenpin, !XPIN_LEVEL_LED_ACTIVE);
//##	ioport_set_pin_level(bluepin, !XPIN_LEVEL_LED_ACTIVE);


    /* Configure IO pins on connector P2 */
//##    ioport_set_pin_dir(XPIN_IO1,         IOPORT_DIR_INPUT);
//##    ioport_set_pin_dir(XPIN_IO2,         IOPORT_DIR_INPUT);
//##    ioport_set_pin_dir(XPIN_IO3,         IOPORT_DIR_INPUT);
//##    ioport_set_pin_dir(XPIN_IO4,         IOPORT_DIR_INPUT);
//##    ioport_set_pin_dir(XPIN_IO5,         IOPORT_DIR_INPUT);
//##    ioport_set_pin_dir(XPIN_IO6,         IOPORT_DIR_INPUT);
#if !defined (DEBUG)
//##    ioport_set_pin_dir(XPIN_IO7_WAKEUP,  IOPORT_DIR_INPUT); // Also used as SerialWireViewer (ITM/DWT trace output)
#endif
    // ioport_set_pin_dir(XPIN_IO8,         IOPORT_DIR_INPUT); // Also used as SWDCLK
    // ioport_set_pin_dir(XPIN_IO9,         IOPORT_DIR_INPUT); // Also used as SWDIO
    ioport_set_pin_dir(XPIN_USBVBUS,     IOPORT_DIR_INPUT);
	xtio_set_pin_mode(XTIO_USB_VBUS, XTIO_PULL_DOWN);
//##    ioport_set_pin_dir(XPIN_SERIAL_SCLK, IOPORT_DIR_INPUT);
//##    ioport_set_pin_dir(XPIN_SERIAL_nSS,  IOPORT_DIR_INPUT);

#ifdef CONF_BOARD_TWI0
	/** TWI0 configuration. */
//##	ioport_set_pin_peripheral_mode(TWI0_DATA_GPIO, TWI0_DATA_FLAGS);
//##	ioport_set_pin_peripheral_mode(TWI0_CLK_GPIO, TWI0_CLK_FLAGS);
#endif



	// NVA radar SWEEP_STATUS input pin.
	//ioport_set_pin_input_mode(NVA_SWEEP_STATUS_GPIO, NVA_SWEEP_STATUS_FLAGS, 0);

//##	ioport_set_pin_dir(NVA_SWEEP_STATUS_GPIO, IOPORT_DIR_INPUT);


	// Radar MCLK comes from PCK1 at PA17
	//ioport_set_pin_peripheral_mode(PIO_PA17_IDX, IOPORT_MODE_MUX_B); // B = PCK1


#if defined (VSDM_HW_XTI)
#ifdef CONF_BOARD_SD_MMC_HSMCI
	/* Configure HSMCI pins */
//##	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCCDA_GPIO, PIN_HSMCI_MCCDA_FLAGS);
//##	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCCK_GPIO, PIN_HSMCI_MCCK_FLAGS);
//##	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCDA0_GPIO, PIN_HSMCI_MCDA0_FLAGS);
//##	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCDA1_GPIO, PIN_HSMCI_MCDA1_FLAGS);
//##	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCDA2_GPIO, PIN_HSMCI_MCDA2_FLAGS);
//##	ioport_set_pin_peripheral_mode(PIN_HSMCI_MCDA3_GPIO, PIN_HSMCI_MCDA3_FLAGS);

	/* Configure SD/MMC card detect pin */
//##	ioport_set_pin_input_mode(SD_MMC_0_CD_GPIO, SD_MMC_0_CD_FLAGS, 0);
#endif
#endif // defined (VSDM_HW_XTI)

	pmc_enable_periph_clk(ID_XDMAC);
	//## freertos_xdma_init();

	irq_initialize_vectors();
	cpu_irq_enable();


	// Enable brown-out detector and reset.
	supc_enable_brownout_detector(SUPC);
	supc_enable_brownout_reset(SUPC);

    // Perform a selftest for shorts between the external SDRAM pins
    // Must be performed before init of SDRAM
    if (xt_get_operation_mode() == XT_OPMODE_FACTORY)
        xt_selftest_ext_ram_shorts();

	status = xtio_extram_init();
    
    // Perform a selftest of functionality of external SDRAM
    if (xt_get_operation_mode() == XT_OPMODE_FACTORY)
    {
#ifdef USING_DATA_CACHE
        // Shut off data cache (implicit clean and invalidate) to make sure 
        // memory is accessed directly
        SCB_DisableDCache();
#endif

        xt_selftest_ext_ram_functionality();

#ifdef USING_DATA_CACHE
        SCB_EnableDCache();
#endif
    }

	/* Enabling the Cache */
//## #if defined (NDEBUG)
    SCB_EnableICache();
    //## SCB_EnableDCache();
//###endif

	status = xtio_irq_init();

// Enable UART 0
    pmc_enable_periph_clk(ID_UART0);

    return 0; /// @todo USE proper error handling
}


int xt_board_init_ext(uint8_t interfaceMode)
{

    ioport_set_pin_peripheral_mode(PIN_USART0_RXD_IDX, PIN_USART0_RXD_FLAGS);
    //## ioport_set_pin_dir(PIN_USART0_TXD_IDX,     IOPORT_DIR_OUTPUT);
    ioport_disable_pin (PIN_USART0_TXD_IDX);
    ioport_set_pin_peripheral_mode(PIN_USART0_TXD_IDX, PIN_USART0_TXD_FLAGS);


    if (interfaceMode == XDEF_COMM_MODE_MANUFACTURE)
    {
//##        ioport_set_pin_peripheral_mode(PIN_USART1_RXD_IDX, PIN_USART1_RXD_FLAGS);
//##        ioport_set_pin_peripheral_mode(PIN_USART1_TXD_IDX, PIN_USART1_TXD_FLAGS);
    }
    else if (interfaceMode == XDEF_COMM_MODE_SERIAL)
	{
		/* Configure UART pins */
//		ioport_set_pin_peripheral_mode(PIN_USART0_RXD_IDX, PIN_USART0_RXD_FLAGS);
//		ioport_set_pin_peripheral_mode(PIN_USART0_TXD_IDX, PIN_USART0_TXD_FLAGS);

//##		ioport_set_pin_peripheral_mode(PIN_USART1_RXD_IDX, PIN_USART1_RXD_FLAGS);
//##		ioport_set_pin_peripheral_mode(PIN_USART1_TXD_IDX, PIN_USART1_TXD_FLAGS);

#if defined(CONF_BOARD_USB_PORT)
		xtio_set_pin_mode(XTIO_USB_VBUS, XTIO_PULL_DOWN);
#endif
	}
	else if (interfaceMode == XDEF_COMM_MODE_GPIO)
	{
		// GPIO 1 (P2 pin 3) as output and low
//##	    ioport_set_pin_dir(XPIN_GPIO_PIN1,IOPORT_DIR_OUTPUT);
//##		ioport_set_pin_level(XPIN_GPIO_PIN1, false);

//##	    ioport_set_pin_dir(XPIN_GPIO_PIN2,IOPORT_DIR_OUTPUT);
//##		ioport_set_pin_level(XPIN_GPIO_PIN2, false);

#if defined(CONF_BOARD_USB_PORT)
		xtio_set_pin_mode(XTIO_USB_VBUS, XTIO_PULL_DOWN);
#endif
	}

	// Enable ACTIVE pin.
#if !defined (DEBUG)
//##	ioport_set_pin_input_mode(UI_ACTIVE_INPUT_GPIO, UI_ACTIVE_INPUT_FLAGS, UI_ACTIVE_INPUT_SENSE);
#endif
	return XT_SUCCESS;
}
