#include "example.h"
#include "matlabreader.h"
#include "matlabwriter.h"
#include "filters.h"
#include <iostream>
#include "Pca.h"

using Eigen::MatrixXcf;

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

    write_matrix("baseband", baseband);

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
