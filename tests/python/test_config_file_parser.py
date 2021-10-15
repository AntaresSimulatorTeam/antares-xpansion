import pytest
import uuid
import touch
from pathlib import Path

from antares_xpansion.config_file_parser import ConfigFileParser

class TestConfigFileParser:

    def test_config_file_not_found(self):
        config_file = ""
        my_config_file_parser = ConfigFileParser(config_file)
        with pytest.raises(FileNotFoundError):
            my_config_file_parser.get_config_Parameters()
            

    def test_config_file_empty(self, tmp_path):

        temp_config_file_path = tmp_path / "toto.kldka"
        content = "" 
        temp_config_file_path.write_text(content)
        my_config_file_parser = ConfigFileParser(temp_config_file_path)
        config_param =  my_config_file_parser.get_config_Parameters()
        assert config_param.ANTARES == "antares-solver"
        assert config_param.MERGE_MPS == "merge_mps"
        assert config_param.BENDERS_MPI == "benders_mpi"
        assert config_param.BENDERS_SEQUENTIAL == "benders_sequential"
        assert config_param.LP_NAMER == "lp_namer"
        assert config_param.STUDY_UPDATER == "study_updater"
        assert config_param.AVAILABLE_SOLVERS == []


    def test_config_file_non_empty(self, tmp_path):

        temp_config_file_path = tmp_path / "toto.kldka"
        content ="ANTARES             : antares-8.0-solver.exe\n"\
                "MERGE_MPS           : merge_mps.exe\n"\
                    "BENDERS_MPI         : bendersmpi.exe\n"\
                    "BENDERS_SEQUENTIAL  : benderssequential.exe\n"\
                    "LP_NAMER            : lp_namer.exe\n"\
                    "STUDY_UPDATER       : xpansion-study-updater.exe\n"\
                    "AVAILABLE_SOLVER :\n"\
                    "- Cbc\n"
                    
        temp_config_file_path.write_text(content)
        my_config_file_parser = ConfigFileParser(temp_config_file_path)
        config_param =  my_config_file_parser.get_config_Parameters()
        assert config_param.ANTARES == "antares-8.0-solver.exe"
        assert config_param.MERGE_MPS == "merge_mps.exe"
        assert config_param.BENDERS_MPI == "bendersmpi.exe"
        assert config_param.BENDERS_SEQUENTIAL == "benderssequential.exe"
        assert config_param.LP_NAMER == "lp_namer.exe"
        assert config_param.STUDY_UPDATER == "xpansion-study-updater.exe"
        assert config_param.AVAILABLE_SOLVERS == ["Cbc"]


 
 
        

    
