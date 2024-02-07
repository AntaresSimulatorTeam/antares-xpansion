import pytest

from tests import build_config_reader


def pytest_addoption(parser):
    parser.addoption("--installDir", action="store",
                     default=build_config_reader.get_install_dir())
    parser.addoption("--allow_run_as_root", action="store", default="")
    parser.addoption("--xpress", action="store_false", default=False)


@pytest.fixture()
def install_dir(request):
    return request.config.getoption("--installDir")


@pytest.fixture()
def allow_run_as_root(request):
    return request.config.getoption("--allow_run_as_root")


@pytest.fixture()
def xpress(request):
    return request.config.getoption("--xpress")
