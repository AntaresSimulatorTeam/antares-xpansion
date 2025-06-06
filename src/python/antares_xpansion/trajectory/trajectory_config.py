from dataclasses import dataclass
from pathlib import Path


@dataclass
class TrajectoryInputParameters:
    step: str
    input_root: Path
    input_file: Path
