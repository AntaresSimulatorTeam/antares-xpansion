import tempfile
from pathlib import Path


def before_scenario(context, scenario):
    context.temp_dir = Path(tempfile.TemporaryDirectory().name)
