import datetime
import json
from enum import Enum
from pathlib import Path
from typing import Any, Dict, List, Optional, Set, Union, Tuple

import yaml
from antares_xpansion.candidates_reader import CandidatesReader, IniFileNotFound
from antares_xpansion.trajectory.user_input_keys import TrajectoryInputKeys as InKeys
from antares_xpansion.trajectory.user_input_keys import TrajectoryOuputKeys as OutKeys
from pydantic import BaseModel, Field, NonNegativeFloat, NonNegativeInt, PositiveInt
from pydantic import parse_obj_as
from typing_extensions import Literal


# Enums
class ConstraintTypeEnum(Enum):
    MAX_INDIVIDUAL_INVESTMENT = "max_investment_per_node_per_candidate"
    MAX_CUMULATIVE_INVESTMENT = "max_cumulative_investment_per_node"
    MAX_INDIVIDUAL_RETIREMENT = "max_retirement_per_node_per_candidate"
    MAX_CUMULATIVE_RETIREMENT = "max_cumulative_retirement_per_node"
    MIN_INDIVIDUAL_INVESTMENT = "min_investment_per_node_per_candidate"
    MIN_INDIVIDUAL_RETIREMENT = "min_retirement_per_node_per_candidate"


class InvestmentVariableTypeEnum(Enum):
    DX_PLUS = "dx_plus"
    X = "x"
    DX_MINUS = "dx_minus"


class ConstraintOperatorEnum(Enum):
    LEQ = "<"
    EQ = "="
    GEQ = ">"


# Only used for checking the input, not strictly needed in this part of the workflow
class FormulationEnum(Enum):
    RELAXED = "relaxed"
    INTEGER = "integer"


# Data storage
class GlobalData(BaseModel):
    # Only used for checking the input, not strictly needed in this part of the workflow
    formulation: FormulationEnum = Field(alias=InKeys.formulation_key())
    studies: Dict[str, Path] = Field(alias=InKeys.studies_key())
    # Other data entries are necessary.
    discount_rate: NonNegativeFloat = Field(alias=InKeys.discount_rate_key())
    scaling: NonNegativeFloat = Field(alias=InKeys.scaling_key())
    first_investment_date: NonNegativeInt = Field(
        alias=InKeys.first_investment_date_key()
    )
    end_of_horizon: NonNegativeInt = Field(alias=InKeys.end_of_horizon_key())

    # forbid_retirement: bool = Field(alias=InKeys.forbid_retirement_key())
    def print(self):
        print("Global trajectory data : ")
        print(f" - Scaling factor  : {self.scaling}")
        print(f" - Discount rate : {self.discount_rate}")
        print(f" - First investment year : {self.first_investment_date}")
        print(f" - End of horizon : {self.end_of_horizon}")
        print(f" - Study paths : {self.studies}")


class CandidateType(BaseModel):
    investment_cost: NonNegativeFloat = Field(alias=InKeys.investment_cost_key())
    retirement_cost: NonNegativeFloat = Field(alias=InKeys.retirement_cost_key())
    oam_cost: NonNegativeFloat = Field(alias=InKeys.oandm_cost_key())


class Tree(BaseModel):
    node_name: str = Field(alias=InKeys.node_key())
    probability_from_parent: float = Field(
        1.0, alias=InKeys.probability_key(), ge=0.0, le=1.0
    )
    children: List["Tree"] = Field([], alias=InKeys.children_key())

    def print(self, prefix=""):
        print(prefix + f"|--{self.probability_from_parent}-{self.node_name}")
        prefix_length = 4 + len(self.node_name) // 2
        for child in self.children:
            child.print(prefix + "|" + prefix_length * " ")


