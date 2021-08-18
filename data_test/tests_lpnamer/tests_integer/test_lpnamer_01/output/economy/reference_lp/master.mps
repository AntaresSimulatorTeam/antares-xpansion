NAME          ClpDefau  FREE
ROWS
 N  OBJROW
 E  R0000000
 E  R0000001
 E  R0000002
COLUMNS
    transmission_line OBJROW 10000
    transmission_line R0000000 1
    peak OBJROW 60000
    peak R0000001 1
    semibase OBJROW 126000
    semibase R0000002 1
    battery OBJROW 60000
    pv OBJROW 55400
    C0000005 R0000000 -400
    C0000006 R0000001 -100
    C0000007 R0000002 -200
RHS
BOUNDS
 UP BOUND transmission_line 3200
 UP BOUND peak 2000
 UP BOUND semibase 2000
 UP BOUND battery 1000
 UP BOUND pv 1000
 UI BOUND C0000005 8
 UI BOUND C0000006 20
 UI BOUND C0000007 10
ENDATA
