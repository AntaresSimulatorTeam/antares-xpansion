"""
    module to perform checks on antares xpansion input data
"""

import os
import shutil
import sys

from antares_xpansion.logger import step_logger
from antares_xpansion.profile_link_checker import ProfileLinkChecker


class ProfileFileNotExists(Exception):
    pass


class ProfileFileWrongNumberOfLines(Exception):
    pass


class ProfileFileWrongNumberOfcolumns(Exception):
    pass


class ProfileFileValueError(Exception):
    pass


class ProfileFileNegativeValue(Exception):
    pass


INFINITY_LIST = ["+Inf", "+infini"]

logger = step_logger(__name__, "input checks")


def _check_profile_file_consistency(filename_path):
    with open(filename_path, "r") as profile_file:
        first_profile = []
        for idx, line in enumerate(profile_file):
            try:
                line_vals = line.strip().split()
                if len(line_vals) != 0:
                    first_profile.append(float(line_vals[0]))
            except ValueError:
                logger.error('Line %d in file %s is not valid: allowed float values in formats "X" or "X\tY".'
                             % (idx + 1, filename_path))
                raise ProfileFileValueError
            if (first_profile[-1] < 0):
                logger.error('Line %d in file %s indicates a negative value'
                             % (idx + 1, filename_path))
                raise ProfileFileNegativeValue

    if len(first_profile) != 8760:
        logger.error('file %s does not have 8760 lines'
                     % filename_path)
        raise ProfileFileWrongNumberOfLines
    return any(first_profile)


def _check_profile_file(filename_path):
    """
    verifies if a given profile file is valid and indicates if it is a null profile or not

    :param filename_path: path to the profile file to check

    :return: returns False if the profile is null
    """
    # check file existence
    if not os.path.isfile(filename_path):
        logger.error(f'{filename_path} is not an existent file')
        raise ProfileFileNotExists

    return _check_profile_file_consistency(filename_path)


##########################################
# Checks related to candidates.ini
##########################################


class UnrecognizedCandidateOptionType(Exception):
    pass


candidate_options_type = {
    "name": "string",
    "link": "string",
    "annual-cost-per-mw": "non-negative",
    "unit-size": "non-negative",
    "max-units": "non-negative",
    "max-investment": "non-negative",
    "direct-link-profile": "string",
    "indirect-link-profile": "string",
    "already-installed-capacity": "non-negative",
    "already-installed-direct-link-profile": "string",
    "already-installed-indirect-link-profile": "string",
}


def _check_candidate_option_type(option, value):
    """
    verifies if a given option value has the correct type corresponding allowed for this option

    :param option: the treated option
    :param value: the value assigned to the option

    :return: True if the value has an appropriate type, False or exist otherwise
    """

    obsolete_options = [
        "c",
        "enable",
        "candidate-type",
        "investment-type",
        "relaxed",
        "has-link-profile",
    ]

    if obsolete_options.count(option):
        logger.warning(
            f"{option} option is no longer used by antares-xpansion")
        return True
    else:
        option_type = candidate_options_type.get(option)
        if option_type is None:
            logger.error(
                f"check_candidate_option_type: {option} option not recognized in candidates file.\n" +
                "Authorized options are: %s"+'\n'.join(
                    candidate_options_type.keys()))
            raise UnrecognizedCandidateOptionType
        if option_type == "string":
            return True
        elif option_type == "non-negative":
            try:
                return float(value) >= 0
            except ValueError:
                return False


class EmptyCandidateName(Exception):
    pass


class IllegalCharsInCandidateName(Exception):
    pass


def _check_candidate_name(name, section):
    """
    checks that the candidate's name is not empty and does not contain a space
    """
    _verify_name_is_not_empty(name, section)
    _verify_name_has_no_invalid_character(name, section)


