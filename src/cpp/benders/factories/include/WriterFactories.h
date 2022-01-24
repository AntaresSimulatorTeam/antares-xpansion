
#ifndef ANTARESXPANSION_WRITERFACTORIES_H
#define ANTARESXPANSION_WRITERFACTORIES_H


#include "OutputWriter.h"

Writer build_void_writer();

Writer build_json_writer(const BendersOptions &options);


#endif //ANTARESXPANSION_WRITERFACTORIES_H
