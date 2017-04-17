#ifndef _ALLMODELS_H_
#define _ALLMODELS_H_

#include "HmmPdfInterface.h"


class GammaModel : public HmmPdfInterface {
public:
    GammaModel(const int32_t obsnum,const float mean, const float stddev,const float weight);
    ~GammaModel();
    
    HmmPdfInterfaceSharedPtr_t reestimate(const HmmDataVec_t & gammaForThisState, const HmmDataMatrix_t & meas, const HmmFloat_t eta) const;
    HmmDataVec_t getLogOfPdf(const HmmDataMatrix_t & x) const;
    std::string serializeToJson() const;
    uint32_t getNumberOfFreeParams() const;

private:
    const float _mean;
    const float _stddev;
    const int32_t _obsnum;
    const HmmFloat_t _weight;

};

///////////////////////////////////

class PoissonModel : public HmmPdfInterface {
public:
    PoissonModel(const int32_t obsnum,const float mu,const float weight);
    ~PoissonModel();
    
    HmmPdfInterfaceSharedPtr_t reestimate(const HmmDataVec_t & gammaForThisState, const HmmDataMatrix_t & meas, const HmmFloat_t eta) const;
    HmmDataVec_t getLogOfPdf(const HmmDataMatrix_t & x) const;
    std::string serializeToJson() const;
    uint32_t getNumberOfFreeParams() const;

private:
    const int32_t _obsnum;
    const float _mu;
    const HmmFloat_t _weight;

};

////////////////////////////////////
class ChiSquareModel : public HmmPdfInterface {
public:
    ChiSquareModel(const int32_t obsnum,const float mu,const float weight);
    ~ChiSquareModel();
    
    HmmPdfInterfaceSharedPtr_t reestimate(const HmmDataVec_t & gammaForThisState, const HmmDataMatrix_t & meas, const HmmFloat_t eta) const;
    HmmDataVec_t getLogOfPdf(const HmmDataMatrix_t & x) const;
    std::string serializeToJson() const;
    uint32_t getNumberOfFreeParams() const;
    
private:
    const int32_t _obsnum;
    const float _mu;
    const HmmFloat_t _weight;
    
};
////////////////////////////////////


class AlphabetModel : public HmmPdfInterface {
public:
    AlphabetModel(const int32_t obsnum,const HmmDataVec_t alphabetprobs,bool allowreestimation,const float weight);
    ~AlphabetModel();
    
    HmmPdfInterfaceSharedPtr_t reestimate(const HmmDataVec_t  & gammaForThisState, const HmmDataMatrix_t & meas, const HmmFloat_t eta) const;
    HmmDataVec_t getLogOfPdf(const HmmDataMatrix_t & x) const ;
    std::string serializeToJson() const;
    uint32_t getNumberOfFreeParams() const;

    
private:
    const int32_t _obsnum;
    const HmmDataVec_t _alphabetprobs;
    const bool _allowreestimation;
    const HmmFloat_t _weight;
    
};

////////////////////////////////////

class OneDimensionalGaussianModel : public HmmPdfInterface {
public:
    OneDimensionalGaussianModel(const int32_t obsnum,const float mean, const float stddev, const float weight);
    ~OneDimensionalGaussianModel();
    
    HmmPdfInterfaceSharedPtr_t reestimate(const HmmDataVec_t  & gammaForThisState, const HmmDataMatrix_t & meas, const HmmFloat_t eta) const;
    HmmDataVec_t getLogOfPdf(const HmmDataMatrix_t & x) const ;
    std::string serializeToJson() const;
    uint32_t getNumberOfFreeParams() const;
    
    
private:
    const float _mean;
    const float _stddev;
    const int32_t _obsnum;
    const HmmFloat_t _weight;
    
};


class MultivariateGaussian : public HmmPdfInterface {
public:
    MultivariateGaussian(const UIntVec_t obsnums,const HmmDataVec_t & mean, const HmmDataMatrix_t & cov, const float minstddev, const float weight);
    ~MultivariateGaussian();
    
    HmmPdfInterfaceSharedPtr_t reestimate(const HmmDataVec_t  & gammaForThisState, const HmmDataMatrix_t & meas, const HmmFloat_t eta) const;
    HmmDataVec_t getLogOfPdf(const HmmDataMatrix_t & x) const ;
    std::string serializeToJson() const;
    uint32_t getNumberOfFreeParams() const;
    
    
private:
    const HmmDataVec_t _mean;
    const HmmDataMatrix_t _covariance;
    const UIntVec_t _obsnums;
    const HmmFloat_t _weight;
    const HmmFloat_t _minstddev;
    
};

#endif //_ALLMODELS_H_
