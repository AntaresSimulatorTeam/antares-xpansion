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
        self._playlist_reset_option: bool = True
        self._active_year_list: List[int] = []
        self._inactive_year_list: List[int] = []

    def get_nb_years(self) -> int:
        return self._mc_years

    def get_nb_activated_year(self):
        self._set_playlist_reset_option()
        self._set_playlist_year_lists()
        return self._compute_nb_activated_years()

    def _set_playlist_reset_option(self):
        # Default : all mc years are activated
        self._playlist_reset_option = self.config.getboolean('playlist', 'playlist_reset', fallback=True)

    def _set_playlist_year_lists(self):
        self._active_year_list = []
        self._inactive_year_list = []
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

    def _compute_nb_activated_years(self) -> int:
        if self._playlist_reset_option:
            nb_activated_year = self._count_years_from_inactive_years_list()
        else:
            nb_activated_year = self._count_years_from_active_years_list()
        return nb_activated_year

    def _count_years_from_active_years_list(self):
        nb_activated_year = 0
        for active_year in self._active_year_list:
            if int(active_year) < self._mc_years:
                nb_activated_year += 1
        return nb_activated_year

    def _count_years_from_inactive_years_list(self):
        nb_activated_year = self._mc_years
        for inactive_year in self._inactive_year_list:
            if int(inactive_year) < self._mc_years:
                nb_activated_year -= 1
        return nb_activated_year

