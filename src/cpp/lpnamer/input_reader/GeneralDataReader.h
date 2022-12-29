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

    public:
     explicit GeneralDataIniReader(const std::filesystem::path& file_path, 
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);

    int GetNbYears() const{
        return mc_years_;
    }

    std::vector<int> GetActiveYears(){
        active_year_list_.clear();
        inactive_year_list_.clear();
        if (user_playlist_){
            SetPlaylistResetOption();
            SetPlaylistYearLists();
            return GetActiveYears_();
            }
        else {
            std::vector<int> active_years(mc_years_ + 1);
            std::iota(std::begin(active_years), std::end(active_years), 1);
            return active_years;
            }
    }

   std::pair<std::vector<int>,std::vector<int>> GetRawPlaylist(){
       std::string current_section = "";
        active_year_list_.clear();
        inactive_year_list_.clear();
        for (auto line : file_lines_){
            if (IniReaderUtils::LineIsNotASectionHeader(line)){
                if (current_section == "playlist"){
                   auto [key, val] = IniReaderUtils::GetKeyValFromLine(line);
                    ReadPlaylistVal(key, val);
                    }
            }
            else{
                current_section = IniReaderUtils::ReadSectionHeader(line);
            }
            }
        return {active_year_list_, inactive_year_list_};
}
    std::vector<int> GetActiveYears_(){
        if (playlist_reset_option_){
            return ActiveYearsFromInactiveList();
            }
        else {
            return ActiveYearsFromActiveList();
            }
            }
    std::vector<int> ActiveYearsFromActiveList(){
        std::vector<int> active_years ;
        for (auto year = 0; year < mc_years_; year++){
            if (std::find( active_year_list_.begin(), active_year_list_.end(), year)!= active_year_list_.end())):
                active_years.append(year+1);
                }
        return active_years;
        }

    std::vector<int> ActiveYearsFromInactiveList(){
        std::vector<int> active_years;
        for (auto year = 0; year < mc_years_; year++) {
            if( std::find( inactive_year_list_.begin(), inactive_year_list_.end(), year)!= inactive_year_list_.end()){
                active_years.push_back(year+1);
                }
            }
        return active_years;
}
    void SetPlaylistResetOption(){
        //  Default : all mc years are activated
        playlist_reset_option_ = reader_.GetBoolean('playlist', 'playlist_reset', true);
}
   void SetPlaylistYearLists(){
        std::string current_section = "";
        for( const auto &line : file_lines_){
            current_section = ReadPlaylist(current_section, common_lpnamer::trim( line ));
}
}
    std::string ReadPlaylist(const std::string& current_section, const std::string& line){
        if (IniReaderUtils::LineIsNotASectionHeader(line) ) {
            if (current_section == "playlist") {
                auto [key, val] = IniReaderUtils::GetKeyValFromLine(line);
                ReadPlaylistVal(key, val);
                }
        }    
        else{
            return IniReaderUtils::ReadSectionHeader(line);
        }
    return current_section;
}
    void ReadPlaylistVal(const std::string& key, int val){
        if (key == "playlist_year +"){
            active_year_list_.push_back(val);
            }
        else if (key == "playlist_year -"){
            inactive_year_list_.push_back(val);
            }
    }
};

#endif // SRC_CPP_LPNAMER_INPUTREADER_GENERALDATAREADER_H
