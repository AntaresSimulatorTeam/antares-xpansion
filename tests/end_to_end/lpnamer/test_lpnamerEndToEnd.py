import filecmp
import os
import shutil
import subprocess
import zipfile
from enum import Enum
from pathlib import Path

import pytest

MPS_ZIP = "economy.zip"
DATA_TEST = Path("../../../data_test/")
DATA_TEST_INTEGER = DATA_TEST / "tests_lpnamer" / "tests_integer"
DATA_TEST_RELAXED = DATA_TEST / "tests_lpnamer" / "tests_relaxed"
TEST_LP_INTEGER_01 = DATA_TEST_INTEGER / \
    "test_lpnamer_01" / "output" / "economy"
TEST_LP_INTEGER_02 = DATA_TEST_INTEGER / \
    "test_one_link_one_candidate_1week" / "output" / "economy/"
TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB = DATA_TEST_INTEGER / "test_one_link_two_candidates_simple_prob" \
    / "output" / "economy"
TEST_LP_INTEGER_MULTIPLE_CANDIDATES = DATA_TEST_INTEGER / \
    "test_one_link_two_candidates_1week" / "output" / "economy"

TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB_HURDLES = DATA_TEST_INTEGER / "test_one_link_two_candidates_simple_prob_hurdle_cost" \
    / "output" / "economy"
TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB_NULL_PROFILE = DATA_TEST_INTEGER / "test_one_link_two_candidates_simple_prob_null_profile" \
    / "output" / "economy"

TEST_LP_RELAXED_01 = DATA_TEST_RELAXED / \
    "test_one_link_one_candidate-relaxed" / "output" / "economy/"
TEST_LP_RELAXED_02 = DATA_TEST_RELAXED / "SmallTestSixCandidatesWithAlreadyInstalledCapacity-relaxed" / "output" \
    / "economy"
test_data = [
    (TEST_LP_INTEGER_01, "integer"),
    (TEST_LP_INTEGER_02, "integer"),
    (TEST_LP_RELAXED_01, "relaxed"),
    (TEST_LP_RELAXED_02, "relaxed")
]

class OptionType(Enum):
    ARCHIVE = 1
    OUTPUT = 2
    STUDY = 3


options_mode = [OptionType.ARCHIVE, OptionType.OUTPUT, OptionType.STUDY]

test_data_multiple_candidates = [
    TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB,
    TEST_LP_INTEGER_MULTIPLE_CANDIDATES,
    TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB_HURDLES,
    TEST_LP_INTEGER_MULTIPLE_CANDIDATES_SIMPLE_PROB_NULL_PROFILE,
]

test_data_study_option = [
    DATA_TEST / "tests_lpnamer" / "SmallTestFiveCandidates"
]

@pytest.fixture
def setup_lp_directory(request, tmp_path):
    tmp_path = request.getfixturevalue('tmp_path')
    source_dir = request.getfixturevalue('test_dir')
    study_path = source_dir.parent.parent
    shutil.copytree(study_path, tmp_path / study_path.stem)
    index = source_dir.parts.index(study_path.stem)
    test_dir = tmp_path.joinpath(*source_dir.parts[index:])
    output_dir = test_dir.parent / test_dir.stem / "lp"
    if Path(output_dir).is_dir():
        shutil.rmtree(output_dir)
    Path(output_dir).mkdir(parents=True, exist_ok=True)

    archive_output_dir = test_dir.parent / (test_dir.stem + "-Xpansion") / "lp"
    if Path(archive_output_dir).is_dir():
        shutil.rmtree(archive_output_dir.parent)
    Path(archive_output_dir).mkdir(parents=True, exist_ok=True)

    list_files = list(Path(test_dir).glob("*.mps"))
    list_files.extend(list(Path(test_dir).glob("variables*.txt")))
    list_files.extend(list(Path(test_dir).glob("area*.txt")))
    list_files.extend(list(Path(test_dir).glob("interco*.txt")))
    if Path(test_dir.parent / MPS_ZIP).exists():
        os.remove(test_dir.parent / MPS_ZIP)
    with zipfile.ZipFile(test_dir.parent / MPS_ZIP, "w") as write_mps_zip:
        for file in list_files:
            write_mps_zip.write(
                file, file.name, compress_type=zipfile.ZIP_DEFLATED)
    yield test_dir


@pytest.fixture
def setup_study(request, tmp_path):
    tmp_path = request.getfixturevalue('tmp_path')
    source_dir = request.getfixturevalue('study_dir')
    shutil.copytree(source_dir, tmp_path / source_dir.stem)
    index = source_dir.parts.index(source_dir.stem)
    test_dir = tmp_path.joinpath(*source_dir.parts[index:])
    yield test_dir

