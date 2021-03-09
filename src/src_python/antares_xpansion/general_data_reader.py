from configparser import ConfigParser
from pathlib import Path


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


class GeneralDataIniReader(IniReader):
    def __init__(self, file_path: Path = None):
        if not file_path or not file_path.is_file():
            raise IniFileNotFound

        self.config = ConfigParser(strict=False)
        self.config.read(file_path)

        self.file_lines = open(file_path, 'r').readlines()

    def get_nb_years(self) -> int:
        return int(self.config["general"]["nbyears"])

    def get_nb_activated_year(self):
        _mc_years = self.get_nb_years()
        _playlist_reset_option = self._get_playlist_reset_option()
        _active_year_list, _inactive_year_list = self._get_playlist_year_lists()
        return self._compute_nb_activated_years(_active_year_list, _inactive_year_list, _mc_years,
                                                _playlist_reset_option)

    def _get_playlist_reset_option(self) -> bool:
        # Default : all mc years are activated
        return self.config.getboolean('playlist', 'playlist_reset', fallback=True)

    def _get_playlist_year_lists(self):
        _active_year_list = []
        _inactive_year_list = []
        current_section = ""
        for line in self.file_lines:
            current_section = self._read_playlist(_active_year_list, _inactive_year_list, current_section, line.strip())

        return _active_year_list, _inactive_year_list

    def _read_playlist(self, active_year: [], inactive_year: [], current_section: str, line: str):
        if self.line_is_not_a_section_header(line):
            if current_section == 'playlist':
                key, val = self.get_key_val_from_line(line)
                self._read_playlist_val(active_year, inactive_year, key, val)
        else:
            current_section = self.read_section_header(line)
        return current_section

    @staticmethod
    def _compute_nb_activated_years(active_year_list: [], inactive_year_list: [int],
                                    mc_years: int, playlist_reset_option: bool) -> int:
        nb_activated_year = mc_years if playlist_reset_option else 0
        if playlist_reset_option:
            for inactive_year in inactive_year_list:
                if int(inactive_year) < mc_years:
                    nb_activated_year -= 1
        else:
            for active_year in active_year_list:
                if int(active_year) < mc_years:
                    nb_activated_year += 1
        return nb_activated_year

    @staticmethod
    def _read_playlist_val(active_year: [], inactive_year: [], key: str, val: str):
        if key == 'playlist_year +':
            active_year.append(val)
        elif key == 'playlist_year -':
            inactive_year.append(val)
