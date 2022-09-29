import configparser
import os
import shutil
from pathlib import Path

from antares_xpansion.flushed_print import flushed_print
from antares_xpansion.general_data_reader import GeneralDataIniReader


class GeneralDataFileExceptions:
    class GeneralDataFileNotFound(Exception):
        pass


class GeneralDataProcessor:
    def __init__(self, general_data_file_root: Path, is_accurate: bool) -> None:

        self.general_data_ini = "generaldata.ini"
        self.general_data_ini_file = Path(
            os.path.normpath(
                os.path.join(general_data_file_root, self.general_data_ini)
            )
        )
        self.is_accurate = is_accurate

    """
        Read and update general data file (generaldata.ini)
    """

    def set_general_data_ini_file(self, general_data_ini_file: Path):
        if general_data_ini_file.is_file():
            self._general_data_ini_file = general_data_ini_file
        else:
            raise GeneralDataFileExceptions.GeneralDataFileNotFound(
                "General data file %s not found " % general_data_ini_file
            )

    def get_general_data_ini_file(self) -> Path:
        return self._general_data_ini_file

    def change_general_data_file_to_configure_antares_execution(self):
        flushed_print("-- pre antares")
        ini_file_backup = self._general_data_ini_file.with_suffix(self._general_data_ini_file.suffix + ".with-playlist")
        shutil.copyfile(self._general_data_ini_file, ini_file_backup)
        config = configparser.ConfigParser(strict=False)
        config.read(self._general_data_ini_file)
        value_to_change = self._get_values_to_change_general_data_file()
        for (section, key) in value_to_change:
            if not config.has_section(section):
                config.add_section(section)
            config.set(section, key, value_to_change[(section, key)])
        with open(self._general_data_ini_file, "w") as writer:
            has_playlist = config.remove_section("playlist")
            config.write(writer)
            if has_playlist:
                self.backport_playlisy(ini_file_backup, writer)

    def backport_playlisy(self, ini_file_backup, writer):
        ini_reader = GeneralDataIniReader(ini_file_backup)
        ini_reader.get_active_years()
        active_years = ini_reader.get_raw_active_years()
        inactive_years = ini_reader.get_raw_inactive_years()
        writer.write("[playlist]\n")
        for year in active_years:
            writer.write(f"playlist_year + = {year}\n")
        for year in inactive_years:
            writer.write(f"playlist_year - = {year}\n")

    general_data_ini_file = property(
        get_general_data_ini_file, set_general_data_ini_file
    )

    def _get_values_to_change_general_data_file(self):
        optimization = "optimization"

        return {
            (optimization, "include-exportmps"): "true",
            (optimization, "include-exportstructure"): "true",
            (optimization, "include-tc-minstablepower"): "true"
            if self.is_accurate
            else "false",
            (optimization, "include-tc-min-ud-time"): "true"
            if self.is_accurate
            else "false",
            (optimization, "include-dayahead"): "true" if self.is_accurate else "false",
            ("general", "mode"): "expansion" if self.is_accurate else "Economy",
            ("output", "storenewset"): "true",
            ("other preferences", "unit-commitment-mode"): "accurate"
            if self.is_accurate
            else "fast",
        }
