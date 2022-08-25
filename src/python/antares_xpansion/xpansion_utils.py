"""
    util functions
"""

import os
from zipfile import ZipFile


def read_and_write_mps(study_archive):
    """
        :return: a dictionary giving instance file, variables file and
        constraints file per (year, week)
    """
    result = {}
    with ZipFile(study_archive, 'r') as zip_object:
        sorted_root_dir = sorted(zip_object.namelist())

    for instance in sorted_root_dir:
        if '.mps' in instance and not '-1.mps' in instance:
            buffer_l = instance.strip().split("-")
            year = int(buffer_l[1])
            week = int(buffer_l[2])
            if (year, week) not in result:
                result[year, week] = [instance, '', '']

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

    return result
