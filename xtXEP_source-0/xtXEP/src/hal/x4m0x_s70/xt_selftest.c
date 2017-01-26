#include "xep_hal.h"
#include "board.h"
#include "xtio_extram.h"
#include "xt_selftest.h"

// Storage for results of pre-run tests
uint32_t pretest_result[PRETEST_SIZE];

int xt_selftest_ext_ram(xt_test_result_t* test_result);
int xt_selftest_ext_ram_mark_faulty_pin(int pin);
int xt_selftest_ext_ram_mark_passed_test(int test);
bool xt_selftest_ext_ram_has_test_passed(int test);
uint8_t xt_selftest_ext_ram_get_storage(int index);

int xt_run_selftest(uint8_t id, xt_test_result_t* test_result)
{
    // Prepare result struct
    test_result->id = id;
    test_result->passed = false;
    for (int i = 0; i < XT_SELFTEST_DATA_LENGTH; i++)
        test_result->data[i] = 0;

    switch (id)
    {
        case XT_SELFTEST_EXT_RAM:
            return xt_selftest_ext_ram(test_result);
        default:
            return XT_NOT_IMPLEMENTED;
    }
}

int xt_selftest_ext_ram(xt_test_result_t* test_result)
{
    // Check that all implemented tests have passed
    if (xt_selftest_ext_ram_has_test_passed(SELFTEST_EXT_RAM_PIN_SHORT_PASSED) &&
        xt_selftest_ext_ram_has_test_passed(SELFTEST_EXT_RAM_DATA_PASSED) &&
        xt_selftest_ext_ram_has_test_passed(SELFTEST_EXT_RAM_MASKING_PASSED) &&
        xt_selftest_ext_ram_has_test_passed(SELFTEST_EXT_RAM_ADDRESSING_PASSED))
        test_result->passed = true;
    
    for (int i = 0; i < 8; i++)
        test_result->data[i] = xt_selftest_ext_ram_get_storage(i);

    return XT_SUCCESS;
}

int xt_selftest_ext_ram_mark_faulty_pin(int pin)
{
    // Pin faulty markers are at the most significant bits of storage
    if (pin < (SDRAM_PIN_COUNT%32))
        pretest_result[PRETEST_STORAGE_EXTRAM_L] |= (1<<(pin + (32 - SDRAM_PIN_COUNT%32)));
    else
        pretest_result[PRETEST_STORAGE_EXTRAM_H] |= (1<<(pin - (SDRAM_PIN_COUNT%32)));

    return XT_SUCCESS;
}

int xt_selftest_ext_ram_mark_passed_test(int test)
{
    pretest_result[PRETEST_STORAGE_EXTRAM_L] |= (1<<test);
    return XT_SUCCESS;
}

bool xt_selftest_ext_ram_has_test_passed(int test)
{
	if (test >= SELFTEST_EXT_RAM_PASSED_FIELD_SIZE)
	{
		return false;
	}
    return ((pretest_result[PRETEST_STORAGE_EXTRAM_L] & (1<<test)) != 0);
}

uint8_t xt_selftest_ext_ram_get_storage(int index)
{
    if (index >= 8) 
        return 0;
    
    if (index < 4)
        return (pretest_result[PRETEST_STORAGE_EXTRAM_L]>>(index*8))&0xFF;
    else
        return (pretest_result[PRETEST_STORAGE_EXTRAM_H]>>((index-4)*8))&0xFF;
}

int xt_selftest_ext_ram_shorts(void)
{
    /////////////////////////////
    // Pin short-circuit detection
    // Checks for contention between the IO pins used for the SDRAM. Set 
    // all pins to input with internal pull-ups. Force one and one line 
    // to 0 and verify that all other lines remain high. 

    bool passed = true;

    // Set SDRAM pins to input pins with pullup
    for (int i = 0; i < SDRAM_PIN_COUNT; i++)
        pio_configure_pin(SDRAM_PINS[i], PIO_TYPE_PIO_INPUT | PIO_PULLUP);

    // Loop through all pins, setting the pin low and checking that all others remain high
    for (int i = 0; i < SDRAM_PIN_COUNT; i++)
    {
        pio_configure_pin(SDRAM_PINS[i], PIO_TYPE_PIO_OUTPUT_0);

        for (int j = 0; j < SDRAM_PIN_COUNT; j++)
        {
            // Skip pin that is tied low
            if (i == j)
                continue;
            
            // This pin should not be low
            if (pio_get_pin_value(SDRAM_PINS[j]) == 0)
            {
                passed = false;

                xt_selftest_ext_ram_mark_faulty_pin(i);
                xt_selftest_ext_ram_mark_faulty_pin(j);
            }
        }
        
        pio_configure_pin(SDRAM_PINS[i], PIO_TYPE_PIO_INPUT | PIO_PULLUP);
    }
    if (passed)
        xt_selftest_ext_ram_mark_passed_test(SELFTEST_EXT_RAM_PIN_SHORT_PASSED);

    return XT_SUCCESS;
}

