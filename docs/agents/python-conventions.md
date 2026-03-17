# Python Conventions

This document covers Python coding conventions for Antares-Xpansion.

## Language Version

- Python 3.x (check `requirements.txt`)
- Type hints are encouraged for new code

## Code Style

### Formatting

- Follow PEP 8
- Use 4 spaces for indentation
- Maximum line length: 100 characters

### Naming

- Modules: `snake_case` (e.g., `benders_driver.py`)
- Classes: `PascalCase` (e.g., `StudyReader`)
- Functions: `snake_case` (e.g., `load_candidates`)
- Constants: `UPPER_SNAKE_CASE`

### Imports

Standard order:
1. Standard library
2. Third-party libraries
3. Local application imports

```python
import logging
from pathlib import Path

import pandas as pd
import numpy as np

from antares_xpansion import xpansion_study_reader
```

## Project Structure

```
src/python/
├── antares_xpansion/
│   ├── __init__.py
│   ├── benders_driver.py
│   ├── config_loader.py
│   ├── logger.py
│   ├── trajectory/          # Trajectory optimization
│   └── ...
└── launch.py                # Entry point
```

## Logging

Use the project's logging utilities:

```python
from antares_xpansion.logger import get_logger

logger = get_logger(__name__)
logger.info("message")
```

## Error Handling

- Use exceptions for errors specific to the module
- Catch specific exceptions, not bare `except:`
- Include helpful error messages

```python
try:
    data = load_file(path)
except FileNotFoundError as e:
    raise XpansionError(f"Study not found: {path}") from e
```

## Testing

- Use pytest (see `requirements-tests.txt`)
- Test files: `tests/python/`
- Follow naming: `test_*.py`

## Type Hints

Recommended for new code:

```python
def process_candidates(candidates: list[str]) -> dict[str, float]:
    ...
```

## Configuration

- Use `dataclasses` for configuration objects
- Default values in `*_default_value.py` modules
- Keys defined in `*_keys.py` modules
