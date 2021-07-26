NAME          Pb        FREE
ROWS
 N  OBJROW
 L  R0000000
 G  R0000001
COLUMNS
    C0000000 OBJROW 1
    C0000000 R0000000 1
    C0000000 R0000001 1
    transmission_line_1 R0000000 -1
    transmission_line_1 R0000001 1
    transmission_line_2 R0000000 -1
    transmission_line_2 R0000001 1
RHS
BOUNDS
 LO BOUND C0000000 -1e+20
 UP BOUND C0000000 1e+20
 LO BOUND transmission_line_1 -1e+20
 UP BOUND transmission_line_1 1e+20
 LO BOUND transmission_line_2 -1e+20
 UP BOUND transmission_line_2 1e+20
ENDATA
