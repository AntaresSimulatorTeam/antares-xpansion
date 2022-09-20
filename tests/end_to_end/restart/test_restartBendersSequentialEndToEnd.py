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
    options_file = study['option_file']
    tmp_study = tmp_path / \
        (Path(instance_path).name+"-"+Path(options_file).stem)
    shutil.copytree(instance_path, tmp_study)

    expansion_dir = tmp_study / "expansion"
    if expansion_dir.is_dir():
        shutil.rmtree(expansion_dir)

    expansion_dir.mkdir()
    shutil.copyfile(tmp_study / study["output_file"],
                    expansion_dir / "out.json")
    shutil.copyfile(tmp_study / study["last_iteration_file"],
                    expansion_dir / "last_iteration.json")
    run_solver(install_dir, 'BENDERS_SEQUENTIAL', tmp_study, study)