def _verify_name_has_no_invalid_character(name, section):
    illegal_chars = " \n\r\t\f\v-+=:[]()"
    for c in illegal_chars:
        if c in name:
            logger.error('Candidates name should not contain %s, found in section %s in "%s"' % (
                c, section, name))
            raise IllegalCharsInCandidateName


def _verify_name_is_not_empty(name, section):
    if (not name) or (name.lower() == "na"):
        logger.error(
            'Candidates name cannot be empty : found in section %s' % section)
        raise EmptyCandidateName


class EmptyCandidateLink(Exception):
    pass


class CandidateLinkWithoutSeparator(Exception):
    pass


def _check_candidate_link(link, section):
    """
    checks that the candidate's link is not empty
    """
    if (not link) or (link.lower() == "na"):
        logger.error(
            'Candidates link cannot be empty : found in section %s' % section)
        raise EmptyCandidateLink
    if " - " not in link:
        logger.error(
            'Candidates link value must contain " - " : found in section %s' % section)
        raise CandidateLinkWithoutSeparator


class CandidateFileWrongTypeValue(Exception):
    pass


class CandidateNameDuplicatedError(Exception):
    pass


class MaxUnitsAndMaxInvestmentNonNullSimultaneously(Exception):
    pass


class MaxUnitsAndMaxInvestmentAreNullSimultaneously(Exception):
    pass


#############################################
# check candidate attributes types and values
#############################################


def _check_candidate_attributes(ini_file):
    # check attributes types and values
    err_msg = ""
    for each_section in ini_file.sections():
        for (option, value) in ini_file.items(each_section):
            if not _check_candidate_option_type(option, value):
                err_msg += f"value {value} for option {option} has the wrong type, it has to be {candidate_options_type[option]}\n"
    
    if(err_msg!=""):
        logger.error(err_msg)                
        raise CandidateFileWrongTypeValue


def _check_name_is_unique(ini_file):
    unique_attributes = ["name"]
    for verified_attribute in unique_attributes:
        unique_values = set()
        for each_section in ini_file.sections():
            value = ini_file[each_section][verified_attribute].strip().lower()
            if value in unique_values:
                logger.error('Error candidates %ss have to be unique, duplicate %s %s in section %s'
                             % (verified_attribute, verified_attribute, value, each_section))
                raise CandidateNameDuplicatedError
            else:
                unique_values.add(value)


def _check_candidate_name_and_link(ini_file):
    # check that name is not empty and does not have space
    # check that link is not empty
    for each_section in ini_file.sections():
        _check_candidate_name(ini_file[each_section]["name"].strip(), each_section)
        _check_candidate_link(ini_file[each_section]["link"].strip(), each_section)

    _check_name_is_unique(ini_file)


def _check_candidate_exclusive_attributes(ini_file):
    # check exclusion between max-investment and (max-units, unit-size) attributes
    for each_section in ini_file.sections():
        max_invest = (
            ini_file.getfloat(each_section, "max-investment")
            if ini_file.has_option(each_section, "max-investment")
            else 0
        )
        unit_size = (
            ini_file.getfloat(each_section, "unit-size")
            if ini_file.has_option(each_section, "unit-size")
            else 0
        )
        max_units = (
            ini_file.getfloat(each_section, "max-units")
            if ini_file.has_option(each_section, "max-units")
            else 0
        )
        if max_invest != 0:
            if max_units != 0 or unit_size != 0:
                logger.error(
                    f"Illegal values in section {each_section}: cannot assign non-null values simultaneously to max-investment and (unit-size or max_units)")
                raise MaxUnitsAndMaxInvestmentNonNullSimultaneously
        elif max_units == 0 or unit_size == 0:
            logger.error(
                f"Illegal values in section {each_section}: need to assign non-null values to max-investment or (unit-size and max_units)")
            raise MaxUnitsAndMaxInvestmentAreNullSimultaneously


