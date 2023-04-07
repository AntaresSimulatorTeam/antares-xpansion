# Documention generation guidelines

This file explains how to generate the onlinedocumentation and the PDF userguide of the project.

## Online documentation

The online documentation is generated using `readthedocs`. The main configuration files are located in the root directory of the project :
- `readthedocs.yml` specifies the Python environment and the mkdocs configuration file.
- `mkdocs.yml` speficies the configuration of mkdocs.

The documentation source files are located in the `docs` directory. Building the online documentation only requires the files specified in the `nav` section of `mkdocs.yml` (along with the depending Python code for the Jupyter notebook). The other files, like reStructuredText files (.rst) are only used for the PDF generation, see [PDF generation](#pdf-generation).

To preview the documentation locally, it is possible to run `mkdocs serve` from the folder containing the `mkdocs.yml` file (in a Python environment that contains the package `mkdocs`).

`readthedocs` follows the github branches configured by the admin and displays the documentation accordingly. These choices can be managed [there](https://readthedocs.org/projects/antares-xpansion/). You must be logged in to readthedocs and granted the appropriate modification rights on the project. 

## PDF generation

The PDF userguide is built using the latex builder of [Sphinx](https://www.sphinx-doc.org/en/master/).

In `docs/pdf-doc-generation-with-sphinx`, you can find :
- A folder `source` that contains the sphinx configuration file `conf.py`, and a file `index.rst` that describes the main sections of the document. Note that the tree structure of the docs is given for the online docs in `mkdocs.yml` whereas it is in `docs/pdf-doc-generation-with-sphinx/index.rst` for the PDF. 
These two structures should agree, otherwise there might be build errors or inconsistencies between the two docs.
- A bash script `create_pdf_doc.sh` that copies the docs markdown source files (only the user guide) in `docs/pdf-doc-generation-with-sphinx/source`. Then, it replaces some characters so that the source files can be correctly interpreted by the latex compiler. Then it builds the PDF userguide and deletes the source files that were copied earlier.

Having a table of contents that is consistent between the online and the PDF documentation is tricky and involves playing with the `.rst` files that are located in the different subfolders of the documentation source files.