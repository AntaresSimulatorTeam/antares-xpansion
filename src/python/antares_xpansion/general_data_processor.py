import configparser
import os
import shutil
from pathlib import Path

from antares_xpansion.general_data_reader import GeneralDataIniReader
from antares_xpansion.logger import step_logger


class GeneralDataFileExceptions:
    class GeneralDataFileNotFound(Exception):
        pass


class GeneralDataProcessor:
    def __init__(self, general_data_file_root: Path, is_accurate: bool) -> None:

        self.logger = step_logger(__name__, __class__.__name__)
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

    def change_general_data_file_to_configure_antares_execution(self, memory_mode=False):
        self.logger.info("Pre Antares")
        ini_file_backup = self._general_data_ini_file.with_suffix(
            self._general_data_ini_file.suffix + ".with-playlist")
        shutil.copyfile(self._general_data_ini_file, ini_file_backup)
        config = configparser.ConfigParser(strict=False)
        config.read(self._general_data_ini_file)
        value_to_change = self._get_values_to_change_general_data_file(memory_mode)
        for (section, key) in value_to_change:
            if not config.has_section(section):
                config.add_section(section)
            config.set(section, key, value_to_change[(section, key)])
        with open(self._general_data_ini_file, "w") as writer:
            has_playlist = config.has_section("playlist")
            if has_playlist:
                playlist_options = dict(config.items("playlist"))
                config.remove_section("playlist")
            config.write(writer)
            if has_playlist:
                self.backport_playlist(
                    ini_file_backup, writer, playlist_options)
        os.remove(ini_file_backup)

    def backport_playlist(self, ini_file_backup, writer, playlist_options: dict):
        ini_reader = GeneralDataIniReader(ini_file_backup)
        active_years, inactive_years = ini_reader.get_raw_playlist()
        writer.write("[playlist]\n")
        for option in playlist_options:
            if option != "playlist_year +" and option != "playlist_year -":
                writer.write(f"{option} = {playlist_options[option]}\n")
        for year in active_years:
            writer.write(f"playlist_year + = {year}\n")
        for year in inactive_years:
            writer.write(f"playlist_year - = {year}\n")

    general_data_ini_file = property(
        get_general_data_ini_file, set_general_data_ini_file
    )

    def _get_values_to_change_general_data_file(self, memory_mode=False):
        optimization = "optimization"
        general_section = "general"
        output_section = "output"

        return {
            (optimization, "include-exportmps"): "false" if memory_mode else "optim-1",
            (optimization, "include-exportstructure"): "true",
            ("adequacy patch", "include-adq-patch"): "false",
            (optimization, "include-tc-minstablepower"): "true"
            if self.is_accurate
            else "false",
            (optimization, "include-tc-min-ud-time"): "true"
            if self.is_accurate
            else "false",
            (optimization, "include-dayahead"): "true" if self.is_accurate else "false",
            (general_section, "mode"): "expansion" if self.is_accurate else "Economy",
            (output_section, "storenewset"): "true",
            ("other preferences", "unit-commitment-mode"): "accurate"
            if self.is_accurate
            else "fast",
            (general_section, "year-by-year"): "false",
            (output_section, "synthesis"): "false",
            ("input", "import"): "",
        }

    def backup_data(self):
        ini_file_backup = self._backup_file_name()
        shutil.copyfile(self._general_data_ini_file, ini_file_backup)

    def backup_data_on_error(self):
        ini_file_backup = self._error_file_name()
        shutil.copyfile(self._general_data_ini_file, ini_file_backup)

    def revert_backup_data(self):
        ini_file_backup = self._backup_file_name()
        shutil.copyfile(ini_file_backup, self._general_data_ini_file)
        os.remove(ini_file_backup)

    def _backup_file_name(self):
        return self._general_data_ini_file.with_suffix(self._general_data_ini_file.suffix + ".backup")

    def _error_file_name(self):
        return self._general_data_ini_file.with_suffix(self._general_data_ini_file.suffix + ".error")
