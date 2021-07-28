NAME          ClpDefau  FREE
ROWS
 N  OBJROW
 E  R0000000
 E  R0000001
COLUMNS
    transmission_line_1 OBJROW 10000
    transmission_line_1 R0000000 1
    transmission_line_2 OBJROW 10000
    transmission_line_2 R0000001 1
    C0000002 R0000000 -200
    C0000003 R0000001 -200
RHS
BOUNDS
 UP BOUND transmission_line_1 800
 UP BOUND transmission_line_2 800
 UI BOUND C0000002 4
 UI BOUND C0000003 4
ENDATA
