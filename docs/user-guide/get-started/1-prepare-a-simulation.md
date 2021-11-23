# Prepare a simulation

## The `user/expansion/` folder of the Antares study

The Antares-Xpansion package is based on the Antares software and
its data format.

Antares-Xpansion is based on an already existing Antares study. Some
of the capacities of this study, usually fixed as input in the Antares
paradigm, will be optimized by the investment optimization module of the
Antares-Xpansion package.

In order to run the investment optimization module, the Antares dataset
must be enriched with - at least - two new files:

- a `candidates.ini` file which contains the definition of
  investment candidates (which capacities of the Antares study are
  expandable? at what cost? with what limits? etc.);

- a `settings.ini` file which contains the settings of the
  Antares-Xpansion algorithm.

These two files must be located in the **user/expansion/** directory of
the Antares study (see ***Figure 3***). To date, the data they contain
are neither visible nor modifiable in the Antares man-machine interface.
These two files must therefore be built "by hand".

```
xpansion-study
└─── input
└─── layers
└─── logs
└─── output
└─── settings
└─── user
│   └───expansion
│       │   candidates.ini
│       │   settings.ini
│       │   ...
```

