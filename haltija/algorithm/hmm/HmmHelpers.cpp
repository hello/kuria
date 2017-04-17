#include "HmmHelpers.h"
#include "LogMath.h"
#include "MatrixHelpers.h"
#include <assert.h>

#define FORBIDDEN_TRANSITION_PENALTY LOGZERO
#define NON_LABEL_PENALTY LOGZERO

template <typename T>
std::vector<size_t> sort_indexes(const std::vector<T> &v) {
    
    // initialize original index locations
    std::vector<size_t> idx(v.size());
    for (size_t i = 0; i != idx.size(); ++i) idx[i] = i;
    
    // sort indexes based on comparing values in v
    sort(idx.begin(), idx.end(),
         [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});
    
    return idx;
}





AlphaBetaResult_t HmmHelpers::getAlphaAndBeta(int32_t numObs,const HmmDataVec_t & pi, const HmmDataMatrix_t & logbmap, const HmmDataMatrix_t & A,const uint32_t numStates) {

    /*
     Calculates 'alpha' the forward variable.
     
     The alpha variable is a numpy array indexed by time, then state (NxT).
     alpha[i][t] = the probability of being in state 'i' after observing the
     first t symbols.
     
     forbidden transitions means at that time index a transition is unavailable
     labels mean that you have to be in that time
     */
    
    int t,j,i;
    HmmDataMatrix_t logalpha = getLogZeroedMatrix(numStates,numObs);
    HmmDataMatrix_t logbeta = getLogZeroedMatrix(numStates,numObs);
    HmmFloat_t temp;
    HmmDataMatrix_t A2 = A; //copy
    
    //nominal A matrix
    for (j = 0; j < numStates; j++) {
        for (i = 0; i < numStates; i++) {
            A2[j][i] = eln(A2[j][i]);
        }
    }
    
    const HmmDataMatrix_t logA = A2;
    
    //init stage - alpha_1(x) = pi(x)b_x(O1)
    
    for (j = 0; j < numStates; j++) {
        logalpha[j][0] = elnproduct(eln(pi[j]), logbmap[j][0]);
    }
    
    
    for (t = 1; t < numObs; t++) {
        
        for (j = 0; j < numStates; j++) {
            temp = LOGZERO;
            
            for (i = 0; i < numStates; i++) {
                //alpha[j][t] += alpha[i][t-1]*A[i][j];
                const HmmFloat_t tempval = elnproduct(logalpha[i][t-1],logA[i][j]);
                temp = elnsum(temp,tempval);
            }
            
            //alpha[j][t] *= bmap[j][t];
            logalpha[j][t] = elnproduct(temp, logbmap[j][t]);
        }
        
        
      
        
    }
    
    /*
     Calculates 'beta' the backward variable.
     
     The beta variable is a numpy array indexed by time, then state (NxT).
     beta[i][t] = the probability of being in state 'i' and then observing the
     symbols from t+1 to the end (T).
     */
    
    
    // init stage
    for (int s = 0; s < numStates; s++) {
        logbeta[s][numObs - 1] = 0.0;
    }
    
    
    
    for (t = numObs - 2; t >= 0; t--) {
        
        for (i = 0; i < numStates; i++) {
            temp = LOGZERO;
            for (j = 0;  j < numStates; j++) {
                const HmmFloat_t tempval  = elnproduct(logbmap[j][t+1], logbeta[j][t+1]);
                const HmmFloat_t tempval2 = elnproduct(tempval, logA[i][j]);
                temp = elnsum(temp,tempval2);
                //beta[i][t] += A[i][j]*bmap[j][t+1] * beta[j][t+1];
            }
            
            logbeta[i][t] = temp;
        }
        
        
    }
    
    temp = LOGZERO;
    for (i = 0; i < numStates; i++) {
        temp = elnsum(temp,logalpha[i][numObs-1]);
    }
    
    
    const AlphaBetaResult_t result(logalpha,logbeta,logA,temp);
    
    return result;
    

    
}

