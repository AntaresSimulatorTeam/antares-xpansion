# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['../src/python/launch.py'],
    pathex=[],
    binaries=[('./bin/*', 'bin')],
    datas=[('../src/python/config.yaml', '.')],
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=['_bootlocale'],
    noarchive=False,
    optimize=0,
)
excludes = ['libmpi.so.40', 'libopen-rte.so.40', 'libopen-pal.so.40']
a.binaries = TOC([x for x in a.binaries if x[0] not in excludes])

pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='antares-xpansion-launcher',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)