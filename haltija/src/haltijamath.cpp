#include "haltijamath.h"
#include <assert.h>
#include "pca.h"

using namespace Eigen;
 
MatrixXf HaltijaMath::project_complex_cols_into_reals(const MatrixXcf & c) {
    
    MatrixXf complex_column = MatrixXf::Zero(c.rows(),2);
    MatrixXf reals(c.rows(),c.cols());
    
    for (int icol = 0; icol < c.cols(); icol++) {
        
        complex_column.col(0) = c.col(icol).real();
        complex_column.col(1) = c.col(icol).imag();
        
        MatrixXf principal_component_vec;
        MatrixXf transform;
        MatrixXf transformed_values;
        
        pca(complex_column,principal_component_vec,transform,transformed_values);
        
        assert (principal_component_vec(0,0) < principal_component_vec(1,0));
        reals.col(icol) = transformed_values.col(1);
    }
    
    return reals;
}
