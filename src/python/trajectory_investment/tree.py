from __future__ import annotations

from pathlib import Path
from dataclasses import dataclass

import yaml
import json

from trajectory_input_keys import TrajectoryInputKeys, TrajectoryOuputKeys
from trajectory_input_keys import (
    ConstraintType,
    ConstraintOperator,
    InvestmentVariableType,
)


class TrajectoryTreeModule:
    """
    Class containing the data of a tree of annual Xpansion studies.
    """

    def __init__(self, input_file: Path):
        self.input_file = input_file

    # Errors
    class InvalidInputFile(Exception):
        pass

    class InvalidTreeStructure(Exception):
        pass

    # Data storage
    class TrajectoryGlobalData:
        """
        Contains the global data regarding the set of studies
        """

        def __init__(self, yaml_data_global, yaml_data_capacities):
            self.discount_rate = yaml_data_global.get(
                TrajectoryInputKeys.discount_rate_key(), 0.0
            )
            self.first_investment_year = yaml_data_global.get(
                TrajectoryInputKeys.first_investment_year_key(), 2025
            )
            # Path to studies
            self.studies = dict[str, Path]()
            studies_data = yaml_data_global.get(TrajectoryInputKeys.studies_key(), {})
            for name, path_str in studies_data.items():
                self.studies[name] = Path(path_str)
            # Initial capacities
            self.initial_capacities = dict[str, float]()
            # Might be overwritten by the input, simply ensures that there is a value
            self.initial_capacities[
                TrajectoryInputKeys.default_initial_capacity_key()
            ] = 0.0
            for candidate, value in yaml_data_capacities.items():
                self.initial_capacities[candidate] = value

        def initial_capacities_to_json(self) -> dict[str, float]:
            return self.initial_capacities

        def print(self):
            print("Global trajectory data : ")
            print(f" - Discount rate : {self.discount_rate}")
            print(f" - First investment year : {self.first_investment_year}")
            print(f" - Study pathes : {self.studies}")
            print(f" - Initial capacities : {self.initial_capacities}")

    class TreeNode:
        """
        Contains the structure of the tree itself
        - `name` refers to the node's name and thus its data
        - `probability` is the probability of going from the node's parent to this node.
        - `children` is a list of children node (and thus subtrees)
        """

        def __init__(self, yaml_data):
            self.name = yaml_data.get(TrajectoryInputKeys.node_key(), None)
            if self.name is None:
                raise TrajectoryTreeModule.InvalidTreeStructure(
                    f"Every node should have key '{TrajectoryInputKeys.node_key()}'"
                )
            self.probability_from_parent = yaml_data.get(
                TrajectoryInputKeys.probability_key(), 1.0
            )
            children_yaml = yaml_data.get(TrajectoryInputKeys.children_key(), [])
            # print(f"Parsed node {self.name} with probability {self.probability_from_parent}")
            self.children = []
            if len(children_yaml) > 0:
                self.children = [
                    TrajectoryTreeModule.TreeNode(subtree) for subtree in children_yaml
                ]

        def print(self, prefix=""):
            print(prefix + f"├─{self.probability_from_parent}─{self.name}")
            prefix_length = 4 + len(self.name) // 2
            for child in self.children:
                child.print(prefix + "|" + prefix_length * " ")

    class TrajectoryNodeData:
        """
        Contains the data relative to each node in the trajectory tree.
        """

        def __init__(self, name: str, yaml_data, studies_pathes: dict[str, Path]):
            self.name: str = name
            self.investment_date: int = yaml_data.get(
                TrajectoryInputKeys.investment_date_key(), 0
            )
            self.duration: int = yaml_data.get(TrajectoryInputKeys.duration_key(), 0)
            self.candidates_costs: dict[str, str] = yaml_data.get(
                TrajectoryInputKeys.candidates_costs_key(), dict[str, str]()
            )
            self.path = studies_pathes[name]

        def print(self):
            print(f"Node {self.name}")
            print(f"Investment date : {self.investment_date}")
            print(f"Duration represented : {self.duration}")
            print(f"Study path : {self.path}")

    class CandidatesTypeCosts:
        """
        Stores the costs of a candidate type
        (allows the type's properties to be shared among candidates of same type)
        """

        def __init__(self, name: str, yaml_data):
            self.name = name
            self.investment = yaml_data.get(TrajectoryInputKeys.investment_cost_key())
            self.operation_maintenance = yaml_data.get(
                TrajectoryInputKeys.oandm_cost_key()
            )
            self.retirement = yaml_data.get(TrajectoryInputKeys.retirement_cost_key())

    class TrajectoryConstraints:
        """
        Stores the trajectory constraints in the input format
        """

        def __init__(self, yaml_data, all_nodes: list[str], all_candidates: list[str]):
            # TBA : possibility for "all" keyword to be used with candidates and nodes
            # This requires the module to know all candidates name's thus reading the studies.
            # Nodes
            nodes_data = yaml_data.get(TrajectoryInputKeys.constraints_nodes_key())
            if nodes_data is None:
                raise TrajectoryTreeModule.InvalidInputFile(
                    f"Every constraint should have key '{TrajectoryInputKeys.constraints_nodes_key()}'"
                )
            if nodes_data == TrajectoryInputKeys.constraint_all_keyword():
                self.nodes = all_nodes
            else:
                self.nodes = nodes_data
            # Candidates
            candidates_data = yaml_data.get(
                TrajectoryInputKeys.constraints_candidates_key()
            )
            if candidates_data is None:
                raise TrajectoryTreeModule.InvalidInputFile(
                    f"Every constraint should have key '{TrajectoryInputKeys.constraints_candidates_key()}'"
                )
            if candidates_data == TrajectoryInputKeys.constraint_all_keyword():
                raise NotImplementedError(
                    "'all'keyword for candidates not yet implemented"
                )
            else:
                self.candidates: list[str] = candidates_data
            # Type
            type_data = yaml_data.get(TrajectoryInputKeys.constraint_type_key())
            if type_data is None:
                raise TrajectoryTreeModule.InvalidInputFile(
                    f"Every constraint should have key '{TrajectoryInputKeys.constraint_type_key()}'"
                )
            self.type = ConstraintType(type_data)
            # Rhs
            rhs_data = yaml_data.get(TrajectoryInputKeys.constraint_rhs_key())
            if rhs_data is None:
                raise TrajectoryTreeModule.InvalidInputFile(
                    f"Every constraint should have key '{TrajectoryInputKeys.constraint_rhs_key()}'"
                )
            self.rhs: float = rhs_data

        @staticmethod
        def build_variable_reference(
            node: str, candidate: str, variable_type: InvestmentVariableType
        ):
            return f"{node}::{candidate}::{variable_type.value}"

        def to_individual_max_investment(self) -> list[dict[str, any]]:
            assert self.type == ConstraintType.MAX_INDIVIDUAL_INVESTMENT
            output = list[dict[str, any]]()
            for node in self.nodes:
                for candidate in self.candidates:
                    constraint = {}
                    constraint[TrajectoryOuputKeys.constraint_coeffs_key()] = {
                        self.build_variable_reference(
                            node, candidate, InvestmentVariableType.DX_PLUS
                        ): 1
                    }
                    constraint[TrajectoryOuputKeys.constraint_rhs_key()] = self.rhs
                    constraint[TrajectoryOuputKeys.constraint_operator_key()] = (
                        ConstraintOperator.LEQ.value
                    )
                    output.append(constraint)
            return output

        def to_individual_max_retirement(self) -> list[dict[str, any]]:
            assert self.type == ConstraintType.MAX_INDIVIDUAL_RETIREMENT
            output = list[dict[str, any]]()
            for node in self.nodes:
                for candidate in self.candidates:
                    constraint = {}
                    constraint[TrajectoryOuputKeys.constraint_coeffs_key()] = {
                        self.build_variable_reference(
                            node, candidate, InvestmentVariableType.DX_MINUS
                        ): 1
                    }
                    constraint[TrajectoryOuputKeys.constraint_rhs_key()] = self.rhs
                    constraint[TrajectoryOuputKeys.constraint_operator_key()] = (
                        ConstraintOperator.LEQ.value
                    )
                    output.append(constraint)
            return output

        def to_cumulative_max_investment(self) -> list[dict[str, any]]:
            assert self.type == ConstraintType.MAX_CUMULATIVE_INVESTMENT
            output = list[dict[str, any]]()
            for node in self.nodes:
                constraint = {}
                constraint[TrajectoryOuputKeys.constraint_coeffs_key()] = {}
                constraint[TrajectoryOuputKeys.constraint_rhs_key()] = self.rhs
                constraint[TrajectoryOuputKeys.constraint_operator_key()] = (
                    ConstraintOperator.LEQ.value
                )
                for candidate in self.candidates:
                    ref = self.build_variable_reference(
                        node, candidate, InvestmentVariableType.DX_PLUS
                    )
                    constraint[TrajectoryOuputKeys.constraint_coeffs_key()][ref] = 1
                output.append(constraint)
            return output

        def to_merger_json(self) -> list[dict[str, any]]:
            """
            Converts an constraint in input format to a list of mathematical formulations for the C++ merger
            """
            if self.type == ConstraintType.MAX_INDIVIDUAL_INVESTMENT:
                return self.to_individual_max_investment()
            elif self.type == ConstraintType.MAX_INDIVIDUAL_RETIREMENT:
                return self.to_individual_max_retirement()
            elif self.type == ConstraintType.MAX_CUMULATIVE_INVESTMENT:
                return self.to_cumulative_max_investment()

    # Verifications
    def verify_tree_probabilities(self):
        def aux(subtree: TrajectoryTreeModule.TreeNode):
            if len(subtree.children) == 0:
                return
            cumulative = 0.0
            for child in subtree.children:
                cumulative += child.probability_from_parent
            if abs(cumulative - 1) > 1e-6:
                raise TrajectoryTreeModule.InvalidTreeStructure(
                    f"Sum of transition probabilities to children for node {subtree.name} is not 1 : got {cumulative}"
                )
            for child in subtree.children:
                aux(child)
            return

        aux(self.tree)
        return

    def verify_tree_investment_dates(self):
        depth_to_investment_date = {}

        def aux(
            subtree: TrajectoryTreeModule.TreeNode,
            depth=0,
            previous_date=None,
            previous_duration=None,
        ):
            # Check node's existence
            if subtree.name not in self.nodes:
                raise TrajectoryTreeModule.InvalidTreeStructure(
                    f"Tree refers to node {subtree.name} which was not found in the"
                    f" '{TrajectoryInputKeys.nodes_key()}' section of the input file"
                )
            node_data = self.nodes[subtree.name]
            # Check investment_year is the same across all nodes at a given depth
            nonlocal depth_to_investment_date
            if depth not in depth_to_investment_date:
                depth_to_investment_date[depth] = node_data.investment_date
            elif depth_to_investment_date[depth] != node_data.investment_date:
                raise TrajectoryTreeModule.InvalidTreeStructure(
                    f"Invalid tree at depth {depth}"
                    " : every node at the same depth must have the same investment date."
                )
            # Check that the previous node has a correct duration
            if (
                previous_date is not None
                and previous_duration is not None
                and previous_date + previous_duration != node_data.investment_date
            ):
                raise TrajectoryTreeModule.InvalidTreeStructure(
                    f"At node {subtree.name} : parent duration does not match."
                    f" Parent investment date : {previous_date}, parent duration : {previous_duration}"
                    f", child investment date : {node_data.investment_date}"
                )
            # Recursively check children
            for child in subtree.children:
                aux(child, depth + 1, node_data.investment_date, node_data.duration)
            return

        aux(self.tree)
        return

    def verify_nodes_candidates_types(self):
        for name, data in self.nodes.items():
            for candidate, candidate_type in data.candidates_costs.items():
                if candidate_type not in self.candidates_types_costs:
                    raise TrajectoryTreeModule.InvalidInputFile(
                        f"Node '{name}''s candidate '{candidate}' has type '{candidate_type}'"
                        f" which is not found in the {TrajectoryInputKeys.candidates_types_key()} section."
                    )

    # Methods
    def parse_trajectory_user_file(self):
        """
        Parse the data contained in the user's input file
        """
        with open(self.input_file) as file:
            content = yaml.full_load(file)
            if content is None:
                content = {}

            # Load the global data
            global_raw_data = content.get(TrajectoryInputKeys.global_key())
            capacities_raw_data = content.get(
                TrajectoryInputKeys.initial_capacities_key()
            )
            if global_raw_data is None:
                raise self.InvalidInputFile(
                    f"Input file must contain key '{TrajectoryInputKeys.global_key()}'"
                )
            if capacities_raw_data is None:
                raise self.InvalidInputFile(
                    f"Input file must contain key '{TrajectoryInputKeys.initial_capacities_key()}'"
                )
            self.global_data = self.TrajectoryGlobalData(
                global_raw_data, capacities_raw_data
            )

            # Load the tree
            tree_data = content.get(TrajectoryInputKeys.tree_key())
            if tree_data is None:
                raise self.InvalidInputFile(
                    f"Input file must contain key '{TrajectoryInputKeys.tree_key()}'"
                )
            if len(tree_data) > 1:
                raise self.InvalidTreeStructure(
                    "There can only be one root node to the tree"
                )
            self.tree = self.TreeNode(tree_data[0])

            # Load each nodes' data
            nodes_data = content.get(TrajectoryInputKeys.nodes_key())
            if nodes_data is None:
                raise self.InvalidInputFile(
                    f"Input file must contain key '{TrajectoryInputKeys.nodes_key()}'"
                )
            self.nodes = dict[str, self.TrajectoryNodeData]()
            for name, data in nodes_data.items():
                self.nodes[name] = self.TrajectoryNodeData(
                    name, data, self.global_data.studies
                )

            # Load candidates costs
            candidates_types_data = content.get(
                TrajectoryInputKeys.candidates_types_key()
            )
            if nodes_data is None:
                raise self.InvalidInputFile(
                    f"Input file must contain key '{TrajectoryInputKeys.candidates_types_key()}'"
                )
            self.candidates_types_costs = dict[str, self.CandidatesTypeCosts]()
            for name, data in candidates_types_data.items():
                self.candidates_types_costs[name] = self.CandidatesTypeCosts(name, data)

            # Load the constraints
            constraints_data = content.get(TrajectoryInputKeys.constraints_key())
            if constraints_data is None:
                raise self.InvalidInputFile(
                    f"Input file expects key '{TrajectoryInputKeys.constraints_key()}' even if left empty"
                )
            self.constraints = list[self.TrajectoryConstraints]()
            all_nodes = self.nodes.keys()
            for constraint_data in constraints_data:
                self.constraints.append(
                    self.TrajectoryConstraints(constraint_data, all_nodes, [])
                )

    def print(self):
        """
        Prints a summary of the data in the console.
        """
        parser.global_data.print()
        print("--------")
        parser.tree.print("")
        print("--------")
        for _, node in parser.nodes.items():
            node.print()
            print("---")

    def write_merger_json(self, output_file: Path):
        output = {}

        # Initial capacities
        output[TrajectoryOuputKeys.initial_capacities_key()] = (
            self.global_data.initial_capacities_to_json()
        )

        # Constraints
        output[TrajectoryOuputKeys.constraint_key()] = []
        for constraint in self.constraints:
            output[TrajectoryOuputKeys.constraint_key()].extend(
                constraint.to_merger_json()
            )

        with open(output_file, "w") as file:
            json.dump(output, file, indent=4)
        pass


parser = TrajectoryTreeModule(
    "merge_master_test/simple_tree/user_input_XpansionTrajectory.yaml"
)
parser.parse_trajectory_user_file()
parser.verify_tree_investment_dates()
parser.verify_tree_probabilities()
parser.verify_nodes_candidates_types()
parser.print()
parser.write_merger_json("merge_master_test/simple_tree/python_outputed_strucutre.json")
