"""
    module to perform checks on antares xpansion input data
"""

import configparser
import sys
import os
import shutil

from antares_xpansion.flushed_print import flushed_print


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


def check_profile_file(filename_path):
    """
        verifies if a given profile file is valid and indicates if it is a null profile or not

        :param filename_path: path to the profile file to check

        :return: returns False if the profile is null
    """
    # check file existence
    if not os.path.isfile(filename_path):
        flushed_print(f'{filename_path} is not an existent file')
        raise ProfileFileNotExists

    two_profiles = False
    with open(filename_path, 'r') as profile_file:
        two_profiles = (len(profile_file.readline().strip().split()) == 2)

    with open(filename_path, 'r') as profile_file:
        first_profile = []
        indirect_profile = []
        for idx, line in enumerate(profile_file):
            try:
                line_vals = line.strip().split()

                if (len(line_vals) == 1) and not two_profiles:
                    first_profile.append(float(line_vals[0]))
                elif (len(line_vals) == 2) and two_profiles:
                    first_profile.append(float(line_vals[0]))
                    indirect_profile.append(float(line_vals[1]))
                else:
                    flushed_print('Line %d in file %s is not valid.'
                                  % (idx + 1, filename_path))
                    raise ProfileFileWrongNumberOfcolumns
            except ValueError:
                flushed_print('Line %d in file %s is not valid: allowed float values in formats "X" or "X\tY".'
                              % (idx + 1, filename_path))
                raise ProfileFileValueError
            if (first_profile[-1] < 0) or (two_profiles and indirect_profile[-1] < 0):
                flushed_print('Line %d in file %s indicates a negative value'
                              % (idx + 1, filename_path))
                raise ProfileFileNegativeValue

    if len(first_profile) != 8760:
        flushed_print('file %s does not have 8760 lines'
                      % filename_path)
        raise ProfileFileWrongNumberOfLines

    return any(first_profile) or any(indirect_profile)

##########################################
# Checks related to candidates.ini
##########################################


class UnrecognizedCandidateOptionType(Exception):
    pass


def check_candidate_option_type(option, value):
    """
        verifies if a given option value has the correct type corresponding allowed for this option

        :param option: the treated option
        :param value: the value assigned to the option

        :return: True if the value has an appropriate type, False or exist otherwise
    """
    options_types = {'name': 'string',
                     'enable': 'string',
                     'candidate-type': 'string',
                     'investment-type': 'string',
                     'link': 'string',
                     'annual-cost-per-mw': 'non-negative',
                     'unit-size': 'non-negative',
                     'max-units': 'non-negative',
                     'max-investment': 'non-negative',
                     'relaxed': 'string',
                     'link-profile': 'string',
                     'already-installed-capacity': 'non-negative',
                     'already-installed-link-profile': 'string',
                     'has-link-profile': 'string'}
    obsolete_options = ["c"]
    option_type = options_types.get(option)
    if option_type is None:
        flushed_print(
            'check_candidate_option_type: %s option not recognized in candidates file.' % option)
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


class IllegalCandidateOptionValue(Exception):
    pass


def check_candidate_option_value(option, value):
    """
        verifies if a given option value belongs to the list of allowed values for that option

        :param option: the treated option
        :param value: the value to check

        :return: True if the value is legal or is not to be checked. Exists otherwise.
    """
    options_legal_values = {'name': None,
                            'enable': ["true", "false"],
                            'candidate-type': None,
                            'investment-type': None,
                            'link': None,
                            'annual-cost-per-mw': None,
                            'unit-size': None,
                            'max-units': None,
                            'max-investment': None,
                            'relaxed': ["true", "false"],
                            'link-profile': None,
                            'already-installed-capacity': None,
                            'already-installed-link-profile': None}
    legal_values = options_legal_values.get(option)
    if (legal_values is None) or (isinstance(value, str) and value.lower() in legal_values):
        return True

    flushed_print('check_candidate_option_value: Illegal value %s for option %s allowed values are: %s'
                  % (value, option, legal_values))
    raise IllegalCandidateOptionValue


