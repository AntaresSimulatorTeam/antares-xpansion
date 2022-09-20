from pathlib import Path
import pytest
from .test_restartBendersEndToEnd import run_solver
from .studies import study_parameters, study_values


@pytest.mark.parametrize(
    study_parameters,
    study_values,
)
@pytest.mark.optim
@pytest.mark.bendersmpi
def test_001_mpibenders(install_dir, tmp_path, study, allow_run_as_root):
    run_solver(install_dir, 'BENDERS_MPI', tmp_path, study, allow_run_as_root)