Hmm3DMatrix_t HmmHelpers::getLogXi(const AlphaBetaResult_t & alphabeta,const HmmDataMatrix_t & logbmap, const uint32_t numObs,const uint32_t numStates) {
    /*
     Calculates 'xi', a joint probability from the 'alpha' and 'beta' variables.
     
     The xi variable is a numpy array indexed by time, state, and state (TxNxN).
     xi[t][i][j] = the probability of being in state 'i' at time 't', and 'j' at
     time 't+1' given the entire observation sequence.
     */
    int32_t t,i,j;
    
    const HmmDataMatrix_t & logalpha = alphabeta.logalpha;
    const HmmDataMatrix_t & logbeta = alphabeta.logbeta;
    const HmmDataMatrix_t & logA = alphabeta.logA;
    Hmm3DMatrix_t logxi = getLogZeroed3dMatrix(numStates,numStates, numObs);
    HmmDataVec_t tempvec = getZeroedVec(numStates);
    HmmDataVec_t logdenomvec = getZeroedVec(numStates);
    HmmFloat_t normalizer;
    
    for (t = 0; t < numObs - 1; t++) {
        normalizer = LOGZERO;
        
        HmmDataMatrix_t logAThisTimeStep = logA;

        for (i = 0; i < numStates; i++) {
            for (j = 0; j < numStates; j++) {
                
                const HmmFloat_t tempval1 = elnproduct(logalpha[i][t], logAThisTimeStep[i][j]);
                const HmmFloat_t tempval2 = elnproduct(logbmap[j][t+1], logbeta[j][t+1]);
                const HmmFloat_t tempval3 = elnproduct(tempval1,tempval2);
                
                logxi[i][j][t] = tempval3;
                
                normalizer = elnsum(tempval3, normalizer);
            }
        }
        
        
        for (i = 0; i < numStates; i++) {
            for (j = 0; j < numStates; j++) {
                logxi[i][j][t] = elnproduct(logxi[i][j][t], -normalizer);
            }
        }
    }
    
    return logxi;
    
}



HmmDataMatrix_t HmmHelpers::getLogBMap(const ModelVec_t & models, const HmmDataMatrix_t & meas) {
    HmmDataMatrix_t logbmap;
    
    
    for (ModelVec_t::const_iterator it = models.begin(); it != models.end(); it++) {
        const HmmPdfInterface * ref = (*it).get();
        logbmap.push_back(ref->getLogOfPdf(meas));
    }
    
    return logbmap;
}

HmmDataMatrix_t HmmHelpers::getLogGamma(const AlphaBetaResult_t & alphabeta,uint32_t numObs, uint32_t numStates) {
    /*
     Calculates 'gamma' from xi.
     
     Gamma is a (TxN) numpy array, where gamma[t][i] = the probability of being
     in state 'i' at time 't' given the full observation sequence.
     */
    
    int32_t t,i;
    HmmFloat_t normalizer;
    HmmFloat_t temp;
    HmmDataMatrix_t loggamma = getLogZeroedMatrix(numStates, numObs);
    
    for (t = 0; t < numObs; t++) {
        normalizer = LOGZERO;
        for (i = 0; i < numStates; i++) {
            temp = elnproduct(alphabeta.logalpha[i][t], alphabeta.logbeta[i][t]);
            loggamma[i][t] = temp;
            normalizer = elnsum(normalizer, temp);
        }
        
        
        for (i = 0; i < numStates; i++) {
            loggamma[i][t] = elnproduct(loggamma[i][t], -normalizer);
        }
    }
    
    return loggamma;
    
}


