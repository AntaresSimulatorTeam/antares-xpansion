

#include "WriterFactories.h"
#include "VoidWriter.h"
#include "JsonWriter.h"
#include "OutputWriter.h"

Writer build_void_writer() {
    return std::make_shared<Output::VoidWriter>();
}

Writer build_json_writer(const BendersOptions &options) {
    Writer writer = std::make_shared<Output::JsonWriter>();
    writer->initialize(options);
    return writer;
}
