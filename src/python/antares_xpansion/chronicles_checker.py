from pathlib import Path

from antares_xpansion.logger import flushed_print
from antares_xpansion.candidates_reader import CandidatesReader


class CandidateProfileColumnNumberMismatch(Exception):
    pass


class CandidateInstalledProfileColumnNumberMismatch(Exception):
    pass


class CandidateInstalledProfileColumnNumberMismatchWithProfile(Exception):
    pass


class NTCColumnMismatch(Exception):
    pass


class NTC_And_Candidate_Mismatch(Exception):
    pass


class ChronicleChecker:
    _study_path = Path()

    def __init__(self, study_path, antares_version):
        self.antares_version = antares_version
        self._study_path = Path(study_path)

    def CheckChronicleConstraints(self):
        # NTC mismatch can only happen after 820
        if self.antares_version < 820:
            return

        candidate_reader = CandidatesReader(
            self._study_path / "user" / "expansion" / "candidates.ini"
        )

        candidates = candidate_reader.get_candidates_list()
        for candidate in candidates:
            antares_direct_link = (
                candidate_reader.get_candidate_antares_direct_link_array(
                    self._study_path, candidate
                )
            )
            antares_indirect_link = (
                candidate_reader.get_candidate_antares_indirect_link_array(
                    self._study_path, candidate
                )
            )
            direct_link_profile = (
                candidate_reader.get_candidate_direct_link_profile_array(
                    self._study_path, candidate
                )
            )
            indirect_link_profile = (
                candidate_reader.get_candidate_indirect_link_profile_array(
                    self._study_path, candidate
                )
            )
            installed_direct_link_profile = candidate_reader.get_candidate_already_installed_direct_link_profile_array(
                self._study_path, candidate
            )
            installed_indirect_link_profile = candidate_reader.get_candidate_already_installed_indirect_link_profile_array(
                self._study_path, candidate
            )

            if antares_direct_link.shape != antares_indirect_link.shape:
                flushed_print(
                    f"For candidate {candidate}, the shapes of the direct and indirect NTC profiles of the virtual link in the Antares study do not match ({antares_direct_link.shape} vs. {antares_indirect_link.shape})"
                )
                raise NTCColumnMismatch
            if direct_link_profile.shape != indirect_link_profile.shape:
                flushed_print(
                    f"For candidate {candidate}, shape of direct link profile {direct_link_profile.shape} does not match shape of indirect link profile {indirect_link_profile.shape}"
                )
                raise CandidateProfileColumnNumberMismatch
            if (
                installed_direct_link_profile.shape
                != installed_indirect_link_profile.shape
            ):
                flushed_print(
                    f"For candidate {candidate}, shape of direct already installed link profile {installed_direct_link_profile.shape} does not match shape of indirect already installed link profile {installed_indirect_link_profile.shape}"
                )
                raise CandidateInstalledProfileColumnNumberMismatch
            if candidate_reader.has_profile(self._study_path, candidate):
                if antares_direct_link.shape != direct_link_profile.shape:
                    flushed_print(
                        f"For candidate {candidate}, the shape of the direct link profile {direct_link_profile.shape} does not match the shape of the NTC direct profile of the virtual link in the Antares study {antares_direct_link.shape}"
                    )
                    raise NTC_And_Candidate_Mismatch
            elif candidate_reader.has_installed_profile(self._study_path, candidate):
                if antares_direct_link.shape != installed_direct_link_profile.shape:
                    flushed_print(
                        f"For candidate {candidate}, the shape of the already installed direct link profile {installed_direct_link_profile.shape} does not match the shape of the NTC direct profile of the virtual link in the Antares study {antares_direct_link.shape}"
                    )
                    raise NTC_And_Candidate_Mismatch
            if (
                candidate_reader.has_installed_profile(
                    self._study_path, candidate)
                and candidate_reader.has_profile(self._study_path, candidate)
                and direct_link_profile.shape != installed_direct_link_profile.shape
            ):
                flushed_print(
                    f"For candidate {candidate}, the shape of the direct link profile {direct_link_profile.shape} does not match the shape of the already installed direct profile {installed_direct_link_profile.shape}"
                )
                raise CandidateInstalledProfileColumnNumberMismatchWithProfile
