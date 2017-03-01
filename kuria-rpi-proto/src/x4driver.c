/**
 * @file
 * @brief Platform independent driver to interface with Xx radar.
 *
 */

#include "x4driver.h"
#include <string.h>
#include "spidriver.h"

#define START_OF_SRAM_LSB       0x00
#define START_OF_SRAM_MSB       0x00
#define SET_PROGRAMMING_MODE    0x01
#define SET_READBACK_MODE       0x02
#define SET_NORMAL_MODE         0x00
#define FIFO_NOT_EMPTY_FLAG     0x04

#define TO_CPU_EMPTY_BIT 2
#define FROM_CPU_VALID_BIT 1
#define BIT_SET 1
#define BIT_NOT_SET 0


#define X4DRIVER_DAC_MAX 2047

#define SPI_ADDRESS_WRITE 0x80
#define PIF_ADDRESS_WRITE 0x80

#define XEP_LOCK_OK                             1
#define XEP_LOCK_NOK                            0

#define FRAME_IS_READY_READ_REGISTERS 1
#define FRAME_NOT_READY 0
#define TRIGGER_SWEEP_READ_REGISTERS 1
#define TRIGGER_SWEEP_TRIGGER_PIN 0
#define RESET_COUNTERS_ACTION 0xff
#define TRX_START_ACTION 0xff


#include "8051_firmware.h"

/**
 * @brief x4driver internal buffer struct.
 */
typedef struct
{
    uint32_t size;
    uint8_t * data;
} X4DriverBuffer_t;



/**
 * @brief X4 protocol command ids
 */
typedef enum {
	PIF_COMMAND				= 0,
	XIF_COMMAND				= 1,
	X4_SW_REGISTER_COMMAND	= 2,
	X4_SW_ACTION_COMMAND	= 3,
} xtx4_command_id_t;



/**
 * @brief X4 software actions enum.
 */
typedef enum {
	X4_SW_ACTION_START_TIMER			= 0,
	X4_SW_ACTION_STOP_TIMER				= 1,	
} xtx4_software_action_address_t;


/**
 * @brief X4 software register enum.
 */
typedef enum {
	X4_SW_REGISTER_FPS_LSB_ADDR					= 0,
	X4_SW_REGISTER_FPS_MSB_ADDR					= 1,
	X4_SW_FRAME_COUNTER_RESET_VALUE_1_ADDR		= 2,
	X4_SW_FRAME_COUNTER_RESET_VALUE_2_ADDR		= 3,
	X4_SW_FRAME_COUNTER_RESET_VALUE_3_ADDR		= 4,
	X4_SW_FRAME_COUNTER_RESET_VALUE_4_ADDR		= 5,
	X4_SW_USE_PERIOD_TRIGGER					= 6,
	X4_SW_PERIOD_0_ADDR							= 7,
	X4_SW_PERIOD_1_ADDR							= 8,
	X4_SW_PERIOD_2_ADDR							= 9,
	X4_SW_PERIOD_3_ADDR							= 10,
	X4_SW_PERIOD_DIVIDER_0_ADDR					= 11,
	X4_SW_PERIOD_DIVIDER_1_ADDR					= 12,
} xtx4_software_register_address_t;		 



#define PIF_COMMAND_MAX_RETRIES 200
#define OSC_LOCK_ATTEMPS_MAX    100
#define MS                      1000

#define BOOT_FROM_SRAM 0x00

#define DVDD_RX_POWER_GOOD_BIT 3
#define DVDD_TX_POWER_GOOD_BIT 2
#define AVDD_RX_POWER_GOOD_BIT 1
#define AVDD_TX_POWER_GOOD_BIT 0

#define MIN_FRAME_LENGTH 4
#define FETCH_DATA_ACTION	0xff

uint64_t signbit_mask = 0x800000000000LL;
uint64_t convert_mask = 0xffffffffffffLL;
//frame_area constants
const float32_t X4DRIVER_MAX_BINS_RANGE_METERS_RF = 9.8765432098765432098765432098765;
const float32_t X4DRIVER_MAX_BINS_RANGE_METERS_DOWN_CONVERSION = 9.6707818930041152263374485596708;

const float32_t X4DRIVER_MAX_RANGE_OFFSET = 0.616856909*255;
const float32_t X4DRIVER_MIN_RANGE = 0;
const float32_t X4DRIVER_RX_WAIT_OFFSET_IN_METERS = 0.616856909;
const float32_t X4DRIVER_METERS_PER_BIN = 0.00643004115226337448559670781893;
const float32_t X4DRIVER_METERS_PER_BIN_DOWN_CONVERTION = 0.05144032921810699588477366255144;
const uint32_t  X4DRIVER_BINS_PER_RX_MFRAMES_COARSE = 96;
const uint32_t  X4DRIVER_BINS_PER_RX_MFRAMES = 12;
#include <math.h>



//prototypes
int _x4driver_get_x4_sw_register(X4Driver_t* x4driver, uint8_t address,uint8_t * read_value);
int _x4driver_set_x4_sw_action(X4Driver_t* x4driver, uint8_t address);
int _x4driver_set_x4_sw_register(X4Driver_t* x4driver, uint8_t address, uint8_t write_value);

int _x4driver_get_x4_internal_register(X4Driver_t* x4driver, uint8_t address,uint8_t * read_value, uint8_t command);
int _x4driver_set_internal_register(X4Driver_t* x4driver, uint8_t address, uint8_t write_value, uint8_t command);
void _x4driver_set_action(X4Driver_t* x4driver,xtx4_x4driver_action_t action);
void _x4driver_clear_action(X4Driver_t* x4driver,xtx4_x4driver_action_t action);

uint32_t _unpack_bin(uint8_t * data, uint8_t bytes_per_counter);
uint32_t _update_normalization_variables(X4Driver_t* x4driver);
int _x4driver_unpack_frame(X4Driver_t* x4driver,uint32_t* bins_data, uint32_t bins_data_size ,uint8_t * raw_data, uint32_t raw_data_length);
void _x4driver_normalize_frame(X4Driver_t* x4driver,uint32_t * data,uint32_t data_length,float32_t * destination,uint32_t destination_length );
void _update_normalization_constansts(X4Driver_t* x4driver);
int _x4driver_unpack_and_normalize_frame(X4Driver_t* x4driver,float* bins_data, uint32_t bins_data_size ,uint8_t * raw_data, uint32_t raw_data_length);
int _x4driver_unpack_and_normalize_downconverted_frame(X4Driver_t* x4driver,float* bins_data, uint32_t bins_data_size ,uint8_t * raw_data, uint32_t raw_data_length);
int _x4driver_enable_dvdd_tx(X4Driver_t* x4driver);
int _x4driver_enable_dvdd_rx(X4Driver_t* x4driver);
int _x4driver_enable_avdd_tx(X4Driver_t* x4driver);
int _x4driver_enable_avdd_rx(X4Driver_t* x4driver);
int _x4driver_get_downconvertion_coef_c1_q_length(uint32_t * size);
int _x4driver_get_downconvertion_coef_c1_i_length(uint32_t * size);
int _x4driver_get_downconvertion_coef_c1_q(X4DriverBuffer_t * buffer);
int _x4driver_get_downconvertion_coef_c1_i(X4DriverBuffer_t * buffer);
int _x4driver_get_data_8051_size(uint32_t * size);
int _x4driver_get_8051_default_firmware(X4DriverBuffer_t * buffer);
uint32_t _get_mask(uint32_t number_of_bytes);
int _x4driver_set_rx_ram_first_line(X4Driver_t* x4driver, uint32_t first_line);
int _x4driver_set_rx_ram_last_line(X4Driver_t* x4driver,uint32_t last_line);
int _x4driver_get_framecounter(X4Driver_t* x4driver, uint32_t * read_value);
void _invert(int8_t * source,int8_t * dst,uint8_t len);
float32_t _get_dr(X4Driver_t* x4driver);

/**
 *  Set of coefficients for down conversion for I.
    The coefficients cannot be read back from X4. 
    Should be written with most significant coefficient first.                  
 */
static  int8_t downconvertion_coef_c1_i[] = {0, 1, -3, 0, 6, -7, -3, 16, -10, -13, 24, -5, -25, 25, 6, -31, 17, 16, -27, 6, 18, -16, -2, 12, -6, -3, 5,-1, -1, 1,0,0};


/**
 * Number of I down conversion coefficients. 
 */
static const uint32_t downconvertion_coef_c1_i_length = 32;


/**
 *  Set of coefficients for down conversion for Q.
    The coefficients cannot be read back from X4. 
    Should be written with most significant coefficient first.                  
 */
static  int8_t downconvertion_coef_c1_q[] = {1, -1, -1, 5, -3, -6, 12, -2, -16, 18, 6, -27, 16, 17, -31, 6, 25, -25, -5, 24, -13, -10, 16, -3, -7, 6, 0,-3, 1, 0,0,0};

/**
 * Number of I down conversion coefficients. 
 */
static const uint32_t downconvertion_coef_c1_q_length = 32;



/**
 * @brief Gets the size of i down conversion coefficients.
 *
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_get_downconvertion_coef_c1_i_length(uint32_t * size)
{
    *size = downconvertion_coef_c1_i_length;
    return XEP_ERROR_X4DRIVER_OK;
}

/**
 * @brief Gets the size of q down conversion coefficients.
 *
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_get_downconvertion_coef_c1_q_length(uint32_t * size)
{
    *size = downconvertion_coef_c1_q_length;
    return XEP_ERROR_X4DRIVER_OK;
}


/**
 * @brief Gets the size of the X4 8051 firmware .
 *
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_get_data_8051_size(uint32_t * size)
{
    *size = data_8051_size;
    return XEP_ERROR_X4DRIVER_OK;
}

/**
 * @brief Utility function to take mutex. 
 */
static uint32_t mutex_take(X4Driver_t* x4driver)
{
	if (x4driver->lock.lock(x4driver->lock.object,x4driver->lock.timeout) == XEP_LOCK_OK)
        return XEP_ERROR_X4DRIVER_OK;
	else
        return XEP_ERROR_X4DRIVER_BUSY;
}


/**
 * @brief Utility function to give mutex. 
 */
static void mutex_give(X4Driver_t* x4driver)
{
	x4driver->lock.unlock(x4driver->lock.object);
}


/**
 * @brief Gets the default firmware used to programming 8051 in X4.
 *
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_get_8051_default_firmware(X4DriverBuffer_t * buffer)
{
    if(buffer->size >data_8051_size)
    {
        return XEP_ERROR_X4DRIVER_NOK;
    }
    memcpy(buffer->data, data_8051_onboard, data_8051_size);    
    return XEP_ERROR_X4DRIVER_OK;
}

/**
 * @brief Gets the default i down conversion coefficients for X4.
 *
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_get_downconvertion_coef_c1_i(X4DriverBuffer_t * buffer)
{
    if(buffer->size >downconvertion_coef_c1_i_length)
    {
        return XEP_ERROR_X4DRIVER_NOK;
    }
    memcpy(buffer->data, downconvertion_coef_c1_i, downconvertion_coef_c1_i_length);        
    return XEP_ERROR_X4DRIVER_OK;
}

/**
 * @brief Gets the default q down conversion coefficients for X4.
 *
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_get_downconvertion_coef_c1_q(X4DriverBuffer_t * buffer)
{
    if(buffer->size >downconvertion_coef_c1_q_length)
    {
        return XEP_ERROR_X4DRIVER_NOK;
    }
    memcpy(buffer->data, downconvertion_coef_c1_q, downconvertion_coef_c1_q_length);        
    return XEP_ERROR_X4DRIVER_OK;
}


 /* 
 *@brief Gets the filter coefficients for X4.
 *
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_get_filter_coefficients(uint8_t * buffer_q, uint8_t * buffer_i, uint8_t size)
{
	if(size != downconvertion_coef_c1_q_length)
	{
		return XEP_ERROR_X4DRIVER_NOK;
	}
	memcpy(buffer_q, downconvertion_coef_c1_q, downconvertion_coef_c1_q_length);
	memcpy(buffer_i, downconvertion_coef_c1_i, downconvertion_coef_c1_i_length);
	return XEP_ERROR_X4DRIVER_OK;
}



 /* 
 *@brief Sets the filter coefficients for X4.
 *
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_set_filter_coefficients(uint8_t * buffer_q, uint8_t * buffer_i, uint8_t size)
{
	if(size != downconvertion_coef_c1_q_length)
	{
		return XEP_ERROR_X4DRIVER_NOK;
	}
	memcpy(downconvertion_coef_c1_q, buffer_q, downconvertion_coef_c1_q_length);
	memcpy(downconvertion_coef_c1_i, buffer_i, downconvertion_coef_c1_i_length);
	return XEP_ERROR_X4DRIVER_OK;
}

/**
 * @brief Updates normalization constants.
 *
 */
void _update_normalization_constansts(X4Driver_t* x4driver)
{
	x4driver->normalization_offset = (X4DRIVER_DAC_MAX - x4driver->dac_max + x4driver->dac_min)/2;
	x4driver->normalization_nfactor = x4driver->iterations *x4driver->pulses_per_step;
}


/**
 * @brief Updates normalization variables and calculates new constants.
 *
 */
uint32_t _update_normalization_variables(X4Driver_t* x4driver)
{
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	uint8_t iterations = 0x00;
	x4driver_get_iterations(x4driver,&iterations);
	x4driver->iterations = iterations;
	uint16_t pulses_per_step = 0x0000;
	x4driver_get_pulses_per_step(x4driver,&pulses_per_step);
	x4driver->pulses_per_step = pulses_per_step;
	uint16_t dac_min = 0x0000;
	x4driver_get_dac_min(x4driver,&dac_min);
	x4driver->dac_min = dac_min;
	uint16_t dac_max = 0x0000;
	x4driver_get_dac_max(x4driver,&dac_max);
	x4driver->dac_max = dac_max;
	uint8_t bytes_per_counter = 0x00;
	x4driver_get_pif_register(x4driver,ADDR_PIF_RX_COUNTER_NUM_BYTES_RW,&bytes_per_counter);
	x4driver->bytes_per_counter =bytes_per_counter;
	_update_normalization_constansts(x4driver);
	mutex_give(x4driver);
	return status;
}



