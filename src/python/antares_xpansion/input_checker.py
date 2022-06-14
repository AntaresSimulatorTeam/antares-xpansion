"""
    module to perform checks on antares xpansion input data
"""

import os
import shutil
import sys

from antares_xpansion.flushed_print import flushed_print
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


def _check_profile_file_consistency(filename_path):
    with open(filename_path, 'r') as profile_file:
        first_profile = []
        for idx, line in enumerate(profile_file):
            try:
                line_vals = line.strip().split()
                if (len(line_vals) != 0):
                    first_profile.append(float(line_vals[0]))
            except ValueError:
                flushed_print('Line %d in file %s is not valid: allowed float values in formats "X" or "X\tY".'
                              % (idx + 1, filename_path))
                raise ProfileFileValueError
            if (first_profile[-1] < 0):
                flushed_print('Line %d in file %s indicates a negative value'
                              % (idx + 1, filename_path))
                raise ProfileFileNegativeValue

    if len(first_profile) != 8760:
        flushed_print('file %s does not have 8760 lines'
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
        flushed_print(f'{filename_path} is not an existent file')
        raise ProfileFileNotExists

    return _check_profile_file_consistency(filename_path)


##########################################
# Checks related to candidates.ini
##########################################


class UnrecognizedCandidateOptionType(Exception):
    pass


candidate_options_type = {'name': 'string',
                          'link': 'string',
                          'annual-cost-per-mw': 'non-negative',
                          'unit-size': 'non-negative',
                          'max-units': 'non-negative',
                          'max-investment': 'non-negative',
                          'direct-link-profile': 'string',
                          'indirect-link-profile': 'string',
                          'already-installed-capacity': 'non-negative',
                          'already-installed-direct-link-profile': 'string',
                          'already-installed-indirect-link-profile': 'string',
                          'has-link-profile': 'string'}


def _check_candidate_option_type(option, value):
    """
        verifies if a given option value has the correct type corresponding allowed for this option

        :param option: the treated option
        :param value: the value assigned to the option

        :return: True if the value has an appropriate type, False or exist otherwise
    """

    obsolete_options = ["c", 'enable',
                        'candidate-type', 'investment-type', 'relaxed', 'link-profile',
                        "already-installed-link-profile"]
    option_type = candidate_options_type.get(option)
    if option_type is None:
        flushed_print(
            'check_candidate_option_type: %s option not recognized in candidates file.' % option)
        flushed_print(f"Authorized options are: ", *
        candidate_options_type, sep="\n")
        raise UnrecognizedCandidateOptionType
    else:
        if obsolete_options.count(option):
            flushed_print(
                '%s option is no longer used by antares-xpansion' % option)
            return True
        if option_type == 'string':
            return True
        elif option_type == 'non-negative':
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
            flushed_print('Error candidates name should not contain %s, found in section %s in "%s"' % (
                c, section, name))
            raise IllegalCharsInCandidateName


def _verify_name_is_not_empty(name, section):
    if (not name) or (name.lower() == "na"):
        flushed_print(
            'Error candidates name cannot be empty : found in section %s' % section)
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
        flushed_print(
            'Error candidates link cannot be empty : found in section %s' % section)
        raise EmptyCandidateLink
    if " - " not in link:
        flushed_print(
            'Error candidates link value must contain " - " : found in section %s' % section)
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
    for each_section in ini_file.sections():
        for (option, value) in ini_file.items(each_section):
            if not _check_candidate_option_type(option, value):
                flushed_print(
                    f"value {value} for option {option} has the wrong type!, it has to be {candidate_options_type[option]}")
                raise CandidateFileWrongTypeValue


def _check_name_is_unique(ini_file):
    unique_attributes = ["name"]
    for verified_attribute in unique_attributes:
        unique_values = set()
        for each_section in ini_file.sections():
            value = ini_file[each_section][verified_attribute].strip().lower()
            if value in unique_values:
                flushed_print('Error candidates %ss have to be unique, duplicate %s %s in section %s'
                              % (verified_attribute, verified_attribute, value, each_section))
                raise CandidateNameDuplicatedError
            else:
                unique_values.add(value)


def _check_candidate_name_and_link(ini_file):
    # check that name is not empty and does not have space
    # check that link is not empty
    for each_section in ini_file.sections():
        _check_candidate_name(
            ini_file[each_section]['name'].strip(), each_section)
        _check_candidate_link(
            ini_file[each_section]['link'].strip(), each_section)

    _check_name_is_unique(ini_file)


def _check_candidate_exclusive_attributes(ini_file):
    # check exclusion between max-investment and (max-units, unit-size) attributes
    for each_section in ini_file.sections():
        max_invest = ini_file.getfloat(
            each_section, 'max-investment') if ini_file.has_option(each_section, 'max-investment') else 0
        unit_size = ini_file.getfloat(
            each_section, 'unit-size') if ini_file.has_option(each_section, 'unit-size') else 0
        max_units = ini_file.getfloat(
            each_section, 'max-units') if ini_file.has_option(each_section, 'max-units') else 0
        if max_invest != 0:
            if max_units != 0 or unit_size != 0:
                flushed_print(
                    f"Illegal values in section {each_section}: cannot assign non-null values simultaneously to max-investment and (unit-size or max_units)")
                raise MaxUnitsAndMaxInvestmentNonNullSimultaneously
        elif max_units == 0 or unit_size == 0:
            flushed_print(
                f"Illegal values in section {each_section}: need to assign non-null values to max-investment or (unit-size and max_units)")
            raise MaxUnitsAndMaxInvestmentAreNullSimultaneously


def _copy_in_backup(ini_file, candidates_ini_filepath):
    backup_path = candidates_ini_filepath.parent / f"{candidates_ini_filepath.name}.bak"
    shutil.copyfile(candidates_ini_filepath,
                    backup_path)
    with open(candidates_ini_filepath, 'w') as out_file:
        ini_file.write(out_file)
    flushed_print("%s file was overwritten! backup file %s created"
                  % (candidates_ini_filepath, backup_path))


def _check_attribute_profile_values(ini_file, capacity_dir_path):
    # check attributes profile is 0, 1 or an existent filename
    config_changed = False
    profile_attributes = ['direct-link-profile',
                          'indirect-link-profile', 'already-installed-direct-link-profile',
                          'already-installed-indirect-link-profile']
    for each_section in ini_file.sections():
        has_a_profile = False
        for attribute in profile_attributes:
            value = ini_file[each_section][attribute].strip(
            ) if ini_file.has_option(each_section, attribute) else "1"
            if value == '0':
                continue
            elif value == '1':
                has_a_profile = True
            else:
                # check file existence
                filename_path = os.path.normpath(
                    os.path.join(capacity_dir_path, value))
                if not os.path.isfile(filename_path):
                    flushed_print('Illegal value : option can be 0, 1 or an existent filename.\
                            %s is not an existent file' % filename_path)
                    raise ProfileFileNotExists
                has_a_profile = has_a_profile or _check_profile_file(
                    filename_path)
        if not has_a_profile:
            # remove candidate if it has no profile
            flushed_print("candidate %s will be removed!" %
                          ini_file[each_section]["name"])
            ini_file.remove_section(each_section)
            config_changed = True

    return config_changed


def _check_attributes_profile(ini_file, candidates_ini_filepath, capacity_dir_path):
    # check attributes profile is 0, 1 or an existent filename
    if _check_attribute_profile_values(ini_file, capacity_dir_path):
        _copy_in_backup(ini_file, candidates_ini_filepath)


def check_candidates_file(candidates_ini_filepath, capacity_dir_path):
    default_values = {'name': 'NA',
                      'link': 'NA',
                      'annual-cost-per-mw': '0',
                      'unit-size': '0',
                      'max-units': '0',
                      'max-investment': '0',
                      'direct-link-profile': '1',
                      'indirect-link-profile': '1',
                      'already-installed-capacity': '0',
                      'already-installed-link-profile': '1'}

    profile_link_checker = ProfileLinkChecker(
        candidates_ini_filepath, capacity_dir_path)
    if profile_link_checker.update():
        profile_link_checker.write()

    ini_file = profile_link_checker.config

    _check_candidate_attributes(ini_file)
    _check_candidate_name_and_link(ini_file)
    _check_candidate_exclusive_attributes(ini_file)
    _check_attributes_profile(
        ini_file, candidates_ini_filepath, capacity_dir_path)


##########################################
# Checks related to settings.ini
##########################################


class NotHandledOption(Exception):
    pass


class NotHandledValue(Exception):
    pass


type_str = str
type_int = int
type_float = float

# "option": (type, legal_value(s))
options_types_and_legal_values = {
    "uc_type": (type_str, ["expansion_accurate", "expansion_fast"]),
    "master": (type_str, ["relaxed", "integer", "full_integer"]),
    "optimality_gap": (type_float, None),
    "relative_gap": (type_float, None),
    "week_selection": (type_str, ["true", "false"]),
    "max_iteration": (type_int, None),
    "relaxed_optimality_gap": (type_str, None),
    "solver": (type_str, None),
    "timelimit": (type_int, None),
    "yearly-weights": (type_str, None),
    "additional-constraints": (type_str, None),
    "log_level": (type_int, ["0", "1", "2", "3"])
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
        flushed_print(
            'check_setting_option_type: Illegal %s option in settings file.' % option)
        raise NotHandledOption

    option_type = options_types_and_legal_values.get(option)[0]

    if option_type == type_float:
        try:
            float(value)
            return True
        except ValueError:
            flushed_print(
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
                    flushed_print(
                        'check_setting_option_type: Illegal %s option in type, integer is expected .' % option)
                    return False
            except ValueError:
                flushed_print(
                    'check_setting_option_type: Illegal %s option in type, integer is expected .' % option)
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
            flushed_print(
                f"Illegal {value} for option max_iteration : only -1 or positive values are allowed")
            raise MaxIterValueError


def _check_timelimit(value) -> bool:
    if value in INFINITY_LIST:
        return True
    else:
        if int(value) > 0:
            return True
        else:
            flushed_print(
                f"Illegal {value} for option timelimit : only positive values are allowed")
            raise TimelimitValueError


def _check_log_level(value) -> bool:
    if int(value) >= 0:
        return True
    else:
        flushed_print(
            f"Illegal {value} for option log_lvel : only greater than or equal to zero values are accepted")
        raise LogLevelValueError


def _check_setting_option_value(option, value):
    """
        checks that an option has a legal value

        :param option: name of the option to verify from settings file
        :param value: value of the option to verify

        :return: True if the option has the correct type, exits if the value has the wrong type
    """

    if not _check_setting_option_type(option, value):
        flushed_print(
            "check_settings : value %s for option %s has the wrong type!" % (value, option))
        raise OptionTypeError

    legal_values = options_types_and_legal_values.get(option)[1]

    skip_verif = ["yearly-weights", "additional-constraints", "solver"]

    if ((legal_values is not None) and (value in legal_values)) or (option in skip_verif):
        return True

    if (option == "optimality_gap") or (option == "relative_gap"):
        if float(value) >= 0:
            return True
        else:
            flushed_print(
                "Illegal value %s for option %s : only positive values are allowed"
                % (value, option)
            )
            raise GapValueError

    elif option == 'max_iteration':
        return _check_max_iteration(value)

    elif option == 'timelimit':
        return _check_timelimit(value)

    elif option == 'log_level':
        return _check_log_level(value)

    flushed_print(
        'check_candidate_option_value: Illegal value %s for option %s' % (value, option))
    sys.exit(1)
