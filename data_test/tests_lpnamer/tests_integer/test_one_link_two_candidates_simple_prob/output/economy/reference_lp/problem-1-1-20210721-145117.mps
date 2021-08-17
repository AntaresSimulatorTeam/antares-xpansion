NAME          Pb        FREE
ROWS
 N  OBJROW
 L  R0000000
 G  R0000001
COLUMNS
    ValeurDeNTCOrigineVersExtremite_0_0_0_ OBJROW 1
    ValeurDeNTCOrigineVersExtremite_0_0_0_ R0000000 1
    ValeurDeNTCOrigineVersExtremite_0_0_0_ R0000001 1
    transmission_line_1 R0000000 -1
    transmission_line_1 R0000001 1
    transmission_line_2 R0000000 -1
    transmission_line_2 R0000001 1
RHS
BOUNDS
 LO BOUND ValeurDeNTCOrigineVersExtremite_0_0_0_ -1e+20
 UP BOUND ValeurDeNTCOrigineVersExtremite_0_0_0_ 1e+20
 LO BOUND transmission_line_1 -1e+20
 UP BOUND transmission_line_1 1e+20
 LO BOUND transmission_line_2 -1e+20
 UP BOUND transmission_line_2 1e+20
ENDATA
