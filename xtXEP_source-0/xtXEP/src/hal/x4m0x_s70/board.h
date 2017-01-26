/**
 * \file
 *
 * \brief User board definition template
 *
 */

 /* This file is intended to contain definitions and configuration details for
 * features and devices that are available on the board, e.g., frequency and
 * startup time for an external crystal, external memory devices, LED and USART
 * pins.
 */

#ifndef USER_BOARD_H
#define USER_BOARD_H

#include <conf_board.h>
#include <inttypes.h>
#include "pio.h"
#include "xep_hal.h"



/** Board oscillator settings */
#define BOARD_FREQ_SLCK_XTAL		(32768U)
#define BOARD_FREQ_SLCK_BYPASS		(32768U)
#define BOARD_FREQ_MAINCK_XTAL		(12000000U)
#define BOARD_FREQ_MAINCK_BYPASS	(12000000U)

/** Master clock frequency */
#define BOARD_MCK					CHIP_FREQ_CPU_MAX


/** board main clock xtal statup time */
#define BOARD_OSC_STARTUP_US   15625

/** Base address of chip select */
#define SRAM_BASE_ADDRESS		(EBI_CS0_ADDR)
#define SRAM_SIZE				(0x80000)

#define XPIN_LED_RED        (PIO_PD24_IDX)  //##
#define XPIN_LED_GREEN      (PIO_PD25_IDX)  //##
#define XPIN_LED_BLUE       (PIO_PD26_IDX)  //##

#define XPIN_LEVEL_LED_ACTIVE	(0)

#define XPIN_IO1            (PIO_PA28_IDX)
#define XPIN_IO2            (PIO_PA25_IDX)
#define XPIN_IO3            (PIO_PA30_IDX)
#define XPIN_IO4            (PIO_PA31_IDX)
#define XPIN_IO5            (PIO_PA26_IDX)
#define XPIN_IO6            (PIO_PA27_IDX)
#define XPIN_IO7_WAKEUP     (PIO_PB5_IDX)
#define XPIN_IO8_SWCLK      (PIO_PB7_IDX)
#define XPIN_IO9_SWDIO      (PIO_PB6_IDX)
#define UI_ACTIVE_INPUT_GPIO		(XPIN_IO7_WAKEUP)
#define UI_ACTIVE_INPUT_FLAGS		(IOPORT_MODE_PULLUP)
#define UI_ACTIVE_INPUT_SENSE		(0)
#define UI_ACTIVE_WKUP_IDX			(XPIN_IO7_WAKEUP)
#define UI_ACTIVE_WKUP_FLAGS		(IOPORT_MODE_MUX_D)
#define XDEF_WAKEUP_ACTIVE			(13)

//extern uint32_t redpin;
//extern uint32_t bluepin;
//extern uint32_t greenpin;

#define XPIN_PCB_REV0            (PIO_PD10_IDX)   //## NB PCB_REV_ID0 MCU01 /PCBA_REV_ID0 MCU02
#define XPIN_PCB_REV1            (PIO_PD11_IDX)   //##

#define XPIN_IO8            (PIO_PA27_IDX)   //##
#define XPIN_IO9            (PIO_PA28_IDX)   //##

#define XPIN_X4ENABLE           (PIO_PA5_IDX)   //##Enable MCU02
#define XPIN_TM				  (PIO_PA22_IDX)   //##TM
#define XPIN_X4IO1            (PIO_PA4_IDX)
#define XPIN_X4IO2            (PIO_PA3_IDX)

#define XPIN_GPIO_PIN1      (PIO_PA21_IDX)  // GPIO mode output pin#1, Pin 3 on P2, Same as UART pins
#define XPIN_GPIO_PIN2      (PIO_PA22_IDX)  // GPIO mode output pin#2, Pin 4 on P2, Same as UART pins
#define XPIN_IO_SEL1        (PIO_PA23_IDX)  // IO mode select pin#1, Pin 5 on P2, Same as SPI SCLK
#define XPIN_IO_SEL2        (PIO_PA25_IDX)  // IO mode select pin#2, Pin 6 on P2, Same as SPI nSS

#define XPIN_USBVBUS        (PIO_PA2_IDX)   //##
#define XPIN_SERIAL_SCLK    (PIO_PA23_IDX)  //##
#define XPIN_SERIAL_nSS     (PIO_PA25_IDX)  //##

//#define XPIN_USB_DP         (PIO_PB11_IDX)  //## TODO: When activating USB, this pin should not be used as GPIO
//#define XPIN_USB_DN         (PIO_PB10_IDX)  //## TODO: When activating USB, this pin should not be used as GPIO