HmmDataMatrix_t HmmHelpers::getLogANumerator(const HmmDataMatrix_t & originalA, const AlphaBetaResult_t & alphabeta,const HmmDataMatrix_t & logbmap,const size_t numObs, const uint32_t numStates) {
    
    int32_t i,j,t;
    HmmDataMatrix_t logANumerator = getLogZeroedMatrix(numStates, numStates);
    
    const HmmDataMatrix_t & logalpha = alphabeta.logalpha;
    const HmmDataMatrix_t & logbeta = alphabeta.logbeta;
    const HmmDataMatrix_t & logA = alphabeta.logA;
    
    
    for (i = 0; i < numStates; i++) {
        for (j = 0; j < numStates; j++) {
            HmmFloat_t numer = LOGZERO;
            
            for (t = 0; t < numObs - 1; t++) {
                const HmmFloat_t tempval1 = elnproduct(logalpha[i][t], logA[i][j]);
                const HmmFloat_t tempval2 = elnproduct(logbmap[j][t+1], logbeta[j][t+1]);
                const HmmFloat_t tempval3 = elnproduct(tempval1,tempval2);
                
                numer = elnsum(numer,tempval3);
            }
            
            if (originalA[i][j] == 0.0) {
                logANumerator[i][j] = LOGZERO;
            }
            else {
                logANumerator[i][j] = numer;
            }
        }
    }
    
    
    
    for (i = 0; i < numStates; i++) {
        for (j = 0; j < numStates; j++) {
            logANumerator[j][i] = elnproduct(logANumerator[j][i], -alphabeta.logmodelcost);
        }
    }
    
    
    return logANumerator;
    
}




HmmDataVec_t HmmHelpers::getLogDenominator(const AlphaBetaResult_t & alphabeta, const uint32_t numStates, const uint32_t numObs) {
    
    int32_t iState,t;
    
    HmmDataVec_t logDenominator = getLogZeroedVec(numStates);
    
    const HmmDataMatrix_t & logalpha = alphabeta.logalpha;
    const HmmDataMatrix_t & logbeta = alphabeta.logbeta;
    
    for (iState = 0; iState < numStates; iState++) {
        for (t = 0; t < numObs; t++) {
            logDenominator[iState] = elnsum(logDenominator[iState],elnproduct(logalpha[iState][t], logbeta[iState][t]));
        }
        
        logDenominator[iState] = elnproduct(logDenominator[iState], -alphabeta.logmodelcost);
    }
    
    return logDenominator;
}

HmmDataMatrix_t HmmHelpers::elnMatrixScalarProduct(const HmmDataMatrix_t & m1, const HmmFloat_t a) {
    const int m = m1.size();
    const int n = m1[0].size();
    
    HmmDataMatrix_t m3 = getZeroedMatrix(m, n);
    HmmFloat_t elnA = (eln(a));
    for (int j = 0; j < m; j++) {
        for (int i = 0; i < n; i++) {
            m3[j][i] = elnproduct(m1[j][i], elnA);
        }
    }
    
    return m3;
}

HmmDataVec_t HmmHelpers::elnVectorScalarProduct(const HmmDataVec_t & m1, const HmmFloat_t a) {
    const int m = m1.size();
    
    HmmDataVec_t m3 = getZeroedVec(m);
    HmmFloat_t elnA = (eln(a));
    for (int j = 0; j < m; j++) {
        m3[j] = elnproduct(m1[j], elnA);
    }
    
    return m3;
}

HmmDataMatrix_t HmmHelpers::elnAddMatrix(const HmmDataMatrix_t & m1, const HmmDataMatrix_t & m2) {
    const int m = m1.size();
    const int n = m1[0].size();
    
    HmmDataMatrix_t m3 = getZeroedMatrix(m, n);
    
    for (int j = 0; j < m; j++) {
        for (int i = 0; i < n; i++) {
            m3[j][i] = elnsum(m1[j][i], m2[j][i]);
        }
    }
    
    return m3;
}

HmmDataVec_t HmmHelpers::elnAddVector(const HmmDataVec_t & m1, const HmmDataVec_t & m2) {
    const int m = m1.size();
    
    HmmDataVec_t m3 = getZeroedVec(m);
    
    for (int j = 0; j < m; j++) {
        m3[j] = elnsum(m1[j], m2[j]);
        
    }
    
    return m3;
}


