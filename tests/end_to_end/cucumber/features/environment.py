import tempfile
from pathlib import Path

from steps.then import clear_subproblem_stats_cache


def before_scenario(context, scenario):
    """Hook called before each scenario starts"""
    context.temp_dir = Path(tempfile.TemporaryDirectory().name)
    # Clear the cache before each scenario to ensure fresh results
    clear_subproblem_stats_cache()


def after_scenario(context, scenario):
    """Hook called after each scenario completes"""
    # Optionally clear cache after scenario to free memory
    # (only needed if memory usage is a concern)
    pass
