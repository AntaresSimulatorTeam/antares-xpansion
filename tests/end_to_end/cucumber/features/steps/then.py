import csv
import glob
import io
import math
import os
import json

import numpy as np
import xpress as xp
from behave import *
from utils_functions import find_in_simulator_log, output_path

# Cache global pour les statistiques des sous-problèmes
_subproblem_stats_cache = {}


class SubProblemStats:
    def __init__(self, name: str, rows=0, cols=0, elements=0):
        self.name = name
        self.rows = rows
        self.cols = cols
        self.elements = elements

    def __repr__(self):
        return f"SubProblemStats(name={self.name}, rows={self.rows}, cols={self.cols}, elements={self.elements})"


def get_subproblem_statistics(lp_path) -> list:
    """Get statistics for all subproblems in the lp directory with caching"""
    # Créer une clé de cache basée sur le chemin et la date de modification du répertoire
    cache_key = _create_cache_key(lp_path)

    # Vérifier si les résultats sont déjà en cache
    if cache_key in _subproblem_stats_cache:
        return _subproblem_stats_cache[cache_key]

    # Calculer les statistiques si pas en cache
    mps_files = glob.glob(str(lp_path / "**/*.mps"), recursive=True)

    if not mps_files:
        result = []
        _subproblem_stats_cache[cache_key] = result
        return []

    result = []

    for mps_file in mps_files:
        if "master" not in mps_file.lower():  # Skip master problem files
            result.append(analyze_mps_file(mps_file))

    # Stocker en cache
    _subproblem_stats_cache[cache_key] = result
    return result


def _create_cache_key(lp_path):
    """Create a cache key based on path and modification times of MPS files"""
    try:
        # Obtenir tous les fichiers MPS
        mps_files = glob.glob(str(lp_path / "**/*.mps"), recursive=True)

        if not mps_files:
            return str(lp_path)

        # Utiliser le chemin et la somme des dates de modification
        mtime_sum = sum(os.path.getmtime(f) for f in mps_files)
        return f"{lp_path}_{len(mps_files)}_{mtime_sum}"
    except (OSError, IOError):
        # En cas d'erreur, utiliser juste le chemin
        return str(lp_path)


def clear_subproblem_stats_cache():
    """Clear the subproblem statistics cache"""
    global _subproblem_stats_cache
    _subproblem_stats_cache.clear()


def analyze_mps_file(mps_file_path):
    """Analyze an MPS file to count rows, columns, and elements"""
    rows = 0
    cols = 0
    elements = 0

    pb = xp.problem(mps_file_path)
    pb.read(mps_file_path)
    rows, cols, elements = pb.getAttrib('rows', 'cols', 'elems').values()

    return SubProblemStats(mps_file_path, rows, cols, elements)


def assert_dict_allclose(actual, expected, rtol=1e-06, atol=0):
    for key in expected:
        np.testing.assert_allclose(
            actual[key],
            expected[key],
            rtol=rtol,
            atol=atol,
            err_msg=f"Mismatch found at key '{key}'"
        )


def read_cucumber_table_from_file(filename):
    with open(filename, 'r') as file:
        reader = csv.reader(file, delimiter='\t')
        header = [item.strip() for item in
                  next(reader)[1:-1]]  # Store the header row, ignoring the first and last columns
        current_results = [{header[index]: item.strip() for index, item in enumerate(row[1:-1])} for row in
                           reader]
        return current_results


def is_file_full_of_zeros(filename, abs_tol=1e-9):
    data = read_cucumber_table_from_file(filename)

    for line_number, line in enumerate(data):

        for key, value in line.items():
            if key in ["Outer loop", "Ite"]:
                continue
            try:
                value = float(value)
            except (ValueError, IndexError):
                print(f"Error parsing line: {line_number} at column {key}")
                return False

            # Use math.isclose to compare to zero with tolerance
            if not math.isclose(value, 0.0, abs_tol=abs_tol):
                print(f"Error {value} is not close to 0")
                return False

    return True


def read_table_from_string(raw_data):
    reader = csv.reader(io.StringIO(raw_data), delimiter='\t')
    header = [item.strip() for item in
              next(reader)[1:-1]]  # Store the header row, ignoring the first and last columns
    current_results = [{header[index]: item.strip() for index, item in enumerate(row[1:-1])} for row in
                       reader]

    return current_results


def check_cucumber_table(context, results):
    headers = context.table.headings
    for i, row in enumerate(context.table):
        for header in headers:
            expected_value = float(row[header])
            actual_value = float(results[i][header])

            np.testing.assert_allclose(actual_value, expected_value, rtol=1e-6, atol=1e-6,
                                       err_msg=f"Mismatch in row {i + 1}, column '{header}': expected {expected_value}, got {actual_value}")


@then(u'the return status is 0')
def check_return_status(context):
    assert context.return_code == 0, f"Expected return code 0, got {context.return_code}"