/**
 * @brief unpacks bin. 
 * Note: returns 32 bit value, max 4 bytes per counter.
 */
__attribute__ ((optimize("-O3"))) uint32_t _unpack_bin(uint8_t * data, uint8_t bytes_per_counter)
{
	uint32_t val = 0;
	for(uint32_t i = 0; i<bytes_per_counter;i++)
	{		
		val +=data[i] << i*8;
	}
	return val;
}


/**
 * @brief Gets mask for reading bins from buffer.  
 */
uint32_t _get_mask(uint32_t number_of_bytes)
{
	uint32_t mask = 0;
	if(number_of_bytes == 1)
	{
		mask = 0x000000ffu;
	}
	else if(number_of_bytes == 2)
	{
		mask = 0x0000ffffu;
	}
	else if(number_of_bytes == 3)
	{
		mask = 0x00ffffffu;
	}
	else if(number_of_bytes == 4)
	{
		mask = 0xffffffffu;
	}
	return mask;
}




/**
 * @brief Unpacks frame.
 *
 * @return Status of execution as defined in x4driver.h
 */
__attribute__ ((optimize("-O3"))) int _x4driver_unpack_frame(X4Driver_t* x4driver,uint32_t* bins_data, uint32_t bins_data_size ,uint8_t * raw_data, uint32_t raw_data_length)
{
	if (x4driver->bytes_per_counter >4)
	{
		return XEP_ERROR_X4DRIVER_UNPACK_FRAME_TO_LARGE_COUNTER;
	}
	if(bins_data_size*x4driver->bytes_per_counter > raw_data_length)
	{
		return XEP_ERROR_X4DRIVER_BUFFER_TO_SMALL;
	}	
	uint32_t mask = _get_mask(x4driver->bytes_per_counter);
	
	uint32_t raw_data_index = 0;
	for(uint32_t i = 0; i<bins_data_size;i++)
	{   
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wcast-align"
		
		uint32_t bin_data = *((uint32_t*)&raw_data[raw_data_index]) & mask;		
		#pragma GCC diagnostic pop
		bins_data[i] = bin_data;
		raw_data_index = (i+1)*x4driver->bytes_per_counter;
	}
	return XEP_ERROR_X4DRIVER_OK;
}

/**
 * @brief gets DAC range based on DAC min, max and dacstep.
 */
float32_t _get_dr(X4Driver_t* x4driver)
{
	xtx4_dac_step_t ds = 1;
	x4driver_get_dac_step(x4driver,&ds);
	 uint8_t dac_step_val =  1 << ds;
	return 	(((float32_t)(1+x4driver->dac_max-x4driver->dac_min))/2)/dac_step_val;
}

/**
 * @brief Unpacks and normalizes down converted frame.
 */
__attribute__ ((optimize("-O3"))) int _x4driver_unpack_and_normalize_downconverted_frame(X4Driver_t* x4driver,float* bins_data, uint32_t bins_data_size ,uint8_t * raw_data, uint32_t raw_data_length)
{
	if (x4driver->bytes_per_counter >6)
	{
		return XEP_ERROR_X4DRIVER_UNPACK_FRAME_TO_LARGE_COUNTER;
	}
	if(bins_data_size*x4driver->bytes_per_counter > raw_data_length)
	{
		return XEP_ERROR_X4DRIVER_BUFFER_TO_SMALL;
	}
	
	
	float32_t dc = _get_dr(x4driver);
	float32_t nfactor = 1.0/(x4driver->normalization_nfactor*dc*154);
	uint32_t q_data_start = bins_data_size/2;
	uint32_t i_data_start = 0;
	uint32_t raw_data_index = 0;
	signbit_mask = 0x0000000000000001ULL<<((8*x4driver->bytes_per_counter)-1);
	convert_mask = (0x0000000000000001ULL<<((8*x4driver->bytes_per_counter)))-1;
	
	for(uint32_t i = 0; i<bins_data_size;i++)
	{				
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wcast-align"
		int64_t bin_data = *((uint32_t*)&raw_data[raw_data_index]);
		if(x4driver->bytes_per_counter == 5)
			bin_data += ((uint64_t)*((uint8_t *)&raw_data[raw_data_index+4])) << 32;
		if(x4driver->bytes_per_counter == 6)
			bin_data += ((uint64_t)*((uint16_t *)&raw_data[raw_data_index+4])) << 32;
		
		#pragma GCC diagnostic pop
		if((bin_data & signbit_mask) != 0)//sign extend
		{
			int64_t inverted = ~bin_data & convert_mask;
			inverted = inverted+1;
			bin_data = -inverted;
		}		
		
		
		float32_t fbin = ((float32_t)bin_data*nfactor); 
		if(x4driver->iq_separate == 1)
		{					
			if(i%2 == 0)
			{
				bins_data[i_data_start] = fbin;
				i_data_start++;
			}
			else
			{
				bins_data[q_data_start] = fbin;
				q_data_start++;
			}
		}
		else
		{
			bins_data[i] = fbin;
		}
		raw_data_index = (i+1)*x4driver->bytes_per_counter;
	}
	return XEP_ERROR_X4DRIVER_OK;
}


/**
 * @brief Unpacks and normalizes frame.
 */
__attribute__ ((optimize("-O3"))) int _x4driver_unpack_and_normalize_frame(X4Driver_t* x4driver,float* bins_data, uint32_t bins_data_size ,uint8_t * raw_data, uint32_t raw_data_length)
{
	if (x4driver->bytes_per_counter >4)
	{
		return XEP_ERROR_X4DRIVER_UNPACK_FRAME_TO_LARGE_COUNTER;
	}
	if(bins_data_size*x4driver->bytes_per_counter > raw_data_length)
	{
		return XEP_ERROR_X4DRIVER_BUFFER_TO_SMALL;
	}
	uint32_t mask = _get_mask(x4driver->bytes_per_counter);
	float32_t dc = _get_dr(x4driver);
	float32_t nfactor = 1.0/x4driver->normalization_nfactor;
	uint32_t raw_data_index = 0;
	
	for(uint32_t i = 0; i<bins_data_size;i++)
	{		
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wcast-align"
		uint32_t bin_data = *((uint32_t*)&raw_data[raw_data_index]) & mask;		
		#pragma GCC diagnostic pop
		bins_data[i] = (((float32_t) bin_data*nfactor) - dc)/dc;
		raw_data_index = (x4driver->frame_area_start_bin_offset+i+1)*x4driver->bytes_per_counter;//set_frame_area skip unused bins in start of ram line.
	}
	return XEP_ERROR_X4DRIVER_OK;
}



/**
 * @brief Normalizes frame. 
 */
__attribute__ ((optimize("-O3"))) void _x4driver_normalize_frame(X4Driver_t* x4driver,uint32_t * data,uint32_t data_length,float32_t * destination,uint32_t destination_length )
{
	for(uint32_t i = 0; i <destination_length;i++)
	{
		destination[i] = (((float32_t)data[i])/x4driver->normalization_nfactor) + x4driver->normalization_offset;
	}
}



/**
 * @brief Gets size of X4Driver struct.
 * @return size of x4driver struct.
 */
int x4driver_get_instance_size(void)
{
	return sizeof(X4Driver_t);
}



/**
 * @brief Creates x4driver instance. Initiates data structures with predefined values.
 *
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_create(X4Driver_t** x4driver, void* instance_memory, X4DriverCallbacks_t* x4driver_callbacks,X4DriverLock_t *lock,X4DriverTimer_t *timer,X4DriverTimer_t *timer_action, void* user_reference)
{
	X4Driver_t* d = (X4Driver_t*)instance_memory;
	memset(d, 0, sizeof(X4Driver_t));
	d->user_reference = user_reference;	
	d->lock.object = lock->object;
    d->lock.lock = lock->lock;
    d->lock.unlock = lock->unlock;    
    d->lock.timeout = 100; // default timeout.
	d->callbacks.pin_set_enable = x4driver_callbacks->pin_set_enable;
	d->callbacks.spi_read = x4driver_callbacks->spi_read;
	d->callbacks.spi_write = x4driver_callbacks->spi_write;
	d->callbacks.spi_write_read = x4driver_callbacks->spi_write_read;    
    d->callbacks.wait_us = x4driver_callbacks->wait_us;
	d->callbacks.notify_data_ready = x4driver_callbacks->notify_data_ready;
	d->callbacks.trigger_sweep = x4driver_callbacks->trigger_sweep;	
	d->callbacks.enable_data_ready_isr = x4driver_callbacks->enable_data_ready_isr;	
	d->frame_counter = 0;
	d->frame_length = 1536; 
	d->frame_is_ready_strategy = FRAME_IS_READY_READ_REGISTERS;
	d->sweep_trigger_strategy = TRIGGER_SWEEP_READ_REGISTERS;
	d->sweep_timer.object = timer->object;
	d->sweep_timer.configure = timer->configure;
	d->action_timer.object = timer_action->object;
	d->action_timer.configure = timer_action->configure;
	d->next_action = 0;
    d->trigger_mode = SWEEP_TRIGGER_MANUAL;
	d->region = X4DRIVER_REGION_EU;
	d->downconvertion_enabled= 0;
	d->iq_separate = 1;
	d->configured_fps = 0;
	d->frame_area_offset_meters =0;
	d->frame_area_start = 0;
	d->frame_area_end = X4DRIVER_MAX_BINS_RANGE_METERS_RF;
	
	*x4driver = d;
	return XEP_ERROR_X4DRIVER_OK;
}



/**
 * @brief Uploads custom 8051 firmware to X4.
 * Assumes Enable has been set.
 * @return Status of execution as defined in x4driver.h.
 */
int x4driver_upload_firmware_custom(X4Driver_t* x4driver, uint8_t * buffer,uint32_t lenght)
{
    uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
    
                  
    x4driver_set_spi_register(x4driver,ADDR_SPI_MEM_FIRST_ADDR_LSB_RW,START_OF_SRAM_LSB);
    x4driver_set_spi_register(x4driver,ADDR_SPI_MEM_FIRST_ADDR_MSB_RW,START_OF_SRAM_MSB);
    x4driver_set_spi_register(x4driver,ADDR_SPI_MEM_MODE_RW,SET_PROGRAMMING_MODE);
    
    for(uint32_t i = 0; i< lenght;i++)
	{
		x4driver_set_spi_register(x4driver,ADDR_SPI_TO_MEM_WRITE_DATA_WE,buffer[i]);
	}
            
           
    mutex_give(x4driver);
    return status;
}


/**
 * @brief Clear action. Will stop action timer if no other actions are set. 
 * @return Status of execution as defined in x4driver.h.
 */
void _x4driver_clear_action(X4Driver_t* x4driver,xtx4_x4driver_action_t action)
{
	x4driver->next_action = (x4driver->next_action  & ~action);
	if(x4driver->next_action == X4DRIVER_ACTION_NONE)
	{
		x4driver->action_timer.configure((void*)&x4driver->action_timer,0);
	}
}



/**
 * @brief Set action. Will start action timer if previous action was not set.
 * @return Status of execution as defined in x4driver.h.
 */
void _x4driver_set_action(X4Driver_t* x4driver,xtx4_x4driver_action_t action)
{	
	xtx4_x4driver_action_t prev = x4driver->next_action; 
	x4driver->next_action = (x4driver->next_action  | action);
	if(prev == X4DRIVER_ACTION_NONE)
	{
		x4driver->action_timer.configure((void*)&x4driver->action_timer,500);	
	}
	
}

/**
 * @brief Action timer callback. Used by mcu sweep controller to execute actions non blocking.
 * @return Status of execution as defined in x4driver.h.
 */
int x4driver_on_action_event(X4Driver_t* x4driver)
{
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	
	if((x4driver->next_action & X4DRIVER_ACTION_CHECK_FRAME_READY) == X4DRIVER_ACTION_CHECK_FRAME_READY)
	{
		uint8_t frame_is_ready = 0x00;
		x4driver_is_frame_ready(x4driver,&frame_is_ready);
		if(frame_is_ready)
		{
			_x4driver_clear_action(x4driver,X4DRIVER_ACTION_CHECK_FRAME_READY);
		}		
	}
	
	mutex_give(x4driver);
    return status;
}


/**
 * @brief Will check if frame is ready from X4 and return when the frame is ready.
 * Assumes Enable has been set.
 * Assumes X4 firmware has been programmed.
 * @return Status of execution as defined in x4driver.h.
 */
int x4driver_is_frame_ready(X4Driver_t* x4driver,uint8_t * is_ready)
{
    uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
    
    if(x4driver->frame_is_ready_strategy == FRAME_IS_READY_READ_REGISTERS)
	{
		uint8_t trx_ctrl_done = 0x00;
		status |= x4driver_get_pif_register(x4driver,ADDR_PIF_TRX_CTRL_DONE_R,&trx_ctrl_done);
		*is_ready  = trx_ctrl_done;
		//notify out
		if(x4driver->trigger_mode == SWEEP_TRIGGER_MCU)
		{					
			if(*is_ready == 1)
			{
				x4driver->callbacks.notify_data_ready(x4driver->user_reference);	
			}
		}
		
	}
	else
	{
		status = XEP_ERROR_X4DRIVER_NOT_SUPPORTED;
	}
	
    mutex_give(x4driver);
    return status;
}



/**
 * @brief Will trigger a radar sweep.
 * Assumes Enable has been set.
 * Assumes X4 firmware has been programmed.
 * @return Status of execution as defined in x4driver.h.
 */
int x4driver_start_sweep(X4Driver_t* x4driver)
{
    uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
    
    if(x4driver->sweep_trigger_strategy == TRIGGER_SWEEP_READ_REGISTERS )
	{						
		status = x4driver_set_pif_register(x4driver,ADDR_PIF_RX_RESET_COUNTERS_W,RESET_COUNTERS_ACTION);
		status = x4driver_set_pif_register(x4driver,ADDR_PIF_TRX_START_W,TRX_START_ACTION);
		if (x4driver->trigger_mode == SWEEP_TRIGGER_MCU)
			_x4driver_set_action(x4driver,X4DRIVER_ACTION_CHECK_FRAME_READY);
	}
	else
	{
		_x4driver_set_x4_sw_action(x4driver,5);// enable trigger pin.
		status = x4driver->callbacks.trigger_sweep(x4driver->user_reference);	
	}
	
    mutex_give(x4driver);
    return status;
}




