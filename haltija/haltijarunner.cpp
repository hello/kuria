#include "example.h"
#include "matlabreader.h"
#include "matlabwriter.h"
#include <iostream>
#include "preprocessor.h"
#include "respiration.h"

using Eigen::MatrixXcf;

#define EXPECTED_SAMPLE_RATE_HZ (20)
#define NUM_FRAMES_IN_SEGMENT (20 * EXPECTED_SAMPLE_RATE_HZ)
#define NUM_FRAMES_TO_WAIT (20 * EXPECTED_SAMPLE_RATE_HZ)

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

	open_new_matfile(argv[2]);

    
    PreprocessorPtr_t preprocessor = Preprocessor::createWithDefaultHighpassFilter(baseband.cols(), NUM_FRAMES_IN_SEGMENT, NUM_FRAMES_TO_WAIT);

    std::cout << baseband.rows() << " x " << baseband.cols() << std::endl;
    
    for (int iframe = 0; iframe < baseband.rows(); iframe++) {
        
        BasebandDataFrame_t frame;
        frame.data.reserve(baseband.cols());
        frame.timestamp = iframe * 1000 / EXPECTED_SAMPLE_RATE_HZ; //timestamp in ms
        for (int irangebin = 0; irangebin < baseband.cols(); irangebin++) {
            frame.data.push_back(baseband(iframe,irangebin));
        }
        
        Eigen::MatrixXcf filtered_frame;
        Eigen::MatrixXcf segment;
        
        bool is_segment = preprocessor->add_frame(frame, filtered_frame, segment);
        
        //DO KALMAN FILTERS HERE
        
        if (is_segment) {
            //DO FRAME PROCESSING HERE
            write_matrix_to_cell_array("segments",segment);

            IntVec_t possible_range_bins = get_possible_respiration_range_bins(segment);
            
            Eigen::MatrixXf features = get_respiration_features(segment, possible_range_bins);
            
            write_matrix_to_cell_array("features",features);

            std::cout << "frame " << iframe << std::endl;
        }
    }
    
    /*
    
    for (int iframe = 0; iframe < NFRAMES; i++) {
        if (!extract_frames(baseband,iframe,num_frames,framedata))  //extracts 20 seconds of dtata{
            break;
        }
        
        //have 20 seconds of data, now what?
        //1) apply high pass filter ("MTI" filter, which might be a differencing???) ?????
    
        //2)  movement detection (large/small/none)  if large, don't bother doing respiration detection.  Also outputs "interesting range bins" (this does not yet exist)
    
        //3) subtract mean, 2x2 PCA per range bin to make complex numbers real numbers
        //   outputs real data, one channel per range bin
    
    
        //4) have say... 9 streams of real valued bins which probably have respiration in them
        //   run LPF, find peaks (look for zero 1st derivitive), count, measure interval variation, etc.
    
        //   Has some rules to filter out bad data (local maximums) by examining peak amplitude vs 1st derivitive.
    
    
    
    }

    */
    
	/*DONT REMOVE THIS!!!!!
	std::cout << baseband.block(0, 0, 100, baseband.cols()) << std::endl;
	*/

/*
	Pca pca(baseband);

	pca.fit();

	pca.getTransformedData();
	*/
	//Process data
	//CALL FUNCTION FROM HALTIJA LIBRARY

	/*
	Eigen::MatrixXf floatmatrix(20, 20);
	MatlabWriter::get_instance()->write_new_matrix("name_of_item", floatmatrix);

	struct foobars {
		int a;
	};

	foobars mystruct;
	foobars * p = &mystruct;

	p->a = 3;
	
	(*p).a = 3;

	*/
  return 0;
}
