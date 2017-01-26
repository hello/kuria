/**
 * @file
 *
 * 
 */

#include "xep_application_mcp_callbacks.h"
#include "xep.h"
#include "xep_hal.h"
#include "xtserial_definitions.h"
#include "xep_dispatch_messages.h"
#include "xtid.h"
#include "xep_hal.h"
#include "build.h"
#include "version.h"
#include "conf_usb.h"

void onMcpPing(uint32_t pingval, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;
    uint32_t pongval=0;
    if (1) // TODO: (sensor_config_control.systemsReady == 0b00011111) // All systems GO.
    {
        pongval = XTS_DEF_PONGVAL_READY;
    }
	
    dispatch_message_hostcom_send_pong(xep->dispatch, pongval);
}

void onMcpUnknown(uint8_t* data, uint32_t length, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;
    dispatch_message_hostcom_send_error(xep->dispatch, XTS_SPRE_NOT_RECOGNIZED);
}

void onMcpModuleReset(void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    dispatch_message_hostcom_send_ack(xep->dispatch);

    // Wait a while for ack to be sent
    xt_wait(500);

    xt_software_reset(XT_SWRST_HOSTCOMMAND);
}

void onMcpSystemRunTest(uint8_t testcode, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;
    xt_test_result_t test_result;
    if (xt_run_selftest(testcode, &test_result) != XT_SUCCESS)
    {
        dispatch_message_hostcom_send_error(xep->dispatch, XTS_SPRE_NOT_RECOGNIZED);
    }
    dispatch_message_hostcom_send_data_byte(xep->dispatch, 
        (XTS_SPC_DIR_COMMAND)|(XTS_SDC_SYSTEM_TEST<<8), 
        test_result.id|((test_result.passed ? 1 : 0) << 8), 
        test_result.data, 
        XT_SELFTEST_DATA_LENGTH);
}

void onMcpStartBootloader(uint32_t key, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    uint32_t key_ref = 0xa2b95ef0;
    if (key != key_ref)
    {
        dispatch_message_hostcom_send_data_string(xep->dispatch, 0, 0, (char*)"Unknown start_bootloader key.");
        dispatch_message_hostcom_send_error(xep->dispatch, XTS_SPRE_NOT_RECOGNIZED);
        return;
    }
    
    dispatch_message_hostcom_send_ack(xep->dispatch);

    // Wait a while for ack to be sent
    xt_wait(500);

    xt_software_reset(XT_SWRST_BOOTLOADER);
}

void onMcpGetSystemInfo(uint32_t contentid, uint8_t infocode, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    if (infocode == XTID_SSIC_FIRMWAREID)
        dispatch_message_hostcom_send_reply_string(xep->dispatch, contentid, infocode, (char*)PRODUCT_NAME);
    else if (infocode == XTID_SSIC_VERSION)
        dispatch_message_hostcom_send_reply_string(xep->dispatch, contentid, infocode, (char*)VERSION_STRING);
    else if (infocode == XTID_SSIC_BUILD)
        dispatch_message_hostcom_send_reply_string(xep->dispatch, contentid, infocode, (char*)BUILD_STRING);
    else if (infocode == XTID_SSIC_VERSIONLIST)
    {
        char version_list[40];
        int len = xep_get_systeminfo_versionlist(version_list, 40);
        if (len > 0)
            dispatch_message_hostcom_send_reply_string(xep->dispatch, contentid, infocode, version_list);
        else
            dispatch_message_hostcom_send_error(xep->dispatch, XTS_SPRE_RESERVED);
    }
    else
        dispatch_message_hostcom_send_error(xep->dispatch, XTS_SPRE_NOT_RECOGNIZED);
}



/* --- X4Driver commands --- */

