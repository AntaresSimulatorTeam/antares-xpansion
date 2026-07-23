from pathlib import Path

import pytest

from antares_xpansion.config_loader import ConfigLoader
from antares_xpansion.input_checker import (
    NotHandledOption,
    OptionTypeError,
    PositiveFloatValueError,
)
from antares_xpansion.xpansion_study_reader import XpansionStudyReader
from antares_xpansion.xpansionConfig import (
    ConfigParameters,
    InputParameters,
    XpansionConfig,
)

AVAILABLE_SOLVERS = ["Cbc", "Xpress"]


def _make_config_parameters():
    return ConfigParameters(
        default_install_dir="install_dir",
        ANTARES="antares",
        MERGE_MPS="merge_mps",
        BENDERS="benders",
        LP_NAMER="lp_namer",
        PRESOLVE="presolve",
        STUDY_UPDATER="study_updater",
        SENSITIVITY_EXE="sensitivity",
        FULL_RUN="full_run",
        OUTER_LOOP="outer_loop",
        ANTARES_ARCHIVE_UPDATER="antares_archive_updater",
        ANTARES_PROBLEM_GENERATOR="antares_problem_generator",
        MPIEXEC="mpiexec",
        AVAILABLE_SOLVERS=AVAILABLE_SOLVERS,
        MULTIPLE_PROBLEM_GEN="multiple_problem_gen",
        MERGE_MASTER_MPS="merge_master_mps",
        MERGE_WEIGHTS_TRAJECTORY="merge_weights_trajectory",
    )


def _make_input_parameters(data_dir: Path, install_dir: Path, simulation_name="last"):
    return InputParameters(
        step="full",
        simulation_name=simulation_name,
        data_dir=str(data_dir),
        install_dir=str(install_dir),
        method="benders_decomposition",
        n_mpi=1,
        antares_n_cpu=1,
        keep_mps=False,
        oversubscribe=False,
        allow_run_as_root=False,
        memory=False,
        run_presolve=False,
        cache_problems=False,
        problem_format="OPTIMIZED",
    )


def _make_study_dir(tmp_path: Path, settings_content=None, gems_candidates=True):
    """
    Builds the minimal study tree ConfigLoader needs: a settings.ini file in
    user/expansion, and either a gems marker file or a (possibly missing)
    candidates.ini, depending on gems_candidates.

    No study.antares or settings/generaldata.ini is created, which keeps the
    study "full gems" so ConfigLoader skips antares-version/NTC/general-data
    reading entirely - out of scope for these settings-file-focused tests.
    """
    data_dir = tmp_path / "study"
    expansion_dir = data_dir / "user" / "expansion"
    expansion_dir.mkdir(parents=True)

    if settings_content is not None:
        (expansion_dir / "settings.ini").write_text(settings_content)

    if gems_candidates:
        input_dir = data_dir / "input"
        input_dir.mkdir(parents=True)
        (input_dir / "optim-config.yml").touch()

    return data_dir


def _make_config_loader(tmp_path: Path, settings_content="", gems_candidates=True):
    data_dir = _make_study_dir(
        tmp_path, settings_content=settings_content, gems_candidates=gems_candidates
    )
    install_dir = tmp_path / "install"
    install_dir.mkdir()
    input_parameters = _make_input_parameters(data_dir, install_dir)
    config = XpansionConfig(input_parameters, _make_config_parameters())
    return ConfigLoader(config)


class TestSettingsFileLoading:
    def test_fails_if_settings_file_is_missing(self, tmp_path):
        data_dir = _make_study_dir(tmp_path, settings_content=None)
        install_dir = tmp_path / "install"
        install_dir.mkdir()
        config = XpansionConfig(
            _make_input_parameters(data_dir, install_dir), _make_config_parameters()
        )

        with pytest.raises(ConfigLoader.MissingFile):
            ConfigLoader(config)

    def test_fails_if_candidates_file_is_missing_and_not_gems(self, tmp_path):
        data_dir = _make_study_dir(
            tmp_path, settings_content="", gems_candidates=False
        )
        install_dir = tmp_path / "install"
        install_dir.mkdir()
        config = XpansionConfig(
            _make_input_parameters(data_dir, install_dir), _make_config_parameters()
        )

        with pytest.raises(ConfigLoader.MissingFile):
            ConfigLoader(config)

    def test_fails_on_unrecognized_option(self, tmp_path):
        with pytest.raises(NotHandledOption):
            _make_config_loader(tmp_path, settings_content="not_an_option = 1\n")

    def test_fails_on_wrong_option_type(self, tmp_path):
        with pytest.raises(OptionTypeError):
            _make_config_loader(tmp_path, settings_content="optimality_gap = not_a_number\n")

    def test_fails_if_solver_not_available(self, tmp_path):
        with pytest.raises(SystemExit):
            _make_config_loader(tmp_path, settings_content="solver = Gurobi\n")

    def test_parses_key_value_pairs_from_settings_file(self, tmp_path):
        config_loader = _make_config_loader(
            tmp_path,
            settings_content="master = relaxed\ntimelimit = 100\n",
        )

        assert config_loader.options["master"] == "relaxed"
        assert config_loader.options["timelimit"] == "100"


