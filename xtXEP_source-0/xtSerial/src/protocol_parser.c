#include "protocol_parser.h"
#include "xtserial_definitions.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


unsigned int getMcpParserInstanceSize(void)
{
    return sizeof(McpParser_t);
}

McpParser_t *createMcpParser(void * user_data, void * instance_memory)
{
    McpParser_t * instance = (McpParser_t *)instance_memory;
    instance->user_data = user_data;

    instance->onMcpPing = NULL;
    instance->onMcpStartBootloader = NULL;

    instance->onMcpSetLedControl = NULL;
    instance->onMcpModuleReset = NULL;
    instance->onMcpSystemRunTest = NULL;
    instance->onMcpGetSystemInfo = NULL;

    instance->onMcpSetSensorMode = NULL;
    instance->onMcpLoadProfile = NULL;
    instance->onMcpProfileSetDetectionZone = NULL;
    instance->onMcpProfileGetDetectionZone = NULL;
    instance->onMcpProfileGetDetectionZoneLimits = NULL;
    instance->onMcpProfileSetSensitivity = NULL;
    instance->onMcpProfileSetProfileParameterFile = NULL;
    instance->onMcpProfileGetProfileParameterFile = NULL;

    instance->onMcpSetOutputControl = NULL;

    instance->onMcpSetIOPinControl = NULL;
    instance->onMcpSetIOPinValue = NULL;
    instance->onMcpGetIOPinValue = NULL;

    instance->onMcpX4DriverSetFps = NULL;
    instance->onMcpX4DriverSetIterations = NULL;
    instance->onMcpX4DriverSetPulsesPerStep = NULL;
    instance->onMcpX4DriverSetDownconversion = NULL;
    instance->onMcpX4DriverSetFrameArea = NULL;
    instance->onMcpX4DriverSetFrameAreaOffset = NULL;
    instance->onMcpX4DriverInit = NULL;
    instance->onMcpX4DriverSetDacStep = NULL;
    instance->onMcpX4DriverSetDacMin = NULL;
    instance->onMcpX4DriverSetDacMax = NULL;
    instance->onMcpX4DriverSetEnable = NULL;
    instance->onMcpX4DriverSetTxCenterFrequency = NULL;
    instance->onMcpX4DriverSetTxPower = NULL;
    instance->onMcpX4DriverGetFps = NULL;
    instance->onMcpX4DriverSetSpiRegister = NULL;
    instance->onMcpX4DriverGetSpiRegister = NULL;
    instance->onMcpX4DriverSetPifRegister = NULL;
    instance->onMcpX4DriverGetPifRegister = NULL;
    instance->onMcpX4DriverSetXifRegister = NULL;
    instance->onMcpX4DriverGetXifRegister = NULL;
    instance->onMcpX4DriverSetPrfDiv = NULL;
    instance->onMcpX4DriverGetPrfDiv = NULL;
    instance->onMcpX4DriverGetFrameArea = NULL;
    instance->onMcpX4DriverGetFrameAreaOffset = NULL;

    instance->onMcpUnknown = NULL;

    return instance;
}

static uint8_t extract_byte(uint8_t* data, uint32_t* index)
{
    uint8_t val = *(data + *index);
    *index += sizeof(uint8_t);
    return val;
}

static uint32_t extract_uint32(uint8_t* data, uint32_t* index)
{
    uint32_t* pval = (uint32_t*)(void*)(data + *index);
    uint32_t val = *pval;
    *index += sizeof(uint32_t);
    return val;
}

static float extract_float(uint8_t* data, uint32_t* index)
{
    float* pval = (float*)(void*)(data + *index);
    *index += sizeof(float);
    float val = *pval;
    return val;
}

