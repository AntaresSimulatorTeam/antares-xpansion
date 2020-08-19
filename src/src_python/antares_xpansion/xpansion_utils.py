"""
    util functions
"""

import os

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
