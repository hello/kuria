/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.1 at Fri Mar 17 10:27:25 2017. */

#include "radar_messages.pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t hello_FeatureVector_fields[5] = {
    PB_FIELD(  1, STRING  , OPTIONAL, CALLBACK, FIRST, hello_FeatureVector, id, id, 0),
    PB_FIELD(  2, STRING  , OPTIONAL, CALLBACK, OTHER, hello_FeatureVector, device_id, id, 0),
    PB_FIELD(  3, FLOAT   , REPEATED, CALLBACK, OTHER, hello_FeatureVector, floatfeats, device_id, 0),
    PB_FIELD(  4, INT64   , OPTIONAL, STATIC  , OTHER, hello_FeatureVector, sequence_number, floatfeats, 0),
    PB_LAST_FIELD
};


