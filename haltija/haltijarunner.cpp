#include "example.h"
#include "matlabreader.h"
#include "matlabwriter.h"
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


	Pca pca(baseband);

	pca.fit();

	pca.getTransformedData();

	//Process data
	//CALL FUNCTION FROM HALTIJA LIBRARY
	MatlabWriter::get_instance()->open_new_matfile(argv[2]);

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
