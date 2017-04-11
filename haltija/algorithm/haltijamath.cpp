#include "haltijamath.h"
#include <assert.h>
#include "pca.h"
#include <iostream>

using namespace Eigen;
 
MatrixXf HaltijaMath::project_complex_cols_into_reals(const MatrixXcf & c) {
    
    MatrixXf complex_column = MatrixXf::Zero(c.rows(),2);
    MatrixXf reals(c.rows(),c.cols());
    
    for (int icol = 0; icol < c.cols(); icol++) {
        
        complex_column.col(0) = c.col(icol).real();
        complex_column.col(1) = c.col(icol).imag();
        

        
        Pca<MatrixXf> pca;
        pca.fit(complex_column);

        MatrixXf transformed_values = pca.get_transformed_values(complex_column);

        if (pca.get_principal_components()(0,0) > pca.get_principal_components()(1,0)) {
            std::cout << pca.get_principal_components() << std::endl;
            //this should never happen
            //but leaving this assert in here is bad bad bad,
            //TODO remove this
            assert (false);
        }
        

        reals.col(icol) = transformed_values.col(1);
    }
    
    return reals;
}