@then(u'the generated subproblems have between {min} and {max} rows')
def check_subproblems_rows(context, min, max):
    lp_path = output_path(context.tmp_study / "output") / "lp"
    stats = get_subproblem_statistics(lp_path)
    errors = []
    for pbStats in stats:
        rows = pbStats.rows
        if not (int(min) <= rows <= int(max)):
            errors.append(f"{pbStats.name}: {rows} rows (expected between {min} and {max})")
    if errors:
        error_message = "\n".join(errors)
        assert False, f"Some subproblems do not meet the row criteria:\n{error_message}"


@then(u'the generated subproblems have between {min} and {max} cols')
def check_subproblems_cols(context, min, max):
    lp_path = output_path(context.tmp_study / "output") / "lp"
    stats = get_subproblem_statistics(lp_path)
    errors = []
    for pbStats in stats:
        cols = pbStats.cols
        if not (int(min) <= cols <= int(max)):
            errors.append(f"{pbStats.name}: {cols} columns (expected between {min} and {max})")
    if errors:
        error_message = "\n".join(errors)
        assert False, f"Some subproblems do not meet the column criteria:\n{error_message}"


@then(u'the generated subproblems have between {min} and {max} elements')
def check_subproblems_elements(context, min, max):
    lp_path = output_path(context.tmp_study / "output") / "lp"
    stats = get_subproblem_statistics(lp_path)
    errors = []
    for pbStats in stats:
        elements = pbStats.elements
        if not (int(min) <= elements <= int(max)):
            errors.append(f"{pbStats.name}: {elements} elements (expected between {min} and {max})")
    if errors:
        error_message = "\n".join(errors)
        assert False, f"Some subproblems do not meet the element criteria:\n{error_message}"


@then(u'the generated subproblems have {n} rows')
def check_subproblems_exact_row(context, n):
    lp_path = output_path(context.tmp_study / "output") / "lp"
    stats = get_subproblem_statistics(lp_path)
    errors = []
    for pbStats in stats:
        if pbStats.rows != int(n):
            errors.append(f"{pbStats.name}: {pbStats.rows} rows (expected exactly {n})")
    if errors:
        error_message = "\n".join(errors)
        assert False, f"Some subproblems do not have exactly {n} rows:\n{error_message}"


@then(u'the generated subproblems have {n} cols')
def check_subproblems_exact_cols(context, n):
    lp_path = output_path(context.tmp_study / "output") / "lp"
    stats = get_subproblem_statistics(lp_path)
    errors = []
    for pbStats in stats:
        if pbStats.cols != int(n):
            errors.append(f"{pbStats.name}: {pbStats.cols} cols (expected exactly {n})")
    if errors:
        error_message = "\n".join(errors)
        assert False, f"Some subproblems do not have exactly {n} cols:\n{error_message}"


@then(u'the generated subproblems have {n} elements')
def check_subproblems_exact_elements(context, n):
    lp_path = output_path(context.tmp_study / "output") / "lp"
    stats = get_subproblem_statistics(lp_path)
    errors = []
    for pbStats in stats:
        if pbStats.elements != int(n):
            errors.append(f"{pbStats.name}: {pbStats.elements} elements (expected exactly {n})")
    if errors:
        error_message = "\n".join(errors)
        assert False, f"Some subproblems do not have exactly {n} elements:\n{error_message}"


@then("the simulation takes less than {seconds:d} seconds")
def check_simu_time(context, seconds):
    assert context.outputs["run_duration"] <= seconds


@then("the simulation succeeds")
def simu_success(context):
    print(context.return_code)
    assert context.return_code == 0


@then("the expected overall cost is {value:g}")
def check_overall_cost(context, value):
    print(f".... overall cost : {context.outputs['solution']['overall_cost']}")
    np.testing.assert_allclose(value, context.outputs["solution"]["overall_cost"], rtol=1e-6, atol=0)


@then("the expected investment cost is {value:g}")
def check_investment_cost(context, value):
    np.testing.assert_allclose(value, context.outputs["solution"]["investment_cost"], rtol=1e-6, atol=0)


@then("the solution is")
def check_solution(context):
    expected_solution = {row['variable']: float(row['value']) for row in context.table}
    assert_dict_allclose(context.outputs["solution"]["values"], expected_solution)


@then("LOLD.txt and UnsuppliedEnergy.txt files are full of zeros")
def check_other_outputs(context):
    assert (is_file_full_of_zeros(context.loss_of_load_file))
    assert (is_file_full_of_zeros(context.unsupplied_energy_file))


@then("the expected positive unsupplied energy is")
def check_unsupplied_energy(context):
    results = read_table_from_string(context.unsupplied_energy)
    check_cucumber_table(context, results)


@then("the expected loss of load is")
def check_loss_of_load_is(context):
    results = read_table_from_string(context.lold)
    check_cucumber_table(context, results)


@then('Simulator has been launched with solver "{string}"')
def check_simulator_solver(context, string):
    string_to_find = f"solver {string} is used for linear problem resolution"
    assert (find_in_simulator_log(context.tmp_study / "output", string_to_find))


@then('Benders has been launched with solver "{string}"')
def check_benders_solver(context, string):
    solver_in_benders = context.options_data["SOLVER_NAME"]
    print(f"Solver in benders: {solver_in_benders}\n")
    assert solver_in_benders.upper() == string.upper()




