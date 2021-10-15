import pytest
import uuid
import touch


from antares_xpansion.config_file_parser import ConfigFileParser

class TestConfigFileParser:

    def test_config_file_not_found(self):
        config_file = ""
        my_config_file_parser = ConfigFileParser(config_file)
        with pytest.raises(FileNotFoundError):
            my_config_file_parser.get_config_Parameters()

    # def test_config_file_empty(self, tmp_path):
    #     temp_config_file = tmp_path / (str(uuid.uuid4())+ ".extension")
    #     print(f"temp_config_file : {temp_config_file}")
    #     touch.touch(temp_config_file)
    #     my_config_file_parser = ConfigFileParser(temp_config_file.name)
    #     with pytest.raises(RuntimeError):
    #         my_config_file_parser.get_config_Parameters()
        
        

    
