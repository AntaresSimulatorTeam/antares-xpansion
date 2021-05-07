import os
import sys
from pathlib import Path
from typing import List


class XpansionStudyReader:
    class BaseException(Exception):
        pass

    class InvalidYearsWeightNumber(BaseException):
        pass

    class InvalidYearsWeightValue(BaseException):
        pass

    class OnlyNullYearsWeightValue(BaseException):
        pass

    @staticmethod
    def check_weights_file(filename_path, nb_active_years: int):
        """
            checks that the yearly-weights file exists and has correct format:
                column of non-negative weights
                sum of weights is positive
				nb_weight equal nb_active_yearse
        """

        # check file existence
        if not os.path.isfile(filename_path):
            raise FileNotFoundError('Illegal value : %s is not an existent yearly-weights file'
                                    % filename_path)

        null_weights = True
        nb_values = 0
        with open(filename_path, 'r') as weights_file:
            nb_values, null_weights = XpansionStudyReader._count_values_and_check_if_all_weights_are_null(filename_path,
                                                                                                          weights_file)

        if nb_values != nb_active_years:
            raise XpansionStudyReader.InvalidYearsWeightNumber(
                f'file {str(filename_path)} : invalid weight number : {nb_values} values / {nb_active_years} expected')

        if null_weights:
            raise XpansionStudyReader.OnlyNullYearsWeightValue('file %s : all values are null'
                                                               % filename_path)

        return True

    @staticmethod
    def _count_values_and_check_if_all_weights_are_null(filename_path, weights_file):
        _null_weights = True
        _nb_values = 0
        for idx, line in enumerate(weights_file):
            if line.strip():
                line_value = XpansionStudyReader._get_line_value(line, idx, filename_path)
                _nb_values += 1
                if line_value > 0:
                    _null_weights = False
                if line_value < 0:
                    raise XpansionStudyReader.InvalidYearsWeightValue(
                        'Line %d in file %s indicates a negative value'
                        % (idx + 1, filename_path))
        return _nb_values, _null_weights

    @staticmethod
    def _get_line_value(line, idx, filename_path) -> float:
        try:
            line_value = float(line.strip())
        except ValueError:
            raise XpansionStudyReader.InvalidYearsWeightValue(
                'Line %d in file %s is not a single non-negative value'
                % (idx + 1, filename_path))
        return line_value

    @staticmethod
    def get_years_weight_from_file(file_path: Path) -> List[float]:
        result = []
        with open(file_path, 'r') as weights_file:
            for line in weights_file:
                if line.strip():
                    line_value = float(line.strip())
                    result.append(line_value)

        return result