//## The SD card interface should only be activated on demand
#if defined (VSDM_HW_XTI)
/** HSMCI pins definition. */
/*! Number of slot connected on HSMCI interface */
#define SD_MMC_HSMCI_MEM_CNT      1
#define SD_MMC_HSMCI_SLOT_0_SIZE  4
#define PINS_HSMCI   {0x3fUL << 26, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_PULLUP}
/** HSMCI MCCDA pin definition. */
#define PIN_HSMCI_MCCDA_GPIO            (PIO_PA28_IDX)
#define PIN_HSMCI_MCCDA_FLAGS           (IOPORT_MODE_MUX_C)
/** HSMCI MCCK pin definition. */
#define PIN_HSMCI_MCCK_GPIO             (PIO_PA25_IDX)
#define PIN_HSMCI_MCCK_FLAGS            (IOPORT_MODE_MUX_C)
/** HSMCI MCDA0 pin definition. */
#define PIN_HSMCI_MCDA0_GPIO            (PIO_PA30_IDX)
#define PIN_HSMCI_MCDA0_FLAGS           (IOPORT_MODE_MUX_C)
/** HSMCI MCDA1 pin definition. */
#define PIN_HSMCI_MCDA1_GPIO            (PIO_PA31_IDX)
#define PIN_HSMCI_MCDA1_FLAGS           (IOPORT_MODE_MUX_C)
/** HSMCI MCDA2 pin definition. */
#define PIN_HSMCI_MCDA2_GPIO            (PIO_PA26_IDX)
#define PIN_HSMCI_MCDA2_FLAGS           (IOPORT_MODE_MUX_C)
/** HSMCI MCDA3 pin definition. */
#define PIN_HSMCI_MCDA3_GPIO            (PIO_PA27_IDX)
#define PIN_HSMCI_MCDA3_FLAGS           (IOPORT_MODE_MUX_C)

//## SD card detect pin is not defined yet on X2M0x
/** SD/MMC card detect pin definition. */
//#define PIN_HSMCI_CD             {PIO_PD23, PIOD, ID_PIOD, PIO_INPUT, PIO_PULLUP}
#define SD_MMC_0_CD_GPIO         (PIO_PD23_IDX)
#define SD_MMC_0_CD_PIO_ID       ID_PIOD
#define SD_MMC_0_CD_FLAGS        (IOPORT_MODE_PULLUP)
#define SD_MMC_0_CD_DETECT_VALUE 0
#endif // defined (VSDM_HW_XTI)

/** TWI0 pins definition */
#define TWI_SUCCESS			TWIHS_SUCCESS
#define TWI					TWIHS0
#define TWI0_DATA_GPIO		PIO_PA3_IDX
#define TWI0_DATA_FLAGS		(IOPORT_MODE_MUX_A)
#define TWI0_CLK_GPIO		PIO_PA4_IDX
#define TWI0_CLK_FLAGS		(IOPORT_MODE_MUX_A)



/** SPI MISO pin definition. */
#define QSPI2_GPIO			  (PIO_PA17_IDX)
#define QSPI2_GPIO_FLAGS       (IOPORT_MODE_MUX_A)

#define QSPI3_GPIO				(PIO_PD31_IDX)
#define QSPI3_GPIO_FLAGS        (IOPORT_MODE_MUX_A)

#define SPI_MISO_GPIO         (PIO_PA12_IDX)
#define SPI_MISO_FLAGS        (IOPORT_MODE_MUX_A)
/** SPI MOSI pin definition. */
#define SPI_MOSI_GPIO         (PIO_PA13_IDX)
#define SPI_MOSI_FLAGS        (IOPORT_MODE_MUX_A)
/** SPI SPCK pin definition. */
#define SPI_SPCK_GPIO         (PIO_PA14_IDX)
#define SPI_SPCK_FLAGS        (IOPORT_MODE_MUX_A)

/** SPI chip select 0 pin definition. (Only one configuration is possible) */
#define SPI_NPCS0_GPIO        (PIO_PA11_IDX)
#define SPI_NPCS0_FLAGS       (IOPORT_MODE_MUX_A)

