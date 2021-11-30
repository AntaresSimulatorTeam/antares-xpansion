# User interface creation
A PyQt5 user interface (UI) is available: `src\python\antares-xpansion-ui.py` To build the UI, the option `BUILD_UI` must be enabled at [configure time](3-Build.md#configure).

## Install requirements
```
pip3 install -r requirements-ui.txt
```

## Force new generation for resource file
A `resource.py` python module is created when configuring CMake. To force the regeneration of this file if images are changed, use `pyrcc5`.
```
cd src\python
pyrcc5 images.qrc > resources.py
```