class TestSettingsFileDefaultsWhenKeyIsMissing:
    """
    Every getter falls back to XpansionConfigConstants.settings_default when the
    corresponding key is absent from settings.ini - none of them should raise
    a KeyError just because the study's settings.ini omits an optional option.
    """

    def test_defaults_used_for_empty_settings_file(self, tmp_path):
        config_loader = _make_config_loader(tmp_path, settings_content="")

        assert config_loader.get_absolute_optimality_gap() == 1
        assert config_loader.get_relative_optimality_gap() == 1e-6
        assert config_loader.get_relaxed_optimality_gap() == 1e-5
        assert config_loader.get_max_iterations() == -1
        assert config_loader.get_master_formulation() == "integer"
        assert config_loader.get_separation() == 0.5
        assert config_loader.get_batch_size() == 0
        assert config_loader.timelimit() == 1e12
        assert config_loader.log_level() == 0
        assert config_loader.get_master_solution_tolerance() == 1e-4
        assert config_loader.get_cut_coefficient_tolerance() == 5e-3
        assert config_loader.is_accurate() is False
        assert config_loader.is_relaxed() is False
        assert config_loader.weight_file_name() == ""
        assert config_loader.additional_constraints() == ""
        # solver is filled in with the default when absent, so downstream
        # code (e.g. last_master_file_path) can safely index options["solver"]
        assert config_loader.options["solver"] == "Cbc"


class TestSettingsFileOverrides:
    def test_optimality_gap_override(self, tmp_path):
        config_loader = _make_config_loader(
            tmp_path, settings_content="optimality_gap = 2.5\n"
        )
        assert config_loader.get_absolute_optimality_gap() == 2.5

    def test_negative_optimality_gap_is_rejected_at_load_time(self, tmp_path):
        # settings-file validation (check_options, run during ConfigLoader.__init__)
        # rejects negative values before get_absolute_optimality_gap()'s own
        # clamp-to-zero branch could ever be reached.
        with pytest.raises(PositiveFloatValueError):
            _make_config_loader(tmp_path, settings_content="optimality_gap = -2.5\n")

    def test_master_formulation_override(self, tmp_path):
        config_loader = _make_config_loader(
            tmp_path, settings_content="master = relaxed\n"
        )
        assert config_loader.get_master_formulation() == "relaxed"
        assert config_loader.is_relaxed()

    def test_uc_type_override(self, tmp_path):
        config_loader = _make_config_loader(
            tmp_path, settings_content="uc_type = expansion_accurate\n"
        )
        assert config_loader.is_accurate()

    def test_infinite_max_iteration_override(self, tmp_path):
        config_loader = _make_config_loader(
            tmp_path, settings_content="max_iteration = +Inf\n"
        )
        assert config_loader.get_max_iterations() == -1

    def test_timelimit_override(self, tmp_path):
        config_loader = _make_config_loader(
            tmp_path, settings_content="timelimit = 42\n"
        )
        assert config_loader.timelimit() == 42

    def test_solver_override(self, tmp_path):
        config_loader = _make_config_loader(
            tmp_path, settings_content="solver = Xpress\n"
        )
        assert config_loader.options["solver"] == "Xpress"


class TestGemsCandidates:
    def test_gems_candidates_detected(self, tmp_path):
        config_loader = _make_config_loader(tmp_path, gems_candidates=True)
        assert config_loader.gems_candidates()

    def test_additional_constraints_not_supported_with_gems_candidates(self, tmp_path):
        data_dir = _make_study_dir(
            tmp_path,
            settings_content="additional-constraints = constraints.txt\n",
            gems_candidates=True,
        )
        install_dir = tmp_path / "install"
        install_dir.mkdir()
        config = XpansionConfig(
            _make_input_parameters(data_dir, install_dir), _make_config_parameters()
        )

        with pytest.raises(
            ConfigLoader.AdditionalConstraintsNotSupportedWithGemsCandidates
        ):
            ConfigLoader(config)


class TestLauncherOptionsSaveAndRestore:
    def test_save_launcher_options_then_restore_does_not_raise_on_resume(
        self, tmp_path
    ):
        # save_launcher_options() must write every key that
        # _restore_launcher_options() later reads back (e.g. "memory"), since
        # a resume run restores its options from a launcher_options.json
        # produced by an earlier full/benders run.
        data_dir = _make_study_dir(tmp_path, settings_content="")
        install_dir = tmp_path / "install"
        install_dir.mkdir()

        xpansion_output_dir = data_dir / "output" / "myoutput-Xpansion"
        (xpansion_output_dir / "lp").mkdir(parents=True)

        full_config = XpansionConfig(
            InputParameters(
                step="full",
                simulation_name="myoutput-Xpansion",
                data_dir=str(data_dir),
                install_dir=str(install_dir),
                method="benders_decomposition",
                n_mpi=1,
                antares_n_cpu=1,
                keep_mps=False,
                oversubscribe=False,
                allow_run_as_root=False,
                memory=True,
                run_presolve=False,
                cache_problems=False,
                problem_format="OPTIMIZED",
            ),
            _make_config_parameters(),
        )
        ConfigLoader(full_config).save_launcher_options()

        resume_config = XpansionConfig(
            InputParameters(
                step="resume",
                simulation_name="",
                data_dir=str(data_dir),
                install_dir=str(install_dir),
                method="benders_decomposition",
                n_mpi=1,
                antares_n_cpu=1,
                keep_mps=False,
                oversubscribe=False,
                allow_run_as_root=False,
                memory=False,
                run_presolve=False,
                cache_problems=False,
                problem_format="OPTIMIZED",
            ),
            _make_config_parameters(),
        )

        # ConfigLoader.__init__ calls _restore_launcher_options() itself when
        # step == "resume", so simply constructing it must not raise.
        ConfigLoader(resume_config)


class TestSimulationName:
    def test_missing_simulation_name_raises(self, tmp_path):
        data_dir = _make_study_dir(tmp_path, settings_content="")
        install_dir = tmp_path / "install"
        install_dir.mkdir()
        config = XpansionConfig(
            _make_input_parameters(data_dir, install_dir, simulation_name=""),
            _make_config_parameters(),
        )

        with pytest.raises(ConfigLoader.MissingSimulationName):
            ConfigLoader(config)