class TrajectoryConstraint(BaseModel):
    name: str = Field(alias=InKeys.constraint_name_key())
    nodes: Union[Literal["all"], List[str]] = Field(
        alias=InKeys.constraints_nodes_key()
    )
    candidates: Union[Literal["all"], List[str]] = Field(
        alias=InKeys.constraints_candidates_key()
    )
    cons_type: ConstraintTypeEnum = Field(alias=InKeys.constraint_type_key())
    rhs: float = Field(alias=InKeys.constraint_rhs_key())

    @staticmethod
    def build_variable_reference(
            node: str, candidate: str, variable_type: InvestmentVariableTypeEnum
    ):
        return f"{node}::{candidate}::{variable_type.value}"

    def to_individual_max_investment(
            self, candidate_appear_in_nodes: Dict[str, Set[str]]
    ) -> List[Dict[str, Any]]:
        assert self.cons_type == ConstraintTypeEnum.MAX_INDIVIDUAL_INVESTMENT
        output: List[Dict[str, Any]] = []
        for node in self.nodes:
            for candidate in self.candidates:
                # Skip if the candidate does not exist in this node
                if node not in candidate_appear_in_nodes[candidate]:
                    continue
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

    def to_individual_max_retirement(
            self, candidate_appear_in_nodes: Dict[str, Set[str]]
    ) -> List[Dict[str, Any]]:
        assert self.cons_type == ConstraintTypeEnum.MAX_INDIVIDUAL_RETIREMENT
        output: List[Dict[str, Any]] = []
        for node in self.nodes:
            for candidate in self.candidates:
                # Skip if the candidate does not exist in this node
                if node not in candidate_appear_in_nodes[candidate]:
                    continue
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

    def to_cumulative_max_investment(
            self, candidate_appear_in_nodes: Dict[str, Set[str]]
    ) -> List[Dict[str, Any]]:
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
                # Skip if the candidate does not exist in this node
                if node not in candidate_appear_in_nodes[candidate]:
                    continue
                ref = self.build_variable_reference(
                    node, candidate, InvestmentVariableTypeEnum.DX_PLUS
                )
                constraint[OutKeys.constraint_coeffs_key()][ref] = 1.0
            output.append(constraint)
        return output

    def to_cumulative_max_retirement(
            self, candidate_appear_in_nodes: Dict[str, Set[str]]
    ) -> List[Dict[str, Any]]:
        assert self.cons_type == ConstraintTypeEnum.MAX_CUMULATIVE_RETIREMENT
        output: List[Dict[str, Any]] = []
        for node in self.nodes:
            constraint = {}
            constraint[OutKeys.constraint_coeffs_key()] = {}
            constraint[OutKeys.constraint_rhs_key()] = self.rhs
            constraint[OutKeys.constraint_operator_key()] = (
                ConstraintOperatorEnum.LEQ.value
            )
            for candidate in self.candidates:
                # Skip if the candidate does not exist in this node
                if node not in candidate_appear_in_nodes[candidate]:
                    continue
                ref = self.build_variable_reference(
                    node, candidate, InvestmentVariableTypeEnum.DX_MINUS
                )
                constraint[OutKeys.constraint_coeffs_key()][ref] = 1.0
            output.append(constraint)
        return output

    def to_individual_min_investment(
            self, candidate_appear_in_nodes: Dict[str, Set[str]]
    ) -> List[Dict[str, Any]]:
        assert self.cons_type == ConstraintTypeEnum.MIN_INDIVIDUAL_INVESTMENT
        output: List[Dict[str, Any]] = []
        for node in self.nodes:
            for candidate in self.candidates:
                # Skip if the candidate does not exist in this node
                if node not in candidate_appear_in_nodes[candidate]:
                    continue
                constraint = {
                    OutKeys.constraint_coeffs_key(): {
                        self.build_variable_reference(
                            node, candidate, InvestmentVariableTypeEnum.DX_PLUS
                        ): 1
                    },
                    OutKeys.constraint_rhs_key(): self.rhs,
                    OutKeys.constraint_operator_key(): ConstraintOperatorEnum.GEQ.value,
                }
                output.append(constraint)
        return output

    def to_individual_min_retirement(
            self, candidate_appear_in_nodes: Dict[str, Set[str]]
    ) -> List[Dict[str, Any]]:
        assert self.cons_type == ConstraintTypeEnum.MIN_INDIVIDUAL_RETIREMENT
        output: List[Dict[str, Any]] = []
        for node in self.nodes:
            for candidate in self.candidates:
                # Skip if the candidate does not exist in this node
                if node not in candidate_appear_in_nodes[candidate]:
                    continue
                constraint = {
                    OutKeys.constraint_coeffs_key(): {
                        self.build_variable_reference(
                            node, candidate, InvestmentVariableTypeEnum.DX_MINUS
                        ): 1
                    },
                    OutKeys.constraint_rhs_key(): self.rhs,
                    OutKeys.constraint_operator_key(): ConstraintOperatorEnum.GEQ.value,
                }
                output.append(constraint)
        return output

    def to_merger_json(
            self, candidate_appear_in_nodes: Dict[str, Set[str]]
    ) -> List[Dict[str, Any]]:
        """
        Converts an constraint in input format to a list of mathematical formulations for the C++ merger
        Skips the candidate when the candidate is not present in the node.
        """
        if self.cons_type == ConstraintTypeEnum.MAX_INDIVIDUAL_INVESTMENT:
            return self.to_individual_max_investment(candidate_appear_in_nodes)
        elif self.cons_type == ConstraintTypeEnum.MAX_INDIVIDUAL_RETIREMENT:
            return self.to_individual_max_retirement(candidate_appear_in_nodes)
        elif self.cons_type == ConstraintTypeEnum.MAX_CUMULATIVE_INVESTMENT:
            return self.to_cumulative_max_investment(candidate_appear_in_nodes)
        elif self.cons_type == ConstraintTypeEnum.MAX_CUMULATIVE_RETIREMENT:
            return self.to_cumulative_max_retirement(candidate_appear_in_nodes)
        elif self.cons_type == ConstraintTypeEnum.MIN_INDIVIDUAL_INVESTMENT:
            return self.to_individual_min_investment(candidate_appear_in_nodes)
        elif self.cons_type == ConstraintTypeEnum.MIN_INDIVIDUAL_RETIREMENT:
            return self.to_individual_min_retirement(candidate_appear_in_nodes)
        else:
            raise UserInputTranslator.InvalidTrajectoryConstraint(
                f"Non implemented constraint type was encountered for constraint {self.name} with type {self.cons_type.value}"
            )


