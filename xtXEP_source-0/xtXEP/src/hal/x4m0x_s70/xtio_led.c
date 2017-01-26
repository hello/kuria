/**
 * @file
 *
 * @brief Implementation of GPIO functions for X4M0x boards with Atmel SAM S70 MCU.
 *
 * See @ref X4M0x_SAMS70/xtio_gpio.h and @ref xt_XEP_HAL.h for more documentation.
 *
 * @todo Implement PWM mode for LEDs
 */


#include "xtio_led.h"
#include "board.h"
#include "ioport.h"

int xtio_decode_led_id(xtio_led_id_t led_id, ioport_pin_t * pin);

int xtio_decode_led_id(xtio_led_id_t led_id, ioport_pin_t * pin)
{
    switch (led_id)
    {
        case XTIO_LED_RED:
            *pin = XPIN_LED_RED;
            break;
        case XTIO_LED_GREEN:
            *pin = XPIN_LED_GREEN;
            break;
        case XTIO_LED_BLUE:
            *pin = XPIN_LED_BLUE;
            break;
        default:
            return XTIO_WRONG_LED_ID;
    }
    return XT_SUCCESS;
}


int xtio_led_init(
    xtio_led_id_t led_id,
    int mode,
    float led_state
)
{
    int pin_level;
    ioport_pin_t pin = 0;
    int status;

    if (0.5 <= led_state)
        pin_level = XTIO_PIN_LEVEL_LOW;
    else
        pin_level = XTIO_PIN_LEVEL_HIGH;

    status = xtio_decode_led_id(led_id, &pin);

    ioport_set_pin_dir(pin, XTIO_OUTPUT);
	ioport_set_pin_level(pin, pin_level);

    return status;
}

int xtio_led_set_state(
    xtio_pin_id_t led_id,
    float led_state
)
{
    int pin_level;
    ioport_pin_t pin = 0;
    int status;

    if (0.5 <= led_state)
        pin_level = XTIO_PIN_LEVEL_LOW;
    else
        pin_level = XTIO_PIN_LEVEL_HIGH;

    status = xtio_decode_led_id(led_id, &pin);

	ioport_set_pin_level(pin, pin_level);

    return status;
}

int xtio_led_toggle_state(
    xtio_pin_id_t led_id
)
{
    /** @todo Support LEDs in PWM mode */

    int pin_level;
    ioport_pin_t pin = 0;
    int status;

    status = xtio_decode_led_id(led_id, &pin);

    pin_level = ioport_get_pin_level(pin);
    pin_level = (pin_level == 1) ? 0 : 1;
    ioport_set_pin_level(pin, pin_level);

    return status;
}
