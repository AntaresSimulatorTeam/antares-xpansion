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
    def _create_candidate_content(section: int, name: str, area1: str, area2: str) -> str:
        content = f"[{section}]\n" \
                  f"name = {name}\n" \
                  f"link = {area1} - {area2}\n"
        return content

    @staticmethod
    def _create_direct_link_profile_content(link_profile: str) -> str:
        content = ""
        if link_profile:
            content = f"direct-link-profile = {link_profile}\n"
        return content

    @staticmethod
    def _create_indirect_link_profile_content(link_profile: str) -> str:
        content = ""
        if link_profile:
            content = f"indirect-link-profile = {link_profile}\n"
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
    def _create_already_installed_direct_link_profile_content(already_installed_link_profile: str) -> str:
        content = ""
        if already_installed_link_profile:
            content = f"already-installed-direct-link-profile = {already_installed_link_profile}\n"
        return content

    @staticmethod
    def _create_already_installed_indirect_link_profile_content(already_installed_link_profile: str) -> str:
        content = ""
        if already_installed_link_profile:
            content = f"already-installed-indirect-link-profile = {already_installed_link_profile}\n"
        return content

    @staticmethod
    def _create_link_profile_file(file_path, array):
        expected_array: np.array = array
        np.savetxt(file_path, array, delimiter='\t')
        return expected_array

    def test_empty_reader(self):
        with pytest.raises(IniFileNotFound):
            CandidatesReader()

    def create_and_test_link_profile(self, study_path):
        print("n")

    def test_read_candidates(self, tmp_path):
        candidates = [
            {"name": "semibase", "area1": "area1", "area2": "semibase",
             "direct-link-profile": "", "indirect-link-profile": "",
             "already-installed-capacity": "", "already-installed-direct-link-profile": "",
             "already-installed-indirect-link-profile": ""},
            {"name": "peak", "area1": "area1", "area2": "peak",
             "direct-link-profile": "direct_capa_pv.ini", "indirect-link-profile": "indirect_capa_pv.ini",
             "already-installed-capacity": "", "already-installed-direct-link-profile": "",
             "already-installed-indirect-link-profile": ""},
            {"name": "peak2", "area1": "area1", "area2": "peak",
             "direct-link-profile": "direct_capa_pv.ini", "indirect-link-profile": "indirect_capa_pv.ini",
             "already-installed-capacity": 1000.0, "already-installed-direct-link-profile": "",
             "already-installed-indirect-link-profile": ""},
            {"name": "peak3", "area1": "area1", "area2": "peak",
             "direct-link-profile": "direct_capa_pv.ini", "indirect-link-profile": "indirect_capa_pv.ini",
             "already-installed-capacity": "",
             "already-installed-direct-link-profile": "direct-link-profile-one-col.txt",
             "already-installed-indirect-link-profile": "indirect-link-profile-one-col.txt",
             "link-profile-array": np.array([1.0, 2.0, 3.0]),
             "installed-direct-link-profile-array": [1.0, 2.0, 3.0]},
            {"name": "peak4", "area1": "area1", "area2": "peak",
             "direct-link-profile": "direct_capa_pv.ini", "indirect-link-profile": "indirect_capa_pv.ini",
             "already-installed-capacity": "",
             "already-installed-direct-link-profile": "direct-link-profile-two-col.txt",
             "already-installed-indirect-link-profile": "indirect-link-profile-two-col.txt",
             "direct-link-profile-array": [1.0, 2.0, 3.0], "indirect-link-profile-array": [0.0, 1.0, 2.0],
             "already-installed-direct-link-profile-array": [1.0, 2.0, 3.0],
             "already-installed-indirect-link-profile-array": [4.0, 5.0, 3.0]},
        ]

        links = [
            {"name": "area1 - semibase", "candidates": ["semibase"], "area1": "area1", "area2": "semibase"},
            {"name": "area1 - peak", "candidates": ["peak", "peak2", "peak3", "peak4"], "area1": "area1",
             "area2": "peak"}
        ]

        index = 0
        content = ""
        candidate_name_list = []
        for candidate in candidates:
            candidate_name_list.append(candidate["name"])
            content += self._create_candidate_content(index, candidate["name"], candidate["area1"], candidate["area2"])
            content += self._create_direct_link_profile_content(candidate["direct-link-profile"])
            content += self._create_indirect_link_profile_content(candidate["indirect-link-profile"])
            content += self._create_already_installed_capacity_content(candidate["already-installed-capacity"])
            content += self._create_already_installed_direct_link_profile_content(
                candidate["already-installed-direct-link-profile"])
            content += self._create_already_installed_indirect_link_profile_content(
                candidate["already-installed-indirect-link-profile"])
            index += 1

        link_name_list = []
        for link in links:
            link_name_list.append(link["name"])

        self._create_reader(tmp_path, content)

        assert self.ini_reader.get_link_list() == link_name_list
        assert self.ini_reader.get_candidates_list() == candidate_name_list

        study_path = Path(tmp_path)

        for link in links:
            link_name = link["name"]
            assert self.ini_reader._get_link_areas(link_name) == (link["area1"], link["area2"])
            assert self.ini_reader.get_link_candidate(link_name) == link["candidates"]
            assert self.ini_reader.get_link_antares_link_file(study_path, link_name) == study_path / "input" / "links" / \
                   link["area1"] / str(link["area2"] + ".txt")

        for candidate in candidates:
            candidate_name = candidate["name"]

            assert self.ini_reader.get_candidate_antares_link_file(study_path,
                                                                   candidate_name) == study_path / "input" / "links" / \
                   candidate["area1"] / str(candidate["area2"] + ".txt")

            direct_link_profile = candidate["direct-link-profile"]
            expected_profile_array = [[], []]
            if direct_link_profile:

                os.makedirs(study_path / "user" / "expansion" / "capa", exist_ok=True)
                expected_profile_path = study_path / "user" / "expansion" / "capa" / direct_link_profile
                assert Path(
                    self.ini_reader.get_candidate_link_profile(study_path, candidate_name)[0]) == expected_profile_path

                if "direct-link-profile-array" in candidate:
                    expected_profile_array[0] = self._create_link_profile_file(expected_profile_path,
                                                                               candidate["direct-link-profile-array"])
                else:
                    array = [1 for k in range(8760)]
                    expected_profile_array[0] = self._create_link_profile_file(expected_profile_path, array)
            else:
                assert not self.ini_reader.get_candidate_link_profile(study_path, candidate_name)[0]

            indirect_link_profile = candidate["indirect-link-profile"]
            if indirect_link_profile:

                os.makedirs(study_path / "user" / "expansion" / "capa", exist_ok=True)
                expected_profile_path = study_path / "user" / "expansion" / "capa" / indirect_link_profile
                assert Path(
                    self.ini_reader.get_candidate_link_profile(study_path, candidate_name)[1]) == expected_profile_path

                if "indirect-link-profile-array" in candidate:
                    expected_profile_array[1] = self._create_link_profile_file(expected_profile_path,
                                                                               candidate["indirect-link-profile-array"])
                else:
                    array = [1 for k in range(8760)]
                    expected_profile_array[1] = self._create_link_profile_file(expected_profile_path, array)
            else:
                assert not self.ini_reader.get_candidate_link_profile(study_path, candidate_name)[1]

            if direct_link_profile and indirect_link_profile:
                profile_array = self.ini_reader.get_candidate_link_profile_array(study_path, candidate_name)
                row, col = profile_array.shape
                assert col == 2
                np.testing.assert_allclose(profile_array[:, 0], expected_profile_array[0], rtol=1e-4, verbose=True)
                np.testing.assert_allclose(profile_array[:, 1], expected_profile_array[1], rtol=1e-4, verbose=True)

            already_install_capacity = candidate["already-installed-capacity"]
            if already_install_capacity:
                assert self.ini_reader.get_candidate_already_install_capacity(
                    candidate_name) == already_install_capacity
            else:
                assert self.ini_reader.get_candidate_already_install_capacity(candidate_name) == 0.0

            already_install_link_profile = candidate["already-installed-direct-link-profile"]
            if already_install_link_profile:
                expected_profile_path = study_path / "user" / "expansion" / "capa" / already_install_link_profile
                assert Path(self.ini_reader.get_candidate_already_install_direct_link_profile(study_path,
                                                                                              candidate_name)) == expected_profile_path
                expected_profile_array = []
                if "already-installed-direct-link-profile-array" in candidate:
                    expected_profile_array = self._create_link_profile_file(expected_profile_path, candidate[
                        "already-installed-direct-link-profile-array"])
                else:
                    array = [1 for k in range(8760)]
                    expected_profile_array = self._create_link_profile_file(expected_profile_path, array)
                profile_array = self.ini_reader.get_candidate_already_installed_direct_link_profile_array(study_path,
                                                                                                          candidate_name)
                np.testing.assert_allclose(profile_array, expected_profile_array, rtol=1e-4, verbose=True)
            else:
                assert not self.ini_reader.get_candidate_already_install_direct_link_profile(study_path, candidate_name)

            already_install_link_profile = candidate["already-installed-indirect-link-profile"]
            if already_install_link_profile:
                expected_profile_path = study_path / "user" / "expansion" / "capa" / already_install_link_profile
                assert Path(self.ini_reader.get_candidate_already_install_indirect_link_profile(study_path,
                                                                                                candidate_name)) == study_path / "user" / "expansion" / "capa" / already_install_link_profile
                expected_profile_array = []
                if "already-installed-indirect-link-profile-array" in candidate:
                    expected_profile_array = self._create_link_profile_file(expected_profile_path, candidate[
                        "already-installed-indirect-link-profile-array"])
                else:
                    array = [1 for k in range(8760)]
                    expected_profile_array = self._create_link_profile_file(expected_profile_path, array)
                profile_array = self.ini_reader.get_candidate_already_installed_indirect_link_profile_array(study_path,
                                                                                                            candidate_name)
                np.testing.assert_allclose(profile_array, expected_profile_array, rtol=1e-4, verbose=True)
            else:
                assert not self.ini_reader.get_candidate_already_install_indirect_link_profile(study_path,
                                                                                               candidate_name)

    def test_installed_link_profile_array(self, tmp_path):
        study_path = Path(tmp_path)
        candidate = {"name": "peak4", "area1": "area1", "area2": "peak",
                     "direct-link-profile": "direct_capa_pv.ini", "indirect-link-profile": "indirect_capa_pv.ini",
                     "already-installed-capacity": "",
                     "already-installed-direct-link-profile": "direct-link-profile-two-col.txt",
                     "already-installed-indirect-link-profile": "indirect-link-profile-two-col.txt",
                     "direct-link-profile-array": [1.0, 2.0, 3.0], "indirect-link-profile-array": [0.0, 1.0, 2.0],
                     "already-installed-direct-link-profile-array": [1.0, 2.0, 3.0],
                     "already-installed-indirect-link-profile-array": [4.0, 5.0, 3.0]}
        content = ""
        content += self._create_candidate_content(0, candidate["name"], candidate["area1"], candidate["area2"])
        content += self._create_direct_link_profile_content(candidate["direct-link-profile"])
        content += self._create_indirect_link_profile_content(candidate["indirect-link-profile"])
        content += self._create_already_installed_capacity_content(candidate["already-installed-capacity"])
        content += self._create_already_installed_direct_link_profile_content(
            candidate["already-installed-direct-link-profile"])
        content += self._create_already_installed_indirect_link_profile_content(
            candidate["already-installed-indirect-link-profile"])

        self._create_reader(tmp_path, content)
        os.makedirs(study_path / "user" / "expansion" / "capa", exist_ok=True)

        already_install_direct_link_profile = candidate["already-installed-direct-link-profile"]
        profile_path = study_path / "user" / "expansion" / "capa" / already_install_direct_link_profile
        self._create_link_profile_file(profile_path, candidate[
            "already-installed-direct-link-profile-array"])
        already_install_indirect_link_profile = candidate["already-installed-indirect-link-profile"]
        profile_path = study_path / "user" / "expansion" / "capa" / already_install_indirect_link_profile
        self._create_link_profile_file(profile_path, candidate[
            "already-installed-indirect-link-profile-array"])




        expected_array = np.array([[1.0, 2.0, 3.0], [4.0, 5.0, 3.0]]).transpose()
        actual_array = self.ini_reader.get_candidate_already_installed_link_profile_array_new(study_path,
                                                                                                   candidate["name"])
        np.testing.assert_allclose(expected_array, actual_array, rtol=1e-4, verbose=True)
