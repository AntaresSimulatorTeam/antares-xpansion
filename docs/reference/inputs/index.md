# Input files

In order to run the investment optimization module, the Antares dataset
must be enriched with at least two new files:

- A `candidates.ini` file which contains the definition of
  investment candidates (which capacities of the Antares study are
  expandable? at what cost? with what limits? and so on),

- A `settings.ini` file which contains the settings of the
  Antares Xpansion algorithm.


These two files must be located in the `user/expansion/` directory of
the Antares study. The data they contain are neither visible nor modifiable
in Antares Web, which is Antares user interface. These two files must therefore 
be handcrafted or manipulated with [Antares Craft](https://antares-craft.readthedocs.io/en/latest/).

```
antares-study
└─ input
└─ layers
└─ logs
└─ output
└─ settings
└─ user
   └─ expansion
      └─  candidates.ini
      └─  settings.ini
      └─  ...
```