class EmptyCandidateName(Exception):
    pass


class IllegalCharsInCandidateName(Exception):
    pass


def check_candidate_name(name, section):
    """
        checks that the candidate's name is not empty and does not contain a space
    """
    if (not name) or (name.lower() == "na"):
        flushed_print(
            'Error candidates name cannot be empty : found in section %s' % section)
        raise EmptyCandidateName
    illegal_chars = " \n\r\t\f\v-+=:[]()"
    for c in illegal_chars:
        if c in name:
            flushed_print('Error candidates name should not contain %s, found in section %s in "%s"' % (
                c, section, name))
            raise IllegalCharsInCandidateName


class EmptyCandidateLink(Exception):
    pass


class CandidateLinkWithoutSeparator(Exception):
    pass


def check_candidate_link(link, section):
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


def check_candidates_file(candidates_ini_filepath, capacity_dir_path):
    """
        checks that a candidate file related to an XpansionDriver has the correct format

        :param driver: the XpansionDriver pointing to the candidates file

        :return: Exits if the candidates files has the wrong format.
    """
    default_values = {'name': 'NA',
                      'enable': 'true',
                      'candidate-type': 'investment',
                      'investment-type': 'generation',
                      'link': 'NA',
                      'annual-cost-per-mw': '0',
                      'unit-size': '0',
                      'max-units': '0',
                      'max-investment': '0',
                      'Relaxed': 'false',
                      'link-profile': '1',
                      'already-installed-capacity': '0',
                      'already-installed-link-profile': '1'}
    ini_file = configparser.ConfigParser(default_values)
    ini_file.read(candidates_ini_filepath)

    config_changed = False

    # check attributes types and values
    for each_section in ini_file.sections():
        for (option, value) in ini_file.items(each_section):
            if not check_candidate_option_type(option, value):
                flushed_print(
                    "value %s for option %s has the wrong type!" % (value, option))
                raise CandidateFileWrongTypeValue
            check_candidate_option_value(option, value)

    # check that name is not empty and does not have space
    # check that link is not empty
    for each_section in ini_file.sections():
        check_candidate_name(
            ini_file[each_section]['name'].strip(), each_section)
        check_candidate_link(
            ini_file[each_section]['link'].strip(), each_section)

    # check some attributes unicity : name and links
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
                # FIXME can also add reverse link

    # check exclusion between max-investment and (max-units, unit-size) attributes
    for each_section in ini_file.sections():
        max_invest = float(ini_file[each_section]['max-investment'].strip())
        unit_size = float(ini_file[each_section]['unit-size'].strip())
        max_units = float(ini_file[each_section]['max-units'].strip())
        if max_invest != 0:
            if max_units != 0 or unit_size != 0:
                flushed_print("Illegal values in section %s: cannot assign non-null values simultaneously \
                      to max-investment and (unit-size or max_units)" % (each_section))
                raise MaxUnitsAndMaxInvestmentNonNullSimultaneously
        elif max_units == 0 or unit_size == 0:
            flushed_print("Illegal values in section %s: need to assign non-null values to max-investment \
                  or (unit-size and max_units)" % (each_section))
            raise MaxUnitsAndMaxInvestmentAreNullSimultaneously

    # check attributes profile is 0, 1 or an existent filename
    profile_attributes = ['link-profile', 'already-installed-link-profile']
    for each_section in ini_file.sections():
        has_a_profile = False
        for attribute in profile_attributes:
            value = ini_file[each_section][attribute].strip()
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
                has_a_profile = has_a_profile or check_profile_file(
                    filename_path)
        if not has_a_profile:
            # remove candidate if it has no profile
            flushed_print("candidate %s will be removed!" %
                          ini_file[each_section]["name"])
            ini_file.remove_section(each_section)
            config_changed = True

    if config_changed:
        shutil.copyfile(candidates_ini_filepath,
                        candidates_ini_filepath + ".bak")
        with open(candidates_ini_filepath, 'w') as out_file:
            ini_file.write(out_file)
        flushed_print("%s file was overwritten! backup file %s created"
                      % (candidates_ini_filepath, candidates_ini_filepath + ".bak"))


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
    "method": (type_str, ["benders_decomposition"]),
    "uc_type": (type_str, ["expansion_accurate", "expansion_fast"]),
    "master": (type_str, ["relaxed", "integer", "full_integer"]),
    "optimality_gap": (type_float, None),
    "relative_gap": (type_float, None),
    "cut_type": (type_str, ["average", "yearly", "weekly"]),
    "week_selection": (type_str,  ["true", "false"]),
    "max_iteration": (type_int, None),
    "relaxed_optimality_gap": (type_str, None),
    "solver": (type_str, None),
    "timelimit": (type_int, None),
    "yearly-weights": (type_str, None),
    "additional-constraints": (type_str, None),
    "log_level": (type_int, None)
}


