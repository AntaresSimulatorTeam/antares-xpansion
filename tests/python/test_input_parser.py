import pytest

from antares_xpansion.input_parser import InputParser
from antares_xpansion.xpansionConfig import InputParameters


class TestInputParser:

    def setup_method(self):
        self.default_parameters = {"step": "full"}

    def test_empty_input_raises_error(self):
        my_parser = InputParser()
        with pytest.raises(SystemExit):
            my_parser.parse_args([])

    def test_with_data_dir_default_parameters_are_set(self):
        my_parser = InputParser()
        result: InputParameters = my_parser.parse_args(["--dataDir=hello"])
        assert result.data_dir == "hello"
        assert result.step == self.default_parameters["step"]

    def test_that_lats_is_the_default_value_of_simulation_name(self):
        my_parser = InputParser()
        result = my_parser.parse_args(["--dataDir=hello"])
        assert result.simulation_name == "last"

    def test_step_fully_is_not_accepted(self):
        my_parser = InputParser()
        with pytest.raises(SystemExit):
            my_parser.parse_args(["--dataDir=hello", "--step=fully"])

    def test_problem_generation_step_is_accepted(self):
        my_parser = InputParser()
        my_parser.parse_args(["--dataDir=hello", "--step=problem_generation"])

    def test_benders_step_is_accepted(self):
        my_parser = InputParser()
        my_parser.parse_args(["--dataDir=hello", "--step=benders"])

    def test_study_update_step_is_accepted(self):
        my_parser = InputParser()
        my_parser.parse_args(["--dataDir=hello", "--step=study_update"])

    def test_resume_step_is_accepted(self):
        my_parser = InputParser()
        my_parser.parse_args(["--dataDir=hello", "--step=resume"])

    def test_getnames_step_is_no_longer_accepted(self):
        my_parser = InputParser()
        with pytest.raises(SystemExit):
            my_parser.parse_args(["--dataDir=hello", "--step=getnames"])

    def test_lp_step_is_no_longer_accepted(self):
        my_parser = InputParser()
        with pytest.raises(SystemExit):
            my_parser.parse_args(["--dataDir=hello", "--step=lp"])

    def test_optim_step_is_no_longer_accepted(self):
        my_parser = InputParser()
        with pytest.raises(SystemExit):
            my_parser.parse_args(["--dataDir=hello", "--step=optim"])

    def test_update_step_is_no_longer_accepted(self):
        my_parser = InputParser()
        with pytest.raises(SystemExit):
            my_parser.parse_args(["--dataDir=hello", "--step=update"])

    def test_antares_n_cpu_default_value_is_one(self):
        my_parser = InputParser()
        result = my_parser.parse_args(["--dataDir=hello", ])
        assert result.antares_n_cpu == 1

    def test_antares_n_cpu_default_value_is_equal_to_np_if_np_is_given(self):
        my_parser = InputParser()
        result = my_parser.parse_args(["--dataDir=hello", "--np=4"])
        assert result.antares_n_cpu == 4

    def test_antares_n_cpu_default_value_is_1_if_np_is_2(self):
        my_parser = InputParser()
        result = my_parser.parse_args(["--dataDir=hello", "--np=2"])
        assert result.antares_n_cpu == 1

    def test_oversubscribe_default_is_false(self):
        my_parser = InputParser()
        result = my_parser.parse_args(["--dataDir=hello", "--np=2"])
        assert result.oversubscribe is False

    def test_if_option_oversubscribe_then_oversubscribe_is_true(self):
        my_parser = InputParser()
        result = my_parser.parse_args(["--dataDir=hello", "--oversubscribe"])
        assert result.oversubscribe is True

    def test_if_option_zip_mps_then_zip_mps_is_true(self):
        my_parser = InputParser()
        result = my_parser.parse_args(["--dataDir=hello", "--zip-mps"])
        assert result.zip_mps is True
