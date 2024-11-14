Feature:
    As a user
    I want the same solver used across all tools
    So that performances and results are coherent

    Scenario:
        Given the study path is "examples/SmallTestFiveCandidatesWithWeights"
        And solver is "Xpress"
        When I run antares-xpansion with the benders method and 1 proc(s)
        Then the simulation succeeds
        And Simulator has been launched with solver "Xpress"
        And Benders has been launched with solver "Xpress"

    Scenario:
        Given the study path is "examples/SmallTestFiveCandidatesWithWeights"
        And solver is "Cbc"
        When I run antares-xpansion with the benders method and 1 proc(s)
        Then the simulation succeeds
        And Simulator has been launched with solver "Sirius"
        And Benders has been launched with solver "Coin"