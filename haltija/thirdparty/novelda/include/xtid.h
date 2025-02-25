#ifndef XTID_XEP_H
#define XTID_XEP_H

#ifdef __cplusplus
extern "C" {
#endif

// Profile IDs
#define XTS_ID_APP_UNKNOWN          0x0
#define XTS_ID_APP_PRESENCE         0x00288912
#define XTS_ID_APP_RESP             0x1423a2d6
#define XTS_ID_APP_SLEEP            0x00f17b17
#define XTS_ID_APP_DECIM            0x9bb3a2c6
#define XTS_ID_APP_PRESENCE_2       0x014d4ab8

// Profile data and feature IDs
#define XTS_ID_DETECTION_ZONE                           0x96a10a1c
#define XTS_ID_DETECTION_ZONE_LIMITS                    0x96a10a1d
#define XTS_ID_SENSITIVITY                              0x10a5112b
#define XTS_ID_PRESENCE_STATUS                          0x991a52be
#define XTS_ID_RESP_STATUS                              0x2375fe26
#define XTS_ID_RESP_STATUS_EXT                          0x2375a16b
#define XTS_ID_SLEEP_STATUS                             0x2375a16c
#define XTS_ID_PRESENCE_SINGLE                        0x723bfa1e
#define XTS_ID_PRESENCE_MOVINGLIST                    0x723bfa1f
#define XTS_ID_PROFILE_PARAMETERFILE                    0x32ba7623
#define XTS_ID_BASEBAND_IQ                              0x0000000c
#define XTS_ID_BASEBAND_AMPLITUDE_PHASE                 0x0000000d
#define XTS_ID_DECIM_STATUS                             0x327645aa
#define XTS_ID_PULSEDOPPLER_FLOAT                       0x00000010
#define XTS_ID_PULSEDOPPLER_BYTE                        0x00000011
#define XTS_ID_NOISEMAP_FLOAT                           0x00000012
#define XTS_ID_NOISEMAP_BYTE                            0x00000013

// System Info Code definitions
#define XTID_SSIC_ITEMNUMBER         (0x00)
#define XTID_SSIC_ORDERCODE          (0x01)
#define XTID_SSIC_FIRMWAREID         (0x02)
#define XTID_SSIC_VERSION            (0x03)
#define XTID_SSIC_BUILD              (0x04)
#define XTID_SSIC_SERIALNUMBER       (0x06)
#define XTID_SSIC_VERSIONLIST        (0x07)

// Sensor mode IDs
#define XTID_SM_RUN                  (0x01)
#define XTID_SM_NORMAL               (0x10)
#define XTID_SM_IDLE                 (0x11)
#define XTID_SM_MANUAL               (0x12)
#define XTID_SM_STOP                 (0x13)


// IOPin definitions
#define XTID_IOPIN_SETUP_INPUT         (0)
#define XTID_IOPIN_SETUP_OUTPUT        (1)

#define XTID_IOPIN_FEATURE_DISABLE     (0)
#define XTID_IOPIN_FEATURE_DEFAULT     (1)
#define XTID_IOPIN_FEATURE_PASSIVE     (2)


// Output control
#define XTID_OUTPUT_CONTROL_DISABLE    (0)
#define XTID_OUTPUT_CONTROL_ENABLE     (1)

// Led control
#define XTID_LED_MODE_OFF       (0)
#define XTID_LED_MODE_SIMPLE    (1)
#define XTID_LED_MODE_FULL      (2)


// Serial protocol IDs
#define XTID_MCP_ERROR_NOT_RECOGNIZED   (1)


// Profile codes
#define XTS_VAL_RESP_STATE_BREATHING			0x00 // Valid RPM sensing
#define XTS_VAL_RESP_STATE_MOVEMENT				0x01 // Detects motion, but can not identify breath
#define XTS_VAL_RESP_STATE_MOVEMENT_TRACKING	0x02 // Detects motion, possible breathing soon
#define XTS_VAL_RESP_STATE_NO_MOVEMENT			0x03 // No movement detected
#define XTS_VAL_RESP_STATE_INITIALIZING			0x04 // Initializing sensor
#define XTS_VAL_RESP_STATE_ERROR				0x05 // Sensor has detected some problem. StatusValue indicates problem.
#define XTS_VAL_RESP_STATE_UNKNOWN				0x06 // Undefined state.

#define XTS_VAL_PRESENCE_PRESENCESTATE_NO_PRESENCE      0 // No presence detected
#define XTS_VAL_PRESENCE_PRESENCESTATE_PRESENCE         1 // Presence detected
#define XTS_VAL_PRESENCE_PRESENCESTATE_INITIALIZING     2 // The sensor initializes after the Presence 2 Profile is executed
#define XTS_VAL_PRESENCE_PRESENCESTATE_UNKNOWN          3 // The sensor is in an unknown state and requires a Profile and User Settings to be loaded



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XTID_XEP_H */
