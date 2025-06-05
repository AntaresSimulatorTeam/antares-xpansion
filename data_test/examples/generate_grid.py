import configparser
import json
import os
import random
from typing import Dict

import numpy as np
import pandas as pd

ALL_STUDIES_PATH = "data_test/examples"
EXCLUDED_STUDIES = [
    "xpansion-test-05-area-uppercase",
    "dummy-3areas-3candidates-3links",
]
OPTIMAL_VALUES = {
    f"{ALL_STUDIES_PATH}/xpansion-test-02": {
        "optimality_gap": 0,
        "investment_cost": 289923159,
        "operational_cost": 1065480665.4781473,
        "overall_cost": 1355403824.589329,
        "relative_gap": 0,
        "accepted_rel_gap_atol": 1e-10,
        "battery": 5.66e02,
        "peak1": 6.0e02,
        "peak2": 1.0e03,
        "pv": 4.4267733994e02,
        "semibase1": 6.0e02,
        "semibase2": 0,
    },
    f"{ALL_STUDIES_PATH}/xpansion-test-02-new": {
        "optimality_gap": 709.42435598373413,
        "investment_cost": 256299462.08039832,
        "operational_cost": 1067318031.6309935,
        "overall_cost": 1323617493.7113919,
        "relative_gap": 5.3597384391960929e-07,
        "accepted_rel_gap_atol": 5e-7,
        "battery": 508.74183466608417,
        "peak1": 800.0,
        "peak2": 1000.0,
        "pv": 447.28156522682048,
        "semibase1": 0.0,
        "semibase2": 200.0,
    },
    f"{ALL_STUDIES_PATH}/different_NTCs": {
        "optimality_gap": 709.42435598373413,
        "investment_cost": 256299462.08039832,
        "operational_cost": 1067318031.6309935,
        "overall_cost": 1323617493.7113919,
        "relative_gap": 5.3597384391960929e-07,
        "accepted_rel_gap_atol": 5e-7,
        "battery": 508.74183466608417,
        "peak1": 800.0,
        "peak2": 1000.0,
        "pv": 447.28156522682048,
        "semibase1": 0.0,
        "semibase2": 200.0,
    },
    f"{ALL_STUDIES_PATH}/xpansion-test-01": {
        "optimality_gap": 0,
        "investment_cost": 224600000.00003052,
        "operational_cost": 22513655727.899261,
        "overall_cost": 22738255727.899292,
        "relative_gap": 0,
        "accepted_rel_gap_atol": 1e-10,
        "battery": 1.0e03,
        "peak": 1.4e03,
        "pv": 1.0e03,
        "semibase": 2.0e02,
        "transmission_line": 0.0,
    },
    f"{ALL_STUDIES_PATH}/xpansion-test-03": {
        "optimality_gap": 0,
        "investment_cost": 201999999.99999976,
        "operational_cost": 1161894495.7433052,
        "overall_cost": 1363894495.743305,
        "relative_gap": 0,
        "accepted_rel_gap_atol": 1e-10,
        "peak": 2500.0,
        "transmission_line": 1600.0,
        "semibase": 400.0,
    },
    f"{ALL_STUDIES_PATH}/xpansion-test-04-mps-rounding": {
        "optimality_gap": 0,
        "investment_cost": 115399999.99999046,
        "operational_cost": 21944788078.597385,
        "overall_cost": 22060188078.597374,
        "relative_gap": 0,
        "accepted_rel_gap_atol": 1e-10,
        "battery": 1000.0000000000124,
        "peak": 0.0,
        "pv": 1000.0,
        "semibase": 0.0,
        "transmission_line": 0.0,
    },
    f"{ALL_STUDIES_PATH}/xpansion-test-01-weights": {
        "optimality_gap": 0,
        "investment_cost": 230600000.00001144,
        "operational_cost": 24001577891.450439,
        "overall_cost": 24232177891.450451,
        "relative_gap": 0,
        "accepted_rel_gap_atol": 1e-10,
        "battery": 1000.0,
        "peak": 1500.0,
        "pv": 1000.0,
        "semibase": 200.0,
        "transmission_line": 0.0,
    },
    f"{ALL_STUDIES_PATH}/xpansion-test-01-hurdles-cost": {
        "optimality_gap": 0,
        "investment_cost": 224599999.99996948,
        "operational_cost": 22516674015.060184,
        "overall_cost": 22741274015.060154,
        "relative_gap": 0,
        "accepted_rel_gap_atol": 1e-10,
        "battery": 1.0e03,
        "peak": 1.4e03,
        "pv": 1.0e03,
        "semibase": 2.0e02,
        "transmission_line": 0.0,
    },
    f"{ALL_STUDIES_PATH}/additionnal-constraints": {
        "optimality_gap": 0,
        "investment_cost": 224600000.00003052,
        "operational_cost": 22513655727.899261,
        "overall_cost": 22738255727.899292,
        "relative_gap": 0,
        "accepted_rel_gap_atol": 1e-10,
        "battery": 1.0e03,
        "peak": 1.4e03,
        "pv": 1.0e03,
        "semibase": 2.0e02,
        "transmission_line": 0.0,
    },
    f"{ALL_STUDIES_PATH}/additionnal-constraints-binary": {
        "optimality_gap": 0,
        "investment_cost": 220499999.99999809,
        "operational_cost": 13501512886.702877,
        "overall_cost": 13722012886.702875,
        "relative_gap": 0,
        "accepted_rel_gap_atol": 1e-10,
        "battery": 885,
        "peak": 1800,
        "pv": 1.0e03,
        "semibase": 0.0,
        "transmission_line": 400.0,
    },
    f"{ALL_STUDIES_PATH}/hurdles-cost-profile-value-over-one": {
        "optimality_gap": 90577,
        "investment_cost": 224000000,
        "operational_cost": 1097431246.5454886,
        "overall_cost": 1321431246.5454886,
        "relative_gap": 6.8545027035272501e-05,
        "accepted_rel_gap_atol": 1e-7,
        "peak": 1500.0,
        "semibase": 1400.0,
        "transmission_line": 800.0,
    },
    f"{ALL_STUDIES_PATH}/link-profile-with-empty-week": {
        "optimality_gap": 0,
        "investment_cost": 27585000000.000004,
        "operational_cost": 10629896636.069275,
        "overall_cost": 38214896636.069275,
        "relative_gap": 0,
        "accepted_rel_gap_atol": 1e-10,
        "base": 51150.0,
        "pointe": 33500,
        "semibase_winter": 0.0,
    },
    f"{ALL_STUDIES_PATH}/empty-link-profile": {
        "optimality_gap": 0,
        "investment_cost": 27584999999.999992,
        "operational_cost": 10629896636.069145,
        "overall_cost": 38214896636.069138,
        "relative_gap": 0,
        "accepted_rel_gap_atol": 1e-10,
        "base": 51150.0,
        "pointe": 33500,
        "semibase_empty": 0.0,
    },
    f"{ALL_STUDIES_PATH}/xpansion-test-one-link-two-candidates": {
        "optimality_gap": 0,
        "investment_cost": 15999999.999994278,
        "operational_cost": 11800333780.786646,
        "overall_cost": 11816333780.78664,
        "relative_gap": 0,
        "accepted_rel_gap_atol": 1e-10,
        "transmission_line": 800.0,
        "transmission_line_2": 800.0,
    },
}


