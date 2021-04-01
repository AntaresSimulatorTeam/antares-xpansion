from configparser import ConfigParser
from pathlib import Path

from typing import List

import numpy as np

class IniFileNotFound(Exception):
    pass


class CandidateNotFound(Exception):
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

    def get_candidates_list(self) -> List[str]:
        return list(self.candidates_map.keys())

    def _get_candidate_index(self, candidate : str):
        if not candidate in self.get_candidates_list():
            raise CandidateNotFound
        return self.candidates_map[candidate]

    def get_candidate_areas(self, candidate : str):
        index = self._get_candidate_index(candidate)
        areas = self.config[index]["link"].split("-")
        area1 = areas[0].strip()
        area2 = areas[1].strip()
        return area1, area2

    def get_candidate_already_install_capacity(self, candidate : str):
        index = self._get_candidate_index(candidate)
        return self.config.getfloat(index, "already-installed-capacity" , fallback=0.0)

    def get_candidate_already_install_link_profile(self, study_path: Path, candidate : str):
        index = self._get_candidate_index(candidate)
        link_profile_path = ""
        if self.config.getboolean(index, "has-link-profile", fallback=False) and self.config.has_option(index, "already-installed-link-profile"):
            link_profile_path = str(study_path / "user" / "expansion" / "capa" / self.config[index]["already-installed-link-profile"])
        return link_profile_path

    def get_candidate_antares_link_file(self, study_path: Path, candidate: str) -> Path:
        area1, area2 = self.get_candidate_areas(candidate)
        return study_path / "input" / "links" / area1 / str(area2 + ".txt")

    def get_candidate_link_profile(self, study_path: Path, candidate : str) -> str:
        index = self._get_candidate_index(candidate)
        link_profile_path =""
        if self.config.getboolean(index,"has-link-profile", fallback=False) :
            link_profile_path = str(study_path / "user" / "expansion" / "capa" / self.config[index]["link-profile"])
        return link_profile_path

    @staticmethod
    def _read_or_create_link_profile_array(file : str):
        link_profile_array = np.ones((8760, 2))
        if file:
            link_profile_array = np.loadtxt(file, delimiter="\t")
            if link_profile_array.ndim == 1:
                link_profile_array = np.c_[link_profile_array, link_profile_array]
        return link_profile_array

    def get_candidate_link_profile_array(self, study_path: Path, candidate : str):
        link_profile = self.get_candidate_link_profile(study_path,candidate)
        return self._read_or_create_link_profile_array(link_profile)

    def get_candidate_already_installed_link_profile_array(self, study_path: Path, candidate : str):
        link_profile = self.get_candidate_already_install_link_profile(study_path,candidate)
        return self._read_or_create_link_profile_array(link_profile)



