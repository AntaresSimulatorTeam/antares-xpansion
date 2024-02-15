import pytest

from antares_xpansion.config_file_parser import ConfigFileParser


class TestConfigFileParser:

    def test_config_file_not_found(self):
        config_file = ""
        my_config_file_parser = ConfigFileParser(config_file)
        with pytest.raises(FileNotFoundError):
            my_config_file_parser.get_config_parameters()

    def test_config_file_empty(self, tmp_path):

        temp_config_file_path = tmp_path / "toto.kldka"
        content = ""
        temp_config_file_path.write_text(content)
        my_config_file_parser = ConfigFileParser(temp_config_file_path)
        config_param = my_config_file_parser.get_config_parameters()
        assert config_param.default_install_dir == ""
        assert config_param.ANTARES == "antares-solver"
        assert config_param.MERGE_MPS == "merge_mps"
        assert config_param.BENDERS == "benders"
        assert config_param.LP_NAMER == "lp_namer"
        assert config_param.STUDY_UPDATER == "study_updater"
        assert config_param.SENSITIVITY_EXE == "sensitivity"
        assert config_param.AVAILABLE_SOLVERS == []

    def test_config_file_non_empty(self, tmp_path):

        temp_config_file_path = tmp_path / "toto.kldka"
        content = "DEFAULT_INSTALL_DIR             : Path/to/install\n"\
            "ANTARES             : antares-8.0-solver.exe\n"\
            "MERGE_MPS           : merge_mps.exe\n"\
            "BENDERS             : bendersmpi.exe\n"\
            "LP_NAMER            : lp_namer.exe\n"\
            "STUDY_UPDATER       : xpansion-study-updater.exe\n"\
            "SENSITIVITY         : sensitivity.exe\n"\
            "AVAILABLE_SOLVER    :\n"\
            "- Cbc\n"

        temp_config_file_path.write_text(content)
        my_config_file_parser = ConfigFileParser(temp_config_file_path)
        config_param = my_config_file_parser.get_config_parameters()
        assert config_param.default_install_dir == "Path/to/install"
        assert config_param.ANTARES == "antares-8.0-solver.exe"
        assert config_param.MERGE_MPS == "merge_mps.exe"
        assert config_param.BENDERS == "bendersmpi.exe"
        assert config_param.LP_NAMER == "lp_namer.exe"
        assert config_param.STUDY_UPDATER == "xpansion-study-updater.exe"
        assert config_param.SENSITIVITY_EXE == "sensitivity.exe"
        assert config_param.AVAILABLE_SOLVERS == ["Cbc"]
