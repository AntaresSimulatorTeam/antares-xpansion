NAME          ClpDefau  FREE
ROWS
 N  OBJROW
 E  R0000000
 E  R0000001
COLUMNS
    transmission_line OBJROW 10000
    transmission_line R0000000 1
    transmission_line_2 OBJROW 10000
    transmission_line_2 R0000001 1
    nb_units_transmission_line R0000000 -200
    nb_units_transmission_line_2 R0000001 -200
RHS
BOUNDS
 UP BOUND transmission_line 800
 UP BOUND transmission_line_2 800
 UI BOUND nb_units_transmission_line 4
 UI BOUND nb_units_transmission_line_2 4
ENDATA
