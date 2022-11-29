
import pytest
from test_bendersEndToEnd import run_solver


@pytest.mark.optim
@pytest.mark.mergemps
def test_001_mergemps(install_dir, tmp_path):
    run_solver(install_dir, 'MERGE_MPS', tmp_path)
