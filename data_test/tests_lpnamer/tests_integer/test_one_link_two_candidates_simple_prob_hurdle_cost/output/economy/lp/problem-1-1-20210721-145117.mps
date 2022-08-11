NAME          Pb        FREE
ROWS
 N  OBJROW
 E  R0000000
 L  R0000001
 G  R0000002
 L  R0000003
 L  R0000004
COLUMNS
    ValeurDeNTCOrigineVersExtremite_0_0_0_ OBJROW 1
    ValeurDeNTCOrigineVersExtremite_0_0_0_ R0000000 1
    ValeurDeNTCOrigineVersExtremite_0_0_0_ R0000001 1
    ValeurDeNTCOrigineVersExtremite_0_0_0_ R0000002 1
    CoutOrigineVersExtremiteDeLInterconnexion_0_0_ OBJROW 0.5
    CoutOrigineVersExtremiteDeLInterconnexion_0_0_ R0000000 -1
    CoutOrigineVersExtremiteDeLInterconnexion_0_0_ R0000003 1
    CoutExtremiteVersOrigineDeLInterconnexion_0_0_ OBJROW 0.3
    CoutExtremiteVersOrigineDeLInterconnexion_0_0_ R0000000 1
    CoutExtremiteVersOrigineDeLInterconnexion_0_0_ R0000004 1
    transmission_line_1 R0000001 -1
    transmission_line_1 R0000002 1
    transmission_line_1 R0000003 -1
    transmission_line_1 R0000004 -1
    transmission_line_2 R0000001 -1
    transmission_line_2 R0000002 1
    transmission_line_2 R0000003 -1
    transmission_line_2 R0000004 -1
RHS
BOUNDS
 LO BOUND ValeurDeNTCOrigineVersExtremite_0_0_0_ -1e+20
 UP BOUND ValeurDeNTCOrigineVersExtremite_0_0_0_ 1e+20
 UP BOUND CoutOrigineVersExtremiteDeLInterconnexion_0_0_ 1e+20
 UP BOUND CoutExtremiteVersOrigineDeLInterconnexion_0_0_ 1e+20
 LO BOUND transmission_line_1 -1e+20
 UP BOUND transmission_line_1 1e+20
 LO BOUND transmission_line_2 -1e+20
 UP BOUND transmission_line_2 1e+20
ENDATA
