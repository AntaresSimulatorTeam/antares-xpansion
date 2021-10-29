import pytest
import os
from pathlib import Path
from antares_xpansion.general_data_reader import IniReader
from antares_xpansion.antares_driver import AntaresDriver
from antares_xpansion.general_data_processor import GeneralDataFileExceptions, GeneralDataProcessor

RESSOURCES_DIR = Path(os.path.dirname(__file__)) / "ressources/"
EMPTY_ANTARES_DIR = RESSOURCES_DIR / "TestEmptyDir"
ANTARES_STUDY_WITH_EMPTY_SETTINGS_DIR = RESSOURCES_DIR / "TestEmptySettingsDir"
ANTARES_STUDY_WITH_EMPTY_EMPTY_GENERAL_DATA_FILE = RESSOURCES_DIR / "TestEmptyGeneralDataFile"
ANTARES_STUDY_TO_TEST_GENERAL_DATA_FILE_VALUES_IN_ACCURATE_MODE = RESSOURCES_DIR / "Test_Accurate_Mode"
ANTARES_STUDY_TO_TEST_GENERAL_DATA_FILE_VALUES_IN_FAST_MODE = RESSOURCES_DIR / "Test_Fast_Mode"

class TestGeneralDataProcessor:

    def setup_method(self):
        self.generaldata =  "generaldata.ini"

    def test_with_no_existing_general_data_file(self):
        with pytest.raises(GeneralDataFileExceptions.GeneralDataFileNotFound):
            GeneralDataProcessor(ANTARES_STUDY_WITH_EMPTY_SETTINGS_DIR, True)

    def test_general_data_file_path(self):
        SETTINGS_DIR = TestGeneralDataProcessor.get_settings_dir(ANTARES_STUDY_WITH_EMPTY_EMPTY_GENERAL_DATA_FILE) 
        expected_path = SETTINGS_DIR / self.generaldata
        gen_data_proc = GeneralDataProcessor(SETTINGS_DIR, True)
        assert gen_data_proc.get_general_data_ini_file() ==  expected_path
    
    def test_no_changes_in_empty_file(self):
        SETTINGS_DIR = TestGeneralDataProcessor.get_settings_dir(ANTARES_STUDY_WITH_EMPTY_EMPTY_GENERAL_DATA_FILE)
        gen_data_proc = GeneralDataProcessor(SETTINGS_DIR, True) 
                          
        with open(gen_data_proc.general_data_ini_file, "r") as reader:
            lines = reader.readlines()

        assert len(lines) == 0    
    
    def test_values_change_in_general_file_accurate_mode(self):

        SETTINGS_DIR = TestGeneralDataProcessor.get_settings_dir(ANTARES_STUDY_TO_TEST_GENERAL_DATA_FILE_VALUES_IN_ACCURATE_MODE)
        gen_data_path = SETTINGS_DIR / self.generaldata

        is_accurate = True
        optimization = '[optimization]'
        general = '[general]'
        random_section = '[random_section]'

        other_preferences = '[other preferences]'
        default_val = "[general] \n"\
                       "mode = expansion\n"\
                       "[optimization] \n"\
                       "include-exportmps = false\n"\
                       "include-tc-minstablepower = false\n"\
                       "include-dayahead = NO\n"\
                       "include-usexprs = value\n"\
                       "include-inbasis = value\n"\
                       "include-outbasis = value\n"\
                       "include-trace = value\n"\
                       "[other preferences] \n"\
                       "unit-commitment-mode = fast\n"\
                       "[random_section] \n"\
                       "key1 = value1\n"\
                       "key2 = value2\n"

        gen_data_path.write_text(default_val)
        expected_val = {(optimization, 'include-exportmps'): 'true',
                (optimization, 'include-exportstructure'): 'true',
                (optimization, 'include-tc-minstablepower'): 'true' ,
                (optimization,'include-tc-min-ud-time'): 'true',
                (optimization,'include-dayahead'): 'true' ,
                (general, 'mode'): 'expansion',
                (other_preferences,'unit-commitment-mode'): 'accurate',
                (random_section, "key1"): "value1", 
                (random_section, "key2"): "value2"  
                }

        
        gen_data_proc = GeneralDataProcessor(SETTINGS_DIR, is_accurate) 

        gen_data_proc.change_general_data_file_to_configure_antares_execution()                          
        with open(gen_data_proc.general_data_ini_file, "r") as reader:
            current_section = ""
            lines = reader.readlines()
            for line in lines:
                if IniReader.line_is_not_a_section_header(line):
                    key = line.split('=')[0].strip()
                    value = line.split('=')[1].strip()
                    if (current_section, key) in expected_val:
                        assert value == expected_val[(current_section, key)]
                else:
                    current_section = line.strip()
   
    
    def test_values_change_in_general_file_fast_mode(self):

        SETTINGS_DIR = TestGeneralDataProcessor.get_settings_dir(ANTARES_STUDY_TO_TEST_GENERAL_DATA_FILE_VALUES_IN_ACCURATE_MODE)
        gen_data_path = SETTINGS_DIR / self.generaldata

        is_accurate = False
        optimization = '[optimization]'
        general = '[general]'
        random_section = '[random_section]'

        other_preferences = '[other preferences]'
        default_val = "[general] \n"\
                       "mode = expansion\n"\
                       "[optimization] \n"\
                       "include-exportmps = false\n"\
                       "include-tc-minstablepower = false\n"\
                       "include-dayahead = NO\n"\
                       "include-usexprs = value\n"\
                       "include-inbasis = value\n"\
                       "include-outbasis = value\n"\
                       "include-trace = value\n"\
                       "[other preferences] \n"\
                       "unit-commitment-mode = dada\n"\
                       "[random_section] \n"\
                       "key1 = value1\n"\
                       "key2 = value2\n"


        gen_data_path.write_text(default_val)
        expected_val = {(optimization, 'include-exportmps'): 'true',
                (optimization, 'include-exportstructure'): 'true',
                (optimization, 'include-tc-minstablepower'): 'false' ,
                (optimization,'include-tc-min-ud-time'): 'false',
                (optimization,'include-dayahead'): 'false' ,
                (general, 'mode'): 'Economy',
                (other_preferences,'unit-commitment-mode'): 'fast',
                (random_section, "key1"): "value1", 
                (random_section, "key2"): "value2" 
                }


        
        gen_data_proc = GeneralDataProcessor(SETTINGS_DIR, is_accurate) 

        gen_data_proc.change_general_data_file_to_configure_antares_execution()                          
        with open(gen_data_proc.general_data_ini_file, "r") as reader:
            current_section = ""
            lines = reader.readlines()
            for line in lines:
                if IniReader.line_is_not_a_section_header(line):
                    key = line.split('=')[0].strip()
                    value = line.split('=')[1].strip()
                    if (current_section, key) in expected_val:
                        assert value == expected_val[(current_section, key)]
                else:
                    current_section = line.strip()
   

    

        

    @staticmethod
    def get_settings_dir(antares_study_path : Path):
        return antares_study_path / "settings"
class TestAntaresDriver:
    
    def test_empty_antares_dir(self):
        antares_driver =  AntaresDriver("")

        #${EMPTY_ANTARES_DIR}/settings//generaldata.ini not Found
        with pytest.raises(GeneralDataFileExceptions.GeneralDataFileNotFound):
            antares_driver.launch_accurate_mode(EMPTY_ANTARES_DIR, 1)
    
    def test_empty_settings_dir(self):
        antares_driver =  AntaresDriver("")

        #${ANTARES_STUDY_WITH_EMPTY_SETTINGS_DIR}/settings//generaldata.ini not Found
        with pytest.raises(GeneralDataFileExceptions.GeneralDataFileNotFound):
            antares_driver.launch_accurate_mode(ANTARES_STUDY_WITH_EMPTY_SETTINGS_DIR, 1)
    
    # def test_empty_general_data_file(self):
    #     antares_driver =  AntaresDriver("")

    #     #${TestEmptyGeneralDataFile}/settings//generaldata.ini not Found
    #     # with pytest.raises(FileNotFoundError):
    #     antares_driver.launch_accurate_mode(ANTARES_STUDY_WITH_EMPTY_EMPTY_GENERAL_DATA_FILE, 1)