/**
 * @brief Will setup x4 in default configuration.
 * Assumes Enable has been set.
 * Assumes X4 firmware has been programmed.
 * @return Status of execution as defined in x4driver.h.
 */
int x4driver_setup_default(X4Driver_t* x4driver)
{
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//Set receiver trimming values
	//dac_trim_a / dac_trim_b
	status = x4driver_set_xif_register(x4driver, ADDR_XIF_DAC_TRIM_RW, 63);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//preamp_trim
	status = x4driver_set_xif_register(x4driver, ADDR_XIF_PREAMP_TRIM_RW, 15);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//Enable Common PLL 243MHz
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_COMMON_PLL_CTRL_1_RW, 96);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//Enable TX PLL
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_TX_PLL_CTRL_2_RW, 3);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//Enable RX PLL
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_RX_PLL_CTRL_2_RW, 3);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//Enable receiver back end clocks
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_CLKOUT_SEL_RW, 16);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_MCLK_TRX_BACKEND_CLK_CTRL_RW, 8);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//Enable sampler
	status = x4driver_set_xif_register(x4driver, ADDR_XIF_SAMPLER_PRESET_MSB_RW, 0);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_SMPL_MODE_RW, 0);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//Configuration of transmitter Transmitter
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_DVDD_TRIM_RW, 80);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//Set TX center frequency to EU
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_TX_PLL_CTRL_1_RW, 48);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//set_dacmax
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_TRX_DAC_MAX_L_RW, 7);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_TRX_DAC_MAX_H_RW, 255);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//set_dacmin
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_TRX_DAC_MIN_L_RW, 0);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_TRX_DAC_MIN_H_RW, 0);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//set_pps
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_TRX_PULSES_PER_STEP_LSB_RW, 10);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_TRX_PULSES_PER_STEP_MSB_RW, 0);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//setting Iterations
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_TRX_ITERATIONS_RW, 8);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//Set up PRF
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_TRX_CLOCKS_PER_PULSE_RW, 16);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//rx_mframes_coarse
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_RX_MFRAMES_COARSE_RW, 16);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//rx_wait
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_RX_WAIT_RW, 0);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//tx_wait
	status = x4driver_set_pif_register(x4driver, ADDR_PIF_TX_WAIT_RW, 1);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	//set_frame_length
	uint8_t rx_mframes_coarse = 0x00;
	status = x4driver_get_pif_register(x4driver,ADDR_PIF_RX_MFRAMES_COARSE_RW,&rx_mframes_coarse);	
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	status = x4driver_set_frame_length(x4driver,rx_mframes_coarse);
	
	
	mutex_give(x4driver);
	return status;
}




/**
 * @brief Upload default 8051 firmware to X4.
 * Assumes Enable has been set.
 * @return Status of execution as defined in x4driver.h.
 */
int x4driver_upload_firmware_default(X4Driver_t* x4driver)
{
    uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) 
		return status;
    
    status = x4driver_upload_firmware_custom(x4driver,data_8051_onboard,data_8051_size);
    if (status != XEP_ERROR_X4DRIVER_OK) 
		return status;    
    status = x4driver_set_spi_register(x4driver,ADDR_SPI_BOOT_FROM_OTP_SPI_RWE,BOOT_FROM_SRAM);
    if (status != XEP_ERROR_X4DRIVER_OK) 
		return status;
    status = x4driver_verify_firmware(x4driver,data_8051_onboard,data_8051_size);     
    if (status != XEP_ERROR_X4DRIVER_OK) 
		return status;
    mutex_give(x4driver);
    return status;
}


/**
 * @brief Read back embedded 8051 firmware.
 * Assumes Enable has been set.
 * @return Status of execution as defined in x4driver.h.
 */
int x4driver_verify_firmware(X4Driver_t* x4driver, uint8_t * buffer, uint32_t size)
{
    uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
    
    x4driver_set_spi_register(x4driver,ADDR_SPI_MEM_FIRST_ADDR_LSB_RW,START_OF_SRAM_LSB);//start of SRAM LSB
    x4driver_set_spi_register(x4driver,ADDR_SPI_MEM_FIRST_ADDR_MSB_RW,START_OF_SRAM_MSB);//start of SRAM MSB
	x4driver_set_spi_register(x4driver,ADDR_SPI_MEM_MODE_RW,SET_NORMAL_MODE);// take out of programming mode.
	
	//clear SRAM fifo.    
	uint32_t errors = 0x00;
	uint8_t fifo_status = 0x00;
	x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MEM_FIFO_STATUS_R,&fifo_status);
	uint8_t read_back = 0x00;
    uint32_t max_retries = 32;// FIFO depth is 8
    uint32_t retries = 0;
	while((fifo_status & FIFO_NOT_EMPTY_FLAG) == FIFO_NOT_EMPTY_FLAG)
	{
		status =  x4driver_get_spi_register(x4driver,ADDR_SPI_FROM_MEM_READ_DATA_RE,&read_back);
		status =  x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MEM_FIFO_STATUS_R,&fifo_status);
        retries++;
        if(retries > max_retries)
        {            
            mutex_give(x4driver);
            return XEP_ERROR_X4DRIVER_SRAM_FIFO_TIMEOUT_FAIL;            
        }            
	}
    x4driver_set_spi_register(x4driver,ADDR_SPI_MEM_MODE_RW,SET_READBACK_MODE);//set into read back mode    
	for(uint32_t i = 0; i< size; i++)
	{
		status =  x4driver_get_spi_register(x4driver,ADDR_SPI_FROM_MEM_READ_DATA_RE,&read_back);
		if(buffer[i] != read_back)
		{
			// Store in dummy variables for debug purposes when optimization is enabled.
			//
			#pragma GCC diagnostic ignored "-Wunused-variable"						
			#pragma GCC diagnostic push
			volatile uint8_t buffer_value = buffer[i];
			volatile uint8_t read_back_value = read_back;
			#pragma GCC diagnostic pop
			errors++;
		}
	}
	x4driver_set_spi_register(x4driver,ADDR_SPI_MEM_MODE_RW,SET_NORMAL_MODE);//set into read back mode                         
    mutex_give(x4driver);
    if(errors >0)
        return XEP_ERROR_X4DRIVER_8051_VERIFY_FAIL;
    return status;
}





/**
 * @brief get frame area
 * Start and end in meter. 
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_get_frame_area(X4Driver_t* x4driver, float32_t *start, float32_t *end)
{
	*start = x4driver->frame_area_start;
	*end = x4driver->frame_area_end;
	return XEP_ERROR_X4DRIVER_OK;
}


/**
 * @brief Set frame area start and end in meters. Takes offset given from x4driver_set_frame_area_offset and setup the frame area relative to this. Uses a combination of rx_wait and bin offset to realize the offset.
 * Will use rx_mframes and rx_mframes coarse to specify the number of needed active bins. Will after this select only the needed ram lines from x4 to reduce transmission size.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_set_frame_area(X4Driver_t* x4driver, float32_t start, float32_t end)
{
	float32_t range_offset_in_meters = x4driver->frame_area_offset_meters;
	
	
	float32_t X4DRIVER_MAX_BINS_RANGE_METERS =X4DRIVER_MAX_BINS_RANGE_METERS_RF;
	if(x4driver->downconvertion_enabled == 1)
	{
		X4DRIVER_MAX_BINS_RANGE_METERS = X4DRIVER_MAX_BINS_RANGE_METERS_DOWN_CONVERSION;
	}
	
	if(start < X4DRIVER_MIN_RANGE)
	{
		return XEP_ERROR_X4DRIVER_ERROR_FRAME_AREA_END_OUT_OF_SCOPE; 
	}
	if(start > end )
	{
		return XEP_ERROR_X4DRIVER_ERROR_FRAME_AREA_END_OUT_OF_SCOPE;
	}
	
	
	
	float32_t length = end-start;
	if(end > (X4DRIVER_MAX_BINS_RANGE_METERS+ X4DRIVER_MAX_RANGE_OFFSET + range_offset_in_meters))
	{
		return  XEP_ERROR_X4DRIVER_ERROR_FRAME_AREA_END_OUT_OF_SCOPE;
	} 	
		
	if((x4driver->downconvertion_enabled) == 1 && (length > X4DRIVER_MAX_BINS_RANGE_METERS))
	{
		end = start+X4DRIVER_MAX_BINS_RANGE_METERS;
		length = end-start;
	}
	
	if(length > X4DRIVER_MAX_BINS_RANGE_METERS)
	{
		return XEP_ERROR_X4DRIVER_ERROR_FRAME_AREA_TOO_LARGE;
	}
	
	uint32_t status = mutex_take(x4driver);	
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	x4driver->frame_area_start_requested = start;
	x4driver->frame_area_end_requested = end;
	
	
	x4driver->rx_wait_offset_m = 0;
	float32_t range = end-start;
	float32_t absolute_start = range_offset_in_meters + start;
	
	uint32_t rx_wait_max = floorf(absolute_start/X4DRIVER_RX_WAIT_OFFSET_IN_METERS);
	float32_t rx_wait_offset_m = rx_wait_max * X4DRIVER_RX_WAIT_OFFSET_IN_METERS;
	x4driver_set_rx_wait(x4driver,rx_wait_max);
	float32_t relative_offset =  rx_wait_offset_m - range_offset_in_meters;
	float32_t relative_start_m = absolute_start - rx_wait_offset_m;
	float32_t relative_end_m = relative_start_m + range;
	float32_t relative_range = relative_end_m - relative_start_m;
	if(relative_range >X4DRIVER_MAX_BINS_RANGE_METERS)
	{
		relative_end_m = X4DRIVER_MAX_BINS_RANGE_METERS;
	}
	
	
		
	
	// This can happen when we where not able to move the offset e.g. offset_required_m > X4DRIVER_RX_WAIT_OFFSET_IN_METERS  = rx_wait = 0
	if(relative_end_m > X4DRIVER_MAX_BINS_RANGE_METERS)//shave bins at end
	{
		relative_end_m = X4DRIVER_MAX_BINS_RANGE_METERS;
	}
	
	float32_t relative_end_m_bins_active = relative_end_m;
	float32_t bins_required_active = 4*8;
	if(x4driver->downconvertion_enabled == 0)	
		bins_required_active = 0;
	bins_required_active += (ceilf(relative_end_m_bins_active/X4DRIVER_METERS_PER_BIN));
	uint8_t min_rx_mframes_coarse = (ceilf(bins_required_active/X4DRIVER_BINS_PER_RX_MFRAMES_COARSE));
	x4driver_set_pif_register(x4driver,ADDR_PIF_RX_MFRAMES_COARSE_RW,min_rx_mframes_coarse);
	uint8_t min_rx_mframes = (ceilf(bins_required_active/X4DRIVER_BINS_PER_RX_MFRAMES));
	x4driver_set_pif_register(x4driver,ADDR_PIF_RX_MFRAMES_RW,min_rx_mframes);
	x4driver->required_bins_active = bins_required_active;
	x4driver->rx_mframes = min_rx_mframes;
	x4driver->rx_mframes_coarse = min_rx_mframes_coarse;

		
	
	

	
	x4driver->frame_area_start = 0;
	x4driver->frame_area_start_bin_requested = 0;
	x4driver->frame_area_start_ram_line = 0;
	x4driver->frame_area_start_ram_line_bin = 0;
	x4driver->frame_area_start_bin_offset = 0;
	x4driver->frame_area_start = relative_offset;	
	if(x4driver->downconvertion_enabled==1)
	{
		//skip 4 fist bins
		uint32_t offset = 4;
		x4driver->frame_area_start_ram_line = offset;
		x4driver->frame_area_start_bin_requested = offset;		
		x4driver->frame_area_start_ram_line_bin = offset;
		if (relative_start_m > 0)
		{
			
			
			int32_t start_bin  = (int32_t)(floorf(relative_start_m/(X4DRIVER_METERS_PER_BIN_DOWN_CONVERTION))-1);//-1 to account for first bin, 8 factor decimation
			float32_t start_bin_m = (1+start_bin) * X4DRIVER_METERS_PER_BIN_DOWN_CONVERTION;
			x4driver->frame_area_start = start_bin_m + relative_offset;
			if(start_bin < 0)
				start_bin = 0;
			x4driver->frame_area_start_bin_requested = start_bin+offset;			
			x4driver->frame_area_start_ram_line = start_bin + offset;
			x4driver->frame_area_start_ram_line_bin = start_bin + offset;			
			x4driver->frame_area_start_bin_offset = 0;			
			
		}

		
		
				
		uint32_t end_bin = (uint32_t)(ceilf(relative_end_m/(X4DRIVER_METERS_PER_BIN_DOWN_CONVERTION)) + offset-1);
		if(end_bin > 191)//rounding error.
		{
			end_bin = 191;
		}
		x4driver->frame_area_end_bin_requested = end_bin;
		x4driver->frame_area_end_ram_line = end_bin ;
		x4driver->frame_area_end_ram_line_bin = end_bin;
		x4driver->frame_area_end_bin_offset =  0;
				
		x4driver->frame_area_end_bin_requested = end_bin;
		float32_t end_bin_m = X4DRIVER_METERS_PER_BIN_DOWN_CONVERTION*(x4driver->frame_area_end_ram_line- offset +1);
		x4driver->frame_area_end = end_bin_m + relative_offset;
		
	}
	else
	{
			
			if (relative_start_m > 0)
			{
				int32_t start_bin = floorf(relative_start_m/X4DRIVER_METERS_PER_BIN)-1;
				float32_t start_bin_m = (start_bin+1)*X4DRIVER_METERS_PER_BIN;//start bin is relative start bin in this case set_frame_area_offset - rx_wait_m /binlen.
				x4driver->frame_area_start = start_bin_m+relative_offset;//relative offset is rx_wait offset - set_frame_area_offset// e.g. what we managed to set by rx_wait
				if(start_bin < 0)
					start_bin = 0;
				x4driver->frame_area_start_bin_requested = start_bin;
				x4driver->frame_area_start_ram_line = start_bin/4;
				x4driver->frame_area_start_ram_line_bin = x4driver->frame_area_start_ram_line * 4;								
				
				x4driver->frame_area_start_bin_offset = x4driver->frame_area_start_bin_requested - x4driver->frame_area_start_ram_line_bin;								
				
			}
			
			
			x4driver->frame_area_end_bin_requested = ceilf(relative_end_m/X4DRIVER_METERS_PER_BIN) - 1 ;// account for bin 0 
			float32_t end_bin_m = X4DRIVER_METERS_PER_BIN*(x4driver->frame_area_end_bin_requested +1);// add one to count for bin 0 
			x4driver->frame_area_end = end_bin_m + relative_offset;
			x4driver->frame_area_end_ram_line = ceilf(((float32_t) (x4driver->frame_area_end_bin_requested+1))/4); // add one for bin 0
			
			x4driver->frame_area_end_ram_line_bin = (x4driver->frame_area_end_ram_line*4) - 1; //account for bin 0 
			
			
			x4driver->frame_area_end_bin_offset =  x4driver->frame_area_end_ram_line_bin - x4driver->frame_area_end_bin_requested;
			x4driver->frame_area_end_ram_line = x4driver->frame_area_end_ram_line-1;			
	}
	
	
	
	_x4driver_set_rx_ram_first_line(x4driver,x4driver->frame_area_start_ram_line);
	_x4driver_set_rx_ram_last_line(x4driver,x4driver->frame_area_end_ram_line );
	if(x4driver->downconvertion_enabled)
	{
		uint32_t bytes_to_read = (x4driver->frame_area_end_ram_line - x4driver->frame_area_start_ram_line+1)*2*x4driver->bytes_per_counter ;
		x4driver->frame_read_size = bytes_to_read;
	}
	else
	{
		uint32_t bytes_to_read = (x4driver->frame_area_end_ram_line - x4driver->frame_area_start_ram_line+1)*4* x4driver->bytes_per_counter ;
		x4driver->frame_read_size = bytes_to_read;
	}
	
		
	mutex_give(x4driver);
	return XEP_ERROR_X4DRIVER_OK;
}


/**
 * @brief Sets frame offset in meters.
 * @return Status of execution as defined in x4driver.h.
 */
