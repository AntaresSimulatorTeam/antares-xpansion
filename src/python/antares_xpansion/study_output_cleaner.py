import os
from pathlib import Path


def remove_files_containing_str_from_dir(contains_str: str, dirpath: Path):
    for instance in os.listdir(dirpath):
        if contains_str in instance:
            os.remove(dirpath / instance)


def rename_master(old_master_path: Path, new_master_path: Path):
    if old_master_path.is_file():
        old_master_path.rename(new_master_path)
    else:
        # Just throw a warning instead of exception ?
        pass


def get_last_master_path(dirpath: Path) -> Path:
    return Path(dirpath / "lp"/'master_last_iteration.mps')


def get_tmp_master_path(master_path: Path) -> Path:
    return Path(master_path.parent, master_path.stem + '.tmp')


class StudyOutputCleaner:

    @staticmethod
    def clean_antares_step(study_output: Path):
        remove_files_containing_str_from_dir('criterion', study_output)

    @staticmethod
    def clean_lpnamer_step(study_output: Path):
        remove_files_containing_str_from_dir('.mps', study_output)
        remove_files_containing_str_from_dir('constraints', study_output)
        remove_files_containing_str_from_dir('variables', study_output)

    @staticmethod
    def clean_benders_step(study_output: Path):

        master_path = get_last_master_path(study_output)
        tmp_master_path = get_tmp_master_path(master_path)

        lp_dir = study_output / 'lp'
        rename_master(master_path, tmp_master_path)
        remove_files_containing_str_from_dir('.mps', lp_dir)
        rename_master(tmp_master_path, master_path)

        remove_files_containing_str_from_dir('.lp', lp_dir)
        remove_files_containing_str_from_dir(".zip", lp_dir)

    @staticmethod
    def clean_study_update_step(study_output: Path):
        remove_files_containing_str_from_dir('area', study_output)
        remove_files_containing_str_from_dir('interco', study_output)
        remove_files_containing_str_from_dir('mps.txt', study_output)
