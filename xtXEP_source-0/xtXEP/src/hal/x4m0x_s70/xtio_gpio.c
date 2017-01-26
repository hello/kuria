/**
 * @file
 *
 * @brief Implementation of GPIO functions for X4M0x boards with Atmel SAM S70 MCU.
 *
 * See @ref X4M0x_SAMS70/xtio_gpio.h and @ref xt_XEP_HAL.h for more documentation.
 */


#include "xtio_gpio.h"
#include "board.h"
#include "ioport.h"
#include "pio.h"
#include "pio_handler.h"

int xtio_decode_pin_id(xtio_pin_id_t pin_id, ioport_pin_t * pin);

int xtio_decode_pin_id(xtio_pin_id_t pin_id, ioport_pin_t * pin)
{
    switch (pin_id)
    {
        case XTIO_XETHRU_IO1:
            *pin = XPIN_IO1;
            break;
        case XTIO_XETHRU_IO2:
            *pin = XPIN_IO2;
            break;
        case XTIO_XETHRU_IO3:
            *pin = XPIN_IO3;
            break;
        case XTIO_XETHRU_IO4:
            *pin = XPIN_IO4;
            break;
        case XTIO_XETHRU_IO5:
            *pin = XPIN_IO5;
            break;
        case XTIO_XETHRU_IO6:
            *pin = XPIN_IO6;
            break;
        case XTIO_XETHRU_IO7:
            *pin = XPIN_IO7_WAKEUP;
            break;
        case XTIO_XETHRU_IO8:
            *pin = XPIN_IO8_SWCLK;
            break;
        case XTIO_XETHRU_IO9:
            *pin = XPIN_IO9_SWDIO;
            break;
        case XTIO_X4_IO1:
            *pin = XPIN_X4IO1;
            break;
        case XTIO_X4_IO2:
            *pin = XPIN_X4IO2;
            break;
        case XTIO_X4_ENABLE:
            *pin = XPIN_X4ENABLE;
            break;
        case XTIO_USB_VBUS:
            *pin = XPIN_USBVBUS;
            break;
        default:
            return XTIO_WRONG_PIN_ID;
    }
    return XT_SUCCESS;
}

int xtio_set_direction(
    xtio_pin_id_t pin_id,
    int direction,
    int level
)
{
    ioport_pin_t pin;
    int status;
    status = xtio_decode_pin_id(pin_id, &pin);
    if (XT_SUCCESS == status)
    {
        ioport_enable_pin(pin);
        ioport_set_pin_dir(pin, direction);
        ioport_set_pin_level(pin, level);
    }
    return status;
}

int xtio_set_level(
    xtio_pin_id_t pin_id,
    int level
)
{
    ioport_pin_t pin;
    int status;
    status = xtio_decode_pin_id(pin_id, &pin);
    if (XT_SUCCESS == status)
    {
        ioport_set_pin_level(pin, level);
    }
    return status;
}

int xtio_get_level(
    xtio_pin_id_t pin_id,
    int * level
)
{
    ioport_pin_t pin;
    int status;
    status = xtio_decode_pin_id(pin_id, &pin);
    if (XT_SUCCESS == status)
    {
        *level = ioport_get_pin_level(pin);
    }
    return status;
}

int xtio_toggle_level(
    xtio_pin_id_t pin_id
)
{
    int level = 0;
    ioport_pin_t pin;
    int status;
    status = xtio_decode_pin_id(pin_id, &pin);
    if (XT_SUCCESS == status)
    {
        level = ioport_get_pin_level(pin);
        level = (level == 1) ? 0 : 1;
        ioport_set_pin_level(pin, level);
    }
    return status;
}

int xtio_set_pin_mode(
    xtio_pin_id_t pin_id,
    uint32_t mode
)
{
    ioport_pin_t pin;
    int status;
    status = xtio_decode_pin_id(pin_id, &pin);
    if (XT_SUCCESS == status)
    {
        if (mode & XTIO_PULL_DOWN)
        {
            ioport_set_pin_dir(pin, IOPORT_DIR_INPUT);
            ioport_set_pin_mode(pin, IOPORT_MODE_PULLDOWN);
        }
        else if (mode & XTIO_PULL_UP)
        {
            ioport_set_pin_dir(pin, IOPORT_DIR_INPUT);
            ioport_set_pin_mode(pin, IOPORT_MODE_PULLUP);
        }
    }
    return status;
}

//////////////////////////////////////////////////////////
// IO Interrupt functions

