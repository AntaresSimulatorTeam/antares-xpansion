"""
    util functions
"""

import os
import configparser

def read_and_write_mps(root_path):
    """
        :return: a dictionary giving instance file, variables file and
        constraints file per (year, week)
    """
    result = {}
    # print('#getcwd() : ', os.getcwd())
    # print('#root_path : ', root_path)
    sorted_root_dir = sorted(os.listdir(root_path))
    # print('#sorted_root_dir : ', sorted_root_dir)

    for instance in sorted_root_dir:
        if '.mps' in instance and not '-1.mps' in instance:
            buffer_l = instance.strip().split("-")
            year = int(buffer_l[1])
            week = int(buffer_l[2])
            if (year, week) not in result:
                result[year, week] = [instance, '', '']
    # print('#result : ', result)
    # for line in result.iteritems():
    #    print line

    for instance in sorted_root_dir:
        if 'variables' in instance:
            buffer_l = instance.strip().split("-")
            year = int(buffer_l[1])
            week = int(buffer_l[2])
            result[year, week][1] = instance

    for instance in sorted_root_dir:
        if 'constraints' in instance:
            buffer_l = instance.strip().split("-")
            year = int(buffer_l[1])
            week = int(buffer_l[2])
            result[year, week][2] = instance

    # for line in result.items():
    #     print(line[1][0] + ' ' + line[1][1] + ' ' + line[1][2] + ' ')
    return result

def read_and_write_exclusions(exclusion_file):
    """
        :return: a multiline string where each line describes an exclusion contraint :
        "constraint_name candidate1_name candidate2_name"
    """

    print("treating file %s" % exclusion_file)

    result = ""
    default_values = {'name' : 'NA',
                      'name-candidate1' : 'NA',
                      'name-candidate2' : 'NA'}
    ini_file = configparser.ConfigParser(default_values)
    ini_file.read(exclusion_file)

    for each_section in ini_file.sections():
        result += ini_file[each_section]["name"].strip().lower() + " "\
                        + ini_file[each_section]["name-candidate1"].strip().lower() + " "\
                        + ini_file[each_section]["name-candidate2"].strip().lower() + "\n"

    print("result :\n%s" % result)

    return result
