from configparser import ConfigParser
from pathlib import Path
from typing import List, Tuple

import numpy as np

from .flushed_print import flushed_print


class IniFileNotFound(Exception):
    pass


class CandidateNotFound(Exception):
    pass


class ProfilesOfDifferentDimensions(Exception):
    pass


class ProfilesValueError(Exception):
    pass


class CandidatesReader:
    def __init__(self, file_path: Path = None):
        if not file_path or not file_path.is_file():
            raise IniFileNotFound

        self.config = ConfigParser(strict=True)
        self.config.read(file_path)

        self.candidates_map = {}
        for section in self.config.sections():
            self.candidates_map[self.config[section]["name"]] = section

    def get_link_list(self) -> List[str]:
        result = []
        for section in self.config.sections():
            link_name = self.config[section]["link"]
            if result.count(link_name) == 0:
                result.append(link_name)
        return result

    def get_link_candidate(self, link: str) -> List[str]:
        result = []
        for candidate in self.get_candidates_list():
            if self.get_candidate_link_name(candidate) == link:
                result.append(candidate)
        return result

    def get_candidates_list(self) -> List[str]:
        return list(self.candidates_map.keys())

    def _get_candidate_index(self, candidate: str):
        if candidate not in self.get_candidates_list():
            flushed_print(
                f"Candidate {candidate} not found in candidate list.")
            raise CandidateNotFound
        return self.candidates_map[candidate]

    def get_candidate_link_name(self, candidate: str):
        index = self._get_candidate_index(candidate)
        return self.config.get(index, "link")

    def get_candidate_areas(self, candidate: str):
        return self._get_link_areas(self.get_candidate_link_name(candidate))

    def get_candidate_already_install_capacity(self, candidate: str):
        index = self._get_candidate_index(candidate)
        return self.config.getfloat(index, "already-installed-capacity", fallback=0.0)

    def get_candidate_already_install_direct_link_profile(
        self, study_path: Path, candidate: str
    ):
        index = self._get_candidate_index(candidate)
        link_profile_path = ""
        if self.config.has_option(index, "already-installed-direct-link-profile"):
            link_profile_path = str(
                study_path
                / "user"
                / "expansion"
                / "capa"
                / self.config[index]["already-installed-direct-link-profile"]
            )
        return link_profile_path

    def get_candidate_already_install_indirect_link_profile(
        self, study_path: Path, candidate: str
    ):
        index = self._get_candidate_index(candidate)
        link_profile_path = ""
        if self.config.has_option(index, "already-installed-indirect-link-profile"):
            link_profile_path = str(
                study_path
                / "user"
                / "expansion"
                / "capa"
                / self.config[index]["already-installed-indirect-link-profile"]
            )
        return link_profile_path

    def get_candidate_direct_link_profile(self, study_path: Path, candidate: str):
        index = self._get_candidate_index(candidate)
        link_profile_path = ""
        if self.config.has_option(index, "direct-link-profile"):
            link_profile_path = str(
                study_path
                / "user"
                / "expansion"
                / "capa"
                / self.config[index]["direct-link-profile"]
            )
        return link_profile_path

    def get_candidate_indirect_link_profile(self, study_path: Path, candidate: str):
        index = self._get_candidate_index(candidate)
        link_profile_path = ""
        if self.config.has_option(index, "indirect-link-profile"):
            link_profile_path = str(
                study_path
                / "user"
                / "expansion"
                / "capa"
                / self.config[index]["indirect-link-profile"]
            )
        return link_profile_path

    def get_link_antares_link_file_pre820(self, study_path: Path, link: str) -> Path:
        area1, area2 = self._get_link_areas(link)
        return study_path / "input" / "links" / area1 / str(area2 + ".txt")

    def get_candidate_antares_direct_link_file(
        self, study_path: Path, candidate: str
    ) -> Path:
        area1, area2 = self.get_candidate_areas(candidate)
        return (
            study_path
            / "input"
            / "links"
            / area1
            / "capacities"
            / str(area2 + "_direct.txt")
        )

    def get_candidate_antares_direct_link_array(self, study_path: Path, candidate: str):
        link_file = self.get_candidate_antares_direct_link_file(
            study_path, candidate)
        return self._read_or_create_link_profile_array_simple(link_file)

    def get_candidate_antares_indirect_link_array(
        self, study_path: Path, candidate: str
    ):
        link_file = self.get_candidate_antares_indirect_link_file(
            study_path, candidate)
        return self._read_or_create_link_profile_array_simple(link_file)

    def get_candidate_antares_indirect_link_file(
        self, study_path: Path, candidate: str
    ) -> Path:
        area1, area2 = self.get_candidate_areas(candidate)
        return (
            study_path
            / "input"
            / "links"
            / area1
            / "capacities"
            / str(area2 + "_indirect.txt")
        )

    def get_candidate_link_profile(
        self, study_path: Path, candidate: str
    ) -> Tuple[str, str]:
        index = self._get_candidate_index(candidate)
        direct_link_profile_path = ""
        indirect_link_profile_path = ""
        if self.config.has_option(index, "direct-link-profile"):
            direct_link_profile_path = str(
                study_path
                / "user"
                / "expansion"
                / "capa"
                / self.config[index]["direct-link-profile"]
            )
            indirect_link_profile_path = str(
                study_path
                / "user"
                / "expansion"
                / "capa"
                / self.config[index]["indirect-link-profile"]
            )
        return direct_link_profile_path, indirect_link_profile_path

    @staticmethod
    def _get_link_areas(link: str):
        areas = link.split("-")
        area1 = areas[0].strip()
        area2 = areas[1].strip()
        return area1, area2

    @staticmethod
    def check_nan_in_profile_link_array(link_profile_array, file: str):
        nan_indices = np.argwhere(np.isnan(link_profile_array))
        if len(nan_indices) > 0:
            msg = f"Value(s) Error detected in file {file} at (row, column):\n"
            for index in nan_indices:
                msg = msg + f"({index[0]+1}, {index[1]+1})\n"
            raise ProfilesValueError(msg)

    @staticmethod
    def _read_or_create_link_profile_array_simple(file: str):
        link_profile_array = np.ones(8760)
        if file:
            link_profile_array = np.genfromtxt(file)
            CandidatesReader.check_nan_in_profile_link_array(
                link_profile_array, file)

        return link_profile_array

    def get_candidate_link_profile_array(self, study_path: Path, candidate: str):
        return self.get_candidate_link_profile_array_new(study_path, candidate)

    def get_candidate_link_profile_array_new(self, study_path: Path, candidate: str):
        direct_link_profile = self.get_candidate_direct_link_profile_array(
            study_path, candidate
        )
        indirect_link_profile = self.get_candidate_indirect_link_profile_array(
            study_path, candidate
        )
        if direct_link_profile.shape != indirect_link_profile.shape:
            flushed_print(
                f"For candidate {candidate}, shape of direct link profile {direct_link_profile.shape} does not match shape of indirect link profile {indirect_link_profile.shape}"
            )
            raise ProfilesOfDifferentDimensions
        link_profile_array = np.array(
            [direct_link_profile, indirect_link_profile]
        ).transpose()
        if not self.get_candidate_direct_link_profile(
            study_path, candidate
        ) or not self.get_candidate_indirect_link_profile(study_path, candidate):
            return False, link_profile_array
        return True, link_profile_array

    def get_candidate_already_install_link_profile(
        self, study_path: Path, candidate: str
    ):
        index = self._get_candidate_index(candidate)
        link_profile_path = ""
        if self.config.has_option(index, "already-installed-link-profile"):
            link_profile_path = str(
                study_path
                / "user"
                / "expansion"
                / "capa"
                / self.config[index]["already-installed-link-profile"]
            )
        return link_profile_path

    def get_candidate_already_installed_link_profile_array(
        self, study_path: Path, candidate: str
    ):
        return self.get_candidate_already_installed_link_profile_array_new(
            study_path, candidate
        )

    def get_candidate_already_installed_link_profile_array_new(
        self, study_path: Path, candidate: str
    ):
        direct_link_profile = (
            self.get_candidate_already_installed_direct_link_profile_array(
                study_path, candidate
            )
        )
        indirect_link_profile = (
            self.get_candidate_already_installed_indirect_link_profile_array(
                study_path, candidate
            )
        )
        if direct_link_profile.shape != indirect_link_profile.shape:
            flushed_print(
                f"For candidate {candidate}, shape of already installed direct link profile {direct_link_profile.shape} does not match shape of already installed indirect link profile {indirect_link_profile.shape}"
            )
            raise ProfilesOfDifferentDimensions
        link_profile_array = np.array(
            [direct_link_profile, indirect_link_profile]
        ).transpose()
        return link_profile_array

    def get_candidate_already_installed_direct_link_profile_array(
        self, study_path: Path, candidate: str
    ):
        link_profile = self.get_candidate_already_install_direct_link_profile(
            study_path, candidate
        )
        return self._read_or_create_link_profile_array_simple(link_profile)

    def get_candidate_already_installed_indirect_link_profile_array(
        self, study_path: Path, candidate: str
    ):
        link_profile = self.get_candidate_already_install_indirect_link_profile(
            study_path, candidate
        )
        return self._read_or_create_link_profile_array_simple(link_profile)

    def get_candidate_direct_link_profile_array(self, study_path: Path, candidate: str):
        link_profile = self.get_candidate_direct_link_profile(
            study_path, candidate)
        return self._read_or_create_link_profile_array_simple(link_profile)

    def get_candidate_indirect_link_profile_array(
        self, study_path: Path, candidate: str
    ):
        link_profile = self.get_candidate_indirect_link_profile(
            study_path, candidate)
        return self._read_or_create_link_profile_array_simple(link_profile)

    def get_link_antares_direct_link_file(self, study_path, link_name):
        return self.link_path_antares_link_file(link_name, study_path, True)

    def link_path_antares_link_file(self, link_name, study_path, is_direct: bool):
        link_path = self.get_link_antares_link_file_pre820(
            study_path, link_name)
        link_path_component = link_path.parts
        link_to_name = link_path_component[-1]
        link_path_until_from = link_path_component[0:-1]
        if is_direct:
            suffix = "_direct."
        else:
            suffix = "_indirect."
        updated_link_to = (
            link_to_name.split(".")[0] + suffix + link_to_name.split(".")[1]
        )
        new_path = Path(*link_path_until_from) / "capacities" / updated_link_to
        return new_path

    def get_link_antares_indirect_link_file(self, study_path, link_name):
        return self.link_path_antares_link_file(link_name, study_path, False)

    def has_profile(self, study_path, candidate):
        link_profile = self.get_candidate_direct_link_profile(
            study_path, candidate)
        indirect_profile = self.get_candidate_indirect_link_profile(
            study_path, candidate
        )
        return link_profile and indirect_profile

    def has_installed_profile(self, study_path, candidate):
        link_profile = self.get_candidate_already_install_direct_link_profile(
            study_path, candidate
        )
        indirect_link_profile = (
            self.get_candidate_already_install_indirect_link_profile(
                study_path, candidate
            )
        )
        return link_profile and indirect_link_profile
