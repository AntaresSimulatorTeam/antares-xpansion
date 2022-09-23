import os
from pathlib import Path

from antares_xpansion.flushed_print import flushed_print
import configparser

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
        config = configparser.ConfigParser()
        config.read(self._general_data_ini_file)
        value_to_change = self._get_values_to_change_general_data_file()
        for (section, key) in value_to_change:
            if config.has_section(section):
                if config.has_option(section, key):
                    config.set(section, key, value_to_change[(section, key)])
        with open(self._general_data_ini_file, "w") as writer:
            config.write(writer)

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
