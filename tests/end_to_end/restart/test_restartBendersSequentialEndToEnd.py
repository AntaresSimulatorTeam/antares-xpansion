import pytest
from .test_restartBendersEndToEnd import run_solver

## TESTS ##


from .studies import study_parameters, study_values


@pytest.mark.parametrize(
    study_parameters,
    study_values,
)
@pytest.mark.optim
@pytest.mark.benderssequential
def test_001_sequential(install_dir, tmp_path, study):
    run_solver(install_dir, 'BENDERS_SEQUENTIAL', tmp_path, study)
