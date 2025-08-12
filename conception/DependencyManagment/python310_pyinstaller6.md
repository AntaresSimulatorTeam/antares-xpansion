# Dependency Upgrade Log

## 1. Dependency Name

* Python
* Pyinstaller

## 2. Previous Version

* python@3.8
* pyinstaller@4.6

## 3. New Version

* python@3.10  <!-- Minimum working version -->
* pyinstaller@6.15  <!-- Latest and working version -->

## 4. Date of Upgrade

August 2025

## 5. Reason for Upgrade

Keeping dependencies relatively up-to-date is important for performance, security, and compatibility.  
In this case, failures occurred on CentOS and Oracle Linux 8 CI environments with the following error:

```ImportError: cannot import name 'asynccontextmanager'```

There were limited search results on this error, but it appeared to stem from using an outdated Python version.

## 6. Upgrade Process

- Work done in [PR #1075](https://github.com/AntaresSimulatorTeam/antares-xpansion/pull/1075)
- Used Miniconda to provide a Python 3.10 environment to avoid system-level incompatibility.

## 7. Challenges Encountered

- Using Conda + PyInstaller did not work out-of-the-box with PyInstaller 4 and Python 3.10.
- Upgrading to PyInstaller 5 worked but resulted in assets >1 GB.
- Upgrading to PyInstaller 6 restored expected asset size but failed due to OpenMPI-related error.
- Conflicts with `setuptools` (or lack thereof) also occurred during the process.

## 8. Resolution

- Used Conda to provide a Python 3.10 environment.
- Pinned PyInstaller to version **6.15**.
- Removed `libmpi.so.15` from the single-file asset.
- Created a custom PyInstaller `.spec` file to manage exclusions.

## 9. Verification Steps

- CentOS assets tested successfully in the OPF environment.
- CI job **test-build** passed successfully.

## 10. Related Links

- [GitHub PR #1075](https://github.com/AntaresSimulatorTeam/antares-xpansion/pull/1075)
- [Jira Ticket ANT-3693](https://gopro-tickets.rte-france.com/browse/ANT-3693)