void onMcpX4DriverSetFps(uint32_t fps, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_fps(xep->x4driver, fps);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetIterations(uint32_t iterations, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_iterations(xep->x4driver, iterations);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetPulsesPerStep(uint32_t pulsesperstep, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_pulses_per_step(xep->x4driver, pulsesperstep);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetDownconversion(uint8_t downconversion, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_downconversion(xep->x4driver,downconversion);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetFrameArea(float start, float end, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_frame_area(xep->x4driver, start, end);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverInit(void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_init(xep->x4driver);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetDacStep(uint8_t dac_step, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_dac_step(xep->x4driver, dac_step);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetDacMin(uint32_t dac_min, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_dac_min(xep->x4driver, dac_min);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetDacMax(uint32_t dac_max, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_dac_max(xep->x4driver, dac_max);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetFrameAreaOffset(float offset, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_frame_area_offset(xep->x4driver, offset);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetEnable(uint8_t enable, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_enable(xep->x4driver, enable);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetTxCenterFrequency(uint8_t tx_center_frequency, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_tx_center_frequency(xep->x4driver, tx_center_frequency);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetTxPower(uint8_t tx_power, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_tx_power(xep->x4driver, tx_power);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverGetFps(void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    uint32_t fps;
    int32_t res = x4driver_get_fps(xep->x4driver, &fps);
    int32_t fps_i = (int32_t)fps;
    if (res == 0)
        dispatch_message_hostcom_send_reply_int(xep->dispatch, XTS_SPCXI_FPS, 0, &fps_i, 1);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetSpiRegister(uint8_t address, uint8_t value, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_spi_register(xep->x4driver, address, value);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverGetSpiRegister(uint8_t address, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    uint8_t value;
    int32_t res = x4driver_get_spi_register(xep->x4driver, address, &value);
    if (res == 0)
        dispatch_message_hostcom_send_reply_byte(xep->dispatch, XTS_SPCXI_SPIREGISTER, 0, &value, 1);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetPifRegister(uint8_t address, uint8_t value, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_pif_register(xep->x4driver, address, value);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverGetPifRegister(uint8_t address, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    uint8_t value;
    int32_t res = x4driver_get_pif_register(xep->x4driver, address, &value);
    if (res == 0)
        dispatch_message_hostcom_send_reply_byte(xep->dispatch, XTS_SPCXI_PIFREGISTER, 0, &value, 1);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetXifRegister(uint8_t address, uint8_t value, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_xif_register(xep->x4driver, address, value);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverGetXifRegister(uint8_t address, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    uint8_t value;
    int32_t res = x4driver_get_xif_register(xep->x4driver, address, &value);
    if (res == 0)
        dispatch_message_hostcom_send_reply_byte(xep->dispatch, XTS_SPCXI_XIFREGISTER, 0, &value, 1);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverSetPrfDiv(uint8_t prf_div, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = x4driver_set_prf_div(xep->x4driver, prf_div);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverGetPrfDiv(void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    uint8_t prf_div;
    int32_t res = x4driver_get_prf_div(xep->x4driver, &prf_div);
    if (res == 0)
        dispatch_message_hostcom_send_reply_byte(xep->dispatch, XTS_SPCXI_PRFDIV, 0, &prf_div, 1);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverGetFrameArea(void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    float32_t frame_area[2];
    int32_t res = x4driver_get_frame_area(xep->x4driver, &frame_area[0], &frame_area[1]);
    if (res == 0)
        dispatch_message_hostcom_send_reply_float(xep->dispatch, XTS_SPCXI_FRAMEAREA, 0, frame_area, 2);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpX4DriverGetFrameAreaOffset(void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    float32_t offset;
    int32_t res = x4driver_get_frame_area_offset(xep->x4driver, &offset);
    if (res == 0)
        dispatch_message_hostcom_send_reply_float(xep->dispatch, XTS_SPCXI_FRAMEAREAOFFSET, 0, &offset, 1);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpSetIOPinControl(uint32_t pin_id, uint32_t pin_setup, uint32_t pin_feature, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    if (pin_feature != XTID_IOPIN_FEATURE_PASSIVE) // For now, only support passive mode.
    {
        dispatch_message_hostcom_send_error(xep->dispatch, XTS_SPRE_NOT_RECOGNIZED);
        return;
    }

    int32_t res = xtio_set_direction(pin_id, pin_setup, 0);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpSetIOPinValue(uint32_t pin_id, uint32_t pin_value, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int32_t res = xtio_set_level(pin_id, pin_value);
    if (res == 0)
        dispatch_message_hostcom_send_ack(xep->dispatch);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}

void onMcpGetIOPinValue(uint32_t pin_id, void * user_data)
{
    XepHandle_t* xep = (XepHandle_t*)user_data;

    int pin_value=0;
    int32_t res = xtio_get_level(pin_id, &pin_value);
    if (res == 0)
        dispatch_message_hostcom_send_reply_int(xep->dispatch, XTS_SPCIOP_GETVALUE, 0, (int32_t*)&pin_value, 1);
    else
        dispatch_message_hostcom_send_error(xep->dispatch, res);
}