def check_setting_option_type(option, value):
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
        if value in ["+Inf", "-Inf", "+infini", "-infini"]:
            return True
        else:
            try:
                int(value)
                return True
            except ValueError:
                flushed_print(
                    'check_setting_option_type: Illegal %s option in type, integer is expected .' % option)
                return False

    else:
        return isinstance(value, type_str)


class OptionTypeError(Exception):
    pass


class GapValueError(ValueError):
    pass


def check_setting_option_value(option, value):
    """
        checks that an option has a legal value

        :param option: name of the option to verify from settings file
        :param value: value of the option to verify

        :return: True if the option has the correct type, exits if the value has the wrong type
    """

    if not check_setting_option_type(option, value):
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
            print(
                "Illegal value %s for option %s : only positive values are allowed"
                % (value, option)
            )
            raise GapValueError

    elif option == 'max_iteration':
        if value in ["+Inf", "+infini"]:
            return True
        else:
            try:
                max_iter = int(value)
                if (max_iter == -1) or (max_iter > 0):
                    return True
            except ValueError:
                flushed_print('Illegal value %s for option %s : only -1 or positive values are allowed'
                              % (value, option))
                sys.exit(1)
    elif option == "relaxed_optimality_gap":
        if value.strip().endswith("%"):
            try:
                gap = float(value[:-1])
                if 0 <= gap <= 100:
                    return True
            except ValueError:
                flushed_print('Illegal value %s for option %s: legal format "X%%" with X between 0 and 100'
                              % (value, option))
                sys.exit(1)
    elif option == 'timelimit':
        if (value in ["+Inf", "+infini"]):
            return True
        else:
            try:
                timelimit = int(value)
                if timelimit > 0:
                    return True
            except ValueError:
                flushed_print('Illegal value %s for option %s : only positive values are allowed'
                              % (value, option))
                sys.exit(1)
    elif option == 'log_level':
        if (value >= 0):
            return True
        else:
            flushed_print('Illegal value %s for option %s : only greater than or equal to zero values are accepted'
                          % (value, option))
            sys.exit(1)

    flushed_print(
        'check_candidate_option_value: Illegal value %s for option %s' % (value, option))
    sys.exit(1)


def check_options(options):
    """
        checks that a settings file related to an XpansionDriver has the correct format
        Exits if the candidates files has the wrong format.

        :param options: the options obtained from the settings.ini file

        :return:
    """

    option_items = options.items()
    for (option, value) in option_items:
        check_setting_option_value(option, value)

    if options.get('yearly-weights', "") != "":
        if options.get("cut_type") == "average":
            flushed_print(
                "check_settings : yearly-weights option can not be used when cut_type is average")
            sys.exit(1)
