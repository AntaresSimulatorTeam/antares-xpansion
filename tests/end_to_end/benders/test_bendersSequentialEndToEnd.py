import pytest
from test_bendersEndToEnd import run_solver

## TESTS ##


@pytest.mark.optim
@pytest.mark.benderssequential
def test_001_sequential(install_dir, tmp_path):
    run_solver(install_dir, 'BENDERS_SEQUENTIAL', tmp_path)