// Definitions of IO pin groups and masks
// Used for decoding correct pin interrupt function
#define XPIN_X4IO1_GROUP_ID (ID_PIOA + (XPIN_X4IO1 >> 5))
#define XPIN_X4IO2_GROUP_ID (ID_PIOA + (XPIN_X4IO2 >> 5))
#define XPIN_IO1_GROUP_ID (ID_PIOA + (XPIN_IO1 >> 5))
#define XPIN_IO2_GROUP_ID (ID_PIOA + (XPIN_IO2 >> 5))
#define XPIN_IO3_GROUP_ID (ID_PIOA + (XPIN_IO3 >> 5))
#define XPIN_IO4_GROUP_ID (ID_PIOA + (XPIN_IO4 >> 5))
#define XPIN_IO5_GROUP_ID (ID_PIOA + (XPIN_IO5 >> 5))
#define XPIN_IO6_GROUP_ID (ID_PIOA + (XPIN_IO6 >> 5))

#define XPIN_X4IO1_MASK (uint32_t)(1 << (XPIN_X4IO1 & 0x1F))
#define XPIN_X4IO2_MASK (uint32_t)(1 << (XPIN_X4IO2 & 0x1F))
#define XPIN_IO1_MASK (uint32_t)(1 << (XPIN_IO1 & 0x1F))
#define XPIN_IO2_MASK (uint32_t)(1 << (XPIN_IO2 & 0x1F))
#define XPIN_IO3_MASK (uint32_t)(1 << (XPIN_IO3 & 0x1F))
#define XPIN_IO4_MASK (uint32_t)(1 << (XPIN_IO4 & 0x1F))
#define XPIN_IO5_MASK (uint32_t)(1 << (XPIN_IO5 & 0x1F))
#define XPIN_IO6_MASK (uint32_t)(1 << (XPIN_IO6 & 0x1F))


struct {
    void (*xethru_io1_callback)(void);
    void (*xethru_io2_callback)(void);
    void (*xethru_io3_callback)(void);
    void (*xethru_io4_callback)(void);
    void (*xethru_io5_callback)(void);
    void (*xethru_io6_callback)(void);
    void (*x4_io1_callback)(void);
    void (*x4_io2_callback)(void);
} irq_callbacks;


void Dummy_Handler(void);
void xtio_pio_irq_handler(uint32_t pin_id, uint32_t mask);
int enable_io_irq(ioport_pin_t pin, uint32_t flags, void (*p_handler) (uint32_t _pin , uint32_t _mask));
int disable_io_irq(ioport_pin_t pin);

int xtio_irq_init(void)
{
	irq_callbacks.xethru_io1_callback = Dummy_Handler;
	irq_callbacks.xethru_io2_callback = Dummy_Handler;
	irq_callbacks.xethru_io3_callback = Dummy_Handler;
	irq_callbacks.xethru_io4_callback = Dummy_Handler;
	irq_callbacks.xethru_io5_callback = Dummy_Handler;
	irq_callbacks.xethru_io6_callback = Dummy_Handler;
	irq_callbacks.x4_io1_callback = Dummy_Handler;
	irq_callbacks.x4_io2_callback = Dummy_Handler;

	return XT_SUCCESS;
}

int xtio_irq_register_callback(
    xtio_pin_id_t pin_id,
    void (*irq_callback)(void),
    xtio_interrupt_modes mode
)
{
    ioport_pin_t pin;
	uint32_t flags;
    int status;
    status = xtio_decode_pin_id(pin_id, &pin);
    if (XT_SUCCESS == status)
    {
        switch (pin_id)
        {
            case XTIO_XETHRU_IO1:
				irq_callbacks.xethru_io1_callback = irq_callback;
				break;
            case XTIO_XETHRU_IO2:
				irq_callbacks.xethru_io2_callback = irq_callback;
				break;
            case XTIO_XETHRU_IO3:
	            irq_callbacks.xethru_io3_callback = irq_callback;
				break;
            case XTIO_XETHRU_IO4:
				irq_callbacks.xethru_io4_callback = irq_callback;
				break;
            case XTIO_XETHRU_IO5:
				irq_callbacks.xethru_io5_callback = irq_callback;
				break;
            case XTIO_XETHRU_IO6:
				irq_callbacks.xethru_io6_callback = irq_callback;
				break;
            case XTIO_X4_IO1:
                irq_callbacks.x4_io1_callback = irq_callback;
                break;
            case XTIO_X4_IO2:
                irq_callbacks.x4_io2_callback = irq_callback;
                break;
			default:
				return XTIO_WRONG_PIN_ID;
        }

        if ((mode & XTIO_INTERRUPT_RISING_EDGE) && (mode & XTIO_INTERRUPT_FALLING_EDGE))
        {
			flags = PIO_IT_EDGE;
        }
        else if (mode & XTIO_INTERRUPT_RISING_EDGE)
        {
			flags = PIO_IT_RISE_EDGE;
        }
        else if (mode & XTIO_INTERRUPT_FALLING_EDGE)
        {
			flags = PIO_IT_FALL_EDGE;
        }

        // Enable deglitching
        //##Pio * pio = pio_get_pin_group(pin);
    	//##uint32_t group_mask = pio_get_pin_group_mask(pin);
		// pio->PIO_IFSCER = group_mask;
		//##pio->PIO_IFER = group_mask;
		//##pio->PIO_SCDR = 1;
        // pio_set_input(pio, group_mask, PIO_DEGLITCH);
        // pio_set_debounce_filter(pio, group_mask, 5000); // Will give minimum time

		if (XT_SUCCESS != enable_io_irq(pin, flags, xtio_pio_irq_handler))
		{
			return XT_ERROR;
		}
    }

    return status;
}

