from typing import List, Dict, Tuple

import pandas as pd
from matplotlib.figure import Figure
from matplotlib.axes import Axes
from matplotlib import pyplot as plt, rc, rcParams, style


class PerfPlotsGenerator:
    def __init__(self, perf_data: pd.DataFrame) -> None:
        self.perf_data = perf_data

        self.figs: Dict[Figure] = {}
        self.axes: Dict[Axes] = {}

        style.use("default")
        rc("font", **{"family": "serif"})
        rcParams.update({"font.size": 16})

    def _columns_to_plot(self, study_data: pd.DataFrame) -> List:
        return [
            column
            for column in self.perf_data.columns[-3:]
            if column in study_data.columns
        ]

    def _xpansion_steps(self) -> List:
        return self.perf_data["Xpansion step"].unique()

    def _create_fig(self, fig_name: str) -> None:

        fig, ax = plt.subplots(figsize=(8, 8))
        self.figs[fig_name] = fig
        self.axes[fig_name] = ax

    def _plot_xpansion_step(
        self,
        fig_name: str,
        study_data: pd.DataFrame,
        xpansion_step: str,
        columns_to_plot: List[str],
    ):

        xpansion_step_data = study_data[study_data["Xpansion step"] == xpansion_step]

        self.axes[fig_name].plot(
            columns_to_plot,
            xpansion_step_data[columns_to_plot].to_numpy()[0],
            label=xpansion_step,
        )
        self.axes[fig_name].set_xticks(columns_to_plot)
        self.axes[fig_name].set_xticklabels(columns_to_plot, rotation=45)

    def _plot_total_time(
        self, fig_name: str, study_data: pd.DataFrame, columns_to_plot: List[str]
    ):

        total_time = study_data[columns_to_plot].sum()
        self.axes[fig_name].plot(columns_to_plot, total_time, label="Total")

    def run(self) -> None:

        for name_master_tuple in self.perf_data.index.unique():

            fig_name = "_".join(name_master_tuple)
            self._create_fig(fig_name)

            study_data = self.perf_data.loc[name_master_tuple].dropna(axis=1)
            columns_to_plot = self._columns_to_plot(study_data)

            for xpansion_step in self._xpansion_steps():

                self._plot_xpansion_step(
                    fig_name, study_data, xpansion_step, columns_to_plot
                )

            self._plot_total_time(fig_name, study_data, columns_to_plot)

            self.axes[fig_name].set_xlabel("Xpansion version")
            self.axes[fig_name].set_ylabel("Time (min)")
            self.axes[fig_name].legend()
