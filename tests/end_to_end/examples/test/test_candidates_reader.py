import os
from pathlib import Path
import numpy as np
import pytest

from ..candidates_reader import CandidatesReader, IniFileNotFound, CandidateNotFound


class TestCandidateReader:

    def _create_reader(self, tmp_path, content):
        file_path = tmp_path / 'test.ini'
        file_path.write_text(content)
        self.ini_reader = CandidatesReader(file_path)

    @staticmethod
    def _create_candidate_content(section: int, name : str , area1 : str , area2 : str) -> str:
        content = f"[{section}]\n" \
                  f"name = {name}\n" \
                  f"link = {area1} - {area2}\n"
        return content

    @staticmethod
    def _create_link_profile_content(link_profile : str) -> str:
        content = ""
        if link_profile:
            content = "has-link-profile = True\n" \
                      f"link-profile = {link_profile}\n"
        return content

    @staticmethod
    def _create_already_installed_capacity_content(already_installed_capacity: float) -> str:
        content = ""
        if already_installed_capacity:
            content = f"already-installed-capacity = {already_installed_capacity}\n"
        return content

    @staticmethod
    def _create_already_installed_link_profile_content(already_installed_link_profile: str) -> str:
        content = ""
        if already_installed_link_profile:
            content = f"already-installed-link-profile = {already_installed_link_profile}\n"
        return content

    @staticmethod
    def _create_link_profile_file(file_path, array):
        expected_array : np.array = array
        np.savetxt(file_path, array, delimiter='\t')
        if expected_array.ndim == 1:
            expected_array= np.c_[expected_array, expected_array]
        return expected_array

    def test_empty_reader(self):
        with pytest.raises(IniFileNotFound):
            CandidatesReader()

    def create_and_test_link_profile(self, study_path):
        print("n")

    def test_read_candidates(self, tmp_path):
        candidates = [
            {"name" : "semibase", "area1" : "area1", "area2" : "semibase", "link-profile" : "", "already-installed-capacity" : "" , "already-installed-link-profile" : ""},
            {"name" : "peak", "area1" : "area1", "area2" : "peak", "link-profile" : "capa_pv.ini", "already-installed-capacity" : "" , "already-installed-link-profile" : "", },
            {"name" : "peak2", "area1" : "area1", "area2" : "peak", "link-profile" : "capa_pv.ini", "already-installed-capacity" : 1000.0 , "already-installed-link-profile" : ""},
            {"name" : "peak3", "area1" : "area1", "area2" : "peak", "link-profile" : "capa_pv.ini", "already-installed-capacity" : "" , "already-installed-link-profile" : "link-profile-one-col.txt",
             "link-profile-array" : np.array([1.0, 2.0, 3.0])},
            {"name" : "peak4", "area1" : "area1", "area2" : "peak", "link-profile" : "capa_pv.ini", "already-installed-capacity" : "" , "already-installed-link-profile" : "link-profile-two-col.txt",
             "link-profile-array" : np.array([[1.0, 2.0, 3.0],[0.0, 1.0, 2.0]])}
        ]

        index = 0
        content = ""
        candidate_name_list = []
        for candidate in candidates:
            candidate_name_list.append(candidate["name"])
            content += self._create_candidate_content(index,candidate["name"],candidate["area1"],candidate["area2"])
            content += self._create_link_profile_content(candidate["link-profile"])
            content += self._create_already_installed_capacity_content(candidate["already-installed-capacity"])
            content += self._create_already_installed_link_profile_content(candidate["already-installed-link-profile"])
            index +=1

        self._create_reader(tmp_path, content)

        assert self.ini_reader.get_candidates_list() == candidate_name_list

        study_path = Path(tmp_path)

        for candidate in candidates:
            candidate_name = candidate["name"]

            assert self.ini_reader.get_candidate_antares_link_file(study_path, candidate_name ) == study_path / "input"/ "links" / candidate["area1"] / str(candidate["area2"] + ".txt")

            link_profile = candidate["link-profile"]
            if link_profile:

                os.makedirs(study_path / "user" / "expansion" / "capa", exist_ok=True)
                expected_profile_path = study_path / "user" / "expansion" / "capa" / link_profile
                assert Path(self.ini_reader.get_candidate_link_profile(study_path, candidate_name)) == expected_profile_path

                if "link-profile-array" in candidate:
                    expected_profile_array = self._create_link_profile_file(expected_profile_path, candidate["link-profile-array"].T)
                else:
                    array = np.ones((8760, 2))
                    expected_profile_array = self._create_link_profile_file(expected_profile_path, array)

                profile_array = self.ini_reader.get_candidate_link_profile_array(study_path, candidate_name)
                row, col = profile_array.shape
                assert col == 2
                np.testing.assert_allclose(profile_array, expected_profile_array, rtol=1e-4, verbose=True)

            else:
                assert not self.ini_reader.get_candidate_link_profile(study_path, candidate_name)

            already_install_capacity = candidate["already-installed-capacity"]
            if already_install_capacity:
                assert self.ini_reader.get_candidate_already_install_capacity(candidate_name) == already_install_capacity
            else:
                assert self.ini_reader.get_candidate_already_install_capacity(candidate_name) == 0.0

            already_install_link_profile = candidate["already-installed-link-profile"]
            if already_install_link_profile:
                assert Path(self.ini_reader.get_candidate_already_install_link_profile(study_path,candidate_name)) == study_path / "user" / "expansion" / "capa" / already_install_link_profile
            else:
                assert not self.ini_reader.get_candidate_already_install_link_profile(study_path, candidate_name)

