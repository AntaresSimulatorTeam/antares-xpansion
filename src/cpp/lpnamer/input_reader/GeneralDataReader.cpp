#include "GeneralDataReader.h"

GeneralDataIniReader::GeneralDataIniReader(const std::filesystem::path& file_path, 
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger):logger_(logger){
        if (file_path.empty() || !std::filesystem::exists( file_path)){
            std::ostringstream msg;
            msg<< "General data file is not found : "<<file_path.string()<<std::endl;
            (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL)<<msg.str();
            throw IniFileNotFound(msg.str());
            }

        reader_ = INIReader (file_path.string());
        std::ifstream input_file(file_path);
        std::copy(std::istream_iterator<std::string>(input_file),
         std::istream_iterator<std::string>(),
         std::back_inserter(file_lines_));

        mc_years_ = reader_.GetInteger("general", "nbyears",0);
        user_playlist_ = reader_.GetBoolean("general", "user-playlist", false);
        playlist_reset_option_ = true;
        
        }