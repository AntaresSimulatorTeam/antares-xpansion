class TrajectoryInputKeys:
    """
    Keys used in the `user_input_XpansionTrajectory.yaml` file
    """

    # Global data
    @staticmethod
    def global_key():
        return "global"

    @staticmethod
    def discount_rate_key():
        return "discount_rate"

    @staticmethod
    def first_investment_date_key():
        return "first_investment_year"

    @staticmethod
    def studies_key():
        return "studies"

    @staticmethod
    def initial_capacities_key():
        return "initial_capacities"

    @staticmethod
    def default_initial_capacity_key():
        return "default"

    # Tree information
    @staticmethod
    def tree_key():
        return "tree"

    @staticmethod
    def node_key():
        return "node"

    @staticmethod
    def probability_key():
        return "probability"

    @staticmethod
    def children_key():
        return "children"

    # Specific node information
    @staticmethod
    def nodes_key():
        return "nodes"

    @staticmethod
    def investment_date_key():
        return "investment_date"

    @staticmethod
    def duration_key():
        return "duration"

    @staticmethod
    def candidates_to_types_key():
        return "candidate_to_type"

    # Candidates costs types
    @staticmethod
    def candidates_types_key():
        return "candidates_types"

    @staticmethod
    def investment_cost_key():
        return "investment"

    @staticmethod
    def oandm_cost_key():
        return "operation_maintenance"

    @staticmethod
    def retirement_cost_key():
        return "retirement"

    # Constraints
    @staticmethod
    def constraints_key():
        return "constraints"

    @staticmethod
    def constraint_name_key():
        return "name"

    @staticmethod
    def constraints_nodes_key():
        return "nodes"

    @staticmethod
    def constraints_candidates_key():
        return "candidates"

    @staticmethod
    def constraint_type_key():
        return "type"

    @staticmethod
    def constraint_rhs_key():
        return "value"

    @staticmethod
    def constraint_all_keyword():
        return "all"


class TrajectoryOuputKeys:
    """
    Keys used in the `master_structure.json` file passed to the C++ executable.
    """

    # Initial capacities
    @staticmethod
    def initial_capacities_key():
        return "initial_capacities"

    # Constraints
    @staticmethod
    def constraint_key():
        return "constraints"

    @staticmethod
    def constraint_coeffs_key():
        return "coeffs"

    @staticmethod
    def constraint_rhs_key():
        return "rhs"

    @staticmethod
    def constraint_operator_key():
        return "operator"

    # Tree
    @staticmethod
    def tree_key():
        return "tree"

    @staticmethod
    def lp_folder_key():
        return "lp_folder"

    @staticmethod
    def master_mps_key():
        return "master_mps_file"

    @staticmethod
    def structure_file_key():
        return "structure_file"

    @staticmethod
    def parent_key():
        return "parent"

    # Node's candidates costs
    @staticmethod
    def candidate_costs():
        return "candidates_costs"

    @staticmethod
    def investment_cost_key():
        return "investment"

    @staticmethod
    def oandm_cost_key():
        return "operation_maintenance"

    @staticmethod
    def retirement_cost_key():
        return "retirement"
    
    # Metadata
    @staticmethod
    def metadata_key():
        return "metadata"

