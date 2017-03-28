#include "matlabreader.h"
#include "matlabwriter.h"
#include <iostream>
#include "preprocessorIIR.h"
#include "preprocessor.h"
#include "rangebincombiner.h"

#include "respiration.h"
#include "pca.h"
#include "peakFinding.h"


using namespace Eigen;

#define EXPECTED_SAMPLE_RATE_HZ (20)
#define NUM_FRAMES_IN_SEGMENT (20 * EXPECTED_SAMPLE_RATE_HZ)
#define NUM_FRAMES_TO_WAIT (20 * EXPECTED_SAMPLE_RATE_HZ)

int main(int argc, char * argv[]) {
    
    IntSet_t range_bins_we_care_about;
    
    for (int i = 8; i < 38; i++) {
        range_bins_we_care_about.insert(i);
    }
    
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
    
    RangebinCombiner combiner;

    std::cout << baseband.rows() << " x " << baseband.cols() << std::endl;
    
    
    Peakfinder respirationpeaks;
    MatrixXf temp(1,300);
    //float temp;
    int peak_ind = 0;
    int iframe_pros = 0; // index for processored frames

    for (int iframe = 0; iframe < baseband.rows(); iframe++) {
        
        BasebandDataFrame_t frame;
        frame.data.reserve(baseband.cols());
        frame.timestamp = iframe * 1000 / EXPECTED_SAMPLE_RATE_HZ; //timestamp in ms
        for (int irangebin = 0; irangebin < baseband.cols(); irangebin++) {
            frame.data.push_back(baseband(iframe,irangebin));
        }
        
        Eigen::MatrixXcf filtered_frame;
        Eigen::MatrixXcf segment;
        
        uint32_t flags = preprocessor->add_frame(frame, filtered_frame, segment);
        
        //DO KALMAN FILTERS HERE
        if (~flags & PREPROCESSOR_FLAGS_FRAME_READY) {
            continue;
        }
        
        if (flags & PREPROCESSOR_FLAGS_SEGMENT_READY) {
            //DO FRAME PROCESSING HERE
            write_matrix_to_cell_array("seg",segment);

            combiner.set_latest_segment(segment, range_bins_we_care_about);
            
            std::cout << "frame " << iframe << std::endl;
        }
        
        
        MatrixXcf transformed_frame;
        MatrixXf filtered_sample;
        if (combiner.get_latest_reduced_measurement(filtered_frame, transformed_frame)) {
            write_matrix_to_cell_array("t",transformed_frame);
            //std::cout << "t " << transformed_frame.real() << std::endl;
            
            // added scaling here to test.
            if (respirationpeaks.isPeak(1e-6 * transformed_frame, iframe_pros, filtered_sample)) {
                
                if (peak_ind <= 300) {
                    temp(0,peak_ind) = (float)iframe_pros;
                    peak_ind = peak_ind + 1;
                    //std::cout << " extrema " << peak_ind << " :";
                }
                
            }
            iframe_pros++;
            write_matrix_to_cell_array("filt",filtered_sample);
            
        }
        
    }
    write_matrix_to_cell_array("temp", temp);
    
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
