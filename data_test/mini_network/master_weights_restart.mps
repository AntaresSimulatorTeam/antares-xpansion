NAME          master    FREE
ROWS
 N  OBJROW
 L  budget
 E  R0000001
 L  R0000002
 L  R0000003
COLUMNS
    t OBJROW 3
    t budget 1
    p OBJROW 2
    p budget 1
    p R0000002 -65.01000000000001
    p R0000003 -0.6566666666666666
    alpha OBJROW 1
    alpha R0000001 1
    alpha_0 R0000001 -1
    alpha_0 R0000002 -1
    alpha_1 R0000001 -1
    alpha_1 R0000003 -1
RHS
    RHS budget 10
    RHS R0000002 -198
    RHS R0000003 -6
BOUNDS
 UP BOUND t 1e+20
 UP BOUND p 1e+20
 LO BOUND alpha -10000000000
 UP BOUND alpha 204
 LO BOUND alpha_0 -10000000000
 UP BOUND alpha_0 1e+20
 LO BOUND alpha_1 -10000000000
 UP BOUND alpha_1 1e+20
ENDATA
