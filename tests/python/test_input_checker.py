import pytest
from antares_xpansion.input_checker import *
from antares_xpansion.input_checker import _check_candidate_option_type, \
    _check_candidate_name, _check_candidate_link, _check_setting_option_value, _check_profile_file, \
    _check_setting_option_type, _check_attribute_profile_values
from antares_xpansion.split_link_profile import SplitLinkProfile

from src.python.antares_xpansion.profile_link_checker import ProfileLinkChecker


class TestCheckProfileFile:

    def test_no_existing_file(self, tmp_path):
        profile_file = tmp_path / "ghost" / "file"

        with pytest.raises(ProfileFileNotExists):
            _check_profile_file(profile_file)

    def test_empty_profile(self, tmp_path):
        profile_file = TestCheckProfileFile.get_empty_file(tmp_path)

        with pytest.raises(ProfileFileWrongNumberOfLines):
            _check_profile_file(profile_file)

    def test_invalid_profile(self, tmp_path):
        profile_file = TestCheckProfileFile.get_empty_file(tmp_path)

        line = 'Word'
        profile_file.write_text(line)
        with pytest.raises(ProfileFileValueError):
            _check_profile_file(profile_file)

    def test_3_columns_in_profile_file(self, tmp_path):
        profile_file = TestCheckProfileFile.get_empty_file(tmp_path)
        line = "1 2 3\n"

        with open(profile_file, 'a+') as file:
            file.writelines([line for k in range(8760)])
        try:
            _check_profile_file(profile_file)
        except ProfileFileWrongNumberOfcolumns:
            pytest.fail("ProfileFileWrongNumberOfcolumns raised improperly")


    def test_negative_values_in_profile_file(self, tmp_path):
        profile_file = TestCheckProfileFile.get_empty_file(tmp_path)
        line = "-0.5 10.02"

        profile_file.write_text(line)

        with pytest.raises(ProfileFileNegativeValue):
            _check_profile_file(profile_file)

    @staticmethod
    def get_empty_file(tmp_path):
        profile_file = tmp_path / "file"
        profile_file.touch()

        return profile_file


class TestCheckCandidateOptionType:

    def test_fail_if_no_existing_option_is_given(self):
        option = "ThisCantBeAnOption"
        value = "notAValue"

        with pytest.raises(UnrecognizedCandidateOptionType):
            _check_candidate_option_type(option, value)

    def test_fail_if_max_units_value_is_negative(self):

        option = "max-units"
        value = -1545

        assert _check_candidate_option_type(option, value) == False

    def test_fail_if_max_units_value_is_str(self):

        option = "max-units"
        value = "str"

        assert _check_candidate_option_type(option, value) == False

    def test_true_for_obsolete_value(self):

        option = "has-link-profile"
        value = "str"

        assert _check_candidate_option_type(option, value) == True

    def test_true_for_string_option_type(self):

        option = "link"
        value = "0-1"

        assert _check_candidate_option_type(option, value) == True


class TestCheckCandidateName:

    def test_empty_candidate_name(self):

        with pytest.raises(EmptyCandidateName):
            _check_candidate_name("", "section1236")

    def test_illegal_chars_in_candidate_name(self):

        with pytest.raises(IllegalCharsInCandidateName):
            _check_candidate_name("newline\n123", "section1236")

        with pytest.raises(IllegalCharsInCandidateName):
            _check_candidate_name("escapeR\r123", "section1236")

        with pytest.raises(IllegalCharsInCandidateName):
            _check_candidate_name("tab\t123", "section1236")

        with pytest.raises(IllegalCharsInCandidateName):
            _check_candidate_name("escapeF\f\v123", "section1236")

        with pytest.raises(IllegalCharsInCandidateName):
            _check_candidate_name("escapeV\v123", "section1236")

        with pytest.raises(IllegalCharsInCandidateName):
            _check_candidate_name("parenthesis(123", "section1236")


class TestCheckCandidateLink:

    def test_empty_link(self):

        with pytest.raises(EmptyCandidateLink):
            _check_candidate_link("", "section0")

    def test_fail_if_link_doesnt_contains_separator(self):

        with pytest.raises(CandidateLinkWithoutSeparator):
            _check_candidate_link("atob", "section")
        with pytest.raises(CandidateLinkWithoutSeparator):
            _check_candidate_link("ato-b", "section")
        with pytest.raises(CandidateLinkWithoutSeparator):
            _check_candidate_link("ato- b", "section")
        with pytest.raises(CandidateLinkWithoutSeparator):
            _check_candidate_link("ato -b", "section")


