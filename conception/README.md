# Xpansion design informations

## Domains

Xpansion aims at performing investment simulations for Antares studies. This means that the core domain of xpansion
could
be described as : **investment simulations for Antares studies**
This core domain is composed of subdomains:

- Investment strategy: how to describe the investment strategy based on an Antares Simulation
- Linear optimization problem-solving

## Mapping design with domains

| Domain                                     | Component           | Details                                                                                                                                                                     |
|--------------------------------------------|---------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Investment simulations for Antares studies | Xpansion as a whole |                                                                                                                                                                             |
| Investment strategy                        | Problem Generation  | Investment strategy is defined by candidates. Those candidates are used to enrich an Antares study simulation. It produces a linear optimization problem                    |
| Linear optimization problem-solv ing       | Benders             | Benders solve optimization problems. It is agnostic of the "business" of the core domain. Mathematical outputs need to be translated as business values in other components |

## Conception folder

In the conception folder you will find design documentation.

### Architecture decision records

Also called ADR. A list of design decisions with date, status and motivation. This allows going back to ADR to
understand
why choices have been made. Choices are not irrevocable, they can be deprecated in favor of new choices. The goal of
ADRs
is exactly to follow the evolution of these choices.

### C4

Various C4 diagrams following [C4 model conventions](https://c4model.com/)

### SequenceDiagrams

Various sequence diagrams to help understand the flow of the application

### SimulationOutputs

Various example of simulation outputs

