import pytest
import os
from pathlib import Path

from antares_xpansion.input_checker import *


class TestCheckProfileFile:

    def test_no_existent_file(self, tmp_path):
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

        with pytest.raises(IllegalCharsInCandidateName):
            check_candidate_name("escapeR\r123", "section1236")

        with pytest.raises(IllegalCharsInCandidateName):
            check_candidate_name("tab\t123", "section1236")

        with pytest.raises(IllegalCharsInCandidateName):
            check_candidate_name("escapeF\f\v123", "section1236")

        with pytest.raises(IllegalCharsInCandidateName):
            check_candidate_name("escapeV\v123", "section1236")

        with pytest.raises(IllegalCharsInCandidateName):
            check_candidate_name("parenthesis(123", "section1236")
