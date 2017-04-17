#ifndef _HMMTYPES_H_
#define _HMMTYPES_H_
#include <stdint.h>
#include <vector>
#include <stdint.h>
#include <iostream>
#include <limits>
#include <algorithm>    
#include <future>
#include <unordered_map>
#include <unordered_set>
#include <map>

typedef float HmmFloat_t;
typedef std::vector<HmmFloat_t> HmmDataVec_t;
typedef std::vector<HmmDataVec_t> HmmDataMatrix_t;
typedef std::vector<HmmDataMatrix_t> Hmm3DMatrix_t;
typedef std::vector<uint32_t> ViterbiPath_t;
typedef std::vector<uint32_t> UIntVec_t;
typedef std::unordered_set<uint32_t> UIntSet_t;
typedef std::unordered_map<std::string,HmmDataMatrix_t> MatrixMap_t;
typedef std::map<uint32_t, uint32_t> LabelMap_t; //key is time index
typedef std::unordered_set<std::string> StringSet_t;
typedef std::vector<ViterbiPath_t> ViterbiPathMatrix_t;

#define  EPSILON (std::numeric_limits<HmmFloat_t>::epsilon())


inline std::ostream & operator << (std::ostream & lhs, const HmmDataVec_t & rhs) {
    bool first = true;
    for (HmmDataVec_t::const_iterator it = rhs.begin() ; it != rhs.end(); it++) {
        if (!first) {
            lhs << ",";
        }
        
        first = false;
        lhs << *it;
    }
    
    return lhs;
}

inline std::ostream & operator << (std::ostream & lhs, const UIntVec_t & rhs) {
    bool first = true;
    for (UIntVec_t::const_iterator it = rhs.begin() ; it != rhs.end(); it++) {
        if (!first) {
            lhs << ",";
        }
        
        first = false;
        lhs << *it;
    }
    
    return lhs;
}

inline std::ostream & operator << (std::ostream & lhs, const UIntSet_t & rhs) {
    bool first = true;
    for (UIntSet_t::const_iterator it = rhs.begin() ; it != rhs.end(); it++) {
        if (!first) {
            lhs << ",";
        }
        
        first = false;
        lhs << *it;
    }
    
    return lhs;
}


typedef std::pair<int32_t,HmmDataVec_t> StateIdxPdfEvalPair_t;
typedef std::pair<int32_t,HmmFloat_t> StateIdxCostPair_t;

typedef std::vector<std::future<StateIdxPdfEvalPair_t> > FuturePdfEvalVec_t;
typedef std::vector<std::future<StateIdxCostPair_t> > FutureCostVec_t;

class StateIdxPair {
public:
    StateIdxPair(const uint32_t i1, const uint32_t i2) : from(i1),to(i2) {}
    uint32_t from;
    uint32_t to;

    bool operator == (const StateIdxPair & t) const {
        return from == t.from && to == t.to;
    }

    StateIdxPair & operator = (const StateIdxPair & t) {
        from = t.from;
        to = t.to;

        return *this;
    }
};

struct StateIdxPairHash {
    std::size_t operator()(const StateIdxPair & k) const{
        using std::size_t;

        // Compute individual hash values for first,
        // second and third and combine them using XOR
        // and bit shifting:
        const size_t hashval = (size_t) k.from + (((size_t)k.to) << 32);
        return hashval;
    }
};


typedef std::map<uint32_t,StateIdxPair> TransitionMap_t; //key is time index
typedef std::unordered_map<StateIdxPair,int32_t,StateIdxPairHash> TransitionAtTime_t; //key is time index
typedef std::unordered_map<StateIdxPair,int32_t,StateIdxPairHash> TransitionAtTime_t; //key is time index

typedef std::vector<TransitionAtTime_t> TransitionAtTimeVec_t;
typedef std::vector<StateIdxPair> TransitionVector_t;
typedef std::unordered_multimap<uint32_t,StateIdxPair> TransitionMultiMap_t; //key is time index

#endif //_HMMTYPES_H_