//## Should the definitions below be removed?? (Kjell-Ivar)
/** SPI chip select 1 pin definition. (multiple configurations are possible) */
//#define SPI_NPCS1_PA9_GPIO    (PIO_PA9_IDX)
//#define SPI_NPCS1_PA9_FLAGS   (IOPORT_MODE_MUX_B)
//#define SPI_NPCS1_PA31_GPIO   (PIO_PA31_IDX)
//#define SPI_NPCS1_PA31_FLAGS  (IOPORT_MODE_MUX_A)
//#define SPI_NPCS1_PB14_GPIO   (PIO_PB14_IDX)
//#define SPI_NPCS1_PB14_FLAGS  (IOPORT_MODE_MUX_A)
//#define SPI_NPCS1_PC4_GPIO    (PIO_PC4_IDX)
//#define SPI_NPCS1_PC4_FLAGS   (IOPORT_MODE_MUX_B)
/** SPI chip select 2 pin definition. (multiple configurations are possible) */
//#define SPI_NPCS2_PA10_GPIO   (PIO_PA10_IDX)
//#define SPI_NPCS2_PA10_FLAGS  (IOPORT_MODE_MUX_B)
//#define SPI_NPCS2_PA30_GPIO   (PIO_PA30_IDX)
//#define SPI_NPCS2_PA30_FLAGS  (IOPORT_MODE_MUX_B)
//#define SPI_NPCS2_PB2_GPIO    (PIO_PB2_IDX)
//#define SPI_NPCS2_PB2_FLAGS   (IOPORT_MODE_MUX_B)
/** SPI chip select 3 pin definition. (multiple configurations are possible) */
//#define SPI_NPCS3_PA3_GPIO    (PIO_PA3_IDX)
//#define SPI_NPCS3_PA3_FLAGS   (IOPORT_MODE_MUX_B)
//#define SPI_NPCS3_PA5_GPIO    (PIO_PA5_IDX)
//#define SPI_NPCS3_PA5_FLAGS   (IOPORT_MODE_MUX_B)
//#define SPI_NPCS3_PA22_GPIO   (PIO_PA22_IDX)
//#define SPI_NPCS3_PA22_FLAGS  (IOPORT_MODE_MUX_B)

/** EBI Data Bus pins */
#define PIN_EBI_DATA_BUS_D0        PIO_PC0_IDX
#define PIN_EBI_DATA_BUS_D1        PIO_PC1_IDX
#define PIN_EBI_DATA_BUS_D2        PIO_PC2_IDX
#define PIN_EBI_DATA_BUS_D3        PIO_PC3_IDX
#define PIN_EBI_DATA_BUS_D4        PIO_PC4_IDX
#define PIN_EBI_DATA_BUS_D5        PIO_PC5_IDX
#define PIN_EBI_DATA_BUS_D6        PIO_PC6_IDX
#define PIN_EBI_DATA_BUS_D7        PIO_PC7_IDX
#define PIN_EBI_DATA_BUS_FLAGS     (IOPORT_MODE_MUX_A)
#define PIN_EBI_DATA_BUS_MASK  0xFF
#define PIN_EBI_DATA_BUS_PIO  PIOC
#define PIN_EBI_DATA_BUS_ID  ID_PIOC
#define PIN_EBI_DATA_BUS_TYPE PIO_PERIPH_A
#define PIN_EBI_DATA_BUS_ATTR PIO_PULLUP
/** EBI NRD pin */
#define PIN_EBI_NRD                 PIO_PC11_IDX
#define PIN_EBI_NRD_FLAGS       (IOPORT_MODE_MUX_A)
#define PIN_EBI_NRD_MASK  1 << 11
#define PIN_EBI_NRD_PIO  PIOC
#define PIN_EBI_NRD_ID  ID_PIOC
#define PIN_EBI_NRD_TYPE PIO_PERIPH_A
#define PIN_EBI_NRD_ATTR PIO_PULLUP
/** EBI NWE pin */
#define PIN_EBI_NWE                  PIO_PC8_IDX
#define PIN_EBI_NWE_FLAGS       (IOPORT_MODE_MUX_A)
#define PIN_EBI_NWE_MASK  1 << 8
#define PIN_EBI_NWE_PIO  PIOC
#define PIN_EBI_NWE_ID  ID_PIOC
#define PIN_EBI_NWE_TYPE PIO_PERIPH_A
#define PIN_EBI_NWE_ATTR PIO_PULLUP
/** EBI NCS0 pin */
#define PIN_EBI_NCS0                PIO_PC14_IDX
#define PIN_EBI_NCS0_FLAGS     (IOPORT_MODE_MUX_A)
#define PIN_EBI_NCS0_MASK  1 << 14
#define PIN_EBI_NCS0_PIO  PIOC
#define PIN_EBI_NCS0_ID  ID_PIOC
#define PIN_EBI_NCS0_TYPE PIO_PERIPH_A
#define PIN_EBI_NCS0_ATTR PIO_PULLUP

/** EBI NLB pin */
#define PIN_EBI_NLB           PIO_PC16_IDX
#define PIN_EBI_NLB_FLAGS     (IOPORT_MODE_MUX_A)

