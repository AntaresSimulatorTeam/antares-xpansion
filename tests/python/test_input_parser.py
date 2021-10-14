import pytest

from antares_xpansion.input_parser import InputParser
from antares_xpansion.xpansionConfig import InputParameters


class TestInputParser:

    def setup_method(self):
        self.default_parameters = {"step" :"full"}

    def test_empty_input_raises_error(self):
        my_parser = InputParser()
        with pytest.raises(SystemExit):
            my_parser.parse_args([])

    def test_with_data_dir_default_parameters_are_set(self):
        my_parser = InputParser()
        result: InputParameters = my_parser.parse_args(["--dataDir=hello"])
        assert result.dataDir == "hello"
        assert result.step == self.default_parameters["step"]

    def test_that_lats_is_the_default_value_of_simulation_name(self):
        my_parser = InputParser()
        result = my_parser.parse_args(["--dataDir=hello"])
        assert result.simulationName == "last"

    def test_step_fully_is_not_accepted(self):
        my_parser = InputParser()
        with pytest.raises(SystemExit):
            my_parser.parse_args(["--dataDir=hello", "--step=fully"])




