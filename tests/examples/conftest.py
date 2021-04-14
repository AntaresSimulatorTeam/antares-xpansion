import pytest

def pytest_addoption(parser):
    parser.addoption("--installDir", action="store")
    
@pytest.fixture()
def installDir(request):
    return request.config.getoption("--installDir")