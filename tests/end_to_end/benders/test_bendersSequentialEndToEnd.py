import pytest
from test_bendersEndToEnd import run_solver

## TESTS ##


@pytest.mark.optim
@pytest.mark.benderssequential
def test_001_sequential(install_dir tmp_path, xpress):
    run_solver(install_dir, 'BENDERS', tmp_path,
               False, xpress)