@pytest.mark.parametrize("test_dir, master_mode", test_data)
@pytest.mark.parametrize("option_mode", [OptionType.ARCHIVE, OptionType.OUTPUT])
def test_lp_directory_files(install_dir, test_dir, master_mode, option_mode, setup_lp_directory, tmp_path):
    # given
    if option_mode == OptionType.ARCHIVE:
        launch_and_compare_lp_with_reference_archive(install_dir, master_mode, setup_lp_directory)
    elif option_mode == OptionType.OUTPUT:
        launch_and_compare_lp_with_reference_output(install_dir, master_mode, setup_lp_directory)


@pytest.mark.parametrize("test_dir", test_data_multiple_candidates)
@pytest.mark.parametrize("master_mode", ["integer"])
@pytest.mark.parametrize("option_mode", [OptionType.ARCHIVE, OptionType.OUTPUT])
def test_lp_multiple_candidates(install_dir, test_dir, master_mode, option_mode, setup_lp_directory):
    if option_mode == OptionType.ARCHIVE:
        launch_and_compare_lp_with_reference_archive(install_dir, master_mode, setup_lp_directory)
    elif option_mode == OptionType.OUTPUT:
        launch_and_compare_lp_with_reference_output(install_dir, master_mode, setup_lp_directory)


@pytest.mark.parametrize("study_dir", test_data_study_option)
@pytest.mark.parametrize("master_mode", ["integer"])
@pytest.mark.parametrize("option_mode", [OptionType.STUDY])
def test_lp_with_study_option(install_dir, study_dir, master_mode, option_mode, setup_study, tmp_path):
    launch_and_compare_lp_with_reference_study(install_dir, master_mode, setup_study)


def launch_and_compare_lp_with_reference_output(install_dir, master_mode, test_dir):
    old_path = os.getcwd()
    reference_lp_dir = test_dir / "reference_lp"
    lp_dir = test_dir / "lp"
    lp_namer_exe = Path(install_dir) / "lp_namer"
    study_dir = test_dir.resolve()
    os.chdir(test_dir.parent)
    launch_command = [str(lp_namer_exe), "-o", str(study_dir),
                      "-e", "contraintes.txt", "-f", master_mode, "--unnamed-problems"]
    # when
    returned_l = subprocess.run(launch_command, shell=False)
    # then
    os.chdir(old_path)
    then(lp_dir, old_path, reference_lp_dir, returned_l)



def launch_and_compare_lp_with_reference_archive(install_dir, master_mode, test_dir):
    old_path = os.getcwd()
    reference_lp_dir = test_dir / "reference_lp"
    lp_dir = test_dir.parent / (test_dir.stem + "-Xpansion") / "lp"
    lp_namer_exe = Path(install_dir) / "lp_namer"
    zip_path = (test_dir.parent / MPS_ZIP).resolve()
    os.chdir(test_dir.parent.parent)
    launch_command = [str(lp_namer_exe), "-a", str(zip_path),
                      "-e", "contraintes.txt", "-f", master_mode, "--unnamed-problems"]
    # when
    returned_l = subprocess.run(launch_command, shell=False)
    # then
    then(lp_dir, old_path, reference_lp_dir, returned_l)


def get_lp_dir(study_dir):
    directory = os.path.abspath(study_dir / "output")
    directories = [d for d in os.listdir(directory) if os.path.isdir(os.path.join(directory, d))]

    # Sort the directories based on creation date
    directories.sort(key=lambda x: os.path.getctime(os.path.join(directory, x)))
    return Path(directory) / directories[-1] / "lp"


def get_constraint_path(study):
    return study / "user/expansion/constraints/contraintes.txt"


def launch_and_compare_lp_with_reference_study(install_dir, master_mode, study_dir):
    old_path = os.getcwd()
    reference_lp_dir = study_dir / "output" / "simulation" / "reference_lp"
    lp_namer_exe = Path(install_dir) / "lp_namer"
    os.chdir(study_dir)
    constraint_path = get_constraint_path(study_dir)
    launch_command = [str(lp_namer_exe), "--study", study_dir,
                      "-e", constraint_path, "-f", master_mode, "--unnamed-problems"]
    # when
    returned_l = subprocess.run(launch_command, shell=False)
    lp_dir = get_lp_dir(study_dir)
    # then
    then(lp_dir, old_path, reference_lp_dir, returned_l)


def then(lp_dir, old_path, reference_lp_dir, returned_l):
    os.chdir(old_path)
    files_to_compare = os.listdir(reference_lp_dir)
    match, mismatch, errors = filecmp.cmpfiles(
        reference_lp_dir, lp_dir, files_to_compare)
    assert len(match) == len(files_to_compare)
    assert len(mismatch) == 0
    assert returned_l.returncode == 0
