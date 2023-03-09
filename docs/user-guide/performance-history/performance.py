from typing import List

import json
import pandas as pd
from matplotlib.figure import Figure
from matplotlib.axes import Axes
from matplotlib import pyplot as plt, rc, rcParams, style

ANTARES_STEP = "antares"
PROBLEM_GENERATION_STEP = "problem_generation"
BENDERS_STEP = "benders"


class JsonFileProcessor:
    def __init__(self, filepath: str) -> None:
        self.json_path = filepath

    def run(self) -> pd.DataFrame:
        with open(self.json_path, "r") as file:
            json_data = json.load(file)

        return self._preprocess_data(json_data)

    def _preprocess_data(self, json_data) -> None:
        processed_data = pd.json_normalize(
            json_data["studies"],
            record_path=["xpansion_data"],
            meta=[
                "display_name",
                "areas",
                "links",
                ["master", "candidates"],
                ["master", "type"],
                ["subproblems", "variables"],
                ["subproblems", "constraints"],
                ["subproblems", "non_zero_coefficients"],
                ["mc_years"],
                ["weeks"],
                ["resolution", "stopping_criterion", "type"],
                ["resolution", "stopping_criterion", "value"],
            ],
        )
        self.processed_data = processed_data.set_index(["display_name", "version"])

    def stylize_data_for_display(self):
        columns_to_display = [
            "areas",
            "links",
            "master.candidates",
            "master.type",
            "subproblems.variables",
            "subproblems.constraints",
            "subproblems.non_zero_coefficients",
            "mc_years",
            "weeks",
            "resolution.stopping_criterion.type",
            "resolution.stopping_criterion.value",
        ]
        stylized_data = self.processed_data.loc[(slice(None), 1.0), columns_to_display]
        stylized_data.index = stylized_data.index.droplevel(1)
        return stylized_data


class PerfPlotsGenerator:
    def __init__(self, perf_data: pd.DataFrame) -> None:
        self.perf_data = perf_data

        self.fig: Figure
        self.ax: Axes

        style.use("default")
        rc("font", **{"family": "serif"})
        rcParams.update({"font.size": 16})

    def _xpansion_versions(self) -> List[float]:
        return self.perf_data.index.unique(level="version").tolist()

    def _create_fig(self) -> None:

        nb_versions = len(self._xpansion_versions())
        fig, ax = plt.subplots(
            1, nb_versions, sharey=True, figsize=(5 * nb_versions, 8)
        )

        self.fig = fig
        self.ax = ax

    def _display_names(self) -> List[str]:
        return self.perf_data.index.unique(level="display_name").tolist()

    def _beautify_fig(self) -> None:
        self.fig.subplots_adjust(bottom=0.2)
        self.fig.suptitle("Xpansion performance evolution")

        self.fig.supylabel("Execution time (s)")
        self.fig.supxlabel("Study id")
        self.ax[-1].legend()

        for axes in self.ax:
            axes.set_xticks(self._display_names())
            axes.set_xticklabels(self._display_names(), rotation=90)

        self.fig.tight_layout()

    def _plot_single_version(self, version_count: int, xpansion_version: float) -> None:
        antares_step_times = self.perf_data.loc[
            (slice(None), xpansion_version), ANTARES_STEP
        ].values
        problem_generation_step_times = self.perf_data.loc[
            (slice(None), xpansion_version), PROBLEM_GENERATION_STEP
        ].values
        benders_step_times = self.perf_data.loc[
            (slice(None), xpansion_version), BENDERS_STEP
        ].values

        self.ax[version_count].bar(
            self._display_names(), antares_step_times, label=ANTARES_STEP
        )
        self.ax[version_count].bar(
            self._display_names(),
            problem_generation_step_times,
            bottom=antares_step_times,
            label=PROBLEM_GENERATION_STEP,
        )
        self.ax[version_count].bar(
            self._display_names(),
            benders_step_times,
            bottom=problem_generation_step_times + antares_step_times,
            label=BENDERS_STEP,
        )
        self.ax[version_count].set_title(f"Version {xpansion_version}")

    def run(self) -> None:

        self._create_fig()

        for version_count, xpansion_version in enumerate(self._xpansion_versions()):
            self._plot_single_version(version_count, xpansion_version)

        self._beautify_fig()
