import datetime
import json
from enum import Enum
from pathlib import Path
from typing import Any, Dict, List, Literal, Optional, Set, Union

import yaml
from pydantic import BaseModel, Field, NonNegativeFloat, NonNegativeInt, PositiveInt

from antares_xpansion.trajectory.user_input_keys import TrajectoryInputKeys as InKeys
from antares_xpansion.trajectory.user_input_keys import TrajectoryOuputKeys as OutKeys


# Enums
class ConstraintTypeEnum(Enum):
    MAX_INDIVIDUAL_INVESTMENT = "max_investment_per_node_per_candidate"
    MAX_CUMULATIVE_INVESTMENT = "max_cumulative_investment_per_node"
    MAX_INDIVIDUAL_RETIREMENT = "max_retirement_per_node_per_candidate"


class InvestmentVariableTypeEnum(Enum):
    DX_PLUS = "dx_plus"
    X = "x"
    DX_MINUS = "dx_minus"


class ConstraintOperatorEnum(Enum):
    LEQ = "<"
    EQ = "="
    GEQ = ">"


class TrajectoryModule:
    """
    Class containing the data of a tree of annual Xpansion studies.
    """

    def __init__(self, input_file: Path):
        self.input_file = input_file
        self.all_candidates: Set[str] = set()
        self.all_nodes: Set[str] = set()
        # TODO Add Optional
        self.tree: TrajectoryModule.Tree = None
        self.nodes: Dict[str, TrajectoryModule.NodeData] = {}
        self.global_data: TrajectoryModule.GlobalData = None
        self.candidates_types_costs: Dict[str, TrajectoryModule.CandidateType] = {}
        self.constraints: List[TrajectoryModule.TrajectoryConstraint] = []
        self.initial_capacities: Dict[str, NonNegativeInt] = {}

    # Errors
    class InvalidTreeStructure(Exception):
        pass

    class InvalidCandidates(Exception):
        pass

    class InvalidTrajectoryConstraint(Exception):
        pass

    # Data storage
    class GlobalData(BaseModel):
        discount_rate: NonNegativeFloat = Field(alias=InKeys.discount_rate_key())
        first_investment_date: NonNegativeInt = Field(
            alias=InKeys.first_investment_date_key()
        )
        end_of_horizon: NonNegativeInt = Field(alias=InKeys.end_of_horizon_key())
        studies: Dict[str, Path] = Field(alias=InKeys.studies_key())
        # forbid_retirement: bool = Field(alias=InKeys.forbid_retirement_key())

        def print(self):
            print("Global trajectory data : ")
            print(f" - Discount rate : {self.discount_rate}")
            print(f" - First investment year : {self.first_investment_date}")
            print(f" - End of horizon : {self.end_of_horizon}")
            print(f" - Study paths : {self.studies}")

    class Tree(BaseModel):
        node_name: str = Field(alias=InKeys.node_key())
        probability_from_parent: float = Field(
            1.0, alias=InKeys.probability_key(), ge=0.0, le=1.0
        )
        children: List["TrajectoryModule.Tree"] = Field([], alias=InKeys.children_key())

        def print(self, prefix=""):
            print(prefix + f"├─{self.probability_from_parent}─{self.node_name}")
            prefix_length = 4 + len(self.node_name) // 2
            for child in self.children:
                child.print(prefix + "|" + prefix_length * " ")

    class TrajectoryConstraint(BaseModel):
        name: str = Field(alias=InKeys.constraint_name_key())
        nodes: Literal["all"] | List[str] = Field(alias=InKeys.constraints_nodes_key())
        candidates: Literal["all"] | List[str] = Field(
            alias=InKeys.constraints_candidates_key()
        )
        cons_type: ConstraintTypeEnum = Field(alias=InKeys.constraint_type_key())
        rhs: float = Field(alias=InKeys.constraint_rhs_key())

        @staticmethod
        def build_variable_reference(
            node: str, candidate: str, variable_type: InvestmentVariableTypeEnum
        ):
            return f"{node}::{candidate}::{variable_type.value}"

        def to_individual_max_investment(self) -> List[Dict[str, Any]]:
            assert self.cons_type == ConstraintTypeEnum.MAX_INDIVIDUAL_INVESTMENT
            output: List[Dict[str, Any]] = []
            for node in self.nodes:
                for candidate in self.candidates:
                    constraint = {}
                    constraint[OutKeys.constraint_coeffs_key()] = {
                        self.build_variable_reference(
                            node, candidate, InvestmentVariableTypeEnum.DX_PLUS
                        ): 1
                    }
                    constraint[OutKeys.constraint_rhs_key()] = self.rhs
                    constraint[OutKeys.constraint_operator_key()] = (
                        ConstraintOperatorEnum.LEQ.value
                    )
                    output.append(constraint)
            return output

        def to_individual_max_retirement(self) -> List[Dict[str, Any]]:
            assert self.cons_type == ConstraintTypeEnum.MAX_INDIVIDUAL_RETIREMENT
            output: List[Dict[str, Any]] = []
            for node in self.nodes:
                for candidate in self.candidates:
                    constraint = {}
                    constraint[OutKeys.constraint_coeffs_key()] = {
                        self.build_variable_reference(
                            node, candidate, InvestmentVariableTypeEnum.DX_MINUS
                        ): 1
                    }
                    constraint[OutKeys.constraint_rhs_key()] = self.rhs
                    constraint[OutKeys.constraint_operator_key()] = (
                        ConstraintOperatorEnum.LEQ.value
                    )
                    output.append(constraint)
            return output

        def to_cumulative_max_investment(self) -> List[Dict[str, Any]]:
            assert self.cons_type == ConstraintTypeEnum.MAX_CUMULATIVE_INVESTMENT
            output: List[Dict[str, Any]] = []
            for node in self.nodes:
                constraint = {}
                constraint[OutKeys.constraint_coeffs_key()] = {}
                constraint[OutKeys.constraint_rhs_key()] = self.rhs
                constraint[OutKeys.constraint_operator_key()] = (
                    ConstraintOperatorEnum.LEQ.value
                )
                for candidate in self.candidates:
                    ref = self.build_variable_reference(
                        node, candidate, InvestmentVariableTypeEnum.DX_PLUS
                    )
                    constraint[OutKeys.constraint_coeffs_key()][ref] = 1.0
                output.append(constraint)
            return output

        def to_merger_json(self) -> List[Dict[str, Any]]:
            """
            Converts an constraint in input format to a list of mathematical formulations for the C++ merger
            """
            if self.cons_type == ConstraintTypeEnum.MAX_INDIVIDUAL_INVESTMENT:
                return self.to_individual_max_investment()
            elif self.cons_type == ConstraintTypeEnum.MAX_INDIVIDUAL_RETIREMENT:
                return self.to_individual_max_retirement()
            elif self.cons_type == ConstraintTypeEnum.MAX_CUMULATIVE_INVESTMENT:
                return self.to_cumulative_max_investment()
            else:
                raise TrajectoryModule.InvalidTrajectoryConstraint(
                    f"Non implemented constraint type was encountered for constraint {self.name} with type {self.cons_type.value}"
                )

    class NodeData(BaseModel):
        name: str = Field("")
        investment_date: NonNegativeInt = Field(alias=InKeys.investment_date_key())
        duration: PositiveInt = Field(1, alias=InKeys.duration_key())
        candidate_to_type: Dict[str, str] = Field(
            alias=InKeys.candidates_to_types_key()
        )
        path: Path = Field(Path(""))
        parent: str = Field("")
        full_probability: float = Field(1.0, ge=0.0, le=1.0)

        def print(self):
            print(f"NodeData {self.name}")
            print(f"Investment date : {self.investment_date}")
            print(f"Duration represented : {self.duration}")
            print(f"Study path : {self.path}")
            print(f"Parent : {self.parent}")
            print(f"Computed full probability : {self.full_probability}")

        def compute_investment_discounting(
            self, global_data: "TrajectoryModule.GlobalData"
        ):
            return (1 + global_data.discount_rate) ** (
                global_data.first_investment_date - self.investment_date
            )

        def compute_retirement_discounting(
            self, global_data: "TrajectoryModule.GlobalData"
        ):
            return (1 + global_data.discount_rate) ** (
                global_data.first_investment_date - self.investment_date
            )

        def compute_operational_discounting(
            self, global_data: "TrajectoryModule.GlobalData"
        ):
            factor = 0.0
            for year in range(
                self.investment_date, self.investment_date + self.duration
            ):
                factor += (1 + global_data.discount_rate) ** (
                    global_data.first_investment_date - year
                )
            return factor

        def to_merger_json(
            self,
            global_data: "TrajectoryModule.GlobalData",
            candidates_types: Dict[str, "TrajectoryModule.CandidateType"],
        ):
            output: Dict[str, Any] = {}
            output[OutKeys.parent_key()] = self.parent

            candidates_costs: Dict[str, Dict[str, float]] = {}
            weight_ic = self.compute_investment_discounting(global_data)
            weight_rc = self.compute_retirement_discounting(global_data)
            weight_omc = self.compute_operational_discounting(global_data)

            output[OutKeys.node_weight_key()] = self.full_probability * weight_omc

            for candidate, type_name in self.candidate_to_type.items():
                costs_data = candidates_types[type_name]
                candidate_costs: Dict[str, float] = {}
                candidate_costs[OutKeys.investment_cost_key()] = (
                    weight_ic * costs_data.investment_cost * self.full_probability
                )
                candidate_costs[OutKeys.retirement_cost_key()] = (
                    weight_rc * costs_data.retirement_cost * self.full_probability
                )
                candidate_costs[OutKeys.oandm_cost_key()] = (
                    weight_omc * costs_data.oam_cost * self.full_probability
                )
                candidates_costs[candidate] = candidate_costs

            output[OutKeys.candidate_costs()] = candidates_costs

            return output

    class CandidateType(BaseModel):
        investment_cost: NonNegativeFloat = Field(alias=InKeys.investment_cost_key())
        retirement_cost: NonNegativeFloat = Field(alias=InKeys.retirement_cost_key())
        oam_cost: NonNegativeFloat = Field(alias=InKeys.oandm_cost_key())

    class TrajectoryInputFile(BaseModel):
        global_data: "TrajectoryModule.GlobalData" = Field(alias=InKeys.global_key())
        tree: "TrajectoryModule.Tree" = Field(alias=InKeys.tree_key())
        constraints: List["TrajectoryModule.TrajectoryConstraint"] = Field(
            alias=InKeys.constraints_key()
        )
        nodes: Dict[str, "TrajectoryModule.NodeData"] = Field(alias=InKeys.nodes_key())
        candidates_types: Dict[str, "TrajectoryModule.CandidateType"] = Field(
            alias=InKeys.candidates_types_key()
        )
        initial_capacities: Dict[str, NonNegativeInt]

        def __init__(self, **kwargs):
            super().__init__(**kwargs)
            # Insert the default key in the initial capacities if not present
            if InKeys.default_initial_capacity_key() not in self.initial_capacities:
                print(
                    f"Inserted key '{InKeys.default_initial_capacity_key()}' with value '0' into the initial capacities"
                )
                self.initial_capacities[InKeys.default_initial_capacity_key()] = 0

    # Verifications
    def verify_tree_probabilities(self):
        def aux(subtree: TrajectoryModule.Tree):
            if len(subtree.children) == 0:
                return
            cumulative = 0.0
            for child in subtree.children:
                cumulative += child.probability_from_parent
            if abs(cumulative - 1) > 1e-6:
                raise TrajectoryModule.InvalidTreeStructure(
                    f"Sum of transition probabilities to children for node {subtree.node_name} is not 1 : got {cumulative}"
                )
            for child in subtree.children:
                aux(child)
            return

        aux(self.tree)
        return

    def verify_tree_investment_dates(self):
        assert self.tree is not None and self.nodes is not None
        depth_to_investment_date: Dict[int, NonNegativeInt] = {}

        def aux(subtree: TrajectoryModule.Tree, depth=0):
            # Check node's existence
            if subtree.node_name not in self.nodes:
                raise TrajectoryModule.InvalidTreeStructure(
                    f"Tree refers to node {subtree.node_name} which was not found in the"
                    f" '{InKeys.nodes_key()}' section of the input file"
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
            # Recursively check children
            for child in subtree.children:
                aux(child, depth + 1)
            return

        aux(self.tree, 0)  # TODO Typing
        return

    def verify_nodes_candidates_types(self):
        for name, data in self.nodes.items():
            for candidate, candidate_type in data.candidate_to_type.items():
                if candidate_type not in self.candidates_types_costs:
                    raise TrajectoryModule.InvalidCandidates(
                        f"NodeData '{name}''s candidate '{candidate}' has type '{candidate_type}'"
                        f" which is not found in the {InKeys.candidates_types_key()} section."
                    )

    def verify_constraint_variable_reference(self):
        """Verifies the the constraints reference existing variable."""
        for constraint in self.constraints:
            # NodeDatas
            for node in constraint.nodes:
                if node not in self.all_nodes:
                    raise self.InvalidTrajectoryConstraint(
                        f"TrajectoryConstraint '{constraint.name}' references node '{node}'"
                        " which does not exist in the study"
                    )
            # Candidates
            for candidate in constraint.candidates:
                if candidate not in self.all_candidates:
                    raise self.InvalidTrajectoryConstraint(
                        f"TrajectoryConstraint '{constraint.name}' references candidate '{candidate}'"
                        " which does not exist in the study"
                    )

    def verify_nodes_candidates_match_with_study(self):
        # TBA
        pass

    def verify_all_nodes_same_candidates(self):
        for name, data in self.nodes.items():
            node_candidates = set(data.candidate_to_type.keys())
            if node_candidates != self.all_candidates:
                diff_exceed = node_candidates - self.all_candidates
                diff_missing = self.all_candidates - node_candidates
                raise self.InvalidCandidates(
                    "All nodes must have the same exact candidates."
                    f" At node '{name}', missing candidates : {diff_missing}"
                    f", candidates not present elsewhere : {diff_exceed}"
                )
            else:
                pass

    def run_all_verification(self):
        self.verify_tree_investment_dates()
        self.verify_tree_probabilities()
        self.verify_constraint_variable_reference()
        self.verify_nodes_candidates_match_with_study()
        self.verify_nodes_candidates_types()
        self.verify_all_nodes_same_candidates()

    # Method
    def set_nodes_names_study_paths(self):
        """After parsing the raw node data, 'copy' the node's name and study path to its data for ease of access"""
        assert self.nodes is not None
        for name, data in self.nodes.items():
            data.name = name
            data.path = self.global_data.studies[name]  # TODO Typing

    def set_nodes_parents_names(self):
        """After parsing the tree and the nodes, go through the tree to write each node's parent in its data"""
        assert self.tree is not None and self.nodes is not None

        def aux_set_parent(subtree: TrajectoryModule.Tree, parent="root"):
            self.nodes[subtree.node_name].parent = parent
            for child in subtree.children:
                aux_set_parent(child, subtree.node_name)

        aux_set_parent(self.tree, "root")
        return

    def compute_node_full_probability(self):
        """After parsing the tree and the node's, add the node's full probability to its data"""
        assert self.tree is not None and self.nodes is not None

        def aux_compute_full_proba(subtree: TrajectoryModule.Tree, parent_proba=1.0):
            full_proba = parent_proba * subtree.probability_from_parent
            self.nodes[subtree.node_name].full_probability = full_proba
            for child in subtree.children:
                aux_compute_full_proba(child, full_proba)

        aux_compute_full_proba(self.tree)
        return

    def compute_node_duration(self):
        """After parsing the nodes and tree, compute the node's duration
        from the next investment date / end of horizon date"""
        assert (
            self.tree is not None
            and self.nodes is not None
            and self.global_data is not None
        )

        def aux_compute_node_represented_duration(subtree: TrajectoryModule.Tree):
            next_date: int = 0
            if subtree.children:
                child_investment_dates = [
                    self.nodes[child.node_name].investment_date
                    for child in subtree.children
                ]
                next_date = child_investment_dates[0]
                if not (all(x == next_date for x in child_investment_dates)):
                    raise TrajectoryModule.InvalidTreeStructure(
                        f"Invalid tree : children of node '{subtree.node_name}' do not all have the same investment date"
                    )
            else:
                next_date = self.global_data.end_of_horizon
            duration = next_date - self.nodes[subtree.node_name].investment_date
            if not (duration > 0):
                raise TrajectoryModule.InvalidTreeStructure(
                    f"Invalid tree : node '{subtree.node_name}' has duration {duration} which should be > 0"
                )
            self.nodes[subtree.node_name].duration = duration

            for child in subtree.children:
                aux_compute_node_represented_duration(child)

        aux_compute_node_represented_duration(self.tree)
        return

    def expand_all_keyword_in_constraints(self):
        for constraint in self.constraints:
            if constraint.candidates == InKeys.constraint_all_keyword():
                constraint.candidates = list(self.all_candidates)
            if constraint.nodes == InKeys.constraint_all_keyword():
                constraint.nodes = list(self.all_nodes)

    def parse_trajectory_user_file(self):
        """
        Parse the data contained in the user's input file
        """
        with open(self.input_file) as file:
            content = yaml.full_load(file)
            validated_content = self.TrajectoryInputFile.model_validate(content)
            # print(validated_content)

            self.global_data = validated_content.global_data
            self.tree = validated_content.tree
            self.nodes = validated_content.nodes
            self.constraints = validated_content.constraints
            self.candidates_types_costs = validated_content.candidates_types
            self.initial_capacities = validated_content.initial_capacities

            # Complete the node's data
            self.set_nodes_names_study_paths()
            self.set_nodes_parents_names()
            self.compute_node_full_probability()
            self.compute_node_duration()

            # Set of all candidates names (should be the same in all nodes, verified later)
            self.all_candidates = set(
                self.nodes[self.tree.node_name].candidate_to_type.keys()
            )
            self.all_nodes = set(self.nodes.keys())

            # Constraint explicit list of nodes and candidates
            self.expand_all_keyword_in_constraints()

    def print(self):
        """
        Prints a summary of the data in the console.
        """
        self.global_data.print()
        print("--------")
        self.tree.print("")
        print("--------")
        for _, node in self.nodes.items():
            node.print()
            print("---")

    def write_merger_json(self, output_file: Path):
        output = {}

        # Metadata
        output[OutKeys.metadata_key()] = {
            "written": datetime.datetime.now().strftime("%Y-%m-%d_%H:%M:%S")
        }

        # Initial capacities
        output[OutKeys.initial_capacities_key()] = self.initial_capacities

        # TrajectoryConstraints
        constraints_list = []
        for constraint in self.constraints:
            constraints_list.extend(constraint.to_merger_json())
        output[OutKeys.constraint_key()] = constraints_list

        # Tree
        nodes_output_dict = {}
        for name, data in self.nodes.items():
            nodes_output_dict[name] = data.to_merger_json(
                self.global_data, self.candidates_types_costs
            )
        output[OutKeys.tree_key()] = nodes_output_dict

        with open(output_file, "w") as file:
            json.dump(output, file, indent=4)
        pass
