NAME          ClpDefau  FREE
ROWS
 N  OBJROW
 E  R0000000
 E  R0000001
 E  R0000002
 L  R0000003
COLUMNS
    battery OBJROW 60000
    peak OBJROW 60000
    peak R0000000 1
    pv OBJROW 55400
    semibase OBJROW 126000
    semibase R0000001 1
    semibase R0000003 1
    transmission_line OBJROW 10000
    transmission_line R0000002 1
    transmission_line R0000003 1
    nb_units_peak R0000000 -100
    nb_units_semibase R0000001 -200
    nb_units_transmission_line R0000002 -400
RHS
    RHS R0000003 300
BOUNDS
 UP BOUND battery 1000
 UP BOUND peak 2000
 UP BOUND pv 1000
 UP BOUND semibase 2000
 UP BOUND transmission_line 3200
 UI BOUND nb_units_peak 20
 UI BOUND nb_units_semibase 10
 UI BOUND nb_units_transmission_line 8
ENDATA
