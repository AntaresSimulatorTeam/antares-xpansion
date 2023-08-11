import pytest

from tests import build_config_reader


def pytest_addoption(parser):
    parser.addoption("--installDir", action="store",
                     default=build_config_reader.get_install_dir())
    parser.addoption("--xpress", action="store", default="")


@pytest.fixture()
def install_dir(request):
    return request.config.getoption("--installDir")


@pytest.fixture()
def xpress(request):
    return request.config.getoption("--xpress")