class NodeData(BaseModel):
    name: str = Field("")
    investment_date: NonNegativeInt = Field(alias=InKeys.investment_date_key())
    duration: PositiveInt = Field(1, alias=InKeys.duration_key())
    candidate_to_type: Dict[str, str] = Field(alias=InKeys.candidates_to_types_key())
    path: Path = Field(Path(""))
    parent: str = Field("")
    full_probability: float = Field(1.0, ge=0.0, le=1.0)

    def print(self):
        print(f"NodeData {self.name}")
        print(f"Investment date : {self.investment_date}")
        print(f"Duration represented : {self.duration}")
        print(f"Parent : {self.parent}")
        print(f"Computed full probability : {self.full_probability}")

    def compute_investment_discounting(self, global_data: GlobalData):
        return (1 + global_data.discount_rate) ** (
                global_data.first_investment_date - self.investment_date
        )

    def compute_retirement_discounting(self, global_data: GlobalData):
        return (1 + global_data.discount_rate) ** (
                global_data.first_investment_date - self.investment_date
        )

    def compute_operational_discounting(self, global_data: GlobalData):
        factor = 0.0
        for year in range(self.investment_date, self.investment_date + self.duration):
            factor += (1 + global_data.discount_rate) ** (
                    global_data.first_investment_date - year
            )
        return factor

    def to_merger_json(
            self,
            global_data: GlobalData,
            candidates_types: Dict[str, CandidateType],
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


class TrajectoryInputFile(BaseModel):
    global_data: GlobalData = Field(alias=InKeys.global_key())
    tree: Tree = Field(alias=InKeys.tree_key())
    constraints: List[TrajectoryConstraint] = Field(alias=InKeys.constraints_key())
    nodes: Dict[str, NodeData] = Field(alias=InKeys.nodes_key())
    candidates_types: Dict[str, CandidateType] = Field(
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


class UserInputTranslator:
    """
    Parsing, verification and translation of  the user input data.
    """

    def __init__(self, input_file: Path):
        self.input_file = input_file
        self.all_candidates: Set[str] = set()
        # Which nodes does the candidate appear in ?
        self.candidate_appears_in_nodes: Dict[str, Set[str]] = dict()
        self.all_nodes: Set[str] = set()

        # User input file data.
        self.tree: Optional[Tree] = None
        self.nodes: Dict[str, NodeData] = {}
        self.global_data: Optional[GlobalData] = None
        self.candidates_types_costs: Dict[str, CandidateType] = {}
        self.constraints: List[TrajectoryConstraint] = []
        self.initial_capacities: Dict[str, NonNegativeInt] = {}

    # Errors
    class InvalidTreeStructure(Exception):
        pass

    class InvalidCandidates(Exception):
        pass

    class InvalidTrajectoryConstraint(Exception):
        pass

    # Verifications
    def verify_tree_probabilities(self):
        def aux(subtree: Tree):
            if len(subtree.children) == 0:
                return
            cumulative = 0.0
            for child in subtree.children:
                cumulative += child.probability_from_parent
            if abs(cumulative - 1) > 1e-6:
                raise UserInputTranslator.InvalidTreeStructure(
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

        def aux(subtree: Tree, depth=0):
            # Check node's existence
            if subtree.node_name not in self.nodes:
                raise UserInputTranslator.InvalidTreeStructure(
                    f"Tree refers to node {subtree.node_name} which was not found in the"
                    f" '{InKeys.nodes_key()}' section of the input file"
                )
            node_data = self.nodes[subtree.node_name]
            # Check investment_year is the same across all nodes at a given depth
            nonlocal depth_to_investment_date
            if depth not in depth_to_investment_date:
                depth_to_investment_date[depth] = node_data.investment_date
            elif depth_to_investment_date[depth] != node_data.investment_date:
                raise UserInputTranslator.InvalidTreeStructure(
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
                    raise UserInputTranslator.InvalidCandidates(
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
                        " which does not exist at the root node of the study"
                    )

    def verify_nodes_candidates_match_with_study(self):
        # TBA : verify that the candidates that appear in the yaml input file are present in the studies
        assert self.global_data is not None
        # For each node declared in the trajectory, check that the corresponding study
        # contains a candidates.ini listing the candidates used in the YAML.
        for node_name in self.all_nodes:
            # Get the study path declared in the global 'studies' mapping
            study_path = self.global_data.studies.get(node_name)
            if not study_path:
                raise UserInputTranslator.InvalidTreeStructure(
                    f"No study path provided for node '{node_name}' in the global '{InKeys.studies_key()}' mapping"
                )

            # The candidates.ini is expected under user/expansion/candidates.ini in the study
            candidates_ini = Path(study_path) / "user" / "expansion" / "candidates.ini"
            if not candidates_ini.is_file():
                raise UserInputTranslator.InvalidTreeStructure(
                    f"Study for node '{node_name}' should contain file '{candidates_ini}', but it was not found"
                )

            # Load candidates from the study using CandidatesReader
            try:
                reader = CandidatesReader(candidates_ini)
            except IniFileNotFound:
                raise UserInputTranslator.InvalidTreeStructure(
                    f"Unable to read candidates file '{candidates_ini}' for node '{node_name}'"
                )

            study_candidates = set(reader.get_candidates_list())

            # Candidates declared in the node data
            node_candidates = set(self.nodes[node_name].candidate_to_type.keys())

            # Compute missing candidates (declared in YAML but not present in study)
            missing = sorted(list(node_candidates - study_candidates))
            extra = sorted(list(study_candidates - node_candidates))
            if missing or extra:
                parts = []
                if missing:
                    parts.append(f"missing in study: {missing}")
                if extra:
                    parts.append(f"present in study but not in {self.input_file}: {extra}")
                parts_str = "; ".join(parts)
                raise UserInputTranslator.InvalidCandidates(
                    f"Candidate mismatch for node '{node_name}' in study '{study_path}' ({candidates_ini}): {parts_str}.\n\t"
                    f"Study contains: {sorted(list(study_candidates))}; {self.input_file} declares: {sorted(list(node_candidates))}"
                )
        return

    def verify_candidates_span_continuous_subtree(self):
        """
        A candidate must appear in a continuous bit of the trajecotry tree : can only appear and eventually disappear once.
        """

        def aux_candidate_only_appear(
                subtree: Tree,
                candidate: str,
                is_in_parent: bool,
                has_already_disappeared: bool,
        ):
            node = subtree.node_name
            is_present = node in self.candidate_appears_in_nodes[candidate]
            # Error if the candidate disappeared but is present in this node
            if has_already_disappeared and is_present:
                raise self.InvalidCandidates(
                    f"Candidate {candidate} is present in nodes : {self.candidate_appears_in_nodes[candidate]}, \
                      this does not represent a continuous subtree"
                )

            disappeared = False
            if is_in_parent and not is_present:
                disappeared = True

            for child in subtree.children:
                aux_candidate_only_appear(child, candidate, is_present, disappeared)

        for candidate in self.all_candidates:
            aux_candidate_only_appear(self.tree, candidate, False, False)

    def run_all_verification(self):
        self.verify_tree_investment_dates()
        self.verify_tree_probabilities()
        self.verify_constraint_variable_reference()
        self.verify_nodes_candidates_match_with_study()
        self.verify_nodes_candidates_types()
        self.verify_candidates_span_continuous_subtree()

    def set_nodes_parents_names(self):
        """After parsing the tree and the nodes, go through the tree to write each node's parent in its data"""
        assert self.tree is not None and self.nodes is not None

        def aux_set_parent(subtree: Tree, parent="root"):
            self.nodes[subtree.node_name].parent = parent
            for child in subtree.children:
                aux_set_parent(child, subtree.node_name)

        aux_set_parent(self.tree, "root")
        return

    def compute_node_full_probability(self):
        """After parsing the tree and the node's, add the node's full probability to its data"""
        assert self.tree is not None and self.nodes is not None

        def aux_compute_full_proba(subtree: Tree, parent_proba=1.0):
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

        def aux_compute_node_represented_duration(subtree: Tree):
            next_date: int = 0
            if subtree.children:
                child_investment_dates = [
                    self.nodes[child.node_name].investment_date
                    for child in subtree.children
                ]
                next_date = child_investment_dates[0]
                if not (all(x == next_date for x in child_investment_dates)):
                    raise UserInputTranslator.InvalidTreeStructure(
                        f"Invalid tree : children of node '{subtree.node_name}' do not all have the same investment date"
                    )
            else:
                next_date = self.global_data.end_of_horizon
            duration = next_date - self.nodes[subtree.node_name].investment_date
            if not (duration > 0):
                raise UserInputTranslator.InvalidTreeStructure(
                    f"Invalid tree : node '{subtree.node_name}' has duration {duration} which should be > 0"
                )
            self.nodes[subtree.node_name].duration = duration

            for child in subtree.children:
                aux_compute_node_represented_duration(child)

        aux_compute_node_represented_duration(self.tree)
        return

    def set_all_candidates(self):
        """Sets all candidates dicts and list from the parsed data."""
        for node in self.all_nodes:
            candidates_this_node = self.nodes[node].candidate_to_type.keys()
            self.all_candidates = self.all_candidates.union(candidates_this_node)

            for candidate in candidates_this_node:
                if candidate not in self.candidate_appears_in_nodes.keys():
                    self.candidate_appears_in_nodes[candidate] = set()
                self.candidate_appears_in_nodes[candidate].add(node)

    def expand_all_keyword_in_constraints(self):
        for constraint in self.constraints:
            if constraint.candidates == InKeys.constraint_all_keyword():
                constraint.candidates = list(self.all_candidates)
            if constraint.nodes == InKeys.constraint_all_keyword():
                constraint.nodes = list(self.all_nodes)

    def detect_fully_decommed_candidates(self):
        """
        Returns a list of pairs (candidate, node_where_candidate_disappears)
        When a candidate disappears, we consider it fully decommissioned
        """
        # Compute a list of pairs of (candidate, node_where_candidate_disappears)
        disappear_cand_node: List[Tuple[str, str]] = []

        def aux_get_disappearing_cand(subtree: Tree, candidate, is_present_in_parent):
            node = subtree.node_name
            is_present = node in self.candidate_appears_in_nodes[candidate]
            if is_present_in_parent and not is_present:
                disappear_cand_node.append((candidate, node))
            else:
                for child in subtree.children:
                    aux_get_disappearing_cand(child, candidate, is_present)

        assert self.all_candidates
        for candidate in self.all_candidates:
            aux_get_disappearing_cand(self.tree, candidate, False)

        return disappear_cand_node

    def add_delta_costs_fully_decommed_candidate(self, master_merger_info: dict):
        """
        Given an already generated output master_merger_info dict,
        We need to manually add the costs of the delta variables for the node
        where a candidate is fully decommissioned and disappears from the study.
        """
        fully_decommed_cand_node = self.detect_fully_decommed_candidates()

        for cand, node in fully_decommed_cand_node:
            # Get the costs from the parent
            current_node_data = self.nodes.get(node)
            parent_data = self.nodes.get(current_node_data.parent)
            cand_type = parent_data.candidate_to_type.get(cand)
            costs_data = self.candidates_types_costs.get(cand_type)
            # Discouting
            disc_dxplus = current_node_data.compute_investment_discounting(
                self.global_data
            )
            disc_dxminus = current_node_data.compute_retirement_discounting(
                self.global_data
            )

            dxplus_cost = (
                    disc_dxplus
                    * costs_data.investment_cost
                    * current_node_data.full_probability
            )
            dxminus_cost = (
                    disc_dxminus
                    * costs_data.retirement_cost
                    * current_node_data.full_probability
            )

            master_merger_info[OutKeys.tree_key()][node][OutKeys.candidate_costs()][
                cand
            ] = {
                OutKeys.investment_cost_key(): dxplus_cost,
                OutKeys.retirement_cost_key(): dxminus_cost,
            }

        return master_merger_info

    def parse_trajectory_user_file(self):
        """
        Parse the data contained in the user's input file
        """
        with open(self.input_file, encoding="utf-8") as file:
            content = yaml.full_load(file)
            validated_content = parse_obj_as(TrajectoryInputFile, content)
            # print(validated_content)

            self.global_data = validated_content.global_data
            self.tree = validated_content.tree
            self.nodes = validated_content.nodes
            self.constraints = validated_content.constraints
            self.candidates_types_costs = validated_content.candidates_types
            self.initial_capacities = validated_content.initial_capacities

            # Complete the node's data
            # self.set_nodes_names_study_paths()
            self.set_nodes_parents_names()
            self.compute_node_full_probability()
            self.compute_node_duration()

            # Set of all candidates names (should be the same in all nodes, verified later)
            self.all_nodes = set(self.nodes.keys())
            self.set_all_candidates()

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

        # Scaling is transferred as is (because it's needed both in MergeMasterMPS and MergeWeights)
        output[OutKeys.scaling_key()] = self.global_data.scaling

        # Initial capacities
        output[OutKeys.initial_capacities_key()] = self.initial_capacities

        # TrajectoryConstraints
        constraints_list = []
        for constraint in self.constraints:
            constraints_list.extend(
                constraint.to_merger_json(self.candidate_appears_in_nodes)
            )
        output[OutKeys.constraint_key()] = constraints_list

        # Tree
        nodes_output_dict = {}
        for name, data in self.nodes.items():
            nodes_output_dict[name] = data.to_merger_json(
                self.global_data, self.candidates_types_costs
            )
        output[OutKeys.tree_key()] = nodes_output_dict

        output = self.add_delta_costs_fully_decommed_candidate(output)

        with open(output_file, "w") as file:
            json.dump(output, file, indent=4)
        pass

    def get_root_study(self):
        assert self.tree is not None and self.global_data is not None
        root_node = self.tree.node_name
        return self.global_data.studies.get(root_node, Path(""))
