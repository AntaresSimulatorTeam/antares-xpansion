import pytest
from test_bendersEndToEnd import run_solver

## TESTS ##


@pytest.mark.optim
@pytest.mark.benderssequential
def test_001_sequential(install_dir, allow_run_as_root, tmp_path):
    run_solver(install_dir, 'BENDERS', tmp_path, allow_run_as_root)
