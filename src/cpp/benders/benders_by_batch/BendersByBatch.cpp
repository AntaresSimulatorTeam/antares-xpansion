#include "BendersByBatch.h"

BendersByBatch::BendersByBatch(BendersBaseOptions const &options, Logger logger,
                               Writer writer)
    : BendersBase(options, std::move(logger), std::move(writer)) {}

void BendersByBatch::launch() {}
void BendersByBatch::free() {}
void BendersByBatch::run() {}
void BendersByBatch::initialize_problems() {}
