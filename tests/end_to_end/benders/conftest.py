import pytest

from tests import build_config_reader


def pytest_addoption(parser):
    parser.addoption("--installDir", action="store",
                     default=build_config_reader.get_install_dir())
    parser.addoption("--mpi_custom_arguments", action="store", default="")


@pytest.fixture()
def install_dir(request):
    return request.config.getoption("--installDir")


@pytest.fixture()
def mpi_custom_arguments(request):
    return request.config.getoption("--mpi_custom_arguments")
