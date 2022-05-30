NAME          ClpDefau  FREE
ROWS
 N  OBJROW
 E  R0000000
 E  R0000001
COLUMNS
    transmission_line_1 OBJROW 10000
    transmission_line_1 R0000000 1
    transmission_line_2 OBJROW 20000
    transmission_line_2 R0000001 1
    nb_units_transmission_line_1 R0000000 -200
    nb_units_transmission_line_2 R0000001 -300
RHS
BOUNDS
 UP BOUND transmission_line_1 800
 UP BOUND transmission_line_2 1500
 UI BOUND nb_units_transmission_line_1 4
 UI BOUND nb_units_transmission_line_2 5
ENDATA