def _copy_in_backup(ini_file, candidates_ini_filepath):
    backup_path = candidates_ini_filepath.parent / f"{candidates_ini_filepath.name}.bak"
    shutil.copyfile(candidates_ini_filepath, backup_path)
    with open(candidates_ini_filepath, "w") as out_file:
        ini_file.write(out_file)
    logger.error("%s file was overwritten! backup file %s created"
                 % (candidates_ini_filepath, backup_path))


def _check_attribute_profile_values(ini_file, capacity_dir_path):
    # check attributes profile is 0, 1 or an existent filename
    config_changed = False
    profile_attributes = [
        "direct-link-profile",
        "indirect-link-profile",
        "already-installed-direct-link-profile",
        "already-installed-indirect-link-profile",
    ]
    for each_section in ini_file.sections():
        for attribute in profile_attributes:
            if ini_file.has_option(each_section, attribute):
                value = ini_file[each_section][attribute].strip()
                # check file existence
                filename_path = os.path.normpath(os.path.join(capacity_dir_path, value))
                if not os.path.isfile(filename_path):
                    logger.error('Illegal value : option can be 0, 1 or an existent filename.\
                            %s is not an existent file' % filename_path)
                    raise ProfileFileNotExists
    return config_changed


def _check_attributes_profile(ini_file, candidates_ini_filepath, capacity_dir_path):
    # check attributes profile is 0, 1 or an existent filename
    if _check_attribute_profile_values(ini_file, capacity_dir_path):
        _copy_in_backup(ini_file, candidates_ini_filepath)


def check_candidates_file(candidates_ini_filepath, capacity_dir_path):
    profile_link_checker = ProfileLinkChecker(
        candidates_ini_filepath, capacity_dir_path
    )
    if profile_link_checker.update():
        profile_link_checker.write()

    ini_file = profile_link_checker.config

    _check_candidate_attributes(ini_file)
    _check_candidate_name_and_link(ini_file)
    _check_candidate_exclusive_attributes(ini_file)
    _check_attributes_profile(ini_file, candidates_ini_filepath, capacity_dir_path)


##########################################
# Checks related to settings.ini
##########################################


class NotHandledOption(Exception):
    pass


class NotHandledValue(Exception):
    pass

# return ->tuple[is_a_bool: bool, result: bool]


# -> tuple[bool, bool]: not working with python <3.9
def str_to_bool(my_str: str):
    if my_str in ["true", "True", "TRUE", "1"]:
        return (True, True)
    elif my_str in ["false", "False", "False", "0"]:
        return (True, False)
    else:
        return (False, False)

type_str = str
type_int = int
type_float = float
type_bool = bool


# "option": (type, legal_value(s))
options_types_and_legal_values = {
    "uc_type": (type_str, ["expansion_accurate", "expansion_fast"]),
    "master": (type_str, ["relaxed", "integer"]),
    "optimality_gap": (type_float, None),
    "relative_gap": (type_float, None),
    "relaxed_optimality_gap": (type_float, None),
    "max_iteration": (type_int, None),
    "solver": (type_str, None),
    "timelimit": (type_int, None),
    "yearly-weights": (type_str, None),
    "additional-constraints": (type_str, None),
    "log_level": (type_int, ["0", "1", "2", "3"]),
    "separation_parameter": (type_float, None),
    "batch_size": (type_int, None),
}


def _check_setting_option_type(option, value):
    """
    checks that a given option value has the correct type

    :param option: name of the option to verify from settings file
    :param value: value of the option to verify

    :return: True if the option has the correct type,
             False or exits if the value has the wrong type
    """

    if options_types_and_legal_values.get(option) is None:
        logger.error(
            'check_setting_option_type: Illegal %s option in settings file.' % option)
        raise NotHandledOption

    option_type = options_types_and_legal_values.get(option)[0]

    if option_type == type_float:
        try:
            float(value)
            return True
        except ValueError:
            logger.error(
                'check_setting_option_type: Illegal %s option in type, numerical value is expected .' % option)
            return False
    elif option_type == type_int:
        if value in INFINITY_LIST:
            return True
        else:
            try:
                if float(value).is_integer():
                    return True
                else:
                    logger.error(
                        'check_setting_option_type: Illegal %s option in type, integer is expected .' % option)
                    return False
            except ValueError:
                logger.error(
                    'check_setting_option_type: Illegal %s option in type, integer is expected .' % option)
                return False
    elif option_type == type_bool:
        [is_a_bool, ret] = str_to_bool(value)
        if is_a_bool:
            return True
        else:
            logger.error(
                'check_setting_option_type: Illegal %s option in type, boolean is expected .' % option)
            return False

    return isinstance(value, type_str)


