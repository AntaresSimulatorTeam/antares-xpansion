import pytest
import os
from pathlib import Path

from antares_xpansion.input_checker import *


class TestCheckProfileFile:

    def test_no_existing_file(self, tmp_path):
        profile_file = tmp_path / "ghost" / "file"

        with pytest.raises(ProfileFileNotExists):
            check_profile_file(profile_file)

    def test_empty_profile(self, tmp_path):
        profile_file = TestCheckProfileFile.get_empty_file(tmp_path)

        with pytest.raises(ProfileFileWrongNumberOfLines):
            check_profile_file(profile_file)

    def test_invalid_profile(self, tmp_path):
        profile_file = TestCheckProfileFile.get_empty_file(tmp_path)

        line = 'Word'
        profile_file.write_text(line)
        with pytest.raises(ProfileFileValueError):
            check_profile_file(profile_file)

    def test_3_columns_in_profile_file(self, tmp_path):
        profile_file = TestCheckProfileFile.get_empty_file(tmp_path)
        line = "1 2 3"

        profile_file.write_text(line)

        with pytest.raises(ProfileFileWrongNumberOfcolumns):
            check_profile_file(profile_file)

    def test_negative_values_in_profile_file(self, tmp_path):
        profile_file = TestCheckProfileFile.get_empty_file(tmp_path)
        line = "-0.5 10.02"

        profile_file.write_text(line)

        with pytest.raises(ProfileFileNegativeValue):
            check_profile_file(profile_file)

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
            check_candidate_option_type(option, value)

    def test_fail_if_max_units_value_is_negative(self):

        option = "max-units"
        value = -1545

        assert check_candidate_option_type(option, value) == False

    def test_fail_if_max_units_value_is_str(self):

        option = "max-units"
        value = "str"

        assert check_candidate_option_type(option, value) == False

    def test_true_for_obsolete_value(self):

        option = "has-link-profile"
        value = "str"

        assert check_candidate_option_type(option, value) == True

    def test_true_for_string_option_type(self):

        option = "link"
        value = "0-1"

        assert check_candidate_option_type(option, value) == True


class TestcheckCandidateOptionValue:

    def test_fail_if_relaxed_is_not_str(self):
        option = "relaxed"
        value = 22

        with pytest.raises(IllegalCandidateOptionValue):
            check_candidate_option_value(option, value)

    def test_accepted_relaxed_value(self):
        option = "relaxed"

        assert check_candidate_option_value(option, "true") == True
        assert check_candidate_option_value(option, "True") == True
        assert check_candidate_option_value(option, "FalSe") == True
        assert check_candidate_option_value(option, "FALSE") == True


class TestCheckCandidateName:

    def test_empty_candidate_name(self):

        with pytest.raises(EmptyCandidateName):
            check_candidate_name("", "section1236")

    def test_illegal_chars_in_candidate_name(self):

        with pytest.raises(IllegalCharsInCandidateName):
            check_candidate_name("newline\n123", "section1236")

            check_candidate_name("escapeR\r123", "section1236")

            check_candidate_name("tab\t123", "section1236")

            check_candidate_name("escapeF\f\v123", "section1236")

            check_candidate_name("escapeV\v123", "section1236")

            check_candidate_name("parenthesis(123", "section1236")


class TestCheckCandidateLink:

    def test_empty_link(self):

        with pytest.raises(EmptyCandidateLink):
            check_candidate_link("", "section0")

    def test_fail_if_link_doesnt_contains_separator(self):

        with pytest.raises(CandidateLinkWithoutSeparator):

            check_candidate_link("atob", "section")
            check_candidate_link("ato-b", "section")
            check_candidate_link("ato- b", "section")
            check_candidate_link("ato -b", "section")


class TestCheckCandidatesFile:

    def test_wrong_option_type_ini_file(self, tmp_path):

        option = "max-units"
        value = -1545

        ini_file = tmp_path / "a.ini"
        ini_file.touch()
        ini_file.write_text(f"""[5] \n
                           {option} = {value}""")

        with pytest.raises(CandidateFileWrongTypeValue):
            check_candidates_file(ini_file, "")

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

        with pytest.raises(CandidateNameDuplicatedError):
            check_candidates_file(ini_file, "")

    def test_non_null_max_units_and_max_investment_simultaneaously(self, tmp_path):

        ini_file = tmp_path / "a.ini"
        ini_file.touch()
        ini_file.write_text(f"""[5] \n
                            name = alpha \n
                            link = a - b \n
                           max-units = 13\n
                           unit-size = 400\n
                           max-investment = 985""")

        with pytest.raises(MaxUnitsAndMaxInvestmentNonNullSimultaneously):
            check_candidates_file(ini_file, "")

    def test_null_max_units_and_max_investment_simultaneaously(self, tmp_path):

        ini_file = tmp_path / "a.ini"
        ini_file.touch()
        ini_file.write_text(f"""[5] \n
                            name = alpha \n
                            link = a - b \n
                           max-units = 0\n
                           unit-size = 0\n
                           max-investment = 0""")

        with pytest.raises(MaxUnitsAndMaxInvestmentAreNullSimultaneously):
            check_candidates_file(ini_file, "")

    def test_profile_file_existence(self, tmp_path):

        ini_file = tmp_path / "a.ini"
        ini_file.touch()
        ini_file.write_text(f"""[5] \n
                            name = alpha \n
                            link = a - b \n
                           max-units = 1\n
                           unit-size = 23\n
                           link-profile = file.ini""")

        with pytest.raises(ProfileFileNotExists):
            check_candidates_file(ini_file, capacity_dir_path=tmp_path)

    # def test_remove_candidate(self, tmp_path):

    #     ini_file = tmp_path / "a.ini"
    #     ini_file.touch()
    #     ini_file.write_text(f"""[5] \n
    #                         name = alpha \n
    #                         link = a - b \n
    #                        max-units = 1\n
    #                        unit-size = 23\n
    #                        link-profile = 1""")

    #     # with pytest.raises(ProfileFileNotExists):
    #     check_candidates_file(ini_file, capacity_dir_path=tmp_path)

    #     with open(ini_file) as ini:
    #         print(ini.readlines())

    #     assert False


class TestCheckSettingOptionType:

    def test_check_setting_option_type(self):

        with pytest.raises(NotHandledOption):
            check_setting_option_type("unknown option", "value")

    def test_str_options(self):

        assert check_setting_option_type("method", "sequential") == True
        assert check_setting_option_type("method", 123) == False

    def test_int_options(self):

        assert check_setting_option_type("timelimit", 1) == True
        assert check_setting_option_type("timelimit", "str") == False
        assert check_setting_option_type("timelimit", -1) == True
        assert check_setting_option_type("timelimit", "inf") == False
        assert check_setting_option_type("timelimit", "+Inf") == True

    def test_double_options(self):

        assert check_setting_option_type("relative_gap", 1.) == True
        assert check_setting_option_type("relative_gap", -1.) == True
        assert check_setting_option_type("relative_gap", "str") == False


class TestCheckSettingOptionValue:

    def test_optimality_gap_str_value(self):
        with pytest.raises(OptionTypeError):
            check_setting_option_value("optimality_gap", -123)

    def test_optimality_gap_str_value(self):

        with pytest.raises(OptionTypeError):
            check_setting_option_value("optimality_gap", "defe")
