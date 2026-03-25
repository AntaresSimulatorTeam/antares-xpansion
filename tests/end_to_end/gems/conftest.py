import pytest
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent / "src" / "python"))

from tests import build_config_reader


def pytest_addoption(parser):
    parser.addoption("--installDir", action="store",
                     default=build_config_reader.get_install_dir())
    parser.addoption("--allow_run_as_root", action="store_true", default=False)


@pytest.fixture()
def install_dir(request):
    return request.config.getoption("--installDir")


@pytest.fixture()
def allow_run_as_root(request):
    return request.config.getoption("--allow_run_as_root")