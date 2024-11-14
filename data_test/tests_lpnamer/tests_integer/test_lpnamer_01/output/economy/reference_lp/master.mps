NAME          empty
ROWS
 N  __OBJ___                        
 E  R1                              
 E  R2                              
 E  R3                              
COLUMNS
    battery                           __OBJ___                          60000
    peak                              __OBJ___                          60000
    peak                              R1                                1
    pv                                __OBJ___                          55400
    semibase                          __OBJ___                          126000
    semibase                          R2                                1
    transmission_line                 __OBJ___                          10000
    transmission_line                 R3                                1
    nb_units_peak                     R1                                -100
    nb_units_semibase                 R2                                -200
    nb_units_transmission_line        R3                                -400
BOUNDS
 UP BND00001                          battery                           1000
 UP BND00001                          peak                              2000
 UP BND00001                          pv                                1000
 UP BND00001                          semibase                          2000
 UP BND00001                          transmission_line                 3200
 UI BND00001                          nb_units_peak                     20
 UI BND00001                          nb_units_semibase                 10
 UI BND00001                          nb_units_transmission_line        8
ENDATA
