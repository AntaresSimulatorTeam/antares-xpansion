import sys
from pathlib import Path

import pytest

# Ensure src/python and package dir are on sys.path regardless of cwd
THIS_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str((THIS_DIR / "../../src/python").resolve()))
sys.path.insert(0, str((THIS_DIR / "../../src/python/antares_xpansion").resolve()))

from antares_xpansion.trajectory.trajectory_config import (
    TrajectoryConfig,
    TrajectoryInputParameters,
)
from antares_xpansion.xpansionConfig import ConfigParameters


def make_config_params(tmp_path: Path) -> ConfigParameters:
    # Minimal config parameters with placeholders
    return ConfigParameters(
        default_install_dir=str(tmp_path),
        ANTARES="antares",
        MERGE_MPS="merge_mps",
        BENDERS="benders",
        LP_NAMER="lp_namer",
        PRESOLVE="presolve",
        STUDY_UPDATER="study_updater",
        SENSITIVITY_EXE="sensitivity",
        FULL_RUN="full_run",
        OUTER_LOOP="outer_loop",
        ANTARES_ARCHIVE_UPDATER="archive_updater",
        MPIEXEC="mpiexec",
        AVAILABLE_SOLVERS=["Cbc"],
        MULTIPLE_PROBLEM_GEN="multiple_problem_gen",
        MERGE_MASTER_MPS="merge_master_mps",
        MERGE_WEIGHTS_TRAJECTORY="merge_weights",
    )


def make_input_params(install_dir: str) -> TrajectoryInputParameters:
    # Minimal input parameters; most fields are unused by get_executable_path
    return TrajectoryInputParameters(
        step="dummy",
        input_root=Path("."),
        input_file=Path("input.txt"),
        memory=False,
        install_dir=install_dir,
        problems_format="OPTIMIZED",
        solver="Cbc",
        cache_problems=False,
        method="",
        n_mpi=1,
        oversubscribe=False,
        allow_run_as_root=False,
    )


def test_get_executable_path_finds_exact_file(tmp_path: Path):
    install_dir = tmp_path / "bin"
    install_dir.mkdir()
    exe = install_dir / "myexec"
    exe.write_text("#!/bin/sh\necho hi")
    exe.chmod(0o755)

    cfg = make_config_params(tmp_path)
    inp = make_input_params(str(install_dir))

    tcfg = TrajectoryConfig(inp, cfg)
    path = tcfg.get_executable_path("myexec")
    assert Path(path) == exe.resolve()


def test_get_executable_path_falls_back_to_exe_suffix(tmp_path: Path):
    install_dir = tmp_path / "bin"
    install_dir.mkdir()
    exe = install_dir / "otherexec.exe"
    exe.write_text("#!/bin/sh\necho hi")
    exe.chmod(0o755)

    cfg = make_config_params(tmp_path)
    inp = make_input_params(str(install_dir))

    tcfg = TrajectoryConfig(inp, cfg)
    path = tcfg.get_executable_path("otherexec")
    assert Path(path) == exe.resolve()


def test_get_executable_path_raises_when_missing(tmp_path: Path):
    install_dir = tmp_path / "bin"
    install_dir.mkdir()

    cfg = make_config_params(tmp_path)
    inp = make_input_params(str(install_dir))

    tcfg = TrajectoryConfig(inp, cfg)
    with pytest.raises(FileNotFoundError):
        tcfg.get_executable_path("nonexistent_exec")


def test_get_merge_weights_executable_is_found(tmp_path: Path):
    """Ensure the executable named by MERGE_WEIGHTS_TRAJECTORY in the config is found."""
    install_dir = tmp_path / "bin"
    install_dir.mkdir()

    cfg = make_config_params(tmp_path)
    # The name configured for the merge weights trajectory executable
    exe_name = cfg.MERGE_WEIGHTS_TRAJECTORY
    exe = install_dir / exe_name
    exe.write_text("#!/bin/sh\necho merge")
    exe.chmod(0o755)

    inp = make_input_params(str(install_dir))
    tcfg = TrajectoryConfig(inp, cfg)

    # The TrajectoryConfig stores the config value in tcfg.MERGE_WEIGHTS
    path = tcfg.get_executable_path(tcfg.MERGE_WEIGHTS)
    assert Path(path) == exe.resolve()
