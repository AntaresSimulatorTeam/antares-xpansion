import os
from pathlib import Path


from antares_xpansion.general_data_reader import IniReader

import functools

print = functools.partial(print, flush=True)

class GeneralDataFileExceptions:
    class GeneralDataFileNotFound(Exception):
        pass

class GeneralDataProcessor:
    """
        Read and update general data file (generaldata.ini)
    """
    def __init__(self, general_data_file_root : Path, is_accurate : bool) -> None:

        self.general_data_ini = 'generaldata.ini'
        self.general_data_ini_file = Path(os.path.normpath(os.path.join(general_data_file_root, self.general_data_ini)))
        self.is_accurate = is_accurate


    def set_general_data_ini_file(self, general_data_ini_file : Path):
        if general_data_ini_file.is_file() :
            self._general_data_ini_file =  general_data_ini_file
        else :
            raise GeneralDataFileExceptions.GeneralDataFileNotFound ("General data file %s not found "%general_data_ini_file)       

    def get_general_data_ini_file(self) -> Path:
        return self._general_data_ini_file

    def change_general_data_file_to_configure_antares_execution(self):
        print("-- pre antares")
        with open(self._general_data_ini_file, 'r') as reader:
            lines = reader.readlines()

        with open(self._general_data_ini_file, 'w') as writer:
            current_section = ""
            for line in lines:
                if IniReader.line_is_not_a_section_header(line):
                    key = line.split('=')[0].strip()
                    line = self._get_new_line(line, current_section, key)
                else:
                    current_section = line.strip()

                if line:
                    writer.write(line)

    general_data_ini_file = property(get_general_data_ini_file, set_general_data_ini_file)

    def _get_new_line(self, line, section, key):
        changed_val = self._get_values_to_change_general_data_file()
        if (section, key) in changed_val:
            new_val = changed_val[(section, key)]
            if new_val:
                line = key + ' = ' + new_val + '\n'
            else:
                line = None
        return line


    def _get_values_to_change_general_data_file(self):
        optimization = '[optimization]'

        return {(optimization, 'include-exportmps'): 'true',
                (optimization, 'include-exportstructure'): 'true',
                (optimization, 'include-tc-minstablepower'): 'true' if self.is_accurate else 'false',
                (optimization,'include-tc-min-ud-time'): 'true' if self.is_accurate else 'false',
                (optimization,'include-dayahead'): 'true' if self.is_accurate else 'false',
                (optimization, 'include-usexprs'): None,
                (optimization, 'include-inbasis'):  None,
                (optimization, 'include-outbasis'): None,
                (optimization, 'include-trace'):    None,
                ('[general]', 'mode'): 'expansion' if self.is_accurate else 'Economy',
                (
                    '[other preferences]',
                    'unit-commitment-mode'): 'accurate' if self.is_accurate else 'fast'
                }


    

        
