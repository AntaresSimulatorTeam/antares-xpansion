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


@dataclass
class NodeDataMerger:
    """Node's data for the merger that is not directly in the input file."""
    lp_folder : str = ""
    master_mps_file : str = ""
    structure_file : str = ""



class TrajectoryModule:
    """
    Class containing the data of a tree of annual Xpansion studies.
    """

    def __init__(self, input_file: Path):
        self.input_file = input_file
        self.all_candidates : set[str] = {}
        self.all_nodes : set[str] = {}
        self.tree : TrajectoryModule.TreeNode = None
        self.nodes : dict[str, TrajectoryModule.NodeData] = None
        self.global_data : TrajectoryModule.GlobalData = None
        self.candidates_types_costs : dict[str, TrajectoryModule.CandidatesTypeCosts] = None
        self.constraints : list[TrajectoryModule.TrajectoryConstraints] = None

    # Errors
    class InvalidInputFile(Exception):
        pass

    class InvalidTreeStructure(Exception):
        pass

    class InvalidCandidates(Exception):
        pass
    
    class InvalidConstraint(Exception):
        pass
    
    # Data storage
    class GlobalData:
        """
        Contains the global data regarding the set of studies
        """

        def __init__(self, yaml_data_global, yaml_data_capacities):
            self.discount_rate = yaml_data_global.get(
                TrajectoryInputKeys.discount_rate_key(), 0.0
            )
            self.first_investment_date = yaml_data_global.get(
                TrajectoryInputKeys.first_investment_date_key(), 2025
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
            print(f" - First investment year : {self.first_investment_date}")
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
                raise TrajectoryModule.InvalidTreeStructure(
                    f"Every node should have key '{TrajectoryInputKeys.node_key()}'"
                )
            self.probability_from_parent = yaml_data.get(
                TrajectoryInputKeys.probability_key(), 1.0
            )
            children_yaml = yaml_data.get(TrajectoryInputKeys.children_key(), [])
            self.children = []
            if len(children_yaml) > 0:
                self.children = [
                    TrajectoryModule.TreeNode(subtree) for subtree in children_yaml
                ]

        def print(self, prefix=""):
            print(prefix + f"├─{self.probability_from_parent}─{self.name}")
            prefix_length = 4 + len(self.name) // 2
            for child in self.children:
                child.print(prefix + "|" + prefix_length * " ")

    class NodeData:
        """
        Contains the data relative to each node in the trajectory tree.
        """

        def __init__(self, name: str, yaml_data, studies_pathes: dict[str, Path]):
            self.name: str = name
            self.investment_date: int = yaml_data.get(
                TrajectoryInputKeys.investment_date_key(), 0
            )
            self.duration: int = yaml_data.get(TrajectoryInputKeys.duration_key(), 0)
            self.candidate_to_type_costs: dict[str, str] = yaml_data.get(
                TrajectoryInputKeys.candidate_to_type_costs(), dict[str, str]()
            )
            self.path = studies_pathes[name]
            self.parent : str = ""

        def print(self):
            print(f"Node {self.name}")
            print(f"Investment date : {self.investment_date}")
            print(f"Duration represented : {self.duration}")
            print(f"Study path : {self.path}")

        def compute_investment_discounting(self, global_data : TrajectoryModule.GlobalData):
            return (1 + global_data.discount_rate) ** (global_data.first_investment_date - self.investment_date)
        
        def compute_retirement_discounting(self, global_data : TrajectoryModule.GlobalData):
            return (1 + global_data.discount_rate) ** (global_data.first_investment_date - self.investment_date)
        
        def compute_oam_discounting(self, global_data : TrajectoryModule.GlobalData):
            factor = 0.0
            for year in range(self.investment_date, self.investment_date + self.duration):
                factor += (1 + global_data.discount_rate) ** (global_data.first_investment_date - year)
            return factor

        def to_merger_json(self,
                           file_data : NodeDataMerger, 
                           global_data : TrajectoryModule.GlobalData,
                           candidates_types : dict[str, TrajectoryModule.CandidatesTypeCosts]):
            output = dict[str, any]()
            output[TrajectoryOuputKeys.lp_folder_key()] = file_data.lp_folder
            output[TrajectoryOuputKeys.master_mps_key()] = file_data.master_mps_file
            output[TrajectoryOuputKeys.structure_file_key()] = file_data.structure_file
            output[TrajectoryOuputKeys.parent_key()] = self.parent

            candidates_costs : dict[str, dict[str, float]] = {}
            weight_ic = self.compute_investment_discounting(global_data)
            weight_rc = self.compute_retirement_discounting(global_data)
            weight_omc = self.compute_oam_discounting(global_data)

            for (candidate, type_name) in self.candidate_to_type_costs.items():
                costs_data = candidates_types[type_name]
                candidate_costs : dict[str, float] = {}
                candidate_costs[TrajectoryOuputKeys.investment_cost_key()] = weight_ic * costs_data.investment
                candidate_costs[TrajectoryOuputKeys.retirement_cost_key()] = weight_rc * costs_data.retirement
                candidate_costs[TrajectoryOuputKeys.oandm_cost_key()] = weight_omc * costs_data.operation_maintenance
                candidates_costs[candidate] = candidate_costs

            output[TrajectoryOuputKeys.candidate_costs()] = candidates_costs

            return output

    class CandidatesTypeCosts:
        """
        Stores the costs of a candidate type
        (allows the type's properties to be shared among candidates of same type)
        """
        def __init__(self, name: str, yaml_data):
            self.name = name
            self.investment = yaml_data.get(TrajectoryInputKeys.investment_cost_key(), 0.)
            self.operation_maintenance = yaml_data.get(
                TrajectoryInputKeys.oandm_cost_key(), 0.
            )
            self.retirement = yaml_data.get(TrajectoryInputKeys.retirement_cost_key(), 0.)

    class TrajectoryConstraints:
        """
        Stores the trajectory constraints in the input format
        """

        def __init__(self, yaml_data, all_nodes: list[str], all_candidates: list[str]):
            # Name
            self.name = yaml_data.get(TrajectoryInputKeys.constraint_name_key(), "")
            # Nodes
            nodes_data = yaml_data.get(TrajectoryInputKeys.constraints_nodes_key())
            if nodes_data is None:
                raise TrajectoryModule.InvalidInputFile(
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
                raise TrajectoryModule.InvalidInputFile(
                    f"Every constraint should have key '{TrajectoryInputKeys.constraints_candidates_key()}'"
                )
            if candidates_data == TrajectoryInputKeys.constraint_all_keyword():
                self.candidates: list[str] = all_candidates
            else:
                self.candidates: list[str] = candidates_data
            # Type
            type_data = yaml_data.get(TrajectoryInputKeys.constraint_type_key())
            if type_data is None:
                raise TrajectoryModule.InvalidInputFile(
                    f"Every constraint should have key '{TrajectoryInputKeys.constraint_type_key()}'"
                )
            self.type = ConstraintType(type_data)
            # Rhs
            rhs_data = yaml_data.get(TrajectoryInputKeys.constraint_rhs_key())
            if rhs_data is None:
                raise TrajectoryModule.InvalidInputFile(
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
        def aux(subtree: TrajectoryModule.TreeNode):
            if len(subtree.children) == 0:
                return
            cumulative = 0.0
            for child in subtree.children:
                cumulative += child.probability_from_parent
            if abs(cumulative - 1) > 1e-6:
                raise TrajectoryModule.InvalidTreeStructure(
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
            subtree: TrajectoryModule.TreeNode,
            depth=0,
            previous_date=None,
            previous_duration=None,
        ):
            # Check node's existence
            if subtree.name not in self.nodes:
                raise TrajectoryModule.InvalidTreeStructure(
                    f"Tree refers to node {subtree.name} which was not found in the"
                    f" '{TrajectoryInputKeys.nodes_key()}' section of the input file"
                )
            node_data = self.nodes[subtree.name]
            # Check investment_year is the same across all nodes at a given depth
            nonlocal depth_to_investment_date
            if depth not in depth_to_investment_date:
                depth_to_investment_date[depth] = node_data.investment_date
            elif depth_to_investment_date[depth] != node_data.investment_date:
                raise TrajectoryModule.InvalidTreeStructure(
                    f"Invalid tree at depth {depth}"
                    " : every node at the same depth must have the same investment date."
                )
            # Check that the previous node has a correct duration
            if (
                previous_date is not None
                and previous_duration is not None
                and previous_date + previous_duration != node_data.investment_date
            ):
                raise TrajectoryModule.InvalidTreeStructure(
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
            for candidate, candidate_type in data.candidate_to_type_costs.items():
                if candidate_type not in self.candidates_types_costs:
                    raise TrajectoryModule.InvalidInputFile(
                        f"Node '{name}''s candidate '{candidate}' has type '{candidate_type}'"
                        f" which is not found in the {TrajectoryInputKeys.candidates_types_key()} section."
                    )

    def verify_constraint_variable_reference(self):
        """ Verifies the the constraints reference existing variable."""
        for constraint in self.constraints:
            # Nodes
            for node in constraint.nodes:
                if node not in self.all_nodes:
                    raise self.InvalidConstraint(
                        f"Constraint '{constraint.name}' references node '{node}'" \
                        " which does not exist in the study"
                    )
            # Candidates
            for candidate in constraint.candidates:
                if candidate not in self.all_candidates:
                    raise self.InvalidConstraint(
                        f"Constraint '{constraint.name}' references candidate '{candidate}'" \
                        " which does not exist in the study"
                    )       

    def verify_nodes_candidates_match_with_study(self):
        pass

    def verify_all_nodes_same_candidates(self):
        for name, data in self.nodes.items():
            node_candidates = set(data.candidate_to_type_costs.keys())
            if node_candidates != self.all_candidates:
                diff_exceed = node_candidates - self.all_candidates
                diff_missing = self.all_candidates - node_candidates
                raise self.InvalidCandidates(
                    "All nodes must have the same exact candidates." \
                    f" At node '{name}', missing candidates : {diff_missing}" \
                    f", candidates not present elsewhere : {diff_exceed}"
                )
            else:
                pass
            

    # Method
    def set_nodes_parents_names(self):
        """After parsing the tree and the nodes, go through the tree to write each node's parent in its data"""
        assert self.tree is not None and self.nodes is not None
        def aux(subtree : TrajectoryModule.TreeNode, parent = "root"):
            self.nodes[subtree.name].parent = parent
            for child in subtree.children:
                aux(child, subtree.name)
        aux(self.tree, "root")
        return 
    
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
            self.global_data = self.GlobalData(
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
            self.nodes = dict[str, self.NodeData]()
            for name, data in nodes_data.items():
                self.nodes[name] = self.NodeData(
                    name, data, self.global_data.studies
                )

            # Add the parent's name to each node's data
            self.set_nodes_parents_names()

            # Set of all candidates names (should be the same in all nodes, verified later)
            self.all_candidates = set(
                self.nodes[self.tree.name].candidate_to_type_costs.keys()
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
            self.all_nodes = self.nodes.keys()
            for constraint_data in constraints_data:
                self.constraints.append(
                    self.TrajectoryConstraints(constraint_data, self.all_nodes, self.all_candidates)
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
        constraints_list = []
        for constraint in self.constraints:
            constraints_list.extend(constraint.to_merger_json())
        output[TrajectoryOuputKeys.constraint_key()] = constraints_list

        # Tree
        nodes_output_dict = {}
        for (name, data) in self.nodes.items():
            file_data : NodeDataMerger = NodeDataMerger(
                "placeholder/folder/for/now",
                "master.mps",
                "structure.txt"
            )
            nodes_output_dict[name] = data.to_merger_json(
                file_data,
                self.global_data,
                self.candidates_types_costs
            )
        output[TrajectoryOuputKeys.tree_key()] = nodes_output_dict

        with open(output_file, "w") as file:
            json.dump(output, file, indent=4)
        pass


parser = TrajectoryModule(
    "merge_master_test/simple_tree/user_input_XpansionTrajectory.yaml"
)
parser.parse_trajectory_user_file()
parser.print()
parser.verify_all_nodes_same_candidates()
parser.verify_constraint_variable_reference()
parser.verify_tree_investment_dates()
parser.verify_tree_probabilities()
parser.verify_nodes_candidates_types()
parser.write_merger_json("merge_master_test/simple_tree/python_outputed_strucutre.json")
