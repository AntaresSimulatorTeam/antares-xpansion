import pytest
from antares_xpansion.candidate_updater import CandidateUpdater


class TestCandidateUpdater:

    def setup_method(self):
        self.old_candidate_file_content = "[1]\n" \
            "name = semibase\n" \
            "link = area1 - semibase\n" \
            "annual-cost-per-mw = 126000\n" \
            "unit-size = 200\n" \
            "max-units = 10\n" \
            "[2]\n" \
            "name = peak\n" \
            "link = area1 - peak\n" \
            "annual-cost-per-mw = 60000\n" \
            "unit-size = 100\n" \
            "max-units = 20\n" \
            "[3]\n" \
            "name = pv\n" \
            "link = area2 - pv\n" \
            "annual-cost-per-mw = 55400\n" \
            "max-investment = 1000\n" \
            "link-profile = capa_pv.ini\n"

    def test_fail_with_non_existing_candidate_file(self, tmp_path):
        file = tmp_path / "nowhere"

        with pytest.raises(CandidateUpdater.CandidateFileNotFound):
            CandidateUpdater(file)

    def test_read_two_candidates_file(self, tmp_path):

        candidate_file = tmp_path / "candidate.ini"
        candidate_file.write_text(self.old_candidate_file_content)
        cand_updater = CandidateUpdater(candidate_file)

        # cand_updater.candidate_name(1) == "semibase"
        # cand_updater.candidate_name(2) == "peak"

        assert cand_updater.has_link_profile_key("1") == False

    def test_update_link_profile_key(self, tmp_path):

        candidates_file = tmp_path / "candidate.ini"
        candidates_file.write_text(self.old_candidate_file_content)
        cand_updater = CandidateUpdater(candidates_file)

        assert cand_updater.has_link_profile_key("3") == True
        assert cand_updater.has_direct_link_profile_key("3") == False
        assert cand_updater.has_indirect_link_profile_key("3") == False
        cand_updater.update()
        assert cand_updater.has_link_profile_key("3") == False
        assert cand_updater.has_direct_link_profile_key("3") == True
        assert cand_updater.has_indirect_link_profile_key("3") == True

    def test_output_file(self, tmp_path):

        candidates_file = tmp_path / "candidate.ini"
        candidates_file.write_text(self.old_candidate_file_content)
        cand_updater = CandidateUpdater(candidates_file)

        cand_updater.update()
        cand_updater.write()

        with open(candidates_file, "r") as file:
            print(file.read())