int x4driver_set_frame_area_offset(X4Driver_t* x4driver, float32_t offset_in_meters)
{	
	x4driver->frame_area_offset_meters =offset_in_meters;
	int status = x4driver_set_frame_area(x4driver,x4driver->frame_area_start_requested, x4driver->frame_area_end_requested);	
	return status;
}

/**
 * @brief Gets calculated frame offset in meters done by x4driver_set_frame_area_offset. 
 * @return Status of execution as defined in x4driver.h.
 */
int x4driver_get_frame_area_offset(X4Driver_t* x4driver, float32_t * offset_in_meters)
{		
	*offset_in_meters  = x4driver->frame_area_offset_meters;	
	return XEP_ERROR_X4DRIVER_OK;
}


/**
 * @brief Reads frame and normalizes that frame.
 * @return Status of execution as defined in x4driver.h.
 */
int x4driver_read_frame_normalized(X4Driver_t* x4driver, uint32_t* frame_counter, float32_t* data, uint32_t length)
{   
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;	       		
	//read_raw_data
    status = x4driver_read_frame_bytes(x4driver, frame_counter,x4driver->frame_buffer, x4driver->frame_read_size); 
		
	
	if(x4driver->downconvertion_enabled == 0)
	{
		_x4driver_unpack_and_normalize_frame(x4driver,data,length,x4driver->frame_buffer,x4driver->frame_read_size);	
	}
	else
	{
		_x4driver_unpack_and_normalize_downconverted_frame(x4driver,data,length,x4driver->frame_buffer,x4driver->frame_read_size);	
	}
							
    
	mutex_give(x4driver);
	return status;
}



/**
 * @brief Reads raw bytes from x4.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_read_frame_bytes(X4Driver_t* x4driver, uint32_t* frame_counter, uint8_t* data, uint32_t length)
{
    
	uint32_t frame_cnt = 0;
	_x4driver_get_framecounter(x4driver,&frame_cnt);
	x4driver->frame_counter = frame_cnt;
	*frame_counter = x4driver->frame_counter;
	
	if(length < x4driver->frame_read_size)
		return XEP_ERROR_X4DRIVER_BUFFER_TO_SMALL;
		
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	uint8_t ram_select = 0x00;
	status = x4driver_get_pif_register(x4driver,ADDR_PIF_RAM_SELECT_RW,&ram_select);
	if (status != XEP_ERROR_X4DRIVER_OK)
	{
		mutex_give(x4driver);
		return status;
	}
	
	if(ram_select == 0x00)	
		status = x4driver_set_pif_register(x4driver,ADDR_PIF_RAM_SELECT_RW,0x01);			
	else	
		status = x4driver_set_pif_register(x4driver,ADDR_PIF_RAM_SELECT_RW,0x00);			
	if (status != XEP_ERROR_X4DRIVER_OK)
	{
		mutex_give(x4driver);
		return status;
	}
	
	status = x4driver_set_pif_register(x4driver,ADDR_PIF_FETCH_RADAR_DATA_SPI_W,FETCH_DATA_ACTION);
	if (status != XEP_ERROR_X4DRIVER_OK) 
	{
		mutex_give(x4driver);
		return status;		
	}
	uint8_t radar_data_addr = ADDR_SPI_RADAR_DATA_SPI_RE;
		
	status = x4driver->callbacks.spi_write_read(x4driver->user_reference, &radar_data_addr, 1,data, x4driver->frame_read_size);		
	
	mutex_give(x4driver);
	return status;
	
}




/**
 * @brief Set SPI register on radar chip.
 *
 * @return Status of execution 
 */
int x4driver_set_spi_register(X4Driver_t* x4driver,uint8_t address, uint8_t value)
{
    uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	address |= 0x80;
	uint8_t register_write_buffer[2] = {address,value};	   
    status = x4driver->callbacks.spi_write(x4driver->user_reference, register_write_buffer, 2);
    mutex_give(x4driver);
	return status;	
}


/**
 * @brief Get SPI register on radar chip.
 *
 * @return Status of execution 
 */
int x4driver_get_spi_register(X4Driver_t* x4driver,uint8_t address, uint8_t * value)
{
    uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
    
	uint8_t register_write_buffer = address;
	uint8_t register_read_buffer = 0x00;    
	status = x4driver->callbacks.spi_write_read(x4driver->user_reference, &register_write_buffer, 1,&register_read_buffer,1);
	mutex_give(x4driver);
    *value = register_read_buffer;
	return status;
}


/**
 * @brief Set enable for X4
 *
 * @return Status of execution 
 */
int x4driver_set_enable(X4Driver_t* x4driver, uint8_t value)
{
    uint32_t status = mutex_take(x4driver);
    if (status != XEP_ERROR_X4DRIVER_OK) return status;
	status = x4driver->callbacks.pin_set_enable(x4driver->user_reference,value);
    mutex_give(x4driver);
	return status;
}



/**
 * @brief Gets current frame length.
 * @return Status of execution as defined in x4driver.h.
 */
int x4driver_get_frame_length(X4Driver_t* x4driver, uint32_t * cycles)
{		
	*cycles = x4driver->frame_length;
	return XEP_ERROR_X4DRIVER_OK;
}



/**
 * @brief utility is bit set function.
 * @return 1 if bit is set.
 */
static uint32_t bit_is_set(uint32_t data,uint32_t bitnr)
{	
	uint32_t res = (1<< bitnr) & data;	
	uint32_t ret = res != 0;	
	return ret;
}



/**
 * @brief Sets internal register value.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_set_internal_register(X4Driver_t* x4driver, uint8_t address, uint8_t write_value, uint8_t command)
{
    uint32_t status = mutex_take(x4driver);
    if (status != XEP_ERROR_X4DRIVER_OK) return status;
	address |= PIF_ADDRESS_WRITE;
	
    
	uint8_t fifo_status = 0x00;
	uint8_t retries = 0;
	uint8_t retries_max = PIF_COMMAND_MAX_RETRIES;    
	x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MB_FIFO_STATUS_R,&fifo_status);
    //wait for cpu to process data
	while(bit_is_set(fifo_status,TO_CPU_EMPTY_BIT) == BIT_NOT_SET)
	{
		x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MB_FIFO_STATUS_R,&fifo_status);	
		retries++;				
		if(retries > retries_max)
		{
			mutex_give(x4driver);
			return XEP_ERROR_X4DRIVER_PIF_TIMEOUT;
		}
	}
    //send data
	x4driver_set_spi_register(x4driver,ADDR_SPI_TO_CPU_WRITE_DATA_WE,address);
	x4driver_set_spi_register(x4driver,ADDR_SPI_TO_CPU_WRITE_DATA_WE,command);
	x4driver_set_spi_register(x4driver,ADDR_SPI_TO_CPU_WRITE_DATA_WE,write_value);
        
	x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MB_FIFO_STATUS_R,&fifo_status);    
    //wait for CPU to process data
	while(bit_is_set(fifo_status,TO_CPU_EMPTY_BIT) == BIT_NOT_SET)
	{		
		x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MB_FIFO_STATUS_R,&fifo_status);//wait for data		
		retries++;
		if(retries > retries_max)
		{
			mutex_give(x4driver);
			return XEP_ERROR_X4DRIVER_PIF_TIMEOUT;			
		}
	}		
	
	mutex_give(x4driver);
	return status;
}


/**
 * @brief Sets x4 software action.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_set_x4_sw_action(X4Driver_t* x4driver, uint8_t address)
{	
	return _x4driver_set_internal_register(x4driver, address, 0xff, X4_SW_ACTION_COMMAND);
}


/**
 * @brief Gets x4 software register value.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_get_x4_sw_register(X4Driver_t* x4driver, uint8_t address,uint8_t * value)
{	
	return _x4driver_get_x4_internal_register(x4driver,address,value,X4_SW_REGISTER_COMMAND);
}


/**
 * @brief Sets x4 software register value.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_set_x4_sw_register(X4Driver_t* x4driver, uint8_t address, uint8_t value)
{	
	return _x4driver_set_internal_register(x4driver, address, value, X4_SW_REGISTER_COMMAND);
}

/**
 * @brief Sets PIF register value.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_set_pif_register(X4Driver_t* x4driver, uint8_t address, uint8_t value)
{	
	return _x4driver_set_internal_register(x4driver, address, value, PIF_COMMAND);
}


/**
 * @brief Sets XIF register value.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_set_xif_register(X4Driver_t* x4driver, uint8_t address, uint8_t value)
{	
	return _x4driver_set_internal_register(x4driver, address, value, XIF_COMMAND);
}



/**
 * @brief Gets reads frame counter from 8051 mailbox
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_get_framecounter(X4Driver_t* x4driver, uint32_t * value)
{	

    uint32_t status = mutex_take(x4driver);
    if (status != XEP_ERROR_X4DRIVER_OK) return status;
    
	uint8_t tmp_rb[4]; 
	uint8_t fifo_status = 0x00;	
	uint8_t data_in_fifo = 0;
	uint8_t index =0 ;
	x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MB_FIFO_STATUS_R,&fifo_status);    
    //clear FIFO
	while(bit_is_set(fifo_status,FROM_CPU_VALID_BIT) == BIT_SET)
	{						
		x4driver_get_spi_register(x4driver,ADDR_SPI_FROM_CPU_READ_DATA_RE,tmp_rb);
		x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MB_FIFO_STATUS_R,&fifo_status);	
		data_in_fifo++;		
	}
	
	_x4driver_set_x4_sw_action(x4driver,9);
	x4driver_get_spi_register(x4driver,ADDR_SPI_FROM_CPU_READ_DATA_RE,&tmp_rb[index]);
	index++;
	x4driver_get_spi_register(x4driver,ADDR_SPI_FROM_CPU_READ_DATA_RE,&tmp_rb[index]);
	index++;
	x4driver_get_spi_register(x4driver,ADDR_SPI_FROM_CPU_READ_DATA_RE,&tmp_rb[index]);
	index++;
	x4driver_get_spi_register(x4driver,ADDR_SPI_FROM_CPU_READ_DATA_RE,&tmp_rb[index]);	
	//
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wcast-align"
	*value =  *((uint32_t*)tmp_rb);	
	#pragma GCC diagnostic pop
    mutex_give(x4driver);
	return status;
}



/**
 * @brief Gets internal register value.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_get_x4_internal_register(X4Driver_t* x4driver, uint8_t address,uint8_t * value, uint8_t command)
{	

    uint32_t status = mutex_take(x4driver);
    if (status != XEP_ERROR_X4DRIVER_OK) return status;
    
	uint8_t tmp_rb = 0x00; 
	uint8_t fifo_status = 0x00;
	uint8_t retries = 0;
	uint8_t retries_max = PIF_COMMAND_MAX_RETRIES;
	uint8_t data_in_fifo = 0;
	x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MB_FIFO_STATUS_R,&fifo_status);
    
    //clear FIFO
	while(bit_is_set(fifo_status,FROM_CPU_VALID_BIT) == BIT_SET)
	{						
		x4driver_get_spi_register(x4driver,ADDR_SPI_FROM_CPU_READ_DATA_RE,&tmp_rb);
		x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MB_FIFO_STATUS_R,&fifo_status);	
		data_in_fifo++;		
	}
    
	x4driver_set_spi_register(x4driver,ADDR_SPI_TO_CPU_WRITE_DATA_WE,address);
	x4driver_set_spi_register(x4driver,ADDR_SPI_TO_CPU_WRITE_DATA_WE,command);
	x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MB_FIFO_STATUS_R,&fifo_status);
	retries = 0;		
	
    //wait for response
    while(bit_is_set(fifo_status,FROM_CPU_VALID_BIT) == BIT_NOT_SET && bit_is_set(fifo_status,TO_CPU_EMPTY_BIT) == BIT_NOT_SET)
	{
		x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MB_FIFO_STATUS_R,&fifo_status);	
		retries++; 
		if(retries > retries_max)
		{
			mutex_give(x4driver);
			return XEP_ERROR_X4DRIVER_PIF_TIMEOUT;
		}
    }	
    
    //read response
    x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MB_FIFO_STATUS_R,&fifo_status);
	while(bit_is_set(fifo_status,FROM_CPU_VALID_BIT) == BIT_SET)
	{								
		x4driver_get_spi_register(x4driver,ADDR_SPI_FROM_CPU_READ_DATA_RE,&tmp_rb);
		x4driver_get_spi_register(x4driver,ADDR_SPI_SPI_MB_FIFO_STATUS_R,&fifo_status);
        if(retries > retries_max)
		{
			mutex_give(x4driver);
			return XEP_ERROR_X4DRIVER_PIF_TIMEOUT;	
		}
		data_in_fifo++;		
	}	
	*value = tmp_rb;
	
	mutex_give(x4driver);
	return status;
}



/**
 * @brief Gets PIF register value.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_get_pif_register(X4Driver_t* x4driver, uint8_t address,uint8_t * value)
{	
	return _x4driver_get_x4_internal_register(x4driver,address,value,PIF_COMMAND);
}


/**
 * @brief Gets XIF register value.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_get_xif_register(X4Driver_t* x4driver, uint8_t address,uint8_t * value)
{	
	return _x4driver_get_x4_internal_register(x4driver,address,value,XIF_COMMAND);
}



/**
 * @brief Setup external osc on X4.
 * requires enable to be set and 8051 SRAM to be program.
 * @return XEP_ERROR_X4DRIVER_OSC_LOCK_FAIL if osc lock fails
 */
