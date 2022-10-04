from configparser import ConfigParser
from pathlib import Path

from typing import List


class IniFileNotFound(Exception):
    pass


class IniReader:
    @staticmethod
    def line_is_not_a_section_header(line: str) -> bool:
        return len(line.strip().split('=')) == 2

    @staticmethod
    def read_section_header(line: str) -> str:
        return line.strip().replace('[', '').replace(']', '')

    @staticmethod
    def get_key_val_from_line(line: str):
        key = line.split('=')[0].strip()
        val = line.split('=')[1].strip()
        return key, val


class GeneralDataIniReader:
    def __init__(self, file_path: Path = None):
        if not file_path or not file_path.is_file():
            raise IniFileNotFound

        self.config = ConfigParser(strict=False)
        self.config.read(file_path)

        self.file_lines = open(file_path, 'r').readlines()

        self._mc_years: int = int(self.config["general"]["nbyears"])
        self._user_playlist = self.config.getboolean("general", "user-playlist")
        self._playlist_reset_option: bool = True
        self._active_year_list: List[int] = []
        self._inactive_year_list: List[int] = []

    def get_nb_years(self) -> int:
        return self._mc_years

    def get_active_years(self):
        self._active_year_list = []
        self._inactive_year_list = []
        if self._user_playlist:
            self._set_playlist_reset_option()
            self._set_playlist_year_lists()
            return self._get_active_years()
        else:
            return list(range(1, self._mc_years + 1))

    def get_raw_playlist(self):
        current_section = ""
        self._active_year_list = []
        self._inactive_year_list = []
        for line in self.file_lines:
            if IniReader.line_is_not_a_section_header(line):
                if current_section == 'playlist':
                    key, val = IniReader.get_key_val_from_line(line)
                    self._read_playlist_val(key, val)
            else:
                current_section = IniReader.read_section_header(line)
        return self._active_year_list, self._inactive_year_list

    def _get_active_years(self):
        if self._playlist_reset_option is True:
            return self._active_years_from_inactive_list()
        else:
            return self._active_years_from_active_list()

    def _active_years_from_active_list(self):
        active_years = []
        for year in range(self._mc_years):
            if year in self._active_year_list:
                active_years.append(year+1)
        return active_years

    def _active_years_from_inactive_list(self):
        active_years = []
        for year in range(self._mc_years):
            if year not in self._inactive_year_list:
                active_years.append(year+1)
        return active_years

    def _set_playlist_reset_option(self):
        # Default : all mc years are activated
        self._playlist_reset_option = self.config.getboolean('playlist', 'playlist_reset', fallback=True)

    def _set_playlist_year_lists(self):
        current_section = ""
        for line in self.file_lines:
            current_section = self._read_playlist(current_section, line.strip())

    def _read_playlist(self, current_section: str, line: str):
        if IniReader.line_is_not_a_section_header(line):
            if current_section == 'playlist':
                key, val = IniReader.get_key_val_from_line(line)
                self._read_playlist_val(key, val)
        else:
            current_section = IniReader.read_section_header(line)
        return current_section

    def _read_playlist_val(self, key: str, val: str):
        if key == 'playlist_year +':
            self._active_year_list.append(int(val))
        elif key == 'playlist_year -':
            self._inactive_year_list.append(int(val))


