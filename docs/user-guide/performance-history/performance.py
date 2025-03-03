import json
from typing import List, Optional

import pandas as pd
from matplotlib import pyplot as plt
from matplotlib import rc, rcParams, style
from matplotlib.axes import Axes
from matplotlib.figure import Figure

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
        stylized_data = self.processed_data.loc[(slice(None), 1.1), columns_to_display]
        stylized_data.index = stylized_data.index.droplevel(1)
        return stylized_data


class PerfPlotsGenerator:
    def __init__(
        self, perf_data: pd.DataFrame, xpansion_versions: Optional[List[int]] = None
    ) -> None:
        self.perf_data = perf_data

        self.fig: Figure
        self.ax: Axes

        style.use("default")
        rc("font", **{"family": "serif"})
        rcParams.update({"font.size": 16})

        self.xpansion_versions = self._xpansion_versions(xpansion_versions)

    def _xpansion_versions(self, xpansion_versions: Optional[List[str]]) -> List[float]:
        valid_versions = self.perf_data.index.unique(level="version").tolist()
        if xpansion_versions is None:
            # If no version is specified by the user, return all versions present in the data
            return valid_versions
        else:
            return_versions = []
            for version in xpansion_versions:
                if version in valid_versions:
                    return_versions.append(version)
            return return_versions

    def _create_fig(self) -> None:
        nb_versions = self._get_nb_versions()
        fig, ax = plt.subplots(1, 1, figsize=(8, 3 * nb_versions))

        self.fig = fig
        self.ax = ax
        self.ax.invert_yaxis()

    def _get_nb_versions(self):
        return len(self.xpansion_versions)

    def _display_names(self) -> List[str]:
        return self.perf_data.index.unique(level="display_name").tolist()

    def _beautify_fig(self) -> None:
        self.ax.set_yticks(
            list(range(len(self._display_names()))),
            [study for study in self._display_names()],
        )
        self.ax.set_ylim(self.ax.get_ylim()[0], 1.5 * self.ax.get_ylim()[1])
        self.ax.legend(loc="upper center", ncol=3, fontsize=12)
        self.ax.set_title("Xpansion performance evolution")

        self.ax.set_xlabel("Execution time (s)")
        self.fig.tight_layout()

    def _plot_study_data(self) -> None:

        nb_versions = self._get_nb_versions()
        height = 0.8 / nb_versions  # Defines space between different study data
        epsilon = 0.03  # Defines space between bars of the data for the different version of the same study
        actual_height = (1 - epsilon) * height
        alpha_decrease_rate = 0.2  # Defines transparency difference between bars of the data for the different version of the same study

        for count, study in enumerate(self._display_names()):
            for version_cnt, xpansion_version in enumerate(self.xpansion_versions):
                antares_time = self.perf_data.loc[
                    (study, xpansion_version), ANTARES_STEP
                ]
                pb_gen_time = self.perf_data.loc[
                    (study, xpansion_version), PROBLEM_GENERATION_STEP
                ]
                benders_time = self.perf_data.loc[
                    (study, xpansion_version), BENDERS_STEP
                ]

                y_pos = self._bar_y_position(nb_versions, height, count, version_cnt)

                antares_line = self.ax.barh(
                    y_pos,
                    antares_time,
                    height=actual_height,
                    color="C0",
                    align="edge",
                    alpha=1 - alpha_decrease_rate * version_cnt,
                )
                pb_gen_line = self.ax.barh(
                    y_pos,
                    pb_gen_time,
                    height=actual_height,
                    left=antares_time,
                    color="C1",
                    align="edge",
                    alpha=1 - alpha_decrease_rate * version_cnt,
                )
                benders_line = self.ax.barh(
                    y_pos,
                    benders_time,
                    left=pb_gen_time + antares_time,
                    height=actual_height,
                    color="C2",
                    align="edge",
                    alpha=1 - alpha_decrease_rate * version_cnt,
                )
                self.ax.bar_label(
                    benders_line,
                    [f"v{xpansion_version}"],
                    label_type="center",
                    fontsize=12,
                )
                if version_cnt == 0 and count == 0:
                    antares_line.set_label(ANTARES_STEP)
                    pb_gen_line.set_label(PROBLEM_GENERATION_STEP)
                    benders_line.set_label(BENDERS_STEP)

    def _bar_y_position(
        self, nb_versions: int, height: float, count: int, version_cnt: int
    ):
        return count + height * (version_cnt - nb_versions / 2)

    def run(self) -> None:
        self._create_fig()
        self._plot_study_data()
        self._beautify_fig()