/** EBI address bus pins  */
#define PIN_EBI_ADDR_BUS_A0     PIO_PC18_IDX
#define PIN_EBI_ADDR_BUS_A1     PIO_PC19_IDX
#define PIN_EBI_ADDR_BUS_A2     PIO_PC20_IDX
#define PIN_EBI_ADDR_BUS_A3     PIO_PC21_IDX
#define PIN_EBI_ADDR_BUS_A4     PIO_PC22_IDX
#define PIN_EBI_ADDR_BUS_A5     PIO_PC23_IDX
#define PIN_EBI_ADDR_BUS_A6     PIO_PC24_IDX
#define PIN_EBI_ADDR_BUS_A7     PIO_PC25_IDX
#define PIN_EBI_ADDR_BUS_A8     PIO_PC26_IDX
#define PIN_EBI_ADDR_BUS_A9     PIO_PC27_IDX
#define PIN_EBI_ADDR_BUS_A10   PIO_PC28_IDX
#define PIN_EBI_ADDR_BUS_A11   PIO_PC29_IDX
#define PIN_EBI_ADDR_BUS_A12   PIO_PC30_IDX
#define PIN_EBI_ADDR_BUS_A13   PIO_PC31_IDX
#define PIN_EBI_ADDR_BUS_FLAG1  (IOPORT_MODE_MUX_A)
#define PIN_EBI_ADDR_BUS_A14   PIO_PA18_IDX
#define PIN_EBI_ADDR_BUS_A15   PIO_PA19_IDX
#define PIN_EBI_ADDR_BUS_A16   PIO_PA20_IDX
#define PIN_EBI_ADDR_BUS_A17   PIO_PA0_IDX
#define PIN_EBI_ADDR_BUS_A18   PIO_PA1_IDX
#define PIN_EBI_ADDR_BUS_FLAG2  (IOPORT_MODE_MUX_C)

/** Address for transferring command bytes to the SDRAM. */
#define BOARD_SDRAM_ADDR     0x70000000

//----------------------------------------------------


/** USART0 pins (UTXD0 and URXD0) definitions, PB0,1. */
#define PIN_USART0_RXD_IDX    (PIO_PB0_IDX)
#define PIN_USART0_RXD_FLAGS  (IOPORT_MODE_MUX_C)
#define PIN_USART0_TXD_IDX    (PIO_PB1_IDX)
#define PIN_USART0_TXD_FLAGS  (IOPORT_MODE_MUX_C)
#define XEP_MAIN_USART	      USART0
#define CONF_BOARD_UART_CONSOLE


//##KIH #define NVA_PWREN_GPIO				(PIO_PB12_IDX)  //##

#define NVA_RESET_GPIO				(PIO_PE3_IDX)
#define NVA_SPI_RADAR_NPCS_GPIO		(SPI_NPCS0_GPIO)
#define NVA_SPI_RADAR_NPCS_FLAGS	(SPI_NPCS0_FLAGS)


//##KIH #define NVA_SWEEP_STATUS_GPIO		(PIO_PD16_IDX) // (PIO_PA17_IDX)
//##KIH #define NVA_SWEEP_STATUS_FLAGS      (0) //IOPORT_MODE_PULLUP)
//##KIH #define XPIN_SWEEP_STATUS			{PIO_PD16, PIOD, ID_PIOD, PIO_INPUT, PIO_DEBOUNCE | PIO_IT_LOW_LEVEL}
//##KIH #define XPIN_SWEEP_STATUS_MASK		PIO_PD16
//##KIH #define XPIN_SWEEP_STATUS_PIO		PIOD
//##KIH #define XPIN_SWEEP_STATUS_ID		ID_PIOD
//##KIH #define XPIN_SWEEP_STATUS_TYPE		PIO_INPUT
//##KIH #define XPIN_SWEEP_STATUS_ATTR		(PIO_DEBOUNCE | PIO_IT_LOW_LEVEL)
//##KIH #define XPIN_SWEEP_STATUS_IRQn		PIOD_IRQn

#if defined (VSDM_FW_REG)
//##KIH #define UI_REGMODE0_GPIO			(XPIN_IO3)
//##KIH #define UI_REGMODE0_FLAGS			(IOPORT_MODE_PULLUP)
//##KIH #define UI_REGMODE0_GPIO_OUT    	(XPIN_IO4)
//##KIH #define UI_REGMODE1_GPIO			(XPIN_IO5)
//##KIH #define UI_REGMODE1_FLAGS			(IOPORT_MODE_PULLUP)
//##KIH #define UI_REGMODE1_GPIO_OUT    	(XPIN_IO6)
#endif // defined (VSDM_FW_REG)

#define NVA_RADARLED				(0)

#define XDEF_COMM_MODE_MANUFACTURE	(0)
#define XDEF_COMM_MODE_SPI			(1)
#define XDEF_COMM_MODE_GPIO			(2)
#define XDEF_COMM_MODE_SERIAL		(3)

#endif // USER_BOARD_H
