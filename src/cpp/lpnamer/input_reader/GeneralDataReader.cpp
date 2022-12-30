#include "GeneralDataReader.h"
#include <iterator>
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
std::vector<int> GeneralDataIniReader::GetActiveYears(){
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

std::vector<int> GeneralDataIniReader::GetActiveYears_(){
        if (playlist_reset_option_){
            return ActiveYearsFromInactiveList();
            }
        else {
            return ActiveYearsFromActiveList();
            }
            } 

std::vector<int> GeneralDataIniReader::ActiveYearsFromActiveList(){
        std::vector<int> active_years ;
        for (auto year = 0; year < mc_years_; year++){
            if (std::find( active_year_list_.begin(), active_year_list_.end(), year)!= active_year_list_.end()){
                active_years.push_back(year+1);
                }
            }
        return active_years;
        }            

   std::vector<int> GeneralDataIniReader::ActiveYearsFromInactiveList(){
        std::vector<int> active_years;
        for (auto year = 0; year < mc_years_; year++) {
            if( std::find( inactive_year_list_.begin(), inactive_year_list_.end(), year)!= inactive_year_list_.end()){
                active_years.push_back(year+1);
                }
            }
        return active_years;
}        

  std::pair<std::vector<int>,std::vector<int>> GeneralDataIniReader::GetRawPlaylist(){
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
void GeneralDataIniReader::SetPlaylistResetOption(){
        //  Default : all mc years are activated
        playlist_reset_option_ = reader_.GetBoolean("playlist", "playlist_reset", true);
}
  void GeneralDataIniReader::SetPlaylistYearLists(){
        std::string current_section = "";
        for( const auto &line : file_lines_){
            current_section = ReadPlaylist(current_section, common_lpnamer::trim( line ));
        }
}

 std::string GeneralDataIniReader::ReadPlaylist(const std::string& current_section, const std::string& line){
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

void GeneralDataIniReader::ReadPlaylistVal(const std::string& key, int val){
        if (key == "playlist_year +"){
            active_year_list_.push_back(val);
            }
        else if (key == "playlist_year -"){
            inactive_year_list_.push_back(val);
            }
    }