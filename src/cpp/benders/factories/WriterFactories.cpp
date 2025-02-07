

#include "antares-xpansion/benders/factories/WriterFactories.h"

#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/benders/output/VoidWriter.h"
#include "antares-xpansion/benders/benders_core/common.h"

std::shared_ptr<Output::OutputWriter> build_void_writer() { return std::make_shared<Output::VoidWriter>(); }

std::shared_ptr<Output::OutputWriter> build_json_writer(const std::filesystem::path &json_file_name,
                         const bool restart) {
  std::shared_ptr<Output::OutputWriter> writer;
  if (restart) {
    auto out_json_content = get_json_file_content(json_file_name);
    writer =
        std::make_shared<Output::JsonWriter>(json_file_name, out_json_content);
  } else {
    auto m_clock = std::make_shared<Clock>();

    writer = std::make_shared<Output::JsonWriter>(m_clock, json_file_name);

    writer->initialize();
  }
  return writer;
}
