# Implémentation d'une nouvelle architecture pour la décomposition de Benders

## Description des entrées

Un fichier de structure est nécessaire pour faire le lien entre les problèmes d'optimisation. En colonne il contient le nom du problème (le nom du fichier sans l'extension .mps), le nom de la variable et l'index de la variable dans le problème.

L'ensemble des fichiers .mps décrits dans le fichier structure doivent exister au même endroit (Typiquement, les mps et le fichier structure sont dans le répertoire "lp" à l'intérieur d'un répertoire de sortie d'une simulation antares).

Enfin, un fichier d'option permet de décrire les paramètres de l'algorithme. Il est possible de le générer en lançant n'importe quel exécutable sans argument.

## Description des binaires

Le flag USE_MPI permet d'indiquer si la compilation des binaires utilisant MPI (et donc toute la suite de compilation dédiée) est utilisée.
Selon la version d'ortools utilisée plusieurs solveurs peuvent être disponible lors de la compilation. Un des flags suivant permet de préciser le solveurs (CPLEX_SOLVER, XPRESS_SOLVER, GUROBI_SOLVER). Par défaut, Coin (Cbc et Clp) sera utilisé.

### benderssequential

Implémente l'algorithme de Benders sans paralléilisation

### bendersmpi

Implémente l'algorithme de Benders en utilisant MPI pour la parallélisation

### merge_mps

Implémente une méthode permettant de fusionner les différents problèmes et les résoudre en frontal (c'est à dire sans décomposition de Benders)


## Lancement
Le script src/src_python/launch.py permet de lancer l'execution du module antaresXpansion:

python3 ./src/src_python/launch.py --step [lp, optim, full, antares, getnames] --dataDir PATH-TO-ANTARES-TEST-CASE --installDir PATH-TO-ALL-BINARIES-FOLDER --method [mpibenders, mergeMPS, both] [--simulationName name]