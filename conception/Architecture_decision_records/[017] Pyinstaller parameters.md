# Add --exclude-module _bootlocale to pyinstaller parameters

## Accepted [24 Feb. 2025]

## Context

There is a bug with pyinstaller and python 3.10
Ubuntu 22.04 is using python 3.10 and pyinstaller is not working with python 3.10
https://stackoverflow.com/questions/68459087/pyinstaller-with-python-3-10-0b4-importerror-no-module-named-bootlocale
https://github.com/pyinstaller/pyinstaller/issues/5693

## Decision

Add --exclude-module _bootlocale to pyinstaller parameters to fix the issue

## Consequences

No issue
