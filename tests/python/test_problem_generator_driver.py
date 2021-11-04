from typing import Text
import pytest
from pathlib import Path
from datetime import date, datetime


from antares_xpansion.problem_generator_driver import ProblemGeneratorData, ProblemGeneratorDriver, ProblemGeneratorException

class TestProblemGeneratorDriver:
    number = 0
    def setup_method(self):
        self.empty_pblm_gen_data = ProblemGeneratorData(LP_NAMER="",
                                                      keep_mps = False,
                                                      additional_constraints="",
                                                      weights_file_path=Path(""),
                                                      weight_file_name="",
                                                      install_dir=Path(""))

    def test_problem_generator_data(self):


        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)

        assert problem_generator_driver.LP_NAMER == self.empty_pblm_gen_data.LP_NAMER                                              
        assert problem_generator_driver.keep_mps == self.empty_pblm_gen_data.keep_mps                                              
        assert problem_generator_driver.additional_constraints == self.empty_pblm_gen_data.additional_constraints                                              
        assert problem_generator_driver.weights_file_path == self.empty_pblm_gen_data.weights_file_path                                              
        assert problem_generator_driver.install_dir == self.empty_pblm_gen_data.install_dir                                              

    def test_output_path(self, tmp_path):
        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorException.OutputPathError):
            problem_generator_driver.launch(tmp_path/ "i_don_t_exist", False)

    def test_no_area_file(self, tmp_path):
    
        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorException.AreaFileException):
            problem_generator_driver.launch(tmp_path, False)

    def test_more_than_1_area_file(self, tmp_path):
    
        self._create_empty_area_file(tmp_path)
        self._create_empty_area_file(tmp_path)


        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorException.AreaFileException):
            problem_generator_driver.launch(tmp_path, False)


    def test_no_interco_file(self, tmp_path):
    
        self._create_empty_area_file(tmp_path)
        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorException.IntercoFilesException):
            problem_generator_driver.launch(tmp_path, False)

    def test_more_than_1_interco_file(self, tmp_path):
    
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorException.IntercoFilesException):
            problem_generator_driver.launch(tmp_path, False)


    def test_lp_namer_exe_not_exit(self, tmp_path):
    
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorException.LPNamerExeError):
            problem_generator_driver.launch(tmp_path, False)


    def test_mps_txt_creation(self, tmp_path):
    
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorException.LPNamerExeError):
            problem_generator_driver.launch(tmp_path, False)

        
        mps_txt_file = tmp_path / "mps.txt"
        assert  mps_txt_file.exists()
   

    def test_mps_txt_content(self, tmp_path):
    
        expected_results = self._get_expected_mps_txt(tmp_path)
        self._create_empty_area_file(tmp_path)
        self._create_empty_interco_file(tmp_path)

        problem_generator_driver = ProblemGeneratorDriver(self.empty_pblm_gen_data)
        with pytest.raises(ProblemGeneratorException.LPNamerExeError):
            problem_generator_driver.launch(tmp_path, False)

        
        mps_txt_file = tmp_path / problem_generator_driver.MPS_TXT
        assert  mps_txt_file.exists()

        with open(mps_txt_file) as mps_txt :
            for line in mps_txt :
                my_list = line.strip().split(" ")
                assert my_list in expected_results

        
   

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