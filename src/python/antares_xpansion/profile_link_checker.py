

import configparser
from pathlib import Path
from sre_constants import IN


from antares_xpansion.split_link_profile import SplitLinkProfile
from antares_xpansion.flushed_print import flushed_print, INFO_MSG


class ProfileLinkChecker:
    def __init__(self, candidate_file: Path, capacity_dir: Path) -> None:
        self.LINK_PROFILE_KEY = "link-profile"
        self.DIRECT_LINK_PROFILE_KEY = "direct-link-profile"
        self.INDIRECT_LINK_PROFILE_KEY = "indirect-link-profile"

        if (not candidate_file.exists()):
            raise ProfileLinkChecker.CandidateFileNotFound(
                f"Candidate file: {candidate_file} was not found!")

        if (not capacity_dir.exists()):
            raise ProfileLinkChecker.CapacityDirNotFound(
                f"Capacity directory: {candidate_file} was not found!")

        self._candidate_file = candidate_file
        self._capacity_dir = capacity_dir
        self._read()

    class CandidateFileNotFound(Exception):
        pass

    class CapacityDirNotFound(Exception):
        pass

    def _read(self):
        # default_values = {
        #     'name': 'NA',
        #     'link': 'NA',
        #     'annual-cost-per-mw': '0',
        #     'unit-size': '0',
        #     'max-units': '0',
        #     'max-investment': '0',
        #     'link-profile': '1',
        #     'already-installed-capacity': '0',
        #     'already-installed-link-profile': '1'
        # }
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

    def update(self):

        update_candidate_file = False
        for candidate in self.config.sections():
            if self.has_link_profile_key(candidate):
                flushed_print(
                    f"{INFO_MSG} in candidate {candidate} description, deprecated option: **{self.LINK_PROFILE_KEY}** is replaced by **{self.DIRECT_LINK_PROFILE_KEY}** and **{self.INDIRECT_LINK_PROFILE_KEY}** options")
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

        return update_candidate_file

    def write(self):
        flushed_print(f"{self._candidate_file.name} is updated.")
        with open(self._candidate_file, "w") as candidate_file:
            self.config.write(candidate_file)