def parse_candidates_ini(file_path: str) -> Dict[str, Dict]:
    config = configparser.ConfigParser()
    config.read(file_path)

    candidates = {}
    for section in config.sections():
        candidates[section] = {}
        for key, value in config.items(section):
            candidates[section][key] = value

    return candidates


def genarate_random_grid_points(
    nb_points: int, candidates: Dict[str, Dict]
) -> pd.DataFrame:
    np.random.seed(0)
    df_points = pd.DataFrame()
    for candidate in candidates.values():
        if "max-investment" in candidate.keys():
            max_invest = float(candidate["max-investment"])
        else:
            max_invest = float(candidate["max-units"]) * float(candidate["unit-size"])
        df_points[candidate["name"]] = (max_invest * np.random.random(nb_points)).round(
            2
        )
    return df_points


def write_grid(output_file, df_points):
    df_points.to_csv(output_file)


def add_optimal_value(df_points: pd.DataFrame, study_path: str) -> pd.DataFrame:
    study_optimal_data = OPTIMAL_VALUES[study_path]
    optimal_grid_point = pd.DataFrame.from_dict(
        {candidate: [study_optimal_data[candidate]] for candidate in df_points.columns}
    )
    df_points = pd.concat([df_points, optimal_grid_point], ignore_index=True)
    # Suffle rows of the data frame, not to have optimal solution always at the end
    df_points = df_points.sample(frac=1, random_state=1, ignore_index=True)
    return df_points


def generate_grid_data_for_test(dir: str):
    random.seed(42)
    for item in os.listdir(dir):
        study_path = os.path.join(dir, item)
        if os.path.isdir(study_path) and item not in EXCLUDED_STUDIES:
            candidates_path = f"{study_path}/user/expansion/candidates.ini"
            grid_output = f"{study_path}/user/expansion/grid.csv"
            optimal_values_output = f"{study_path}/user/expansion/optimal.json"
            candidates = parse_candidates_ini(candidates_path)
            df_points = genarate_random_grid_points(random.randint(5, 30), candidates)
            df_points = add_optimal_value(df_points, study_path)
            write_grid(grid_output, df_points)
            write_optimal_values(optimal_values_output, study_path)


def write_optimal_values(output_file: str, study_path: str) -> None:
    keys_to_delete = ["relative_gap", "accepted_rel_gap_atol", "optimality_gap"]
    for key in keys_to_delete:
        OPTIMAL_VALUES[study_path].pop(key, None)
    with open(output_file, "w") as file:
        file.write(json.dumps(OPTIMAL_VALUES[study_path], indent=4))


generate_grid_data_for_test(ALL_STUDIES_PATH)

# # Example usage
# file_path = "/home/bittartho/development/antares-xpansion/data_test/examples/xpansion-test-02/user/expansion/candidates.ini"
# output_file = "/home/bittartho/development/antares-xpansion/data_test/examples/xpansion-test-02/user/expansion/grid.csv"
# candidates = parse_candidates_ini(file_path)
# print(candidates)
# genarate_random_grid_points(10, candidates, output_file)