int x4driver_init_clock(X4Driver_t* x4driver)
{
    uint32_t status = mutex_take(x4driver);
    if (status != XEP_ERROR_X4DRIVER_OK) return status;
    uint8_t OSC_CTRL_REGISTER = 0x00;    
    uint32_t ossc_lock_attemps = OSC_LOCK_ATTEMPS_MAX;
        
    x4driver_get_pif_register(x4driver,ADDR_PIF_OSC_CTRL_RW,&OSC_CTRL_REGISTER);
    OSC_CTRL_REGISTER = 64;// disable level.
    x4driver_set_pif_register(x4driver,ADDR_PIF_OSC_CTRL_RW,OSC_CTRL_REGISTER);
    x4driver_get_pif_register(x4driver,ADDR_PIF_OSC_CTRL_RW,&OSC_CTRL_REGISTER);
    OSC_CTRL_REGISTER = 66; //Enable external oscillator.
    x4driver_set_pif_register(x4driver,ADDR_PIF_OSC_CTRL_RW,OSC_CTRL_REGISTER);
    uint8_t common_pll_status_register_value = 0x00;  
    uint8_t osc_lock_bit = 1 << 6;
    uint32_t has_lock = 0;
    for(uint32_t i = 0; i < ossc_lock_attemps; i++)
    {
        x4driver_get_pif_register(x4driver,ADDR_PIF_COMMON_PLL_STATUS_R,&common_pll_status_register_value);        
        if((common_pll_status_register_value & osc_lock_bit) != 0x00)
        {
            //lock
            has_lock = 1;
            break;
        }
        x4driver->callbacks.wait_us(1*MS);
    }        
	
	
	x4driver_set_pif_register(x4driver,ADDR_PIF_OSC_CTRL_RW,OSC_CTRL_REGISTER | (0x01 <<5) );
    mutex_give(x4driver);
    if(has_lock == 0)
    {
        status = XEP_ERROR_X4DRIVER_OSC_LOCK_FAIL;
    }
	return status;
}



/**
 * @brief Enables dvdd_tx,dvdd_rx, avdd_tx dvdd_rx ldos on X4 .
 * requires enable to be set and 8051 SRAM to be program.
 * @return XEP_ERROR_X4DRIVER_LDO_ENABLE_FAIL in case of error.
 */
int x4driver_ldo_enable_all(X4Driver_t* x4driver)
{
    uint32_t status = 0;    
	x4driver_ldo_enable(x4driver,X4DRIVER_LDO_ALL);
	return status;
}


/**
 * @brief Enables enables ldos based on parameter.
 * requires enable to be set and 8051 SRAM to be program.
 * @return XEP_ERROR_X4DRIVER_LDO_ENABLE_FAIL in case of error.
 */
int x4driver_ldo_enable(X4Driver_t* x4driver,uint32_t ldo)
{
    uint32_t status = 0;
    if((X4DRIVER_LDO_DVDD_TX & ldo) != 0)
	{
		status = _x4driver_enable_dvdd_tx(x4driver);
		if (status != XEP_ERROR_X4DRIVER_OK) return status;
	}
	if((X4DRIVER_LDO_DVDD_RX & ldo) != 0)
	{
		status = _x4driver_enable_dvdd_rx(x4driver);
		if (status != XEP_ERROR_X4DRIVER_OK) return status;
	}
	
	if((X4DRIVER_LDO_AVDD_RX & ldo) != 0)
	{
		 status = _x4driver_enable_avdd_rx(x4driver);
		if (status != XEP_ERROR_X4DRIVER_OK) return status;
	}
	if((X4DRIVER_LDO_AVDD_TX & ldo) != 0)
	{
		status = _x4driver_enable_avdd_tx(x4driver);
		if (status != XEP_ERROR_X4DRIVER_OK) return status; 
	}
	return status;
}
 



/**
 * @brief Enable dvdd ldo on X4
 * NOTE: Implements workaround to kick start LDO.
 * requires enable to be set and 8051 SRAM to be program.
 * @return XEP_ERROR_X4DRIVER_LDO_ENABLE_FAIL in case of error.
 */
int _x4driver_enable_dvdd_tx(X4Driver_t* x4driver)
{
    uint32_t status = mutex_take(x4driver);
    if (status != XEP_ERROR_X4DRIVER_OK) return status;
    //set dvdd_testmode 5    
    x4driver_set_pif_register(x4driver,ADDR_PIF_APC_DVDD_TESTMODE_RW,5);
    uint8_t dvdd_tx_ctrl_value = 0x00;
    x4driver_get_pif_register(x4driver, ADDR_PIF_DVDD_TX_CTRL_RW,&dvdd_tx_ctrl_value);
    dvdd_tx_ctrl_value  &= ~(1 << 6);//unset disable bit
    x4driver_set_pif_register(x4driver,ADDR_PIF_DVDD_TX_CTRL_RW,dvdd_tx_ctrl_value);
           
    //check power good    
    uint32_t power_good = 0;
    for(unsigned char i = 0; i <16;i++)
    {
        uint8_t ldo_status_2_value = 0x00;
        x4driver_get_pif_register(x4driver, ADDR_PIF_LDO_STATUS_2_R,&ldo_status_2_value);
        if((ldo_status_2_value & (1 <<DVDD_TX_POWER_GOOD_BIT)) != 0x00)
        {
            power_good = 1;
            break;
        }
        //power not good
        x4driver_get_pif_register(x4driver, ADDR_PIF_DVDD_TX_CTRL_RW,&dvdd_tx_ctrl_value);
        uint8_t trim_value = dvdd_tx_ctrl_value & 0x1f;
        //trim value 0        
        x4driver_set_pif_register(x4driver,ADDR_PIF_DVDD_TX_CTRL_RW,0);                  
        
        x4driver_set_pif_register(x4driver,ADDR_PIF_DVDD_TESTMODE_RW,13);
        x4driver->callbacks.wait_us(i*MS);
        //disable test_mode                       
        x4driver_set_pif_register(x4driver,ADDR_PIF_DVDD_TESTMODE_RW,0);
        //write back trim value
        x4driver_set_pif_register(x4driver,ADDR_PIF_DVDD_TX_CTRL_RW,trim_value);        
        x4driver->callbacks.wait_us(i*MS);
    }
    
    //disable test mode.
    x4driver_set_pif_register(x4driver,ADDR_PIF_APC_DVDD_TESTMODE_RW,0);    
    if(power_good == 0)
    {
        x4driver_get_pif_register(x4driver, ADDR_PIF_DVDD_TX_CTRL_RW,&dvdd_tx_ctrl_value);
        dvdd_tx_ctrl_value |= (1 << 6);//set disable bit
        x4driver_set_pif_register(x4driver,ADDR_PIF_DVDD_TX_CTRL_RW,dvdd_tx_ctrl_value);
        status = XEP_ERROR_X4DRIVER_LDO_ENABLE_FAIL;
    }
    mutex_give(x4driver);
	return status;
}



/**
 * @brief Enable dvdd ldo on X4
 * NOTE: Implements workaround to kick start LDO.
 * requires enable to be set and 8051 SRAM to be program.
 * @return XEP_ERROR_X4DRIVER_LDO_ENABLE_FAIL in case of error.
 */
int _x4driver_enable_dvdd_rx(X4Driver_t* x4driver)
{
    uint32_t status = mutex_take(x4driver);
    if (status != XEP_ERROR_X4DRIVER_OK) return status;
    //set dvdd_testmode 5    
    x4driver_set_pif_register(x4driver,ADDR_PIF_APC_DVDD_TESTMODE_RW,5);
    uint8_t dvdd_rx_ctrl_value = 0x00;
    x4driver_get_pif_register(x4driver, ADDR_PIF_DVDD_RX_CTRL_RW,&dvdd_rx_ctrl_value);
    dvdd_rx_ctrl_value  &= ~(1 << 6);//unset disable bit
    x4driver_set_pif_register(x4driver,ADDR_PIF_DVDD_RX_CTRL_RW,dvdd_rx_ctrl_value);
           
    //check power good    
    uint32_t power_good = 0;
    for(unsigned char i = 0; i <16;i++)
    {
        uint8_t ldo_status_2_value = 0x00;
        x4driver_get_pif_register(x4driver, ADDR_PIF_LDO_STATUS_2_R,&ldo_status_2_value);
        if((ldo_status_2_value & (1 <<DVDD_RX_POWER_GOOD_BIT)) != 0x00)
        {
            power_good = 1;
            break;
        }
        //power not good
        x4driver_get_pif_register(x4driver, ADDR_PIF_DVDD_RX_CTRL_RW,&dvdd_rx_ctrl_value);
        uint8_t trim_value = dvdd_rx_ctrl_value & 0x1f;
        //trim value 0        
        x4driver_set_pif_register(x4driver,ADDR_PIF_DVDD_RX_CTRL_RW,0);                  
        
        x4driver_set_pif_register(x4driver,ADDR_PIF_DVDD_TESTMODE_RW,208);
        x4driver->callbacks.wait_us(i*MS);
        //disable test_mode                       
        x4driver_set_pif_register(x4driver,ADDR_PIF_DVDD_TESTMODE_RW,0);
        //write back trim value
        x4driver_set_pif_register(x4driver,ADDR_PIF_DVDD_RX_CTRL_RW,trim_value);        
        x4driver->callbacks.wait_us(i*MS);
    }
    
    //disable test mode.
    x4driver_set_pif_register(x4driver,ADDR_PIF_APC_DVDD_TESTMODE_RW,0);    
    if(power_good == 0)
    {
        x4driver_get_pif_register(x4driver, ADDR_PIF_DVDD_RX_CTRL_RW,&dvdd_rx_ctrl_value);
        dvdd_rx_ctrl_value |= (1 << 6);//set disable bit
        x4driver_set_pif_register(x4driver,ADDR_PIF_DVDD_RX_CTRL_RW,dvdd_rx_ctrl_value);
        status = XEP_ERROR_X4DRIVER_LDO_ENABLE_FAIL;
    }
    mutex_give(x4driver);
	return status;
}


    


/**
 * @brief Enable avdd ldo on X4
 * NOTE: Implements workaround to kick start LDO.
 * requires enable to be set and 8051 SRAM to be program.
 * @return XEP_ERROR_X4DRIVER_LDO_ENABLE_FAIL in case of error.
 */
int _x4driver_enable_avdd_tx(X4Driver_t* x4driver)
{
    uint32_t status = mutex_take(x4driver);
    if (status != XEP_ERROR_X4DRIVER_OK) return status;
    //set dvdd_testmode 5    
    x4driver_set_pif_register(x4driver,ADDR_PIF_APC_DVDD_TESTMODE_RW,5);
    uint8_t avdd_tx_ctrl_value = 0x00;
    x4driver_get_pif_register(x4driver, ADDR_PIF_AVDD_TX_CTRL_RW,&avdd_tx_ctrl_value);
    avdd_tx_ctrl_value  &= ~(1 << 6);//unset disable bit
    x4driver_set_pif_register(x4driver,ADDR_PIF_AVDD_TX_CTRL_RW,avdd_tx_ctrl_value);
           
    //check power good    
    uint32_t power_good = 0;
    for(unsigned char i = 0; i <16;i++)
    {
        uint8_t ldo_status_2_value = 0x00;
        x4driver_get_pif_register(x4driver, ADDR_PIF_LDO_STATUS_2_R,&ldo_status_2_value);
        if((ldo_status_2_value & (1 <<AVDD_TX_POWER_GOOD_BIT)) != 0x00)
        {
            power_good = 1;
            break;
        }
        //power not good
        x4driver_get_pif_register(x4driver, ADDR_PIF_AVDD_TX_CTRL_RW,&avdd_tx_ctrl_value);
        uint8_t trim_value = avdd_tx_ctrl_value & 0x1f;
        //trim value 0        
        x4driver_set_pif_register(x4driver,ADDR_PIF_AVDD_TX_CTRL_RW,0);                  
        
        x4driver_set_pif_register(x4driver,ADDR_PIF_AVDD_TESTMODE_RW,13);
        x4driver->callbacks.wait_us(i*MS);
        //disable test_mode                       
        x4driver_set_pif_register(x4driver,ADDR_PIF_AVDD_TESTMODE_RW,0);
        //write back trim value
        x4driver_set_pif_register(x4driver,ADDR_PIF_AVDD_TX_CTRL_RW,trim_value);        
        x4driver->callbacks.wait_us(i*MS);
    }
    
    //disable test mode.
    x4driver_set_pif_register(x4driver,ADDR_PIF_APC_DVDD_TESTMODE_RW,0);    
    if(power_good == 0)
    {
        x4driver_get_pif_register(x4driver, ADDR_PIF_AVDD_TX_CTRL_RW,&avdd_tx_ctrl_value);
        avdd_tx_ctrl_value |= (1 << 6);//set disable bit
        x4driver_set_pif_register(x4driver,ADDR_PIF_AVDD_TX_CTRL_RW,avdd_tx_ctrl_value);
        status = XEP_ERROR_X4DRIVER_LDO_ENABLE_FAIL;
    }
    mutex_give(x4driver);
	return status;
}

    

