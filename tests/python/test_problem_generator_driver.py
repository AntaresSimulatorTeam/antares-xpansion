from typing import Text
import pytest
import os
from pathlib import Path
from datetime import date, datetime
from unittest.mock import ANY, mock_open, patch


from .file_creation import _create_weight_file
from antares_xpansion.problem_generator_driver import ProblemGeneratorData, ProblemGeneratorDriver
from antares_xpansion.antares_driver import AntaresDriver
from antares_xpansion.xpansion_study_reader import XpansionStudyReader
from antares_xpansion.general_data_reader import GeneralDataIniReader

from tests.build_config_reader import get_install_dir, get_lp_namer_exe, get_antares_solver_path

SUBPROCESS_RUN =  "antares_xpansion.problem_generator_driver.subprocess.run"
class TestProblemGeneratorDriver:
    number = 0
    def setup_method(self):
        self.lp_exe = "lp_namer.exe"
        self.empty_pblm_gen_data = ProblemGeneratorData(LP_NAMER="",
                                                      keep_mps = False,
                                                      additional_constraints="",
                                                      weights_file_path=Path(""),
                                                      weight_file_name="",
                                                      install_dir=Path(""),
                                                      nb_active_years= 0)

    def test_problem_generator_data(self):


        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)

        assert problem_generator_driver.LP_NAMER == self.empty_pblm_gen_data.LP_NAMER                                              
        assert problem_generator_driver.keep_mps == self.empty_pblm_gen_data.keep_mps                                              
        assert problem_generator_driver.additional_constraints == self.empty_pblm_gen_data.additional_constraints                                              
        assert problem_generator_driver.weights_file_path == self.empty_pblm_gen_data.weights_file_path                                              
        assert problem_generator_driver.install_dir == self.empty_pblm_gen_data.install_dir                                              

    def test_output_path(self, tmp_path):
        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.OutputPathError):
            problem_generator_driver.launch(tmp_path/ "i_don_t_exist", False)

    def test_no_area_file(self, tmp_path):
    
        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.AreaFileException):
            problem_generator_driver.launch(tmp_path, False)

    def test_more_than_1_area_file(self, tmp_path):
    
        self._create_empty_area_file(tmp_path)
        self._create_empty_area_file(tmp_path)


        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.AreaFileException):
            problem_generator_driver.launch(tmp_path, False)


    def test_no_interco_file(self, tmp_path):
    
        self._create_empty_area_file(tmp_path)
        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.IntercoFilesException):
            problem_generator_driver.launch(tmp_path, False)

    def test_more_than_1_interco_file(self, tmp_path):
    
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.IntercoFilesException):
            problem_generator_driver.launch(tmp_path, False)


    def test_lp_namer_exe_not_exit(self, tmp_path):
    
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.LPNamerExeError):
            problem_generator_driver.launch(tmp_path, False)


    def test_mps_txt_creation(self, tmp_path):
    
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.LPNamerExeError):
            problem_generator_driver.launch(tmp_path, False)

        
        mps_txt_file = tmp_path / "mps.txt"
        assert  mps_txt_file.exists()
   

    def test_mps_txt_content(self, tmp_path):
    
        expected_results = self._get_expected_mps_txt(tmp_path)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.LPNamerExeError):
            problem_generator_driver.launch(tmp_path, False)

        
        mps_txt_file = tmp_path / problem_generator_driver.MPS_TXT
        assert  mps_txt_file.exists()

        with open(mps_txt_file) as mps_txt :
            for line in mps_txt :
                my_list = line.strip().split(" ")
                assert my_list in expected_results


    def test_lp_namer_log_filename(self, tmp_path):

        
        lp_exe_file = tmp_path / self.lp_exe
        lp_exe_file.write_text("") 
        additional_constraints = "my additionals constraints"
        pblm_gen_data = ProblemGeneratorData(LP_NAMER= self.lp_exe,
                                                      keep_mps = False,
                                                      additional_constraints=additional_constraints,
                                                      weights_file_path=Path(""),
                                                      weight_file_name="",
                                                      install_dir=tmp_path,
                                                      nb_active_years= 1)
        
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        problem_generator_driver.output_path = tmp_path
        assert problem_generator_driver.get_lp_namer_log_filename() == os.path.join(tmp_path,"lp", pblm_gen_data.LP_NAMER+ '.log') 

    def test_lp_namer_log_filename_with_non_existing_lp_dir(self):

        
        additional_constraints = "my additionals constraints"
        pblm_gen_data = ProblemGeneratorData(LP_NAMER= self.lp_exe,
                                                      keep_mps = False,
                                                      additional_constraints=additional_constraints,
                                                      weights_file_path=Path(""),
                                                      weight_file_name="",
                                                      install_dir="",
                                                      nb_active_years= 1)
        
        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with pytest.raises(ProblemGeneratorDriver.LPNamerPathError):
            problem_generator_driver.get_lp_namer_log_filename()

    def test_clear_old_log(self, tmp_path):
        
        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        pblm_gen_data = ProblemGeneratorData(LP_NAMER=self.lp_exe,
                                                      keep_mps = False,
                                                      additional_constraints="",
                                                      weights_file_path=Path(""),
                                                      weight_file_name="",
                                                      install_dir= tmp_path,
                                                      nb_active_years= 1)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        
        log_file_name = self.lp_exe +".log" 
        log_file = tmp_path / log_file_name
        log_file.write_text("bla bla")
        assert log_file.exists()

        pblm_gen = ProblemGeneratorDriver(pblm_gen_data)
        with patch(SUBPROCESS_RUN, autospec=True):
            with pytest.raises(ProblemGeneratorDriver.LPNamerExecutionError):
                pblm_gen.launch(tmp_path, False)

        assert not log_file.exists()

    def test_clean_lp_dir_before_run(self, tmp_path):
        
        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        pblm_gen_data = ProblemGeneratorData(LP_NAMER=self.lp_exe,
                                                      keep_mps = False,
                                                      additional_constraints="",
                                                      weights_file_path=Path(""),
                                                      weight_file_name="",
                                                      install_dir= tmp_path,
                                                      nb_active_years= 1)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        
        log_file_name = self.lp_exe +".log" 
        log_file = tmp_path / log_file_name
        log_file.write_text("bla bla")

        lp_dir = tmp_path / "lp"
        lp_dir.mkdir()
        lp_dir_sub_file_1 = lp_dir / "file1"
        lp_dir_sub_file_1.write_text("")
        lp_dir_sub_file_2 = lp_dir / "file2"
        lp_dir_sub_file_2.write_text("")

        assert lp_dir.exists()
        assert lp_dir_sub_file_1.exists()
        assert lp_dir_sub_file_2.exists()
        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with patch(SUBPROCESS_RUN, autospec=True) as run_function :
            run_function.return_value.returncode = 0
            problem_generator_driver.launch(tmp_path, False)

        assert lp_dir.exists()
        assert not lp_dir_sub_file_1.exists()
        assert not lp_dir_sub_file_2.exists()


    def test_weight_file_name_fails_if_file_does_not_exist(self, tmp_path):
        
        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        file_path: Path = tmp_path / "toto_file"
        pblm_gen_data = ProblemGeneratorData(LP_NAMER=self.lp_exe,
                                                      keep_mps = False,
                                                      additional_constraints="",
                                                      weights_file_path=file_path,
                                                      weight_file_name=Path(""),
                                                      install_dir= tmp_path,
                                                      nb_active_years= 1)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        
        expected_message = f'Illegal value : {str(file_path)} is not an existent yearly-weights file'

        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with pytest.raises(FileNotFoundError) as expect:
            problem_generator_driver.launch(tmp_path, False)
            assert str(expect.value) == expected_message

    def test_weight_file_name_fails_if_there_is_one_negative_value(self, tmp_path):
        
        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        weight_file_name = "weight_file.txt"
        file_path: Path = tmp_path / weight_file_name
        weight_list = [1, -1]
        _create_weight_file(file_path, weight_list)
        pblm_gen_data = ProblemGeneratorData(LP_NAMER=self.lp_exe,
                                                      keep_mps = False,
                                                      additional_constraints="",
                                                      weights_file_path=file_path,
                                                      weight_file_name=weight_file_name,
                                                      install_dir= tmp_path,
                                                      nb_active_years= 2)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        
        expected_message = f'Line 2 in file {str(file_path)} indicates a negative value'

        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with pytest.raises(XpansionStudyReader.InvalidYearsWeightValue) as expect:
            problem_generator_driver.launch(tmp_path, False)
            assert str(expect.value) == expected_message

    def test_weight_file_name_fails_if_all_values_are_zero(self, tmp_path):
        
        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        weight_file_name = "weight_file.txt"
        file_path: Path = tmp_path / weight_file_name
        weight_list = [0, 0]
        _create_weight_file(file_path, weight_list)
        pblm_gen_data = ProblemGeneratorData(LP_NAMER=self.lp_exe,
                                                      keep_mps = False,
                                                      additional_constraints="",
                                                      weights_file_path=file_path,
                                                      weight_file_name=weight_file_name,
                                                      install_dir= tmp_path,
                                                      nb_active_years= 2)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        expected_message = f'file {str(file_path)} : all values are null'

        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with pytest.raises(XpansionStudyReader.OnlyNullYearsWeightValue) as expect:
            problem_generator_driver.launch(tmp_path, False)
            assert str(expect.value) == expected_message

    def test_fails_if_nb_activated_years_different_from_content(self, tmp_path):
        
        lp_namer_file = tmp_path / self.lp_exe
        lp_namer_file.write_text("")
        weight_file_name = "weight_file.txt"
        file_path: Path = tmp_path / weight_file_name
        weight_list = [1, 2, 3, 4]
        _create_weight_file(file_path, weight_list)
        pblm_gen_data = ProblemGeneratorData(LP_NAMER=self.lp_exe,
                                                      keep_mps = False,
                                                      additional_constraints="",
                                                      weights_file_path=file_path,
                                                      weight_file_name=weight_file_name,
                                                      install_dir= tmp_path,
                                                      nb_active_years= 5)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        expected_message = f'file {str(file_path)} : invalid weight number : 4 values / 5 expected'

        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        with pytest.raises(XpansionStudyReader.InvalidYearsWeightNumber) as expect:
            problem_generator_driver.launch(tmp_path, False)
            assert str(expect.value) == expected_message

    def tests_additionnal_constraints(self):
        study_dir  = TestProblemGeneratorDriver.run_antares_step_in_examples("additionnal-constraints")
        problem_generator_driver, simu_abs_path, is_relaxed = TestProblemGeneratorDriver._get_problem_generator_for_study(study_dir)
        problem_generator_driver.launch(simu_abs_path, is_relaxed)
    
    def tests_additionnal_constraints_binary(self):
        study_dir  = TestProblemGeneratorDriver.run_antares_step_in_examples("additionnal-constraints-binary")
        problem_generator_driver, simu_abs_path, is_relaxed = TestProblemGeneratorDriver._get_problem_generator_for_study(study_dir)
        problem_generator_driver.launch(simu_abs_path, is_relaxed)
        
    
    def tests_small_test_five_candidates_with_weights(self):
        study_dir  = TestProblemGeneratorDriver.run_antares_step_in_examples("SmallTestFiveCandidatesWithWeights")
        problem_generator_driver, simu_abs_path, is_relaxed = TestProblemGeneratorDriver._get_problem_generator_for_study(study_dir)
        problem_generator_driver.launch(simu_abs_path, is_relaxed)

    def tests_small_test_five_candidates(self):
        study_dir  = TestProblemGeneratorDriver.run_antares_step_in_examples("SmallTestFiveCandidates")
        problem_generator_driver, simu_abs_path, is_relaxed = TestProblemGeneratorDriver._get_problem_generator_for_study(study_dir)
        problem_generator_driver.launch(simu_abs_path, is_relaxed)



    def _get_expected_mps_txt(self, tmp_path):

        weeks = [1, 2, 3]
        years = [1, 2]
        expected_results = []

        for year in years :
            for week in weeks :
                fname_mps = self._get_file_name("problem", str(year), str(week), "mps")
                mps_file = tmp_path / fname_mps
                mps_file.write_text("")
                fname_vars = self._get_file_name("variables", str(year), str(week), "txt")
                variables_file = tmp_path / fname_vars
                variables_file.write_text("")
                fname_cts = self._get_file_name("constraints", str(year), str(week), "txt")
                constraints_file = tmp_path / fname_cts
                constraints_file.write_text("")
                
                expected_results.append([fname_mps, fname_vars, fname_cts])   

        return expected_results

    def _get_file_name(self, prefix, year, week, extension):
        
        month = str(date.today().month) if date.today().month > 9 else  "0"+ str(date.today().month)
        day = str(date.today().day) if date.today().day > 9 else  "0"+ str(date.today().day)
        today_date = str(date.today().year)+month+day
        
        hour = str(datetime.now().hour) if datetime.now().hour > 9 else  "0"+ str(datetime.now().hour)
        minute = str(datetime.now().minute) if datetime.now().minute > 9 else  "0"+ str(datetime.now().minute)
        second = str(datetime.now().second) if datetime.now().second > 9 else  "0"+ str(datetime.now().second)

        what_time = hour+minute+second
        file_name = prefix +"-"+ year +"-"+ week + "-"+ today_date+ "-"+ what_time+"."+extension
        return file_name


    def _create_empty_area_file(self, tmp_path):
        self._create_empty_n_file(tmp_path, "area", "txt")

    def _create_empty_interco_file(self, tmp_path):
        self._create_empty_n_file(tmp_path, "interco", "txt")

    def _create_empty_n_file(self, tmp_path, prefix, extension):

        TestProblemGeneratorDriver.number +=1
        fname = prefix+ str(TestProblemGeneratorDriver.number) +"."+ extension
        file = tmp_path / fname
        file.write_text("")

    @staticmethod
    def _get_options_from_settings_inifile(settings_ini_filepath):
        with open(settings_ini_filepath, 'r') as file_l:
            options = dict(
                {line.strip().split('=')[0].strip(): line.strip().split('=')[1].strip()
                 for line in file_l.readlines() if line.strip()})
        return options
    
    @staticmethod
    def _get_additional_constraints_from_settings(settings_ini_filepath):
        options = TestProblemGeneratorDriver._get_options_from_settings_inifile(settings_ini_filepath)
        return options.get("additional-constraints", "")
    
    @staticmethod
    def _get_weight_file_name(settings_ini_filepath):
        options =  TestProblemGeneratorDriver._get_options_from_settings_inifile(settings_ini_filepath)
        return options.get('yearly-weights', "")
        
    @staticmethod
    def _get_weights_file_path(study_dir):
        """
            returns the path to a yearly-weights file

            :return: path to input yearly-weights file
        """
        settings_ini_filepath = Path(study_dir) / "user" / "expansion" / "settings.ini"
        yearly_weights_filename = TestProblemGeneratorDriver._get_weight_file_name(settings_ini_filepath)
        if yearly_weights_filename:
            return Path(study_dir) / "user" / "expansion" / yearly_weights_filename
        else:
            return ""

    @staticmethod
    def _get_nb_active_years(study_dir):
        general_data = Path(study_dir) / "settings" / "generaldata.ini"
        return GeneralDataIniReader(Path(general_data)).get_nb_activated_year()

    @staticmethod
    def _get_last_simulation_name(antares_output):
        """
            return last simulation name    
        """
        # Get list of all dirs only in the given directory
        list_of_dirs_filter = filter( lambda x: os.path.isdir(os.path.join(antares_output, x)),
                                os.listdir(antares_output) )
        # Sort list of files based on last modification time in ascending order
        list_of_dirs = sorted( list_of_dirs_filter,
                    key = lambda x: os.path.getmtime(os.path.join(antares_output, x))
                    )
        return list_of_dirs[-1]
    
    @staticmethod
    def _is_relaxed(settings_ini_filepath):
        """
            indicates if method to use is relaxed by reading the relaxation_type
            from the settings file
        """
        options =  TestProblemGeneratorDriver._get_options_from_settings_inifile(settings_ini_filepath)
        relaxation_type = options.get('master', "integer")
        assert relaxation_type in ['integer', 'relaxed', 'full_integer']
        return relaxation_type == 'relaxed'

    @staticmethod
    def run_antares_step_in_examples(example_name):
        project_dir = Path(os.path.abspath(__file__)).parent.parent.parent
        project_dir_to_additionnal_constraints = Path("examples") / example_name
        study_dir =   project_dir / project_dir_to_additionnal_constraints
        TestProblemGeneratorDriver._launch_antares_step(study_dir)
        return study_dir

    @staticmethod
    def _launch_antares_step(study_dir):
        # Running Antares Driver
        antares_driver = AntaresDriver(get_antares_solver_path())
        antares_driver.launch(study_dir, 1)

    @staticmethod
    def _get_problem_generator_for_study(study_dir):

        settings_dir = study_dir / "user" / "expansion"/ "settings.ini"
        addi_constraints = TestProblemGeneratorDriver._get_additional_constraints_from_settings(settings_dir)
        file_path =  TestProblemGeneratorDriver._get_weights_file_path(study_dir)
        file_name = TestProblemGeneratorDriver._get_weight_file_name(settings_dir)
        nb_act_yrs = TestProblemGeneratorDriver._get_nb_active_years(study_dir)
        pblm_gen_data = ProblemGeneratorData(LP_NAMER=get_lp_namer_exe(),
                                                      keep_mps = False,
                                                      additional_constraints=addi_constraints,
                                                      weights_file_path=file_path,
                                                      weight_file_name=file_name,
                                                      install_dir= get_install_dir(),
                                                      nb_active_years = nb_act_yrs)

        problem_generator_driver = ProblemGeneratorDriver(pblm_gen_data)
        antares_output_dir = study_dir / "output"
        antares_simulation = TestProblemGeneratorDriver._get_last_simulation_name(antares_output_dir)
        simu_abs_path = Path(os.path.normpath(os.path.join(antares_output_dir, antares_simulation)))
        is_relaxed = TestProblemGeneratorDriver._is_relaxed(settings_dir)

        return (problem_generator_driver, simu_abs_path, is_relaxed)
