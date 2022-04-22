

import configparser
from pathlib import Path

from antares_xpansion.split_link_profile import SplitLinkProfile


class CandidateUpdater:
    def __init__(self, candidate_file: Path) -> None:
        self.LINK_PROFILE_KEY = "link-profile"
        self.DIRECT_LINK_PROFILE_KEY = "direct-link-profile"
        self.INDIRECT_LINK_PROFILE_KEY = "indirect-link-profile"

        if (not candidate_file.exists()):
            raise CandidateUpdater.CandidateFileNotFound(
                f"Candidate file: {candidate_file} was not found!")

        self._candidate_file = candidate_file
        self._read()

    class CandidateFileNotFound(Exception):
        pass

    def _read(self):
        self._config = configparser.ConfigParser()
        self._config.read(self._candidate_file)

    def candidate_name(self, candidate_number: int) -> str:
        return self._config.get(str(candidate_number), "name")

    def has_link_profile_key(self, section) -> bool:
        return self._config.has_option(section=section, option=self.LINK_PROFILE_KEY)

    def has_direct_link_profile_key(self, section) -> bool:
        return self._config.has_option(section=section, option=self.DIRECT_LINK_PROFILE_KEY)

    def has_indirect_link_profile_key(self, section) -> bool:
        return self._config.has_option(section=section, option=self.INDIRECT_LINK_PROFILE_KEY)

    def update(self):
        for candidate in self._config.sections():
            if self.has_link_profile_key(candidate):
                link_file = self._config.get(candidate, self.LINK_PROFILE_KEY)
                spliter = SplitLinkProfile(
                    link_file, self._candidate_file.parent)
                direct_link_profile, indirect_link_profile = spliter.split()
                self._config.remove_option(candidate, self.LINK_PROFILE_KEY)
                self._config.set(
                    candidate, self.DIRECT_LINK_PROFILE_KEY, str(direct_link_profile))
                self._config.set(
                    candidate, self.INDIRECT_LINK_PROFILE_KEY, str(indirect_link_profile))

    def write(self):
        with open(self._candidate_file, "w") as candidate_file:
            self._config.write(candidate_file)
