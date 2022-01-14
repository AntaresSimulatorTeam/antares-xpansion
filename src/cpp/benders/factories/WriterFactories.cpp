

#include "WriterFactories.h"
#include "VoidWriter.h"
#include "JsonWriter.h"
#include "OutputWriter.h"

Writer build_void_writer() {
    return std::make_shared<Output::VoidWriter>();
}

Writer build_json_writer(const BendersOptions &options) {
    auto timer= std::make_shared<TimeUtil>();
    Writer writer = std::make_shared<Output::JsonWriter>(timer);

    writer->initialize(options);
    return writer;
}
