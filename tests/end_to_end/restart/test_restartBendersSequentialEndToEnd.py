from pathlib import Path
import shutil
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

    instance_path = study['path']
    if(tmp_path.exists()):
        shutil.rmtree(tmp_path)
    shutil.copytree(instance_path, tmp_path)

    expansion_dir = tmp_path / "expansion"
    if expansion_dir.is_dir():
        shutil.rmtree(expansion_dir)

    expansion_dir.mkdir()
    shutil.copyfile(tmp_path / study["output_file"],
                    expansion_dir / "out.json")
    shutil.copyfile(tmp_path / study["last_iteration_file"],
                    expansion_dir / "last_iteration.json")
    run_solver(install_dir, 'BENDERS_SEQUENTIAL', tmp_path, study)