/**
 * @brief Enable avdd ldo on X4
 * NOTE: Implements workaround to kick start LDO.
 * requires enable to be set and 8051 SRAM to be program.
 * @return XEP_ERROR_X4DRIVER_LDO_ENABLE_FAIL in case of error.
 */
int _x4driver_enable_avdd_rx(X4Driver_t* x4driver)
{
    uint32_t status = mutex_take(x4driver);
    if (status != XEP_ERROR_X4DRIVER_OK) return status;
    //set dvdd_testmode 5    
    x4driver_set_pif_register(x4driver,ADDR_PIF_APC_DVDD_TESTMODE_RW,5);
    uint8_t avdd_rx_ctrl_value = 0x00;
    x4driver_get_pif_register(x4driver, ADDR_PIF_AVDD_RX_CTRL_RW,&avdd_rx_ctrl_value);
    avdd_rx_ctrl_value  &= ~(1 << 6);//unset disable bit
    x4driver_set_pif_register(x4driver,ADDR_PIF_AVDD_RX_CTRL_RW,avdd_rx_ctrl_value);
           
    //check power good    
    uint32_t power_good = 0;
    for(unsigned char i = 0; i <16;i++)
    {
        uint8_t ldo_status_2_value = 0x00;
        x4driver_get_pif_register(x4driver, ADDR_PIF_LDO_STATUS_2_R,&ldo_status_2_value);
        if((ldo_status_2_value & (1 <<AVDD_RX_POWER_GOOD_BIT)) != 0x00)
        {
            power_good = 1;
            break;
        }
        //power not good
        x4driver_get_pif_register(x4driver, ADDR_PIF_AVDD_RX_CTRL_RW,&avdd_rx_ctrl_value);
        uint8_t trim_value = avdd_rx_ctrl_value & 0x1f;
        //trim value 0        
        x4driver_set_pif_register(x4driver,ADDR_PIF_AVDD_RX_CTRL_RW,0);                  
        
        x4driver_set_pif_register(x4driver,ADDR_PIF_AVDD_TESTMODE_RW,208);
        x4driver->callbacks.wait_us(i*MS);
        //disable test_mode                       
        x4driver_set_pif_register(x4driver,ADDR_PIF_AVDD_TESTMODE_RW,0);
        //write back trim value
        x4driver_set_pif_register(x4driver,ADDR_PIF_AVDD_RX_CTRL_RW,trim_value);        
        x4driver->callbacks.wait_us(i*MS);
    }
    
    //disable test mode.
    x4driver_set_pif_register(x4driver,ADDR_PIF_APC_DVDD_TESTMODE_RW,0);    
    if(power_good == 0)
    {
        x4driver_get_pif_register(x4driver, ADDR_PIF_AVDD_RX_CTRL_RW,&avdd_rx_ctrl_value);
        avdd_rx_ctrl_value |= (1 << 6);//set disable bit
        x4driver_set_pif_register(x4driver,ADDR_PIF_AVDD_RX_CTRL_RW,avdd_rx_ctrl_value);
        status = XEP_ERROR_X4DRIVER_LDO_ENABLE_FAIL;
    }
    mutex_give(x4driver);
	return status;
}


/** @brief Set frame length in cycles. Cycles are in rx microframes.
 *
 * @return Status of execution
 */
int x4driver_set_frame_length(X4Driver_t* x4driver, uint8_t cycles)
{
	if(cycles < MIN_FRAME_LENGTH)
		return 	XEP_ERROR_X4DRIVER_FRAME_LENGTH_TO_LOW;
		
	uint32_t status = mutex_take(x4driver);				
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	
	x4driver_set_pif_register(x4driver,ADDR_PIF_RX_MFRAMES_COARSE_RW,cycles);
	x4driver->frame_length = cycles;		
	uint8_t rx_mframes_val = cycles * (96/12);
	x4driver_set_pif_register(x4driver,ADDR_PIF_RX_MFRAMES_RW,rx_mframes_val);
	uint32_t nrangebins = cycles*96;
	uint8_t rx_counter_num_bytes = 0x00;
	x4driver_get_pif_register(x4driver, ADDR_PIF_RX_COUNTER_NUM_BYTES_RW,&rx_counter_num_bytes);
	x4driver->frame_read_size = rx_counter_num_bytes*nrangebins;
	mutex_give(x4driver);
	return status;
}


/**
 * @brief Inits x4driver.
 * Will make sure that enable is set, 8051 SRAM is programmed, ldos are enabled, and that the external oscillator has been enabled. 
 * @return XEP_ERROR_X4DRIVER_OK in case of success. XEP_ERROR_X4DRIVER_BUSY,XEP_ERROR_X4DRIVER_8051_VERIFY_FAIL,XEP_ERROR_X4DRIVER_PIF_TIMEOUT,XEP_ERROR_X4DRIVER_OSC_LOCK_FAIL or XEP_ERROR_X4DRIVER_LDO_ENABLE_FAIL in case of errors.
 */
int x4driver_init(X4Driver_t* x4driver)
{
	x4driver->callbacks.enable_data_ready_isr(x4driver->user_reference,0);
    uint32_t status = 0;   
	status = x4driver_set_enable(x4driver,0); 
    status = x4driver_set_enable(x4driver,1);
	// Wait until X4 is stable.	
	for (volatile int i = 0; i < 2000; i++)
	{
		__asm__ volatile ("nop");		
	}	
	uint8_t force_one = 0x00;
	uint8_t force_zero = 0x00;
    x4driver_get_spi_register(x4driver, ADDR_SPI_FORCE_ONE_R, &force_one);
	x4driver_get_spi_register(x4driver, ADDR_SPI_FORCE_ZERO_R, &force_zero);
	if (force_one != 0xff && force_zero != 0x00)
	{
		return XEP_ERROR_X4DRIVER_NOK;
	}
    if (status != XEP_ERROR_X4DRIVER_OK) 
		return status;
    status = x4driver_upload_firmware_default(x4driver);
    if (status != XEP_ERROR_X4DRIVER_OK) 
		return status;
    status = x4driver_ldo_enable_all(x4driver);
    if (status != XEP_ERROR_X4DRIVER_OK) 
		return status;
    status = x4driver_init_clock(x4driver);  
    if (status != XEP_ERROR_X4DRIVER_OK) 
		return status;    
	status = x4driver_setup_default(x4driver); 
	if (status != XEP_ERROR_X4DRIVER_OK) 
		return status;    
	status = _update_normalization_variables(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) 
		return status;    
	status = x4driver_set_frame_area(x4driver,x4driver->frame_area_start,x4driver->frame_area_end);
	if (status != XEP_ERROR_X4DRIVER_OK) 
		return status;  
	status = x4driver_set_downconversion(x4driver,0);
	if (status != XEP_ERROR_X4DRIVER_OK) 
		return status;  
	x4driver->callbacks.enable_data_ready_isr(x4driver->user_reference, 1);
	return status;
}



/**
 * @brief Gets Iterations.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_get_iterations(X4Driver_t* x4driver, uint8_t * iterations)
{	
	return x4driver_get_pif_register(x4driver,ADDR_PIF_TRX_ITERATIONS_RW,iterations);
}

/**
 * @brief Sets Iterations.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_set_iterations(X4Driver_t* x4driver, uint8_t iterations)
{	
	x4driver->iterations = iterations;
	_update_normalization_constansts(x4driver);
	return x4driver_set_pif_register(x4driver,ADDR_PIF_TRX_ITERATIONS_RW,iterations);
}




/**
 * @brief Gets pulses per step.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_get_pulses_per_step(X4Driver_t* x4driver, uint16_t * pps)
{	
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	uint16_t _pps = 0x00;
	uint8_t pps_lsb = 0x00;
	uint8_t pps_msb = 0x00;
    status = x4driver_get_pif_register(x4driver,ADDR_PIF_TRX_PULSES_PER_STEP_LSB_RW,&pps_lsb);
	status = x4driver_get_pif_register(x4driver,ADDR_PIF_TRX_PULSES_PER_STEP_MSB_RW,&pps_msb);
	_pps = pps_msb << 8;
	_pps |= pps_lsb;
	*pps = _pps;
	mutex_give(x4driver);
	return status;
}


/**
 * @brief Sets pulses per step.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_set_pulses_per_step(X4Driver_t* x4driver, uint16_t  pps)
{	
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
		
	uint8_t pps_lsb = pps &0x00FF;
	uint8_t pps_msb = (pps &0xFF00) >> 8;
    status = x4driver_set_pif_register(x4driver,ADDR_PIF_TRX_PULSES_PER_STEP_LSB_RW,pps_lsb);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	status = x4driver_set_pif_register(x4driver,ADDR_PIF_TRX_PULSES_PER_STEP_MSB_RW,pps_msb);	
	x4driver->pulses_per_step = pps;
	_update_normalization_constansts(x4driver);
	mutex_give(x4driver);
	return status;
}



/**
 * @brief Sets a register segment. Takes mask, will shift data to fit segment.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_set_pif_segment(X4Driver_t* x4driver,uint8_t segment_address,uint8_t mask , uint8_t segment_value)
{	
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	uint8_t _mask = mask;
	uint8_t _segment_value = segment_value;
	uint8_t start_bit = 0;
	for(uint8_t i = 0; i<8;i++)
	{
		if(_mask &0x01)
		{
			start_bit = i;
			break;
		}
		_mask = _mask >> 1;
	}
	uint8_t reg_value = 0x00;		
    status = x4driver_get_pif_register(x4driver,segment_address,&reg_value);
	_segment_value =  segment_value <<start_bit;
	_segment_value = (reg_value  & ~mask) | _segment_value;		
	status = x4driver_set_pif_register(x4driver,segment_address,_segment_value);	
	mutex_give(x4driver);
	return status;
}




/**
 * @brief Gets a register segment. Takes mask, will shift data to fit segment.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_get_pif_segment(X4Driver_t* x4driver,uint8_t segment_address,uint8_t mask, uint8_t * segment_value)
{	
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	uint8_t _mask = mask;	 
	uint8_t start_bit = 0;
	for(uint8_t i = 0;i<8;i++)
	{
		if(_mask &0x01)
		{
			start_bit = i;
			break;
		}
		_mask = _mask >> 1;
	}
	uint8_t reg_value = 0x00;		
    status = x4driver_get_pif_register(x4driver,segment_address,&reg_value);
	
	uint8_t _segment_value =  (reg_value & mask) >>start_bit;
	*segment_value = _segment_value;			
	mutex_give(x4driver);
	return status;
}


/*
 * @brief Sets trx_dac_step_clog2 segment of TRX_DAC_STEP register.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_set_dac_step(X4Driver_t* x4driver, xtx4_dac_step_t  dac_step)
 {
	 uint32_t status = mutex_take(x4driver);
	 if (status != XEP_ERROR_X4DRIVER_OK) return status;
 
	 if(dac_step > 3)
	 {
		status = XEP_ERROR_X4DRIVER_INVALID_DACSTEP_INPUT;
		mutex_give(x4driver);
		return status;	  
	 }
 
	 status = x4driver_set_pif_segment(x4driver,ADDR_PIF_TRX_DAC_STEP_RW,0x03,dac_step);
	 mutex_give(x4driver);
	 return status;
 }
 
 /*
 * @brief Gets trx_dac_step_clog2 segment of TRX_DAC_STEP register.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_dac_step(X4Driver_t* x4driver, xtx4_dac_step_t * dac_step)
 {
	 uint32_t status = mutex_take(x4driver);
	 if (status != XEP_ERROR_X4DRIVER_OK) return status;
	 uint8_t _dac_step = 0x00;
	 status = x4driver_get_pif_segment(x4driver,ADDR_PIF_TRX_DAC_STEP_RW,0x03,&_dac_step);
	 *dac_step = _dac_step;
	 mutex_give(x4driver);
	 return status;
 }



/*
 * @brief Gets DAC min.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_dac_min(X4Driver_t* x4driver, uint16_t * dac_min)
 {
	 uint32_t status = mutex_take(x4driver);
	 if (status != XEP_ERROR_X4DRIVER_OK) return status;
	 uint16_t _dac_min = 0x00;
	 uint8_t _dac_min_lsb = 0x00;
	 uint8_t _dac_min_msb = 0x00;
	 
	 status = x4driver_get_pif_register(x4driver,ADDR_PIF_TRX_DAC_MIN_L_RW,&_dac_min_lsb);
	 status = x4driver_get_pif_register(x4driver,ADDR_PIF_TRX_DAC_MIN_H_RW,&_dac_min_msb);
	 _dac_min = (_dac_min_msb << 3) | (_dac_min_lsb & 0x07);
	 
	 
	 *dac_min = _dac_min;
	 mutex_give(x4driver);
	 return status;
 }
 
 /*
 * @brief Sets DAC min.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_set_dac_min(X4Driver_t* x4driver, uint16_t dac_min)
 {
	 if(dac_min > 0x07ff)
		return  XEP_ERROR_X4DRIVER_NOK;
	 uint32_t status = mutex_take(x4driver);
	 if (status != XEP_ERROR_X4DRIVER_OK) return status;
	 uint16_t _dac_min = dac_min;
	 uint8_t _dac_min_lsb = (uint8_t)(_dac_min & 0x0007);
	 uint8_t _dac_min_msb = (uint8_t)((_dac_min >> 3) & 0x00ff);
	 status = x4driver_set_pif_register(x4driver,ADDR_PIF_TRX_DAC_MIN_L_RW,_dac_min_lsb);
	 status = x4driver_set_pif_register(x4driver,ADDR_PIF_TRX_DAC_MIN_H_RW,_dac_min_msb);	
	 x4driver->dac_min = dac_min;
	 _update_normalization_constansts(x4driver);	 	 	 
	 mutex_give(x4driver);
	 return status;
 }
 
 
 
 /*
 * @brief Gets DAC max.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_dac_max(X4Driver_t* x4driver, uint16_t * dac_max)
 {
	 uint32_t status = mutex_take(x4driver);
	 if (status != XEP_ERROR_X4DRIVER_OK) return status;
	 uint16_t _dac_max = 0x00;
	 uint8_t _dac_max_lsb = 0x00;
	 uint8_t _dac_max_msb = 0x00;
	 status = x4driver_get_pif_register(x4driver,ADDR_PIF_TRX_DAC_MAX_L_RW,&_dac_max_lsb);
	 status = x4driver_get_pif_register(x4driver,ADDR_PIF_TRX_DAC_MAX_H_RW,&_dac_max_msb);
	 _dac_max = (_dac_max_msb << 3) | (_dac_max_lsb & 0x07);
	 
	 
	 *dac_max = _dac_max;
	 mutex_give(x4driver);
	 return status;
 }
 
 /*
 * @brief Sets DAC max.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_set_dac_max(X4Driver_t* x4driver, uint16_t dac_max)
 {
	 if(dac_max > 0x07ff)
		return  XEP_ERROR_X4DRIVER_NOK;
	 
	 uint32_t status = mutex_take(x4driver);
	 if (status != XEP_ERROR_X4DRIVER_OK) return status;
	 uint16_t _dac_max = dac_max;
	 uint8_t _dac_max_lsb = (uint8_t)(_dac_max & 0x0007);
	 uint8_t _dac_max_msb = (uint8_t)((_dac_max >> 3) & 0x00ff);
	 status = x4driver_set_pif_register(x4driver,ADDR_PIF_TRX_DAC_MAX_L_RW,_dac_max_lsb);
	 status = x4driver_set_pif_register(x4driver,ADDR_PIF_TRX_DAC_MAX_H_RW,_dac_max_msb);	
	  x4driver->dac_max = dac_max;
	 _update_normalization_constansts(x4driver);	 		 	 	 
	 mutex_give(x4driver);
	 return status;
 }
 
 
 
 /*
 * @brief Gets RX wait.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_rx_wait(X4Driver_t* x4driver, uint8_t * rx_wait)
 {
	 uint32_t status = mutex_take(x4driver);
	 if (status != XEP_ERROR_X4DRIVER_OK) return status;
	 uint8_t _rx_wait = 0x00;
	
	 status = x4driver_get_pif_register(x4driver,ADDR_PIF_RX_WAIT_RW,&_rx_wait);	 	 	 
	 *rx_wait = _rx_wait;
	 mutex_give(x4driver);
	 return status;
 }
 
 
  /*
 * @brief Sets rx wait.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_set_rx_wait(X4Driver_t* x4driver, uint8_t rx_wait)
 {
	 uint32_t status = mutex_take(x4driver);
	 if (status != XEP_ERROR_X4DRIVER_OK) return status;	
	 status = x4driver_set_pif_register(x4driver,ADDR_PIF_RX_WAIT_RW,rx_wait);	 	 	 
	 x4driver->rx_wait = rx_wait;
	 mutex_give(x4driver);
	 return status;
 }




 /*
 * @brief Sets fps.
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_set_fps(X4Driver_t* x4driver, uint32_t fps)
 {
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	x4driver->configured_fps = fps;
	if (x4driver->trigger_mode == SWEEP_TRIGGER_MCU)
	{
		 //change timing on mcu timer.
		 status = x4driver->sweep_timer.configure((void*)&x4driver->sweep_timer,fps);
	}
	else if (x4driver->trigger_mode == SWEEP_TRIGGER_X4)
	{		 
		 _x4driver_set_x4_sw_action(x4driver,X4_SW_ACTION_STOP_TIMER);
		 if(fps != 0)
		 {
			 _x4driver_set_x4_sw_register(x4driver,X4_SW_USE_PERIOD_TRIGGER,0);
			uint8_t fps_lsb = (uint8_t)(fps & 0x000000ff);
			uint8_t fps_msb = (uint8_t)((fps & 0x0000ff00)>>8);
			_x4driver_set_x4_sw_register(x4driver,X4_SW_REGISTER_FPS_LSB_ADDR,fps_lsb);
			_x4driver_set_x4_sw_register(x4driver,X4_SW_REGISTER_FPS_MSB_ADDR,fps_msb);
			_x4driver_set_x4_sw_action(x4driver,X4_SW_ACTION_START_TIMER);
		 }
	}
	else if (x4driver->trigger_mode == SWEEP_TRIGGER_MANUAL)
	{
		//do nothing..
	}
	else
	{
		
		status = XEP_ERROR_X4DRIVER_NOT_SUPPORTED;
	}
	
	 mutex_give(x4driver);
	 return status;
 }
 
/*
 * @brief Gets configured fps.
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_fps(X4Driver_t* x4driver, uint32_t * fps)
{
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	*fps = x4driver->configured_fps;
	mutex_give(x4driver);
	return status;
}


/*
 * @brief Gets calculated fps i.e. for a software timer running timer ticks on ms resolution it will give the configured fps from the timer.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_calculated_fps(X4Driver_t* x4driver, uint32_t * fps)
{
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	
	if (x4driver->trigger_mode == SWEEP_TRIGGER_MCU)
	{
		*fps = x4driver->sweep_timer.configured_frequency;
	}
	else if (x4driver->trigger_mode == SWEEP_TRIGGER_X4)
	{
		*fps = x4driver->configured_fps;
	}
	else if (x4driver->trigger_mode == SWEEP_TRIGGER_MANUAL)
	{
		*fps = 0;		
	}
	mutex_give(x4driver);
	return status;
}


 

 
 
 
/*
 * @brief Sets sweep trigger control.
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_set_sweep_trigger_control(X4Driver_t* x4driver, xtx4_sweep_trigger_control_mode_t mode)
 {
	 uint32_t status = mutex_take(x4driver);
	 if (status != XEP_ERROR_X4DRIVER_OK) return status;	
	 if(x4driver->trigger_mode != mode )
	 {	
		 uint32_t fps = 0;
		 x4driver_get_fps(x4driver,&fps);	 		 
		 //stop previous mode..
		if(fps != 0)	 		 
			x4driver_set_fps(x4driver,0);	 		 		
		 x4driver->trigger_mode = mode;	 		 
		 //start new mode.
		 if(mode != SWEEP_TRIGGER_MANUAL)
		 {
			if(fps != 0)
				x4driver_set_fps(x4driver,fps);	 
		 }
		 
	 }
	 
	 mutex_give(x4driver);
	 return status;
 }


/*
 * @brief Inverts buffer.
 */
