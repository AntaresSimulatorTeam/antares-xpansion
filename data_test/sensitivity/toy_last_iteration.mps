NAME          ClpDefau  FREE
ROWS
 N  OBJROW
 E  R0000000
 L  R0000001
 L  R0000002
 L  R0000003
 L  R0000004
COLUMNS
    peak OBJROW 10
    peak R0000001 -10
    peak R0000002 -25
    semibase OBJROW 90
    semibase R0000001 -1
    semibase R0000002 -2
    semibase R0000003 -20
    semibase R0000004 -30
    alpha OBJROW 1
    alpha R0000000 1
    alpha_0 R0000000 -1
    alpha_0 R0000001 -1
    alpha_0 R0000003 -1
    alpha_1 R0000000 -1
    alpha_1 R0000002 -1
    alpha_1 R0000004 -1
RHS
    RHS R0000001 -300
    RHS R0000002 -400
    RHS R0000003 -350
    RHS R0000004 -500
BOUNDS
 UP BOUND peak 3000
 UP BOUND semibase 400
 LO BOUND alpha -60
 UP BOUND alpha 1000
 LO BOUND alpha_0 -100
 UP BOUND alpha_0 150
 LO BOUND alpha_1 -200
 UP BOUND alpha_1 2000
ENDATA
