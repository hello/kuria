#include "matlabreader.h"
#include "matlabwriter.h"
#include <iostream>
#include "preprocessorIIR.h"
#include "preprocessor.h"
#include "novelda_radar_subscriber.h"
#include "debug_publisher_interface.h"
#include "respiration.h"
#include "pca.h"
#include "peakFinding.h"


using namespace Eigen;

#define EXPECTED_SAMPLE_RATE_HZ (20)
#define NUM_FRAMES_IN_SEGMENT (20 * EXPECTED_SAMPLE_RATE_HZ)
#define NUM_FRAMES_TO_WAIT (20 * EXPECTED_SAMPLE_RATE_HZ)

class ResultPublisher : public RadarResultPublisherInterface {
public:
    void publish(const char * prefix, const RadarMessage_t & message) {
        
    }
    
};

class MatlabDebugPublisher : public DebugPublisherInterface {
    
public:
    MatlabDebugPublisher(const std::string & output_file) {
        open_new_matfile(output_file);
    }
    
    void publish(const std::string & id, const Eigen::MatrixXcf & mat) {
        write_matrix_to_cell_array(id,mat);
    }
    
    void publish(const std::string & id, const Eigen::MatrixXf & mat) {
        write_matrix_to_cell_array(id,mat);
    }
};

int main(int argc, char * argv[]) {
    
    //TODO read input CSV file or whatever format the radar is
    if (argc <= 2) {
        std::cerr << "you need to specify an input and output file" << std::endl;
        return 0;
    }
    
    
    MatrixXcf baseband;
    if (!MatlabReader::read_baseband_from_file_v1(argv[1], baseband)) {
        std::cerr << "error reading baseband" << std::endl;
        return 0;
    }
    
    NoveldaRadarSubsciberConfig novelda_config;
    novelda_config.min_range_bin = 8;
    novelda_config.max_range_bin = 70;

    
    //create radar data subscriber and result publisher
    NoveldaRadarSubscriber subscriber(novelda_config,new ResultPublisher(),new MatlabDebugPublisher(argv[2]));
    
    for (int iframe = 0; iframe < baseband.rows(); iframe++) {
        
        NoveldaData_t novelda_data;
        
        novelda_data.is_base_band = true;
        novelda_data.frame_id = iframe;
        novelda_data.range_bins.reserve(baseband.cols()*2);
        
        for (int irangebin = 0; irangebin < baseband.cols(); irangebin++) {
            Complex128_t complex_data(baseband(iframe,irangebin).real(),baseband(iframe,irangebin).imag());
            novelda_data.range_bins.push_back(complex_data);
        }
        
        subscriber.receive_message(novelda_data);
        
    }
    
    return 0;
}