void _invert(int8_t * source,int8_t * dst,uint8_t len)
{
	for(int i = 0; i<len;i++)
	{
		int8_t src_val = source[i];
		int8_t src_i = src_val *-1;
		dst[i] = src_i;
	}
}



/*
 * @brief Sets first ram line
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_set_rx_ram_first_line(X4Driver_t* x4driver, uint32_t first_line)
{
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	
	uint32_t rx_ram_line_first = first_line;
	uint8_t rx_ram_line_first_lsb = (uint8_t)rx_ram_line_first & 0x00000001;
	uint8_t rx_ram_line_first_msb = (uint8_t)(rx_ram_line_first >>1) & 0x000000ff;
	
	x4driver_set_pif_segment(x4driver,ADDR_PIF_RX_RAM_LSBS_RW,0x02,rx_ram_line_first_lsb);	
	x4driver_set_pif_register(x4driver,ADDR_PIF_RX_RAM_LINE_FIRST_MSB_RW,rx_ram_line_first_msb);
	mutex_give(x4driver);
	return XEP_ERROR_X4DRIVER_OK;
}

/*
 * @brief Sets Last ram line
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
int _x4driver_set_rx_ram_last_line(X4Driver_t* x4driver,uint32_t last_line)
{
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	
	uint32_t rx_ram_line_last = last_line;
	uint8_t rx_ram_line_last_lsb = (uint8_t)rx_ram_line_last & 0x00000001;
	uint8_t rx_ram_line_last_msb = (uint8_t)(rx_ram_line_last >>1) & 0x000000ff;
	x4driver_set_pif_segment(x4driver,ADDR_PIF_RX_RAM_LSBS_RW,0x01,rx_ram_line_last_lsb);
	x4driver_set_pif_register(x4driver,ADDR_PIF_RX_RAM_LINE_LAST_MSB_RW,rx_ram_line_last_msb);
	mutex_give(x4driver);
	return XEP_ERROR_X4DRIVER_OK;
}

/*
 * @brief Sets down convention
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_set_downconversion(X4Driver_t* x4driver,uint8_t enable)
 {
	 uint32_t status = mutex_take(x4driver);
	 if (status != XEP_ERROR_X4DRIVER_OK) return status;	
	 x4driver_set_pif_segment(x4driver,ADDR_PIF_SMPL_MODE_RW,0x01,enable);//enable down convention	 
	 
	 if(enable == 1)
	 {		 	 
		 uint32_t c1_i_len = 0;
		 uint32_t c1_q_len = 0;
		 _x4driver_get_downconvertion_coef_c1_i_length(&c1_i_len);
		 _x4driver_get_downconvertion_coef_c1_q_length(&c1_q_len);	 
		 int8_t c2_i[c1_i_len];
		 int8_t c2_q[c1_i_len];
		 if(x4driver->region == X4DRIVER_REGION_AUTO)
		 {
			 uint8_t tx_pll_ctrl_1 = 0x00;
			 x4driver_get_pif_register(x4driver,ADDR_PIF_TX_PLL_CTRL_1_RW,&tx_pll_ctrl_1);
			 uint8_t tx_pll_ctrl_1_tx_pll_fbdiv = (tx_pll_ctrl_1 >> 4) & 0x07;
			 if(tx_pll_ctrl_1_tx_pll_fbdiv == 3)
			 {
				 x4driver->region = X4DRIVER_REGION_EU;
			 }
			 else if(tx_pll_ctrl_1_tx_pll_fbdiv == 4)
			 {
				 x4driver->region = X4DRIVER_REGION_KCC;
			 }
			 else
			 {
				 return XEP_ERROR_X4DRIVER_DOWNCONVERTION_REGION_NOT_SUPPORTED;
			 }
		 }
		if(x4driver->region == X4DRIVER_REGION_EU)
		{
			_invert(downconvertion_coef_c1_q,c2_q,c1_q_len);
			_invert(downconvertion_coef_c1_i,c2_i,c1_i_len);
		}
		else if(x4driver->region == X4DRIVER_REGION_KCC)
		{
			memcpy(c2_i,downconvertion_coef_c1_i,c1_i_len);
			memcpy(c2_q,downconvertion_coef_c1_q,c1_q_len);
		}
		else
		{
			 return XEP_ERROR_X4DRIVER_DOWNCONVERTION_REGION_NOT_SUPPORTED;
		}
	
		for( int i = 31; i>=0; i--)
		{
		
			uint8_t value =  0x3f & downconvertion_coef_c1_i[i];		
			x4driver_set_pif_register(x4driver,ADDR_PIF_RX_DOWNCONVERSION_COEFF_I1_WE,value);
		}
	
		for( int i = 31; i>=0; i--)
		{
			uint8_t value =  0x3f & downconvertion_coef_c1_q[i];	
			x4driver_set_pif_register(x4driver,ADDR_PIF_RX_DOWNCONVERSION_COEFF_Q1_WE,value);
		}
	
		for( int i = 31; i>=0; i--)
		{
			uint8_t value =  (uint8_t)(0x3f & c2_q[i]);	
			x4driver_set_pif_register(x4driver,ADDR_PIF_RX_DOWNCONVERSION_COEFF_Q2_WE,value);
		}
	
		for( int i = 31; i>=0; i--)
		{
			uint8_t value =  (uint8_t)(0x3f & c2_i[i]);	
			x4driver_set_pif_register(x4driver,ADDR_PIF_RX_DOWNCONVERSION_COEFF_I2_WE,value);
		}
		
					
		x4driver->bytes_per_counter = 6;
		x4driver_set_pif_register(x4driver,ADDR_PIF_RX_COUNTER_NUM_BYTES_RW,x4driver->bytes_per_counter);		
	 }
	 else
	 {				
		x4driver->bytes_per_counter = 3;
		x4driver_set_pif_register(x4driver,ADDR_PIF_RX_COUNTER_NUM_BYTES_RW,x4driver->bytes_per_counter);				
	 }
	x4driver->downconvertion_enabled= enable;
	x4driver_set_frame_area(x4driver,x4driver->frame_area_start_requested, x4driver->frame_area_end_requested);
	mutex_give(x4driver);
	return XEP_ERROR_X4DRIVER_OK;	
 }
 
 
 
 
 /*
 * @brief Gets Number of bins
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_frame_bin_count(X4Driver_t* x4driver, uint32_t * bins)
 {
	 if(x4driver->downconvertion_enabled == 1)
	 {
		*bins = (x4driver->frame_read_size / x4driver->bytes_per_counter)/2;	 	 //i and q
	 }
	 else
	 {
		 *bins = x4driver->frame_read_size / x4driver->bytes_per_counter;	
		 *bins = *bins - x4driver->frame_area_start_bin_offset - x4driver->frame_area_end_bin_offset;
	 }
	 return XEP_ERROR_X4DRIVER_OK;
	
 }
 
 
 
 /*
 * @brief Gets down convention
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_downconvertion(X4Driver_t* x4driver, uint8_t * enable)
 {
	 *enable = x4driver->downconvertion_enabled;
	 return XEP_ERROR_X4DRIVER_OK;
 }
 
 
  /*
 * @brief Sets Pulse Repetition Frequency(PRF) divider
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_set_prf_div(X4Driver_t* x4driver, uint8_t  prf_div)
 {
	 if(prf_div < 4)
	 {
		 return XEP_ERROR_X4DRIVER_PRF_DIV_TOO_SMALL;
	 }
	 x4driver_set_pif_register(x4driver,ADDR_PIF_TRX_CLOCKS_PER_PULSE_RW,prf_div);	 
	 
	 return XEP_ERROR_X4DRIVER_OK;
 }


  /*
 * @brief Gets Pulse Repetition Frequency(PRF) divider
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_prf_div(X4Driver_t* x4driver, uint8_t * prf_div)
 {
	 uint8_t prf_dif_val = 0x00;
	 x4driver_get_pif_register(x4driver,ADDR_PIF_TRX_CLOCKS_PER_PULSE_RW,&prf_dif_val);	 
	 *prf_div = prf_dif_val; 
	 return XEP_ERROR_X4DRIVER_OK;
 }
 
 
 
 
 
  /*
 * @brief Sets the TX center frequency
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_set_tx_center_frequency(X4Driver_t* x4driver, xtx4_tx_center_frequency_t tx_center_frequency)
 {
	 if(tx_center_frequency < TX_CENTER_FREQUENCY_MIN  || tx_center_frequency > TX_CENTER_FREQUENCY_MAX)
		return XEP_ERROR_X4DRIVER_INVALID_TX_CENTER_FREQUENCY;
		 
	 x4driver_set_pif_segment(x4driver,ADDR_PIF_TX_PLL_CTRL_1_RW,0x70,tx_center_frequency);	 	 	 
	 return XEP_ERROR_X4DRIVER_OK;
 }
 
   /*
 * @brief Gets the TX center frequency
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_tx_center_frequency(X4Driver_t* x4driver, xtx4_tx_center_frequency_t * tx_center_frequency)
 {
	 uint8_t read_back = 0x00;
	 x4driver_get_pif_register(x4driver,ADDR_PIF_TX_PLL_CTRL_1_RW,&read_back);
	 read_back = (read_back >> 4) & 0x07;
	 *tx_center_frequency = read_back;
	 return XEP_ERROR_X4DRIVER_OK;
 }
 
 
 /*
 * @brief Sets the TX output power
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_set_tx_power(X4Driver_t* x4driver, xtx4_tx_power_t tx_power)
 {
	 if(tx_power < TX_POWER_OFF  || tx_power > TX_POWER_HIGH)
	 return XEP_ERROR_X4DRIVER_INVALID_TX_POWER_SETTING;
 
	 x4driver_set_pif_segment(x4driver,ADDR_PIF_DVDD_TRIM_RW,0x60,tx_power);
	 return XEP_ERROR_X4DRIVER_OK;
 }
 
 
  /*
 * @brief Gets the TX output power
 * requires enable to be set and 8051 SRAM to be programed.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_tx_power(X4Driver_t* x4driver, xtx4_tx_power_t *tx_power)
 {
	uint8_t read_back = 0x00;
	 x4driver_get_pif_register(x4driver,ADDR_PIF_DVDD_TRIM_RW,&read_back);
	 read_back = (read_back&0x60) >>5;
	 *tx_power = read_back;	 
	 return XEP_ERROR_X4DRIVER_OK;
 }
 
 
  
/*
 * @brief Gets the length between data bins. 
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_bin_length(X4Driver_t* x4driver, float32_t * bin_length)
 {	 
	 if(x4driver->downconvertion_enabled == 1)		
		*bin_length = X4DRIVER_METERS_PER_BIN_DOWN_CONVERTION;
	 else	
		*bin_length = X4DRIVER_METERS_PER_BIN;
	
	 return XEP_ERROR_X4DRIVER_OK;
 }
 
 
 
 /*
 * @brief Gets sweep time. 
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_sweep_time(X4Driver_t* x4driver, float32_t * sweep_time)
 {	 
	 uint16_t dac_min =0;
	 uint16_t dac_max =0;
	 xtx4_dac_step_t dac_step =DAC_STEP_1;
	 x4driver_get_dac_min(x4driver,&dac_min);
	 x4driver_get_dac_max(x4driver,&dac_max);	 
	 x4driver_get_dac_step(x4driver,&dac_step);
	 uint8_t dac_step_val =  1 << dac_step;
	 
	 uint16_t pulses_per_step = 0;
	 x4driver_get_pulses_per_step(x4driver,&pulses_per_step);
	 uint8_t iterations = 0;
	 x4driver_get_iterations(x4driver,&iterations);
	 
	 float32_t dac_steps = (dac_max - dac_min)/dac_step_val;
	 float32_t total_steps = dac_steps*pulses_per_step*iterations;
	 uint8_t prf_div = 0;
	 x4driver_get_prf_div(x4driver,&prf_div);
	 float32_t prf = 243e6/prf_div;
	 float32_t total_time = (1/prf)*total_steps;
	 *sweep_time = total_time;	 
	 return XEP_ERROR_X4DRIVER_OK;
 }
 
 
/*
 * @brief Gets sampler frequency of 23.328 Ghz when RF mode is selected and  23.328 Ghz/8 when down conversion is selected.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_get_sampler_frequency(X4Driver_t* x4driver, float32_t * frequency)
 {

	 float32_t tmp = 1944.0*12*1000000;
	 if(x4driver->downconvertion_enabled == 1)
	 {
		 tmp = tmp /8;
	 }
	 *frequency = tmp;
	 return XEP_ERROR_X4DRIVER_OK; 
 }
 
 
 
 /*
 * @brief Checks configuration. 
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_check_configuration(X4Driver_t* x4driver)
 {
	 uint8_t iterations = 0;
	 x4driver_get_iterations(x4driver,&iterations);
	 uint8_t noiseless_ghost_order = 0;
	 x4driver_get_pif_segment(x4driver,ADDR_PIF_TRX_LFSR_TAPS_2_RW,0x70,&noiseless_ghost_order);
	 uint8_t trx_auto_bidir_enable = 0;
	 x4driver_get_pif_segment(x4driver,ADDR_PIF_TRX_DAC_STEP_RW,0x20,&trx_auto_bidir_enable);
	 	 	 
	 if(iterations % (uint32_t)(pow(2,noiseless_ghost_order) * pow(2,trx_auto_bidir_enable)) != 0)
	   return XEP_ERROR_X4DRIVER_INVALID_CONFIGURATION_NOISELESS_GHOST;//"trx_iterations should be divisible by (2^noiseless_ghost_order) * (2^trx_auto_bidir_enable)"
	   
	   
	   	
	 
	 uint8_t rx_counter_lsb = 0;
	 x4driver_get_pif_register(x4driver,ADDR_PIF_RX_COUNTER_LSB_RW,&rx_counter_lsb);
	 if(rx_counter_lsb != 0)//support not added.
		return XEP_ERROR_X4DRIVER_NOK;
	
	uint8_t trx_clocks_per_pulse =0;
	x4driver_get_prf_div(x4driver,&trx_clocks_per_pulse);
	uint8_t rx_mframes_coarse = 0;
	x4driver_get_pif_register(x4driver,ADDR_PIF_RX_MFRAMES_COARSE_RW,&rx_mframes_coarse);
	if(trx_clocks_per_pulse<rx_mframes_coarse)
		return XEP_ERROR_X4DRIVER_INVALID_CONFIGURATION_RX_MFRAMES_COARSE; // trx_clocks_per_pulse must be >= rx_mframes_coarse
	
	uint16_t dac_min =0;
	uint16_t dac_max =0;
	xtx4_dac_step_t dac_step =DAC_STEP_1;
	x4driver_get_dac_min(x4driver,&dac_min);
	x4driver_get_dac_max(x4driver,&dac_max);
	x4driver_get_dac_step(x4driver,&dac_step);
	uint8_t dac_step_val =  1 << dac_step;
	
	uint16_t pulses_per_step = 0;
	x4driver_get_pulses_per_step(x4driver,&pulses_per_step);
	float32_t dac_range = ((dac_max-dac_min)/dac_step_val)-1;
	uint32_t max_counter = 16777215;
	if((iterations*pulses_per_step*dac_range) > max_counter)
		return XEP_ERROR_X4DRIVER_INVALID_CONFIGURATION_MAX_COUNTER;
	
	uint8_t pllauxclk_sel = 0;
	x4driver_get_pif_segment(x4driver,ADDR_PIF_CLKOUT_SEL_RW,0x30,&pllauxclk_sel);
	pllauxclk_sel = (pllauxclk_sel *2)+2;
	
	uint8_t trx_backend_clk_prescale = 0;
	x4driver_get_pif_segment(x4driver,ADDR_PIF_MCLK_TRX_BACKEND_CLK_CTRL_RW,0x38,&trx_backend_clk_prescale);
	if(trx_backend_clk_prescale != 0)
	{
		trx_backend_clk_prescale = 1 <<(trx_backend_clk_prescale-1);
	}
	
	uint8_t trx_backend_clk_div = 0;
	x4driver_get_pif_segment(x4driver,ADDR_PIF_MCLK_TRX_BACKEND_CLK_CTRL_RW,0x07,&trx_backend_clk_div);
	trx_backend_clk_div = trx_backend_clk_div+1;
	uint32_t trx_backend_clk_totaldiv = trx_backend_clk_div*trx_backend_clk_prescale*pllauxclk_sel;
	uint8_t trx_dac_settle_clog2 = 0;
	x4driver_get_pif_segment(x4driver,ADDR_PIF_TRX_DAC_STEP_RW,0x0C,&trx_dac_settle_clog2);
	trx_dac_settle_clog2 = 1 <<trx_dac_settle_clog2;
	if((trx_dac_settle_clog2* trx_backend_clk_totaldiv) >=trx_clocks_per_pulse)
		return XEP_ERROR_X4DRIVER_INVALID_CONFIGURATION_RARE_DROP_OF_BITS; //"Safe conditions for avoiding rare drop of bits issue in receiver backend may not be met."
	
	
	uint8_t rx_mframes = 0;
	
	x4driver_get_pif_register(x4driver,ADDR_PIF_RX_MFRAMES_RW,&rx_mframes);
	if(trx_backend_clk_totaldiv>(uint32_t)((511 * trx_clocks_per_pulse)/((rx_mframes*3)+3)))
		return XEP_ERROR_X4DRIVER_INVALID_CONFIGURATION_TRX_BACKEND_CLK;//"Total trx_backend_clk division must be must be less than ((511 * trx_clocks_per_pulse)/(rx_mframes*3 + 3))"
	

	 return XEP_ERROR_X4DRIVER_OK;
 }
 
 
 
 /*
 * @brief Sets DAC values automatically.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_set_dac_auto(X4Driver_t* x4driver,uint32_t daclength)
 {
	 if( daclength > 2047)
	 {
		 return XEP_ERROR_X4DRIVER_NOK;	  
	 }
	 if(x4driver->downconvertion_enabled == 1)
		return XEP_ERROR_X4DRIVER_NOK;
	 
	 uint32_t fc = 0;
	 uint32_t bins = 0;
	 xtx4_sweep_trigger_control_mode_t org_tm = x4driver->trigger_mode;
	 uint32_t org_fps = 0;
	 x4driver_get_fps(x4driver,&org_fps);
	 x4driver_set_sweep_trigger_control(x4driver,SWEEP_TRIGGER_MANUAL); 
	 x4driver_get_frame_bin_count(x4driver,&bins);
	 float32_t tmp[bins];
	 
	 x4driver_start_sweep(x4driver);
	 uint8_t trx_ctrl_done = 0;
	 x4driver_get_pif_register(x4driver,ADDR_PIF_TRX_CTRL_DONE_R,&trx_ctrl_done);
	 
	 while(trx_ctrl_done == 0)
	 {		
		x4driver_get_pif_register(x4driver,ADDR_PIF_TRX_CTRL_DONE_R,&trx_ctrl_done);	 
	 }
	 x4driver_read_frame_normalized(x4driver,&fc,tmp,bins);
	 float32_t avg = 0;
	 float32_t dl2 = daclength/2;
	 for(uint32_t i = 0; i <bins;i++)
	 {
		 avg += ((tmp[i] +1)*1024)  /bins;
	 }
	 if( avg > 2047)//something is wrong.
	 {
		 return XEP_ERROR_X4DRIVER_NOK;	 
	 }
	 uint32_t dac_max = avg+dl2;
	 int32_t dac_min = avg-dl2;
	 if(dac_max > 2047)//saturated
	 {
		 x4driver_set_dac_min(x4driver,2047-daclength);
		 x4driver_set_dac_max(x4driver,2047);
		
	 }	
	 else if(dac_min < 0)
	 {		 
		  x4driver_set_dac_min(x4driver,0);
		  x4driver_set_dac_max(x4driver,daclength);
	 }
	 else
	 {
		x4driver_set_dac_min(x4driver,avg-dl2);
		x4driver_set_dac_max(x4driver,avg+dl2);	 
	 }
	 //setback old values.	 
	 x4driver_set_sweep_trigger_control(x4driver,org_tm);
	 x4driver_set_fps(x4driver,org_fps);	 
	 
	 return XEP_ERROR_X4DRIVER_OK;	
 }
 


/*
 * @brief Gets RF sampler frequency of  23.328 Ghz.
 * @return Status of execution as defined in x4driver.h
 */