int xt_selftest_ext_ram_functionality(void)
{
    bool passed = true;
    volatile uint8_t* external_memory_to_test = (uint8_t*) BOARD_SDRAM_ADDR;
   
    /////////////////////////////
    // Data line test
    // Make sure all data lines are connected and functional. 
    // There are 16 lines to test, so at least 2 Byte-sized accesses are required. 
    passed = true;

    for (int i = 0; i < 8; i++)
    {
        external_memory_to_test[0] = (1<<i); // Test pattern
        external_memory_to_test[0xFE] = 0; // Make sure to discharge potential floating line
        if (external_memory_to_test[0] != (1<<i))
        {
            passed = false;

            xt_selftest_ext_ram_mark_faulty_pin(12 + i); // Lower data bits
        }
    }

    for (int i = 0; i < 8; i++)
    {
        external_memory_to_test[1] = (1<<i); // Test pattern
        external_memory_to_test[0xFF] = 0; // Make sure to discharge potential floating line
        if (external_memory_to_test[1] != (1<<i))
        {
            passed = false;

            xt_selftest_ext_ram_mark_faulty_pin(12 + i + 8); // Upper data bits
        }
    }
    
    if (passed)
        xt_selftest_ext_ram_mark_passed_test(SELFTEST_EXT_RAM_DATA_PASSED);

    /////////////////////////////
    // Masking line test
    // The 16-bit data bus has mask bits to support 8-bit access.
    // This is tested by doing side-by-side accesses and checking
    // that no value is corrupted. 
    passed = true;

    external_memory_to_test[0] = 0x55; // Initial write to lower byte
    external_memory_to_test[1] = 0xAA; // Write upper byte
    if (external_memory_to_test[0] != 0x55) // Test that lower byte remains
    {
        passed = false;

        xt_selftest_ext_ram_mark_faulty_pin(12 + 16 + 2 + 2); // LQDM
    }
    external_memory_to_test[0] = 0x55; // Lower write
    if (external_memory_to_test[1] != 0xAA) // Test that upper byte remains
    {
        passed = false;

        xt_selftest_ext_ram_mark_faulty_pin(12 + 16 + 2 + 2 + 1); // UQDM
    }

    if (passed)
        xt_selftest_ext_ram_mark_passed_test(SELFTEST_EXT_RAM_MASKING_PASSED);


    /////////////////////////////
    // Address line test (Ax, CAS, RAS, BAx)
    // Make sure all address lines are connected and functional. 
    // Columns are lower 8 bits, rows next 12 bits and bank 2 upper bits.
    // Column and row uses same address pins, but CAS and RAS to commit.
    passed = true;

    // 12 address pins and RAS by activating one and one line
    for (int i = 0; i < 12; i++)
        external_memory_to_test[1<<(i + 8+1)] = ((((i^0x55)<<4) + i)&0xFF);
    external_memory_to_test[0] = 0x00; // Make sure disconnected pins are caught
    for (int i = 0; i < 12; i++)
    {
        if (external_memory_to_test[1<<(i + 8+1)] != ((((i^0x55)<<4) + i)&0xFF))
        {
            passed = false;
            
            // Address pins are first in pin array
            xt_selftest_ext_ram_mark_faulty_pin(i);
            xt_selftest_ext_ram_mark_faulty_pin(12 + 16 + 2); // RAS line index
        }
    }

    // Test CAS
    if (passed)
    {
        for (int i = 0; i < 8; i++)
            external_memory_to_test[1<<(i + 1)] = ((((i^0x7)<<4) + (i^0x55))&0xFF);
        external_memory_to_test[0] = 0x00; // Make sure disconnected pins are caught
        for (int i = 0; i < 8; i++)
        {
            if (external_memory_to_test[1<<(i + 1)] != ((((i^0x7)<<4) + (i^0x55))&0xFF))
            {
                passed = false;
                
                // All address pins were tested in previous step, assuming CAS problem
                xt_selftest_ext_ram_mark_faulty_pin(12 + 16 + 2 + 1);
            }
        }
    }

    // Test banking pins
    if (passed)
    {
        external_memory_to_test[1<<21] = 0x13;
        external_memory_to_test[1<<22] = 0x37;
        external_memory_to_test[0] = 0x00;
		for (volatile int i = 0; i < 1000; i++);
        if (external_memory_to_test[1<<21] != 0x13)
        {
            passed = false;
                
            // All address pins were tested in previous step, assuming BA0 problem
            xt_selftest_ext_ram_mark_faulty_pin(12 + 16 + 0);
        }
        if (external_memory_to_test[1<<22] != 0x37)
        {
            passed = false;
                
            // All address pins were tested in previous step, assuming BA1 problem
            xt_selftest_ext_ram_mark_faulty_pin(12 + 16 + 1);
        }
        external_memory_to_test[(1<<22)|(1<<21)] = 0x5E;
        external_memory_to_test[0] = 0xD9;
        external_memory_to_test[(1<<22)|(1<<21)] = 0x5E;
        for (volatile int i = 0; i < 1000; i++);
        if ((external_memory_to_test[(1<<22)|(1<<21)] != 0x5E) || 
			(external_memory_to_test[0] != 0xD9))
        {
            passed = false;
                
            // All address pins were tested in previous step, assuming BA0 + BA1 problem
            xt_selftest_ext_ram_mark_faulty_pin(12 + 16 + 1);
            xt_selftest_ext_ram_mark_faulty_pin(12 + 16 + 0);
        }
    }

    if (passed)
        xt_selftest_ext_ram_mark_passed_test(SELFTEST_EXT_RAM_ADDRESSING_PASSED);

    return XT_SUCCESS;
}