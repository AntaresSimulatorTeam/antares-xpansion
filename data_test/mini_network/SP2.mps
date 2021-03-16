*SENSE:Minimize
NAME          SP2
ROWS
 N  	OBJ
 L		prod
 L 		lines
 G 		demand
COLUMNS
    t		lines		-1.0
    p 		prod 		-1.0	
    z		OBJ			1.5
    z 		prod		1.5
    z		lines		1.0
    z 		demand		1.0
    perte 	OBJ			100.0
    perte	demand		1.0
RHS
 	RHS 	demand		6.0
BOUNDS
ENDATA