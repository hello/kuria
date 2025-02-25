#include "ModuleConnector.hpp"
#include "XEP.hpp"
#include "Data.hpp"
#include "Datarecorder.hpp"
#include "datatypes.h"
#include <unistd.h>
#include <iostream>
#include "yaml-cpp/yaml.h"

using namespace XeThru;

#define SUCCESS_ERROR_CODE

#define CHECK_ERROR_OR_GOTO_CLEANUP(x)\
  if ( (x) != 0)  { \
      std::cout << "error with " << #x << std::endl;\
      goto CLEANUP;\
   }\


int main(int argc, char * argv[])
{
    
    if (argc < 2) {
        std::cerr << "requires config yml file, which must at least have the device field filled (/dev/ttyACM0, or something)" << std::endl;
        return 0;
    }
    
    uint32_t pulses_per_step = 550;
    uint32_t frame_rate = 20;
    uint32_t dac_max = 1100;
    uint32_t dac_min = 950;
    
    YAML::Node config = YAML::LoadFile(argv[1]);
    
    if (!config["device"]) {
        std::cerr << "device field not found in " << argv[1] << std::endl;
        return 0;
    }
    
    std::string device = config["device"].as<std::string>();

    
    if (config["pulses_per_step"]) {
        pulses_per_step = config["pulses_per_step"].as<uint32_t>();
    }
    
    if (config["frame_rate"]) {
        frame_rate = config["frame_rate"].as<uint32_t>();
    }
    
    if (config["dac_max"]) {
        dac_max = config["dac_max"].as<uint32_t>();
    }
    
    if (config["dac_min"]) {
        dac_min = config["dac_min"].as<uint32_t>();
    }
    

    
    std::cout << "pulses_per_step" << ": " << pulses_per_step << std::endl;
    std::cout << "frame_rate" << ": "  << frame_rate << std::endl;
    std::cout << "dac_max" << ": "  << dac_max << std::endl;
    std::cout << "dac_min" << ": "  << dac_min << std::endl;

    
    
    const unsigned int log_level = 3;
    ModuleConnector mc(device.c_str(), log_level);
    usleep(500000);
    //mc.open(device.c_str());
    mc.set_default_timeout(60);

    XEP & xep = mc.get_xep();

    usleep(500000);
    
    std::cout << "init..." << std::endl;
    CHECK_ERROR_OR_GOTO_CLEANUP(xep.x4driver_init());
    
    std::cout << "setting options..." << std::endl;
    CHECK_ERROR_OR_GOTO_CLEANUP(xep.x4driver_set_dac_max(1100));
    CHECK_ERROR_OR_GOTO_CLEANUP(xep.x4driver_set_dac_min(950));
    CHECK_ERROR_OR_GOTO_CLEANUP(xep.x4driver_set_enable(1));

    //CHECK_ERROR_OR_GOTO_CLEANUP(xep.x4driver_set_downconversion(1));
    CHECK_ERROR_OR_GOTO_CLEANUP(xep.x4driver_set_pulses_per_step(pulses_per_step));
    CHECK_ERROR_OR_GOTO_CLEANUP(xep.x4driver_set_fps(frame_rate));
    //CHECK_ERROR_OR_GOTO_CLEANUP(xep.x4driver_set_frame_area(0,2.0));
    //xep.x4driver_set_iterations(uint32_t iterations) ???


    
    
    usleep(500000);
    {
        uint32_t pong = 1234;
        CHECK_ERROR_OR_GOTO_CLEANUP(xep.ping(&pong));
        std::cout << pong << std::endl;

    }

    while (1) {
        
        uint32_t content_id = BasebandIqDataType;
        uint32_t info = 0;
        std::string data;
        
        std::cout << "WAITING FOR MESSAGE..." <<std::endl;
        std::cout << xep.peek_message_data_string() << std::endl;
        int code = xep.read_message_data_string(&content_id, &info, &data);
        
        std::cout << code << "," << content_id << "," << data << std::endl;
        
        usleep((0.5 / (float)frame_rate) * 1000000);
        
    }
    /*
    device.set_output_control(XTS_ID_BASEBAND_IQ,1);
    device.load_profile(XTS_VAL_RESP_STATE_BREATHING);
    device.set_sensor_mode(XTID_SM_RUN, 0);

    
    std::cout << "sleeping..." << std::endl;
    usleep(1000000);
    std::cout << "trying to read..." << std::endl;


    
    BasebandIqData data;
    device.read_message_baseband_iq(&data);
    */
    /*
    const std::string subscription_name = "baseband_subscription";
    x2m200.subscribe_to_baseband_ap(subscription_name); //subscribe to amplitude/phase baseband
    x2m200.load_sleep_profile(); // load a profile
    x2m200.enable_baseband_ap(); // turn on baseband output
    x2m200.set_sensor_mode_run(); // start
    
    usleep(1000000); // wait for some packets
    
    x2m200.set_sensor_mode_idle(); //stop
    if (x2m200.get_number_of_packets(subscription_name) > 0) {
        BasebandApData apdata;
        int val = x2m200.get_baseband_ap_data(subscription_name,&apdata); // get first packet
        std::cout << "frame counter: " << apdata.frame_counter << std::endl;
    }
    
    */
    
CLEANUP:
    usleep(1000000);

    return 0;
}
