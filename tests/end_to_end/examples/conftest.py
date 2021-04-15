import pytest

from tests import build_config_reader

def pytest_addoption(parser):
    parser.addoption("--installDir", action="store", default = build_config_reader.get_install_dir())
    
@pytest.fixture()
def installDir(request):
    return request.config.getoption("--installDir")