int mcpParseMessage(McpParser_t* mcp_parser, uint8_t* data, uint32_t length)
{
    uint32_t index = 0;
    uint8_t command = extract_byte(data, &index);

    if (command == XTS_SPC_PING)
    {
        uint32_t pingval = extract_uint32(data, &index);
        if (mcp_parser->onMcpPing)
        {
            mcp_parser->onMcpPing(pingval, mcp_parser->user_data);
            return 0;
        }
    }
    else if (command == XTS_SPC_START_BOOTLOADER)
    {
        uint32_t key = extract_uint32(data, &index);
        if (mcp_parser->onMcpStartBootloader)
        {
            mcp_parser->onMcpStartBootloader(key, mcp_parser->user_data);
            return 0;
        }
    }
    else if (command == XTS_SPC_MOD_SETLEDCONTROL)
    {
        uint8_t led_mode = extract_byte(data, &index);
        uint8_t intensity = extract_byte(data, &index);
        if (mcp_parser->onMcpSetLedControl)
        {
            mcp_parser->onMcpSetLedControl(led_mode, intensity, mcp_parser->user_data);
            return 0;
        }
    }
    else if (command == XTS_SPC_MOD_RESET)
    {
        if (mcp_parser->onMcpModuleReset)
        {
            mcp_parser->onMcpModuleReset(mcp_parser->user_data);
            return 0;
        }
    }
    else if (command == XTS_SPC_DIR_COMMAND)
    {
        uint8_t direct_command = extract_byte(data, &index);
        if (direct_command == XTS_SDC_SYSTEM_TEST)
        {
            uint8_t testcode = extract_byte(data, &index);
            if (mcp_parser->onMcpSystemRunTest)
            {
                mcp_parser->onMcpSystemRunTest(testcode, mcp_parser->user_data);
                return 0;
            }
        }
        else if (direct_command == XTS_SDC_SYSTEM_GET_INFO)
        {
            uint8_t infocode = extract_byte(data, &index);
            if (mcp_parser->onMcpGetSystemInfo)
            {
                mcp_parser->onMcpGetSystemInfo(direct_command, infocode, mcp_parser->user_data);
                return 0;
            }
        }
    }
    else if (command == XTS_SPC_HIL)
    {
        if (mcp_parser->onMcpUnknown)
        {
            mcp_parser->onMcpUnknown(data, length, mcp_parser->user_data);
            return 0;
        }
    }
    else if (command == XTS_SPC_MOD_LOADAPP)
    {
        uint32_t profileid = extract_uint32(data, &index);
        if (mcp_parser->onMcpLoadProfile)
        {
            mcp_parser->onMcpLoadProfile(profileid, mcp_parser->user_data);
            return 0;
        }
    }
    else if (command == XTS_SPC_APPCOMMAND)
    {
        uint8_t app_command = extract_byte(data, &index);
        if (app_command == XTS_SPCA_SET)
        {
            uint32_t app_command_object = extract_uint32(data, &index);
            if (app_command_object == XTS_ID_DETECTION_ZONE)
            {
                float start, end;
                start = extract_float(data, &index);
                end = extract_float(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpProfileSetDetectionZone)
                {
                    mcp_parser->onMcpProfileSetDetectionZone(start, end, mcp_parser->user_data);
                    return 0;
                }
            }
            if (app_command_object == XTS_ID_SENSITIVITY)
            {
                uint32_t sensitivity = extract_uint32(data, &index);
                if (mcp_parser->onMcpProfileSetSensitivity)
                {
                    mcp_parser->onMcpProfileSetSensitivity(sensitivity, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (app_command_object == XTS_ID_PROFILE_PARAMETERFILE)
            {
                uint32_t filename_length = extract_uint32(data, &index);
                uint32_t data_length = extract_uint32(data, &index);
                unsigned char * filename = (unsigned char *)(data + index);
                index += filename_length;
                unsigned char * mdata = (unsigned char *)(data + index);

                // Send to application layer.
                if (mcp_parser->onMcpProfileSetProfileParameterFile)
                {
                    mcp_parser->onMcpProfileSetProfileParameterFile(filename_length, data_length, filename, mdata, mcp_parser->user_data);
                    return 0;
                }
            }
        }
        else if (app_command == XTS_SPCA_GET)
        {
            uint32_t app_command_object = extract_uint32(data, &index);
            if (app_command_object == XTS_ID_PROFILE_PARAMETERFILE)
            {
                uint32_t filename_length = extract_uint32(data, &index);
                unsigned char * filename = (unsigned char *)(data + index);

                if (mcp_parser->onMcpProfileGetProfileParameterFile)
                {
                    mcp_parser->onMcpProfileGetProfileParameterFile(filename_length, filename, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (app_command_object == XTS_ID_DETECTION_ZONE)
            {
                if (mcp_parser->onMcpProfileGetDetectionZone)
                {
                    mcp_parser->onMcpProfileGetDetectionZone(mcp_parser->user_data);
                    return 0;
                }
            }
            else if (app_command_object == XTS_ID_DETECTION_ZONE_LIMITS)
            {
                if (mcp_parser->onMcpProfileGetDetectionZoneLimits)
                {
                    mcp_parser->onMcpProfileGetDetectionZoneLimits(mcp_parser->user_data);
                    return 0;
                }
            }
        }
    }    
    else if (command == XTS_SPC_IOPIN)
    {
        uint8_t iopin_command = extract_byte(data, &index);
        if (iopin_command == XTS_SPCIOP_SETCONTROL)
        {
            uint32_t pin_id = extract_uint32(data, &index);
            uint32_t pin_setup = extract_uint32(data, &index);
            uint32_t pin_feature = extract_uint32(data, &index);

            // Send to application layer.
            if (mcp_parser->onMcpSetIOPinControl)
            {
                mcp_parser->onMcpSetIOPinControl(pin_id, pin_setup, pin_feature, mcp_parser->user_data);
                return 0;
            }
        }
        else if (iopin_command == XTS_SPCIOP_SETVALUE)
        {
            uint32_t pin_id = extract_uint32(data, &index);
            uint32_t pin_value = extract_uint32(data, &index);

            // Send to application layer.
            if (mcp_parser->onMcpSetIOPinValue)
            {
                mcp_parser->onMcpSetIOPinValue(pin_id, pin_value, mcp_parser->user_data);
                return 0;
            }
        }
        else if (iopin_command == XTS_SPCIOP_GETVALUE)
        {
            uint32_t pin_id = extract_uint32(data, &index);

            // Send to application layer.
            if (mcp_parser->onMcpGetIOPinValue)
            {
                mcp_parser->onMcpGetIOPinValue(pin_id, mcp_parser->user_data);
                return 0;
            }
        }
    }
    else if (command == XTS_SPC_OUTPUT)
    {
        uint8_t output_command = extract_byte(data, &index);
        if (output_command == XTS_SPCO_SETCONTROL)
        {
            uint32_t output_feature = extract_uint32(data, &index);
            uint32_t output_control = extract_uint32(data, &index);

            // Send to application layer.
            if (mcp_parser->onMcpSetOutputControl)
            {
                mcp_parser->onMcpSetOutputControl(output_feature, output_control, mcp_parser->user_data);
                return 0;
            }
        }
    }
    else if (command == XTS_SPC_X4DRIVER)
    {
        uint8_t x4driver_command = extract_byte(data, &index);
        if (x4driver_command == XTS_SPCX_INIT)
        {
            // Send to application layer.
            if (mcp_parser->onMcpX4DriverInit)
            {
                mcp_parser->onMcpX4DriverInit(mcp_parser->user_data);
                return 0;
            }
        }
        else if (x4driver_command == XTS_SPCX_SET)
        {
            uint32_t x4driver_id = extract_uint32(data, &index);
            if (x4driver_id == XTS_SPCXI_FPS)
            {
                uint32_t fps = extract_uint32(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetFps)
                {
                    mcp_parser->onMcpX4DriverSetFps(fps, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_ITERATIONS)
            {
                uint32_t iterations = extract_uint32(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetIterations)
                {
                    mcp_parser->onMcpX4DriverSetIterations(iterations, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_PULSESPERSTEP)
            {
                uint32_t pulsesperstep = extract_uint32(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetPulsesPerStep)
                {
                    mcp_parser->onMcpX4DriverSetPulsesPerStep(pulsesperstep, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_DOWNCONVERSION)
            {
                uint8_t downconversion = extract_byte(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetDownconversion)
                {
                    mcp_parser->onMcpX4DriverSetDownconversion(downconversion, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_FRAMEAREA)
            {
                float start, end;
                start = extract_float(data, &index);
                end = extract_float(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetFrameArea)
                {
                    mcp_parser->onMcpX4DriverSetFrameArea(start, end, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_FRAMEAREAOFFSET)
            {
                float offset;
                offset = extract_float(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetFrameAreaOffset)
                {
                    mcp_parser->onMcpX4DriverSetFrameAreaOffset(offset, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_DACSTEP)
            {
                uint32_t dac_step = extract_uint32(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetDacStep)
                {
                    mcp_parser->onMcpX4DriverSetDacStep(dac_step, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_DACMIN)
            {
                uint32_t dac_min = extract_uint32(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetDacMin)
                {
                    mcp_parser->onMcpX4DriverSetDacMin(dac_min, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_DACMAX)
            {
                uint32_t dac_max = extract_uint32(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetDacMax)
                {
                    mcp_parser->onMcpX4DriverSetDacMax(dac_max, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_ENABLE)
            {
                uint8_t enable = extract_byte(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetEnable)
                {
                    mcp_parser->onMcpX4DriverSetEnable(enable, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_TXCENTERFREQUENCY)
            {
                uint8_t tx_center_frequency = extract_byte(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetTxCenterFrequency)
                {
                    mcp_parser->onMcpX4DriverSetTxCenterFrequency(tx_center_frequency, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_TXPOWER)
            {
                uint8_t tx_power = extract_byte(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetTxPower)
                {
                    mcp_parser->onMcpX4DriverSetTxPower(tx_power, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_SPIREGISTER)
            {
                uint8_t address = extract_byte(data, &index);
                uint8_t value = extract_byte(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetSpiRegister)
                {
                    mcp_parser->onMcpX4DriverSetSpiRegister(address, value, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_PIFREGISTER)
            {
                uint8_t address = extract_byte(data, &index);
                uint8_t value = extract_byte(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetPifRegister)
                {
                    mcp_parser->onMcpX4DriverSetPifRegister(address, value, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_XIFREGISTER)
            {
                uint8_t address = extract_byte(data, &index);
                uint8_t value = extract_byte(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetXifRegister)
                {
                    mcp_parser->onMcpX4DriverSetXifRegister(address, value, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_PRFDIV)
            {
                uint8_t prf_div = extract_byte(data, &index);

                // Send to application layer.
                if (mcp_parser->onMcpX4DriverSetPrfDiv)
                {
                    mcp_parser->onMcpX4DriverSetPrfDiv(prf_div, mcp_parser->user_data);
                    return 0;
                }
            }
        }
        else if (x4driver_command == XTS_SPCX_GET)
        {
            uint32_t x4driver_id = extract_uint32(data, &index);
            if (x4driver_id == XTS_SPCXI_FPS)
            {
                // Send to application layer.
                if (mcp_parser->onMcpX4DriverGetFps)
                {
                    mcp_parser->onMcpX4DriverGetFps(mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_FRAMEAREA)
            {
                if (mcp_parser->onMcpX4DriverGetFrameArea)
                {
                    mcp_parser->onMcpX4DriverGetFrameArea(mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_FRAMEAREAOFFSET)
            {
                if (mcp_parser->onMcpX4DriverGetFrameAreaOffset)
                {
                    mcp_parser->onMcpX4DriverGetFrameAreaOffset(mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_SPIREGISTER)
            {
                uint8_t address = extract_byte(data, &index);
                // Send to application layer.
                if (mcp_parser->onMcpX4DriverGetSpiRegister)
                {
                    mcp_parser->onMcpX4DriverGetSpiRegister(address, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_PIFREGISTER)
            {
                uint8_t address = extract_byte(data, &index);
                // Send to application layer.
                if (mcp_parser->onMcpX4DriverGetPifRegister)
                {
                    mcp_parser->onMcpX4DriverGetPifRegister(address, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_XIFREGISTER)
            {
                uint8_t address = extract_byte(data, &index);
                // Send to application layer.
                if (mcp_parser->onMcpX4DriverGetXifRegister)
                {
                    mcp_parser->onMcpX4DriverGetXifRegister(address, mcp_parser->user_data);
                    return 0;
                }
            }
            else if (x4driver_id == XTS_SPCXI_PRFDIV)
            {
                // Send to application layer.
                if (mcp_parser->onMcpX4DriverGetPrfDiv)
                {
                    mcp_parser->onMcpX4DriverGetPrfDiv(mcp_parser->user_data);
                    return 0;
                }
            }
        }
    }
    else if (command == XTS_SPC_MOD_SETMODE)
    {
        uint8_t mode = extract_byte(data, &index);
        uint8_t param = extract_byte(data, &index);
        if (mcp_parser->onMcpSetSensorMode)
        {
            mcp_parser->onMcpSetSensorMode(mode, param, mcp_parser->user_data);
            return 0;
        }
    }

    // If this code is reached, no parsing was successful.
    if (mcp_parser->onMcpUnknown)
    {
        mcp_parser->onMcpUnknown(data, length, mcp_parser->user_data);
        return 0;
    }

    return -1;
}



#ifdef __cplusplus
}
#endif /* __cplusplus */

/*



// <Start> + <XTS_SPR_APPDATA> + [XTS_ID_RESP_STATUS(i)] + [Counter(i)] + [StateCode(i)] + [StateData(i)] + [Distance(f)] + [Movement(f)] + [SignalQuality(i)] + <CRC> + <End>
typedef struct xtDatamsgRespStatus
{
    uint32_t frameCtr;
    uint32_t stateCode;
    uint32_t stateData;
    float32_t distance;
    float32_t movement;
    uint32_t signalQuality;
} xtDatamsgRespStatus_t;

// <Start> + <XTS_SPR_APPDATA> + [XTS_ID_SLEEP_STATUS(i)] + [Counter(i)] + [StateCode(i)] + [StateData(f)] + [Distance(f)] [SignalQuality(i)] + [MovementSlow(f)] + [MovementFast(f)] <CRC> + <End>
typedef struct xtDatamsgSleepStatus
{
    uint32_t frameCtr;
    uint32_t stateCode;
    float32_t stateData;
    float32_t distance;
    uint32_t signalQuality;
    float32_t movementSlow;
    float32_t movementFast;
} xtDatamsgSleepStatus_t;

*/
