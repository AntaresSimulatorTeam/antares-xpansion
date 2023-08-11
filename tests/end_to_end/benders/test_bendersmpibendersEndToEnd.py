import pytest
from test_bendersEndToEnd import run_solver


@pytest.mark.optim
@pytest.mark.bendersmpi
def test_001_mpibenders(install_dir, tmp_path, xpress):
    run_solver(install_dir, 'BENDERS', tmp_path,
                True, xpress)
