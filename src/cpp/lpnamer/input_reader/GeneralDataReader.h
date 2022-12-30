#ifndef SRC_CPP_LPNAMER_INPUTREADER_GENERALDATAREADER_H
#define SRC_CPP_LPNAMER_INPUTREADER_GENERALDATAREADER_H

#include "INIReader.h"
#include <filesystem>
#include <vector>
#include <stdexcept>
#include "ProblemGenerationLogger.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include "common_lpnamer.h"


class IniFileNotFound: public std::runtime_error 
    {
        public:
        IniFileNotFound(const std::string&msg): std::runtime_error(msg){}
    };


class IniReaderUtils{
    public:
    static bool LineIsNotASectionHeader(const std::string& line) {
        return common_lpnamer::split(common_lpnamer::trim(line), '=').size() == 2;
}
    
    static std::string ReadSectionHeader(const std::string& line) {
        auto str= common_lpnamer::trim(line);
        str.erase(std::remove(str.begin(), str.end(), '['), str.end());
        str.erase(std::remove(str.begin(), str.end(), ']'), str.end());
        return str;
}
    
    static std::pair<std::string, int > GetKeyValFromLine(const std::string& line){
        auto key =common_lpnamer::trim( common_lpnamer::split(line, '=')[0]);
        auto val =std::atoi((common_lpnamer::trim( common_lpnamer::split(line, '=')[1]).c_str()));
        return {key, val};
}

};

class GeneralDataIniReader{
    private:
    
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
    INIReader reader_;
    int mc_years_;
    bool user_playlist_;
    bool playlist_reset_option_;
    std::vector<int> active_year_list_;
    std::vector<int> inactive_year_list_;
    std::vector<std::string> file_lines_;
    
    std::vector<int> GetActiveYears_();
    std::vector<int> ActiveYearsFromActiveList();
    std::vector<int> ActiveYearsFromInactiveList();
    void SetPlaylistResetOption();
    void SetPlaylistYearLists();
    std::string ReadPlaylist(const std::string& current_section, const std::string& line);
    void ReadPlaylistVal(const std::string& key, int val);

    public:
     explicit GeneralDataIniReader(const std::filesystem::path& file_path, 
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);

    int GetNbYears() const{
        return mc_years_;
    }

    std::vector<int> GetActiveYears();
    std::pair<std::vector<int>,std::vector<int>> GetRawPlaylist();

};

#endif // SRC_CPP_LPNAMER_INPUTREADER_GENERALDATAREADER_H
