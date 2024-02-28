* Generated by MPModelProtoExporter
*   Name             : 
*   Format           : Free
*   Constraints      : 10
*   Variables        : 25
*     Binary         : 0
*     Integer        : 0
*     Continuous     : 25
NAME          
ROWS
 N  COST
 E  N0_Balance
 E  N0_Balance_1
 E  N1_Balance
 E  N1_Balance_2
 E  N2_Balance
 E  N2_Balance_3
 E  N3_Balance
 E  N3_Balance_4
 L  G_Max_generation
 L  G_Max_generation_5
COLUMNS
    N0_NegativeUnsuppliedEnergy_0_0  COST                                0.5  N0_Balance                           -1
    N0_NegativeUnsuppliedEnergy_0_1  COST                                0.5  N0_Balance_1                         -1
    N0_PositiveUnsuppliedEnergy_0_0  COST                                  5  N0_Balance                            1
    N0_PositiveUnsuppliedEnergy_0_1  COST                                  5  N0_Balance_1                          1
    N1_NegativeUnsuppliedEnergy_0_0  COST                                0.5  N1_Balance                           -1
    N1_NegativeUnsuppliedEnergy_0_1  COST                                0.5  N1_Balance_2                         -1
    N1_PositiveUnsuppliedEnergy_0_0  COST                                  5  N1_Balance                            1
    N1_PositiveUnsuppliedEnergy_0_1  COST                                  5  N1_Balance_2                          1
    N2_NegativeUnsuppliedEnergy_0_0  COST                                0.5  N2_Balance                           -1
    N2_NegativeUnsuppliedEnergy_0_1  COST                                0.5  N2_Balance_3                         -1
    N2_PositiveUnsuppliedEnergy_0_0  COST                                  5  N2_Balance                            1
    N2_PositiveUnsuppliedEnergy_0_1  COST                                  5  N2_Balance_3                          1
    N3_NegativeUnsuppliedEnergy_0_0  COST                                0.5  N3_Balance                           -1
    N3_NegativeUnsuppliedEnergy_0_1  COST                                0.5  N3_Balance_4                         -1
    N3_PositiveUnsuppliedEnergy_0_0  COST                                  5  N3_Balance                            1
    N3_PositiveUnsuppliedEnergy_0_1  COST                                  5  N3_Balance_4                          1
    L01_flow_0_0                     COST                                  1  N0_Balance                           -1
    L01_flow_0_0                     N1_Balance                            1
    L01_flow_0_1                     COST                                  1  N0_Balance_1                         -1
    L01_flow_0_1                     N1_Balance_2                          1
    L02_flow_0_0                     COST                                0.5  N0_Balance                           -1
    L02_flow_0_0                     N2_Balance                            1
    L02_flow_0_1                     COST                                0.5  N0_Balance_1                         -1
    L02_flow_0_1                     N2_Balance_3                          1
    L03_flow_0_0                     COST                                1.5  N0_Balance                           -1
    L03_flow_0_0                     N3_Balance                            1
    L03_flow_0_1                     COST                                1.5  N0_Balance_1                         -1
    L03_flow_0_1                     N3_Balance_4                          1
    G_generation_0_0                 COST                                0.5  N0_Balance                            1
    G_generation_0_0                 G_Max_generation                      1
    G_generation_0_1                 COST                                0.5  N0_Balance_1                          1
    G_generation_0_1                 G_Max_generation_5                    1
    G_p_max_0_0                      COST                                 20  G_Max_generation                     -1
    G_p_max_0_0                      G_Max_generation_5                   -1
RHS
    RHS                              N0_Balance                            0  N0_Balance_1                          0
    RHS                              N1_Balance                            1  N1_Balance_2                          3
    RHS                              N2_Balance                            3  N2_Balance_3                          3
    RHS                              N3_Balance                            0  N3_Balance_4                          3
    RHS                              G_Max_generation                      1  G_Max_generation_5                    1
BOUNDS
 PL BOUND                            N0_NegativeUnsuppliedEnergy_0_0
 PL BOUND                            N0_NegativeUnsuppliedEnergy_0_1
 PL BOUND                            N0_PositiveUnsuppliedEnergy_0_0
 PL BOUND                            N0_PositiveUnsuppliedEnergy_0_1
 PL BOUND                            N1_NegativeUnsuppliedEnergy_0_0
 PL BOUND                            N1_NegativeUnsuppliedEnergy_0_1
 PL BOUND                            N1_PositiveUnsuppliedEnergy_0_0
 PL BOUND                            N1_PositiveUnsuppliedEnergy_0_1
 PL BOUND                            N2_NegativeUnsuppliedEnergy_0_0
 PL BOUND                            N2_NegativeUnsuppliedEnergy_0_1
 PL BOUND                            N2_PositiveUnsuppliedEnergy_0_0
 PL BOUND                            N2_PositiveUnsuppliedEnergy_0_1
 PL BOUND                            N3_NegativeUnsuppliedEnergy_0_0
 PL BOUND                            N3_NegativeUnsuppliedEnergy_0_1
 PL BOUND                            N3_PositiveUnsuppliedEnergy_0_0
 PL BOUND                            N3_PositiveUnsuppliedEnergy_0_1
 UP BOUND                            L01_flow_0_0                        100
 UP BOUND                            L01_flow_0_1                        100
 UP BOUND                            L02_flow_0_0                        100
 UP BOUND                            L02_flow_0_1                        100
 UP BOUND                            L03_flow_0_0                        100
 UP BOUND                            L03_flow_0_1                        100
 PL BOUND                            G_generation_0_0
 PL BOUND                            G_generation_0_1
 UP BOUND                            G_p_max_0_0                          10
ENDATA