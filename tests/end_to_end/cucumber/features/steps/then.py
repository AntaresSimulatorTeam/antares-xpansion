import csv
import glob
import io
import math
import os

import numpy as np
import xpress as xp
from behave import *
from utils_functions import find_in_simulator_log, output_path

# Cache global pour les statistiques des sous-problèmes
_subproblem_stats_cache = {}


def get_subproblem_statistics(lp_path):
    """Get statistics for all subproblems in the lp directory with caching"""
    # Créer une clé de cache basée sur le chemin et la date de modification du répertoire
    cache_key = _create_cache_key(lp_path)

    # Vérifier si les résultats sont déjà en cache
    if cache_key in _subproblem_stats_cache:
        return _subproblem_stats_cache[cache_key]

    # Calculer les statistiques si pas en cache
    mps_files = glob.glob(str(lp_path / "**/*.mps"), recursive=True)

    if not mps_files:
        result = (0, 0, 0)
        _subproblem_stats_cache[cache_key] = result
        return result

    total_rows = 0
    total_cols = 0
    total_elements = 0

    for mps_file in mps_files:
        if "master" not in mps_file.lower():  # Skip master problem files
            rows, cols, elements = analyze_mps_file(mps_file)
            total_rows += rows
            total_cols += cols
            total_elements += elements

    # Return average per subproblem
    num_subproblems = len([f for f in mps_files if "master" not in f.lower()])
    if num_subproblems > 0:
        result = (total_rows // num_subproblems, total_cols // num_subproblems, total_elements // num_subproblems)
    else:
        result = (0, 0, 0)

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

    if mps_file_path.endswith('.mps'):
        pb = xp.problem(mps_file_path)
        pb.read(mps_file_path)
        x = pb.getAttrib()
        rows, cols, elements = pb.getAttrib('rows', 'cols', 'elems').values()
    return rows, cols, elements


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

            np.testing.assert_allclose(actual_value, expected_value, rtol=1e-6, atol=0,
                                       err_msg=f"Mismatch in row {i + 1}, column '{header}': expected {expected_value}, got {actual_value}")


@then(u'the return status is 0')
def check_return_status(context):
    assert context.return_code == 0, f"Expected return code 0, got {context.return_code}"


@then(u'the generated subproblems have between {min} and {max} rows')
def check_subproblems_rows(context, min, max):
    lp_path = context.tmp_study / "lp"
    rows, _, _ = get_subproblem_statistics(lp_path)
    assert int(min) <= rows <= int(max), f"Expected rows between {min}-{max}, got {rows}"


@then(u'the generated subproblems have between {min} and {max} cols')
def check_subproblems_cols(context, min, max):
    lp_path = context.tmp_study / "lp"
    _, cols, _ = get_subproblem_statistics(lp_path)
    assert int(min) <= cols <= int(max), f"Expected cols between {min}-{max}, got {cols}"


@then(u'the generated subproblems have between {min} and {max} elements')
def check_subproblems_elements(context, min, max):
    lp_path = context.tmp_study / "lp"
    _, _, elements = get_subproblem_statistics(lp_path)
    assert int(min) <= elements <= int(max), f"Expected elements between {min}-{max}, got {elements}"


@then(u'the generated subproblems have {n} rows')
def check_subproblems_exact_row(context, n):
    lp_path = output_path(context.tmp_study / "output") / "lp"
    rows, _, _ = get_subproblem_statistics(lp_path)
    assert rows == int(n), f"Expected exactly {n} rows, got {rows}"


@then(u'the generated subproblems have {n} cols')
def check_subproblems_exact_cols(context, n):
    lp_path = output_path(context.tmp_study / "output") / "lp"
    _, cols, _ = get_subproblem_statistics(lp_path)
    assert cols == int(n), f"Expected exactly {n} cols, got {cols}"


@then(u'the generated subproblems have {n} elements')
def check_subproblems_exact_elements(context, n):
    lp_path = output_path(context.tmp_study / "output") / "lp"
    _, _, elements = get_subproblem_statistics(lp_path)
    assert elements == int(n), f"Expected exactly {n} elements, got {elements}"


@then("the simulation takes less than {seconds:d} seconds")
def check_simu_time(context, seconds):
    assert context.outputs["run_duration"] <= seconds


@then("the simulation succeeds")
def simu_success(context):
    assert context.return_code == 0


@then("the expected overall cost is {value:g}")
def check_overall_cost(context, value):
    np.testing.assert_allclose(value, context.outputs["solution"]["overall_cost"], rtol=1e-6, atol=0)


@then("the expected investment cost is {value:g}")
def check_overall_cost(context, value):
    np.testing.assert_allclose(value, context.outputs["solution"]["investment_cost"], rtol=1e-6, atol=0)


@then("the solution is")
def check_solution(context):
    expected_solution = {row['variable']: float(row['value']) for row in context.table}
    assert_dict_allclose(context.outputs["solution"]["values"], expected_solution)


@then("LOLD.txt and PositiveUnsuppliedEnergy.txt files are full of zeros")
def check_other_outputs(context):
    assert (is_file_full_of_zeros(context.loss_of_load_file))
    assert (is_file_full_of_zeros(context.positive_unsupplied_energy_file))


@then("the expected positive unsupplied energy is")
def check_positive_unsupplied_energy(context):
    results = read_table_from_string(context.positive_unsupplied_energy)
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