class TestCheckCandidatesFile:

    def test_wrong_option_type_ini_file(self, tmp_path):

        option = "max-units"
        value = -1545

        ini_file = tmp_path / "a.ini"
        ini_file.touch()
        capa_dir = tmp_path / 'capa'
        capa_dir.mkdir()
        ini_file.write_text(f"""[5] \n
                           {option} = {value}""")

        with pytest.raises(CandidateFileWrongTypeValue):
            check_candidates_file(ini_file, capa_dir)

    def test_duplicated_candidate_name_ini_file(self, tmp_path):

        option = "max-units"
        value = 1545

        ini_file = tmp_path / "a.ini"
        ini_file.touch()
        ini_file.write_text(f"""[5] \n
                            name = alpha \n
                            link = a - b \n
                           {option} = {value}\n
                           unit-size = 400\n
                           [6] \n
                            name = alpha \n
                            link = a - b \n
                           {option} = {value}\n
                           unit-size = 400\n""")
        capa_dir = tmp_path / 'capa'
        capa_dir.mkdir()

        with pytest.raises(CandidateNameDuplicatedError):
            check_candidates_file(ini_file, capa_dir)

    def test_non_null_max_units_and_max_investment_simultaneaously(self, tmp_path):

        ini_file = tmp_path / "a.ini"
        ini_file.touch()
        ini_file.write_text(f"""[5] \n
                            name = alpha \n
                            link = a - b \n
                           max-units = 13\n
                           unit-size = 400\n
                           max-investment = 985""")
        capa_dir = tmp_path / 'capa'
        capa_dir.mkdir()

        with pytest.raises(MaxUnitsAndMaxInvestmentNonNullSimultaneously):
            check_candidates_file(ini_file, capa_dir)

    def test_null_max_units_and_max_investment_simultaneaously(self, tmp_path):

        ini_file = tmp_path / "a.ini"
        ini_file.touch()
        ini_file.write_text(f"""[5] \n
                            name = alpha \n
                            link = a - b \n
                           max-units = 0\n
                           unit-size = 0\n
                           max-investment = 0""")
        capa_dir = tmp_path / 'capa'
        capa_dir.mkdir()

        with pytest.raises(MaxUnitsAndMaxInvestmentAreNullSimultaneously):
            check_candidates_file(ini_file, capa_dir)

    def test_profile_file_existence(self, tmp_path):

        ini_file = tmp_path / "a.ini"

        ini_file.touch()
        ini_file.write_text(f"""[5] \n
                            name = alpha \n
                            link = a - b \n
                           max-units = 1\n
                           unit-size = 23\n
                           link-profile = file.ini""")

        with pytest.raises(SplitLinkProfile.LinkProfileFileNotFound):
            check_candidates_file(ini_file, capacity_dir_path=tmp_path)

    def test_no_change_when_profile_exists(self, tmp_path):
        ini_file = tmp_path / "candidate.ini"
        ini_file.touch()
        ini_file.write_text(
            f"""[5]
                            name = alpha
                            link = a - b
                           max-units = 1
                           unit-size = 23
                           direct-link-profile = direct-file.ini
                           indirect-link-profile = direct-file.ini""")
        capa_dir = tmp_path / 'capa'
        capa_dir.mkdir()
        profile_file = capa_dir / "direct-file.ini"
        profile_file.touch()
        with open(profile_file, 'w') as f:
            f.writelines(["0\n" for k in range(8760)])
        profile = ProfileLinkChecker(
            ini_file, capa_dir)
        assert _check_attribute_profile_values(profile.config, capa_dir) == False

class TestCheckSettingOptionType:

    def test_check_setting_option_type(self):

        with pytest.raises(NotHandledOption):
            _check_setting_option_type("unknown option", "value")

    def test_str_options(self):

        assert _check_setting_option_type(
            "uc_type", "expansion_accurate") == True
        assert _check_setting_option_type("uc_type", 123) == False
        assert _check_setting_option_type("master", "a string") == True

    def test_int_options(self):

        assert _check_setting_option_type("timelimit", 1) == True
        assert _check_setting_option_type("timelimit", "str") == False
        assert _check_setting_option_type("timelimit", -1) == True
        assert _check_setting_option_type("timelimit", "inf") == False
        assert _check_setting_option_type("timelimit", "+Inf") == True
        assert _check_setting_option_type("timelimit", 12.2) == False

    def test_double_options(self):

        assert _check_setting_option_type("relative_gap", 1.) == True
        assert _check_setting_option_type("relative_gap", -1.) == True
        assert _check_setting_option_type("relative_gap", "str") == False


class TestCheckSettingOptionValue:

    def test_optimality_gap_negative_int(self):
        with pytest.raises(GapValueError):
            _check_setting_option_value("optimality_gap", -123)

    def test_optimality_gap_str_value(self):

        with pytest.raises(OptionTypeError):
            _check_setting_option_value("optimality_gap", "defe")

    def test_optimality_gap_negative_float(self):

        with pytest.raises(GapValueError):
            _check_setting_option_value("optimality_gap", -1.2)

    def test_float_max_iteration(self):

        with pytest.raises(OptionTypeError):
            _check_setting_option_value("max_iteration", -1.2)

    def test_negative_max_iteration(self):

        with pytest.raises(MaxIterValueError):
            _check_setting_option_value("max_iteration", -2)

    def test_wrong_time_limit(self):

        with pytest.raises(TimelimitValueError):
            _check_setting_option_value("timelimit", -30)

    def test_log_level(self):

        with pytest.raises(LogLevelValueError):
            _check_setting_option_value("log_level", -30)

    def test_separation_parameter_illegal_value(self):
        with pytest.raises(SeparationParameterValueError):
            _check_setting_option_value("separation_parameter", -1)
    
    def test_separation_parameter_legal_value(self):
        assert _check_setting_option_value("separation_parameter", 0.39) == True