static ViterbiPath_t decodePath(int32_t startidx,const ViterbiPathMatrix_t & paths) {
    size_t len = paths[0].size();
    
    
    ViterbiPath_t path;
    path.resize(len);
    
    path[len-1] = startidx;
    for(int i = len - 2; i >= 0; i--) {
        path[i] = paths[path[i+1]][i];
    }
    
    
    return path;
}

static ViterbiDecodeResult_t decodePathAndGetCost(int32_t startidx,const ViterbiPathMatrix_t & paths,const HmmDataMatrix_t & phi)  {
    
    const size_t len = paths[0].size();
    
    
    //get viterbi path
    ViterbiPath_t path = decodePath(startidx,paths);
    
    //compute cost stuff
    const HmmFloat_t cost = phi[path[len-1]][len-1];
    
    //really -bic
//    const HmmFloat_t bic = 2*cost - _alphabetNumerator[0].size() * _numStates * log(len);
    
    return ViterbiDecodeResult_t(path,cost,cost);
}


ViterbiDecodeResult_t HmmHelpers::decodeWithoutLabels(const HmmDataMatrix_t & A, const HmmDataMatrix_t & logbmap, const HmmDataVec_t & pi, const uint32_t numStates,const uint32_t numObs) {
    int j,i,t;

    HmmDataVec_t costs;
    costs.resize(numStates);
    
    HmmDataMatrix_t phi = getLogZeroedMatrix(numStates, numObs);
    ViterbiPathMatrix_t vindices = getZeroedPathMatrix(numStates, numObs);
    HmmDataMatrix_t logAThisIndex = getELNofMatrix(A);
    
    //init
    for (i = 0; i < numStates; i++) {
        phi[i][0] = elnproduct(logbmap[i][0], eln(pi[i]));
    }
    
    for (t = 1; t < numObs; t++) {
        
        for (j = 0; j < numStates; j++) {
            const HmmFloat_t obscost = logbmap[j][t];
            
            for (i = 0; i < numStates; i++) {
                costs[i] = elnproduct(logAThisIndex[i][j], obscost);
            }
            
            for (i = 0; i < numStates; i++) {
                costs[i] = elnproduct(costs[i], phi[i][t-1]);
            }
            
            const int32_t maxidx = getArgMaxInVec(costs);
            const HmmFloat_t maxval = costs[maxidx];
            
            phi[j][t] = maxval;
            vindices[j][t] = maxidx;
        }
    }
    
    const ViterbiDecodeResult_t result = decodePathAndGetCost(A.size() - 1, vindices, phi);
    
    
    return result;
    
}







static TransitionAtTime_t getPathTransitions(const ViterbiPath_t & path) {
    TransitionAtTime_t results;
    for (int t = 1; t < path.size(); t++) {
        if (path[t] != path[t - 1]) {
            results.insert(std::make_pair (StateIdxPair(path[t - 1],path[t]),t));
        }
    }
    
    return results;
}

static TransitionMap_t getPathTransitionsByTime(const ViterbiPath_t & path) {
    TransitionMap_t results;
    for (int t = 1; t < path.size(); t++) {
        if (path[t] != path[t - 1]) {
            results.insert(std::make_pair (t,StateIdxPair(path[t - 1],path[t])));
        }
    }
    
    return results;
}


static void printTransitions(const ViterbiPath_t & path) {
    const TransitionAtTime_t pt = getPathTransitions(path);
    
    for (auto it = pt.begin(); it != pt.end(); it++) {
        StateIdxPair transition = (*it).first;
        std::cout << "PAIR: " << transition.from << "," << transition.to << "," << (*it).second << std::endl;
        int32_t t = (*it).second;
        t -= 1;
        int hour = t * 5.0 / 60.0;
        int min = t * 5.0 - hour * 60.0;
        hour += 20;
        
        if (hour >= 24) {
            hour -= 24;
        }
        
        
        char buf[16];
        snprintf(buf, 16, "%02d:%02d",hour,min);
        
        std::cout << transition.from << " ---> " << transition.to << " at time " << buf << std::endl;
    }
    
    std::cout << "----------" << std::endl;
}


