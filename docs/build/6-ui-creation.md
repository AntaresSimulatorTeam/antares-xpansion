# User Interface creation
A PyQt5 User Interface (UI) is available : `src\python\antares-xpansion-ui.py`

To generate needed resources `BUILD_UI` option must be enabled.

## Install requirements
```
pip3 install -r requirements-ui.txt
```

## Force new generation for resource file
A `resource.py` python module is created when configuring CMake.

To force the regeneration of this file if images are changed, use `pyrcc5`
```
cd src\python
pyrcc5 images.qrc > resource.py
```

