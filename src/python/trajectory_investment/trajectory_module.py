from __future__ import annotations

from pathlib import Path
from trajectory_input_keys import TrajectoryInputKeys, TrajectoryOuputKeys
from trajectory_input_keys import ConstraintTypeEnum, ConstraintOperatorEnum, InvestmentVariableTypeEnum
from pydantic import BaseModel, Field, PositiveInt, NonNegativeFloat, NonNegativeInt
from typing import Literal
from dataclasses import dataclass

import yaml
import json


@dataclass
class NodeDataDataMerger:
    """NodeData's data for the merger that is not directly in the input file."""
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
        self.tree : TrajectoryModule.Tree = None
        self.nodes : dict[str, TrajectoryModule.NodeData] = None
        self.global_data : TrajectoryModule.GlobalData = None
        self.candidates_types_costs : dict[str, TrajectoryModule.CandidateType] = None
        self.constraints : list[TrajectoryModule.TrajectoryConstraint] = None
        self.initial_capacities : list[str, float] = None

    # Errors
    class InvalidInputFile(Exception):
        pass

    class InvalidTreeStructure(Exception):
        pass

    class InvalidCandidates(Exception):
        pass
    
    class InvalidTrajectoryConstraint(Exception):
        pass
    
    # Data storage
    class GlobalData(BaseModel):
        discount_rate : NonNegativeFloat = Field(alias= TrajectoryInputKeys.discount_rate_key())
        first_investment_date : NonNegativeInt = Field(alias= TrajectoryInputKeys.first_investment_date_key())
        studies : dict[str, str] = Field(alias= TrajectoryInputKeys.studies_key())

        def print(self):
            print("Global trajectory data : ")
            print(f" - Discount rate : {self.discount_rate}")
            print(f" - First investment year : {self.first_investment_date}")
            print(f" - Study pathes : {self.studies}")

    class Tree(BaseModel):
        node_name : str = Field(alias= TrajectoryInputKeys.node_key())
        probability_from_parent : float = Field(
            1.0,
            alias= TrajectoryInputKeys.probability_key(),
            ge= 0.0,
            le= 1.0
        )
        children : list[TrajectoryModule.Tree] = Field([], alias= TrajectoryInputKeys.children_key())

        def print(self, prefix=""):
            print(prefix + f"├─{self.probability_from_parent}─{self.node_name}")
            prefix_length = 4 + len(self.node_name) // 2
            for child in self.children:
                child.print(prefix + "|" + prefix_length * " ")

    class TrajectoryConstraint(BaseModel):
        name : str = Field(alias= TrajectoryInputKeys.constraint_name_key())
        nodes : Literal['all'] | list[str] = Field(alias= TrajectoryInputKeys.constraints_nodes_key())
        candidates : Literal['all'] | list[str] = Field(alias= TrajectoryInputKeys.constraints_candidates_key())
        cons_type : ConstraintTypeEnum = Field(alias= TrajectoryInputKeys.constraint_type_key())
        rhs : float = Field(alias= TrajectoryInputKeys.constraint_rhs_key())

        @staticmethod
        def build_variable_reference(
            node: str, candidate: str, variable_type: InvestmentVariableTypeEnum
        ):
            return f"{node}::{candidate}::{variable_type.value}"

        def to_individual_max_investment(self) -> list[dict[str, any]]:
            assert self.cons_type == ConstraintTypeEnum.MAX_INDIVIDUAL_INVESTMENT
            output = list[dict[str, any]]()
            for node in self.nodes:
                for candidate in self.candidates:
                    constraint = {}
                    constraint[TrajectoryOuputKeys.constraint_coeffs_key()] = {
                        self.build_variable_reference(
                            node, candidate, InvestmentVariableTypeEnum.DX_PLUS
                        ): 1
                    }
                    constraint[TrajectoryOuputKeys.constraint_rhs_key()] = self.rhs
                    constraint[TrajectoryOuputKeys.constraint_operator_key()] = (
                        ConstraintOperatorEnum.LEQ.value
                    )
                    output.append(constraint)
            return output

        def to_individual_max_retirement(self) -> list[dict[str, any]]:
            assert self.cons_type == ConstraintTypeEnum.MAX_INDIVIDUAL_RETIREMENT
            output = list[dict[str, any]]()
            for node in self.nodes:
                for candidate in self.candidates:
                    constraint = {}
                    constraint[TrajectoryOuputKeys.constraint_coeffs_key()] = {
                        self.build_variable_reference(
                            node, candidate, InvestmentVariableTypeEnum.DX_MINUS
                        ): 1
                    }
                    constraint[TrajectoryOuputKeys.constraint_rhs_key()] = self.rhs
                    constraint[TrajectoryOuputKeys.constraint_operator_key()] = (
                        ConstraintOperatorEnum.LEQ.value
                    )
                    output.append(constraint)
            return output

        def to_cumulative_max_investment(self) -> list[dict[str, any]]:
            assert self.cons_type == ConstraintTypeEnum.MAX_CUMULATIVE_INVESTMENT
            output = list[dict[str, any]]()
            for node in self.nodes:
                constraint = {}
                constraint[TrajectoryOuputKeys.constraint_coeffs_key()] = {}
                constraint[TrajectoryOuputKeys.constraint_rhs_key()] = self.rhs
                constraint[TrajectoryOuputKeys.constraint_operator_key()] = (
                    ConstraintOperatorEnum.LEQ.value
                )
                for candidate in self.candidates:
                    ref = self.build_variable_reference(
                        node, candidate, InvestmentVariableTypeEnum.DX_PLUS
                    )
                    constraint[TrajectoryOuputKeys.constraint_coeffs_key()][ref] = 1
                output.append(constraint)
            return output

        def to_merger_json(self) -> list[dict[str, any]]:
            """
            Converts an constraint in input format to a list of mathematical formulations for the C++ merger
            """
            if self.cons_type == ConstraintTypeEnum.MAX_INDIVIDUAL_INVESTMENT:
                return self.to_individual_max_investment()
            elif self.cons_type == ConstraintTypeEnum.MAX_INDIVIDUAL_RETIREMENT:
                return self.to_individual_max_retirement()
            elif self.cons_type == ConstraintTypeEnum.MAX_CUMULATIVE_INVESTMENT:
                return self.to_cumulative_max_investment()


    class NodeData(BaseModel):
        name : str = Field("")
        investment_date : NonNegativeInt = Field(alias= TrajectoryInputKeys.investment_date_key())
        duration : PositiveInt = Field(alias= TrajectoryInputKeys.duration_key())
        candidate_to_type : dict[str, str] = Field(alias= TrajectoryInputKeys.candidates_to_types_key())
        path : Path = Field(Path(""))
        parent : str = Field("")

        def print(self):
            print(f"NodeData {self.name}")
            print(f"Investment date : {self.investment_date}")
            print(f"Duration represented : {self.duration}")
            print(f"Study path : {self.path}")
            print(f"Parent : {self.parent}")

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
                        file_data : NodeDataDataMerger, 
                        global_data : TrajectoryModule.GlobalData,
                        candidates_types : dict[str, TrajectoryModule.CandidateType]):
            output = dict[str, any]()
            output[TrajectoryOuputKeys.lp_folder_key()] = file_data.lp_folder
            output[TrajectoryOuputKeys.master_mps_key()] = file_data.master_mps_file
            output[TrajectoryOuputKeys.structure_file_key()] = file_data.structure_file
            output[TrajectoryOuputKeys.parent_key()] = self.parent

            candidates_costs : dict[str, dict[str, float]] = {}
            weight_ic = self.compute_investment_discounting(global_data)
            weight_rc = self.compute_retirement_discounting(global_data)
            weight_omc = self.compute_oam_discounting(global_data)

            for (candidate, type_name) in self.candidate_to_type.items():
                costs_data = candidates_types[type_name]
                candidate_costs : dict[str, float] = {}
                candidate_costs[TrajectoryOuputKeys.investment_cost_key()] = weight_ic * costs_data.investment_cost
                candidate_costs[TrajectoryOuputKeys.retirement_cost_key()] = weight_rc * costs_data.retirement_cost
                candidate_costs[TrajectoryOuputKeys.oandm_cost_key()] = weight_omc * costs_data.oam_cost
                candidates_costs[candidate] = candidate_costs

            output[TrajectoryOuputKeys.candidate_costs()] = candidates_costs

            return output

    class CandidateType(BaseModel):
        investment_cost : NonNegativeFloat = Field(alias= TrajectoryInputKeys.investment_cost_key())
        retirement_cost : NonNegativeFloat = Field(alias= TrajectoryInputKeys.retirement_cost_key())
        oam_cost : NonNegativeFloat = Field(alias= TrajectoryInputKeys.oandm_cost_key())


    class TrajectoryInputFile(BaseModel):
        global_data : TrajectoryModule.GlobalData = Field(alias= TrajectoryInputKeys.global_key())
        tree : TrajectoryModule.Tree = Field(alias= TrajectoryInputKeys.tree_key())
        constraints : list[TrajectoryModule.TrajectoryConstraint] = Field(alias= TrajectoryInputKeys.constraints_key())
        nodes : dict[str, TrajectoryModule.NodeData] = Field(alias= TrajectoryInputKeys.nodes_key())
        candidates_types : dict[str, TrajectoryModule.CandidateType] = Field(alias= TrajectoryInputKeys.candidates_types_key())
        initial_capacities : dict[str, NonNegativeInt]

        def __init__(self, **kwargs):
            super().__init__(**kwargs)
            # Insert the default key in the initial capacities if not present
            if TrajectoryInputKeys.default_initial_capacity_key() not in self.initial_capacities:
                print(f"Inserted key '{TrajectoryInputKeys.default_initial_capacity_key()}' with value '0.0' into the initial capacities")
                self.initial_capacities[TrajectoryInputKeys.default_initial_capacity_key()] = 0.0

    # Verifications
    def verify_tree_probabilities(self):
        def aux(subtree: TrajectoryModule.TreeNodeData):
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
            subtree: TrajectoryModule.Tree,
            depth=0,
            previous_date=None,
            previous_duration=None,
        ):
            # Check node's existence
            if subtree.node_name not in self.nodes:
                raise TrajectoryModule.InvalidTreeStructure(
                    f"Tree refers to node {subtree.node_name} which was not found in the"
                    f" '{TrajectoryInputKeys.nodes_key()}' section of the input file"
                )
            node_data = self.nodes[subtree.node_name]
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
                    f"At node {subtree.node_name} : parent duration does not match."
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
            for candidate, candidate_type in data.candidate_to_type.items():
                if candidate_type not in self.candidates_types_costs:
                    raise TrajectoryModule.InvalidInputFile(
                        f"NodeData '{name}''s candidate '{candidate}' has type '{candidate_type}'"
                        f" which is not found in the {TrajectoryInputKeys.candidates_types_key()} section."
                    )

    def verify_constraint_variable_reference(self):
        """ Verifies the the constraints reference existing variable."""
        for constraint in self.constraints:
            # NodeDatas
            for node in constraint.nodes:
                if node not in self.all_nodes:
                    raise self.InvalidTrajectoryConstraint(
                        f"TrajectoryConstraint '{constraint.name}' references node '{node}'" \
                        " which does not exist in the study"
                    )
            # Candidates
            for candidate in constraint.candidates:
                if candidate not in self.all_candidates:
                    raise self.InvalidTrajectoryConstraint(
                        f"TrajectoryConstraint '{constraint.name}' references candidate '{candidate}'" \
                        " which does not exist in the study"
                    )       

    def verify_nodes_candidates_match_with_study(self):
        pass

    def verify_all_nodes_same_candidates(self):
        for name, data in self.nodes.items():
            node_candidates = set(data.candidate_to_type.keys())
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
    def set_nodes_names_study_pathes(self):
        """After parsing the raw node data, 'copy' the node's name and study path to its data for ease of access"""
        assert self.nodes is not None
        for (name, data) in self.nodes.items():
            data.name = name
            data.path = self.global_data.studies[name]
        
    def set_nodes_parents_names(self):
        """After parsing the tree and the nodes, go through the tree to write each node's parent in its data"""
        assert self.tree is not None and self.nodes is not None
        def aux(subtree : TrajectoryModule.Tree, parent = "root"):
            self.nodes[subtree.node_name].parent = parent
            for child in subtree.children:
                aux(child, subtree.node_name)
        aux(self.tree, "root")
        return 
    
    def expand_all_keyword_in_constraints(self):
        for constraint in self.constraints:
            if constraint.candidates == TrajectoryInputKeys.constraint_all_keyword():
                constraint.candidates = self.all_candidates
            if constraint.nodes == TrajectoryInputKeys.constraint_all_keyword():
                constraint.nodes = self.all_nodes
        
    def parse_trajectory_user_file(self):
        """
        Parse the data contained in the user's input file
        """
        with open(self.input_file) as file:
            content = yaml.full_load(file)
            validated_content = self.TrajectoryInputFile.model_validate(content)
            print(validated_content)

            self.global_data = validated_content.global_data
            self.tree = validated_content.tree
            self.nodes = validated_content.nodes
            self.constraints = validated_content.constraints
            self.candidates_types_costs = validated_content.candidates_types
            self.initial_capacities = validated_content.initial_capacities

            # Complete the node's data
            self.set_nodes_names_study_pathes()
            self.set_nodes_parents_names()

            # Set of all candidates names (should be the same in all nodes, verified later)
            self.all_candidates = set(self.nodes[self.tree.node_name].candidate_to_type.keys())
            self.all_nodes = self.nodes.keys()

            # Constraint explicit list of nodes and candidates
            self.expand_all_keyword_in_constraints()

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
        output[TrajectoryOuputKeys.initial_capacities_key()] = self.initial_capacities

        # TrajectoryConstraints
        constraints_list = []
        for constraint in self.constraints:
            constraints_list.extend(constraint.to_merger_json())
        output[TrajectoryOuputKeys.constraint_key()] = constraints_list

        # Tree
        nodes_output_dict = {}
        for (name, data) in self.nodes.items():
            file_data : NodeDataDataMerger = NodeDataDataMerger(
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