int x4driver_get_sampler_frequency_rf(X4Driver_t*  x4driver, float32_t * rf_frequency)
{
	 float32_t tmp = 1944.0*12*1000000;
	 *rf_frequency = tmp;
	 return XEP_ERROR_X4DRIVER_OK;	
}



 /*
 * @brief Sets frame trigger period in 1/10000 second increments 
 * requires enable to be set and 8051 SRAM to be program.
 * @return Status of execution as defined in x4driver.h
 */
 int x4driver_set_frame_trigger_period(X4Driver_t* x4driver, uint32_t period)
 {
	uint32_t status = mutex_take(x4driver);
	if (status != XEP_ERROR_X4DRIVER_OK) return status;
	

	if (x4driver->trigger_mode == SWEEP_TRIGGER_X4)
	{
		float32_t period_seconds = (1.0/10000)*period;
		x4driver->configured_fps = 1/period_seconds;
		 _x4driver_set_x4_sw_action(x4driver,X4_SW_ACTION_STOP_TIMER);
		 if(period != 0)
		 {
			 _x4driver_set_x4_sw_register(x4driver,X4_SW_USE_PERIOD_TRIGGER,1);
			uint8_t period_0 = (uint8_t)(period & 0x000000ff);
			_x4driver_set_x4_sw_register(x4driver,X4_SW_PERIOD_0_ADDR,period_0);
			uint8_t period_1 = (uint8_t)((period & 0x0000ff00)>>8);
			_x4driver_set_x4_sw_register(x4driver,X4_SW_PERIOD_1_ADDR,period_1);
			uint8_t period_2 = (uint8_t)((period & 0x00ff0000)>>8);
			_x4driver_set_x4_sw_register(x4driver,X4_SW_PERIOD_2_ADDR,period_2);
			uint8_t period_3 = (uint8_t)((period & 0xff000000)>>8);
			_x4driver_set_x4_sw_register(x4driver,X4_SW_PERIOD_3_ADDR,period_3);
			
			_x4driver_set_x4_sw_action(x4driver,X4_SW_ACTION_START_TIMER);
		 }
	}
	else
	{
		
		status = XEP_ERROR_X4DRIVER_NOK;
	}
	
	 mutex_give(x4driver);
	 return status;
 }
