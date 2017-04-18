#ifndef _HMMHELPERS_H_
#define _HMMHELPERS_H_

#include "HmmPdfInterface.h"


class ViterbiDecodeResult_t {
public:
    ViterbiDecodeResult_t(const ViterbiPath_t & vpath,const TransitionMap_t & transitions, const HmmFloat_t vcost, const HmmFloat_t bicScore)
    :_path(vpath)
    ,_transitions(transitions)
    ,_cost(vcost)
    ,_bic(bicScore)
    {
    
    
    }
    
    ViterbiDecodeResult_t() : _path(ViterbiPath_t()), _cost(-INFINITY),_bic(-INFINITY) {}
    
    const ViterbiPath_t & getPath() const {
        return _path;
    }
    
    const TransitionMap_t & getTransitions() const {
        return _transitions;
    }
    
    HmmFloat_t getCost() const {
        return _cost;
    }
    
    HmmFloat_t getBIC() const {
        return _bic;
    }
    
    
private:
    
    ViterbiPath_t _path;
    TransitionMap_t _transitions;
    HmmFloat_t _cost;
    HmmFloat_t _bic;
};

class AlphaBetaResult_t  {
public:
    
    AlphaBetaResult_t(const HmmDataMatrix_t & a, const HmmDataMatrix_t & b,const HmmDataMatrix_t & la, HmmFloat_t c)
    : logalpha(a)
    , logbeta(b)
    , logA(la)
    , logmodelcost(c)
    {}
    
    const HmmDataMatrix_t logalpha;
    const HmmDataMatrix_t logbeta;
    const HmmDataMatrix_t logA;
    const HmmFloat_t logmodelcost;
} ;

class HmmHelpers {
public:
    static AlphaBetaResult_t getAlphaAndBeta(int32_t numObs,const HmmDataVec_t & pi, const HmmDataMatrix_t & logbmap, const HmmDataMatrix_t & A,const uint32_t numStates);
    
    
    static Hmm3DMatrix_t getLogXi(const AlphaBetaResult_t & alphabeta,const HmmDataMatrix_t & logbmap, const uint32_t numObs,const uint32_t numStates);

    static HmmDataMatrix_t getLogBMap(const ModelVec_t & models, const HmmDataMatrix_t & meas);

    static HmmDataMatrix_t getLogGamma(const AlphaBetaResult_t & alphabeta,uint32_t numObs, uint32_t numStates);
    
    static HmmDataMatrix_t getLogANumerator(const HmmDataMatrix_t & originalA,const AlphaBetaResult_t & alphabeta,const HmmDataMatrix_t & logbmap,const size_t numObs, const uint32_t numStates);
    
 
    static HmmDataVec_t getLogDenominator(const AlphaBetaResult_t & alphabeta, const uint32_t numStates, const uint32_t numObs);
    
    static HmmDataVec_t elnAddVector(const HmmDataVec_t & m1, const HmmDataVec_t & m2);
        
    static HmmDataMatrix_t elnAddMatrix(const HmmDataMatrix_t & m1, const HmmDataMatrix_t & m2);
    
    static HmmDataMatrix_t elnMatrixScalarProduct(const HmmDataMatrix_t & m1, const HmmFloat_t a);

    static HmmDataVec_t elnVectorScalarProduct(const HmmDataVec_t & m1, const HmmFloat_t a);
    
    
    static ViterbiDecodeResult_t decodeWithoutLabels(const HmmDataMatrix_t & A, const HmmDataMatrix_t & logbmap, const HmmDataVec_t & pi,const uint32_t numStates,const uint32_t numObs,const UIntSet_t & allowed_final_states);
    
  
};


#endif
