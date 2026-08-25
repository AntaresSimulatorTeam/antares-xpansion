# Candidates

The candidates are defined in the `candidates.ini` file, which must be located 
inside the `user/expansion/` directory of the Antares study folder:
`antares-study-folder/user/expansion/candidates.ini`. 

!!! info 

    For more info about the signification of the parameters, visit the 
    user documentation page for 
    [Xpansion candidates](https://antares-doc.readthedocs.io/en/latest/reference/xpansion-candidates).

Each investment candidate is characterized
by the following properties:

- `name` (mandatory): name of the investment candidate (:warning: must not
  contain spaces and unique)
- `link` (mandatory): link on which there is a capacity investment
- `annual-cost-per-mw` (mandatory): investment cost, per year and per MW
- `unit-size`: size, in MW, of a single investment unit (e.g. one group
  of 300 MW)
- `max-units`: maximum number of units that can be built
- `max-investment`: maximum capacity in MW that can be invested in the candidate
- `already-installed-capacity`: capacity in MW that is
already installed on the investment candidate's link
- `direct-link-profile`: name of a file that links 
  the invested capacity and the available capacity for the direct way
- `indirect-link-profile`: name of a file that links 
  the invested capacity and the available capacity for the indirect way
- `already-installed-direct-link-profile`: 
  name of a file that links the already installed capacity 
  and the available capacity for the direct way
- `already-installed-indirect-link-profile`:
  name of a file that links the already installed capacity 
  and the available capacity for the indirect way

The format is a standard `.ini` file and should follow this template:

```ini
[1]
name =  # string
link = area_from - area_to # string
annual-cost-per-mw =    # float
max-investment =        # float
already-installed-capacity =    # float
direct-link-profile = capa_pv.csv
indirect-direct-link-profile = capa_pv.csv
already-installed-direct-link-profile = direct_installed_capa_pv.csv
already-installed-indirect-link-profile = indirect_installed_capa_pv.csv
```

