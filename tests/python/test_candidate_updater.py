import configparser
import pytest
from antares_xpansion.candidate_updater import CandidateUpdater


class TestCandidateUpdater:

    def setup_method(self):
        self.link_profile_file = "capa_pv.ini"
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
            f"link-profile = {self.link_profile_file}\n"

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
        self.get_link_profile(tmp_path)

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

        self.get_link_profile(tmp_path)

        cand_updater.update()
        cand_updater.write()

        config = configparser.ConfigParser()
        config.read(candidates_file)

        assert config.has_option("3", "link-profile") == False

    def get_link_profile(self, tmp_path):
        tmp_link_file = tmp_path / self.link_profile_file
        direct_ = 0.5
        indirect_ = 0.6
        with open(tmp_link_file, "a") as file:
            for i in range(0, 8760):
                file.write(f"{direct_}\t{indirect_}\n")