class OptionTypeError(Exception):
    pass


class GapValueError(Exception):
    pass


class MaxIterValueError(Exception):
    pass


class TimelimitValueError(Exception):
    pass


class LogLevelValueError(Exception):
    pass


class SeparationParameterValueError(Exception):
    pass


class BatchSizeValueError(Exception):
    pass


class ExpertLogsValueError(Exception):
    pass


def check_options(options):
    """
    checks that a settings file related to an XpansionDriver has the correct format
    Exits if the candidates files has the wrong format.

    :param options: the options obtained from the settings.ini file

    :return:
    """

    option_items = options.items()
    for (option, value) in option_items:
        _check_setting_option_value(option, value)


def _check_max_iteration(value) -> bool:
    if value in INFINITY_LIST:
        return True
    else:
        max_iter = int(value)
        if (max_iter == -1) or (max_iter > 0):
            return True
        else:
            logger.error(
                f"Illegal {value} for option max_iteration : only -1 or positive values are allowed")
            raise MaxIterValueError


def _check_timelimit(value) -> bool:
    if value in INFINITY_LIST:
        return True
    else:
        if int(value) > 0:
            return True
        else:
            logger.error(
                f"Illegal {value} for option timelimit : only positive values are allowed")
            raise TimelimitValueError


def _check_log_level(value) -> bool:
    if int(value) >= 0:
        return True
    else:
        logger.error(
            f"Illegal {value} for option log_level : only greater than or equal to zero values are accepted")
        raise LogLevelValueError


def _check_batch_size(value) -> bool:
    if int(value) >= 0:
        return True
    else:
        logger.error(
            f"Illegal {value} for option batch_size : only greater than or equal to zero values are accepted")
        raise BatchSizeValueError




def _check_separation(value) -> bool:
    if 0 <= float(value) <= 1:
        return True
    else:
        logger.error(
            f"Illegal {value} for option separation_parameter : only values within the interval [0,1] are accepted")
        raise SeparationParameterValueError


def _check_setting_option_value(option, value):
    """
    checks that an option has a legal value

    :param option: name of the option to verify from settings file
    :param value: value of the option to verify

    :return: True if the option has the correct type, exits if the value has the wrong type
    """

    if not _check_setting_option_type(option, value):
        logger.error(
            "check_settings : value %s for option %s has the wrong type!" % (value, option))
        raise OptionTypeError

    legal_values = options_types_and_legal_values.get(option)[1]

    skip_verif = ["yearly-weights", "additional-constraints", "solver"]

    if ((legal_values is not None) and (value in legal_values)) or (
        option in skip_verif
    ):
        return True

    if (
        (option == "optimality_gap")
        or (option == "relative_gap")
        or (option == "relaxed_optimality_gap")
    ):
        if float(value) >= 0:
            return True
        else:
            logger.error(
                "Illegal value %s for option %s : only positive values are allowed"
                % (value, option)
            )
            raise GapValueError

    elif option == "max_iteration":
        return _check_max_iteration(value)

    elif option == "timelimit":
        return _check_timelimit(value)

    elif option == "log_level":
        return _check_log_level(value)

    elif option == "separation_parameter":
        return _check_separation(value)

    elif option == "batch_size":
        return _check_batch_size(value)

    logger.error(
        'check_candidate_option_value: Illegal value %s for option %s' % (value, option))
    sys.exit(1)
