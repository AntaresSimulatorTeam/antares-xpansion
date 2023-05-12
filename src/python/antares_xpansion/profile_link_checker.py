

import configparser
from pathlib import Path
from sre_constants import IN


from antares_xpansion.split_link_profile import SplitLinkProfile
from antares_xpansion.logger import step_logger


class ProfileLinkChecker:
    def __init__(self, candidate_file: Path, capacity_dir: Path) -> None:
        self.LINK_PROFILE_KEY = "link-profile"
        self.ALREADY_INSTALLED_LINK_PROFILE_KEY = "already-installed-link-profile"
        self.DIRECT_ALREADY_INSTALLED_LINK_PROFILE_KEY = "already-installed-direct-link-profile"
        self.INDIRECT_ALREADY_INSTALLED_LINK_PROFILE_KEY = "already-installed-indirect-link-profile"
        self.DIRECT_LINK_PROFILE_KEY = "direct-"+self.LINK_PROFILE_KEY
        self.INDIRECT_LINK_PROFILE_KEY = "indirect-"+self.LINK_PROFILE_KEY
        self.logger = step_logger(__name__)

        if (not candidate_file.exists()):
            raise ProfileLinkChecker.CandidateFileNotFound(
                f"Candidate file: {candidate_file} was not found!")

        self._candidate_file = candidate_file
        self._capacity_dir = capacity_dir
        self._read()

    class CandidateFileNotFound(Exception):
        pass

    class CapacityDirNotFound(Exception):
        pass

    def _read(self):
        self.config = configparser.ConfigParser()
        self.config.read(self._candidate_file)

    def candidate_name(self, candidate_number: int) -> str:
        return self.config.get(str(candidate_number), "name")

    def has_link_profile_key(self, section) -> bool:
        return self.config.has_option(section=section, option=self.LINK_PROFILE_KEY)

    def has_direct_link_profile_key(self, section) -> bool:
        return self.config.has_option(section=section, option=self.DIRECT_LINK_PROFILE_KEY)

    def has_indirect_link_profile_key(self, section) -> bool:
        return self.config.has_option(section=section, option=self.INDIRECT_LINK_PROFILE_KEY)

    def has_already_installed_link_profile_key(self, section) -> bool:
        return self.config.has_option(section=section, option=self.ALREADY_INSTALLED_LINK_PROFILE_KEY)

    def update(self):

        update_candidate_file = False
        for candidate in self.config.sections():
            if self.has_link_profile_key(candidate):
                self.logger.info(
                    f"In candidate {candidate} description, deprecated option: **{self.LINK_PROFILE_KEY}** is replaced by **{self.DIRECT_LINK_PROFILE_KEY}** and **{self.INDIRECT_LINK_PROFILE_KEY}** options")
                update_candidate_file = True
                link_file = self.config.get(candidate, self.LINK_PROFILE_KEY)
                spliter = SplitLinkProfile(
                    link_file, self._capacity_dir)
                direct_link_profile, indirect_link_profile = spliter.split()
                self.config.remove_option(candidate, self.LINK_PROFILE_KEY)
                self.config.set(
                    candidate, self.DIRECT_LINK_PROFILE_KEY, direct_link_profile.name)
                self.config.set(
                    candidate, self.INDIRECT_LINK_PROFILE_KEY, indirect_link_profile.name)

            if self.has_already_installed_link_profile_key(candidate):
                self.logger.info(
                    f"in candidate {candidate} description, deprecated option: **{self.ALREADY_INSTALLED_LINK_PROFILE_KEY}** is replaced by **{self.DIRECT_ALREADY_INSTALLED_LINK_PROFILE_KEY}** and **{self.INDIRECT_ALREADY_INSTALLED_LINK_PROFILE_KEY}** options")
                update_candidate_file = True
                link_file = self.config.get(
                    candidate, self.ALREADY_INSTALLED_LINK_PROFILE_KEY)
                spliter = SplitLinkProfile(
                    link_file, self._capacity_dir)
                direct_link_profile, indirect_link_profile = spliter.split()
                self.config.remove_option(
                    candidate, self.ALREADY_INSTALLED_LINK_PROFILE_KEY)
                self.config.set(
                    candidate, self.DIRECT_ALREADY_INSTALLED_LINK_PROFILE_KEY, direct_link_profile.name)
                self.config.set(
                    candidate, self.INDIRECT_ALREADY_INSTALLED_LINK_PROFILE_KEY, indirect_link_profile.name)

        return update_candidate_file

    def write(self):
        self.logger.info(f"{self._candidate_file.name} is updated.")
        with open(self._candidate_file, "w") as candidate_file:
            self.config.write(candidate_file)
