import json

import pytest
import yaml

from antares_xpansion.trajectory.user_input_translation import (
    UserInputTranslator,
)


def _write_candidates_ini(study_path):
    expansion_dir = study_path / "user" / "expansion"
    expansion_dir.mkdir(parents=True)
    (expansion_dir / "candidates.ini").write_text(
        "[1]\n"
        "name = cand1\n"
        "link = area1 - area2\n"
    )


class TestTrajectoryInputTranslationEndToEnd:
    """
    Test that the discounted costs written to master_merger_info.json by
    UserInputTranslator match the expected discounting of the user's raw costs.
    """

    def test_write_merger_json_discounts_costs_from_user_input(self, tmp_path):
        discount_rate = 0.05
        first_investment_year = 2020
        end_of_horizon = 2025

        # investment_cost, retirement_cost, oam_cost as entered by the user (undiscounted)
        investment_cost = 1000.0
        retirement_cost = 200.0
        oam_cost = 50.0

        # Two studies : root node (investing at the reference date) and a child node
        # investing two years later, each with a single candidate "cand1".
        root_study = tmp_path / "root_study"
        child_study = tmp_path / "child_study"
        for study_path in (root_study, child_study):
            study_path.mkdir()
            _write_candidates_ini(study_path)

        input_data = {
            "global": {
                "formulation": "relaxed",
                "discount_rate": discount_rate,
                "first_investment_year": first_investment_year,
                "end_of_horizon": end_of_horizon,
                "scaling": 1,
                "studies": {
                    "root": str(root_study),
                    "child": str(child_study),
                },
            },
            "tree": {
                "node": "root",
                "children": [{"node": "child", "probability": 1.0}],
            },
            "constraints": [],
            "nodes": {
                "root": {
                    "investment_date": first_investment_year,
                    "candidate_to_type": {"cand1": "type1"},
                },
                "child": {
                    "investment_date": first_investment_year + 2,
                    "candidate_to_type": {"cand1": "type1"},
                },
            },
            "candidates_types": {
                "type1": {
                    "investment": investment_cost,
                    "operation_maintenance": oam_cost,
                    "retirement": retirement_cost,
                }
            },
            "initial_capacities": {"default": 0},
        }

        input_file = tmp_path / "input-trajectory.yaml"
        input_file.write_text(yaml.safe_dump(input_data))

        translator = UserInputTranslator(input_file)
        translator.parse_trajectory_user_file()
        translator.run_all_verification()

        output_file = tmp_path / "master_merger_info.json"
        translator.write_merger_json(output_file)

        output = json.loads(output_file.read_text())
        tree = output["tree"]

        # Root node invests exactly at the reference date : discounting factors of 1
        # for investment/retirement, and a sum over its 2-year duration for O&M.
        root_costs = tree["root"]["candidates_costs"]["cand1"]
        assert root_costs["investment"] == pytest.approx(investment_cost)
        assert root_costs["retirement"] == pytest.approx(retirement_cost)
        expected_root_omc_factor = sum(
            (1 + discount_rate) ** (first_investment_year - year)
            for year in range(first_investment_year + 1, first_investment_year + 2 + 1)
        )
        assert root_costs["operation_maintenance"] == pytest.approx(
            expected_root_omc_factor * oam_cost
        )

        # Child node invests 2 years after the reference date : investment/retirement
        # are discounted by (1+r)^-2, and O&M is summed over its remaining duration.
        child_investment_date = first_investment_year + 2
        child_costs = tree["child"]["candidates_costs"]["cand1"]
        expected_ic_factor = (1 + discount_rate) ** (
            first_investment_year - child_investment_date
        )
        assert child_costs["investment"] == pytest.approx(expected_ic_factor * investment_cost)
        assert child_costs["retirement"] == pytest.approx(expected_ic_factor * retirement_cost)
        expected_child_omc_factor = sum(
            (1 + discount_rate) ** (first_investment_year - year)
            for year in range(child_investment_date + 1, end_of_horizon + 1)
        )
        assert child_costs["operation_maintenance"] == pytest.approx(
            expected_child_omc_factor * oam_cost
        )

        assert tree["child"]["parent"] == "root"
