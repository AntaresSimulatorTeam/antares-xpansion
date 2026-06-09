# Documention guidelines

This file explains how to generate the online documentation.

## Documentation settings

The online documentation is generated using `readthedocs`. 
The main configuration files are located in the root directory of the project :

- `readthedocs.yml` specifies the Python environment and the mkdocs configuration file.
- `mkdocs.yml` speficies the configuration of mkdocs.

The documentation source files are located in the `docs` directory.
Building the online documentation only requires the files specified in the `nav` 
section of `mkdocs.yml` (along with the depending Python code for the Jupyter notebook).

To preview the documentation locally, it is possible to run `mkdocs serve` 
from the folder containing the `mkdocs.yml` file 
(in a Python environment that contains the package `mkdocs`).

`readthedocs` follows the github branches configured by the admin 
and displays the documentation accordingly. These choices can be managed 
[there](https://readthedocs.org/projects/antares-xpansion/).
You must be logged in to readthedocs and granted the appropriate modification rights on the project. 