/**
 * Unregister interrupt handler
 *
 * @return Status of execution as defined in @ref xtio_error_codes_t
 */
int xtio_irq_unregister_callback(
    xtio_pin_id_t pin_id          ///< IO Pin ID as defined in @ref xtio_pin_id_t
)
{
    ioport_pin_t pin;
    int status;
    status = xtio_decode_pin_id(pin_id, &pin);
    if (XT_SUCCESS == status)
    {
		if (XT_SUCCESS != disable_io_irq(pin))
		{
			return XT_ERROR;
		}
        switch (pin_id)
        {
            case XTIO_XETHRU_IO1:
				irq_callbacks.xethru_io1_callback = Dummy_Handler;
				break;
            case XTIO_XETHRU_IO2:
				irq_callbacks.xethru_io2_callback = Dummy_Handler;
				break;
            case XTIO_XETHRU_IO3:
	            irq_callbacks.xethru_io3_callback = Dummy_Handler;
				break;
            case XTIO_XETHRU_IO4:
				irq_callbacks.xethru_io4_callback = Dummy_Handler;
				break;
            case XTIO_XETHRU_IO5:
				irq_callbacks.xethru_io5_callback = Dummy_Handler;
				break;
            case XTIO_XETHRU_IO6:
				irq_callbacks.xethru_io6_callback = Dummy_Handler;
				break;
            case XTIO_X4_IO1:
                irq_callbacks.x4_io1_callback = Dummy_Handler;
                break;
            case XTIO_X4_IO2:
                irq_callbacks.x4_io2_callback = Dummy_Handler;
                break;
			default:
				return XTIO_WRONG_PIN_ID;
        }
	}
	return XT_SUCCESS;
}

int enable_io_irq(ioport_pin_t pin, uint32_t flags, void (*p_handler) (uint32_t _pin , uint32_t _mask))
{
    uint32_t status;
    Pio * pio = pio_get_pin_group(pin);
	uint32_t group_id =  pio_get_pin_group_id(pin);
	uint32_t group_mask = pio_get_pin_group_mask(pin);
    NVIC_DisableIRQ((IRQn_Type)group_id);
	status = pio_handler_set(pio, group_id, group_mask, flags, p_handler);
    if (status != 0)
    {
        // Max interrupt sources reached
        return XT_ERROR;
    }
    pio_handler_set_priority(pio, (IRQn_Type)group_id, 5);
    pio_enable_interrupt(pio, group_mask);
    NVIC_EnableIRQ((IRQn_Type)group_id);
	return XT_SUCCESS;
}

int disable_io_irq(ioport_pin_t pin)
{
    uint32_t status = XT_SUCCESS;
    Pio * pio = pio_get_pin_group(pin);
	uint32_t group_mask = pio_get_pin_group_mask(pin);
	pio_disable_interrupt(pio, group_mask);
	return status;
}


void xtio_pio_irq_handler(uint32_t pin_id, uint32_t mask)
{
	if ((XPIN_X4IO1_GROUP_ID == pin_id) && (XPIN_X4IO1_MASK == mask))
    {
        irq_callbacks.x4_io1_callback();
    }
	else if ((XPIN_X4IO2_GROUP_ID == pin_id) && (XPIN_X4IO2_MASK == mask))
    {
	    irq_callbacks.x4_io2_callback();
    }
	else if ((XPIN_IO1_GROUP_ID == pin_id) && (XPIN_IO1_MASK == mask))
    {
	    irq_callbacks.xethru_io1_callback();
    }
	else if ((XPIN_IO2_GROUP_ID == pin_id) && (XPIN_IO2_MASK == mask))
    {
	    irq_callbacks.xethru_io2_callback();
    }
	else if ((XPIN_IO3_GROUP_ID == pin_id) && (XPIN_IO3_MASK == mask))
    {
	    irq_callbacks.xethru_io3_callback();
    }
	else if ((XPIN_IO4_GROUP_ID == pin_id) && (XPIN_IO4_MASK == mask))
    {
	    irq_callbacks.xethru_io4_callback();
    }
	else if ((XPIN_IO5_GROUP_ID == pin_id) && (XPIN_IO5_MASK == mask))
    {
	    irq_callbacks.xethru_io5_callback();
    }
	else if ((XPIN_IO6_GROUP_ID == pin_id) && (XPIN_IO6_MASK == mask))
    {
	    irq_callbacks.xethru_io6_callback();
    }
}

