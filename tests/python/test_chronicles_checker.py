from pathlib import Path

import pytest
from antares_xpansion.chronicles_checker import ChronicleChecker, CandidateProfileColumnNumberMismatch, \
    CandidateInstalledProfileColumnNumberMismatch, CandidateInstalledProfileColumnNumberMismatchWithProfile, \
    NTCColumnMismatch, NTC_And_Candidate_Mismatch

DATA_PATH = Path("./data") / "ntc_column_constraint"

class TestChroniclesChecker:

    @pytest.fixture(autouse=True)
    def return_to_test_directory(self, request, monkeypatch):
        monkeypatch.chdir(request.fspath.dirname)

    def test_version_under_820(self):
        study_path = DATA_PATH / "all_ntc_ok"
        checker = ChronicleChecker(study_path, 811)
        try:
            checker.CheckChronicleConstraints()
        except Exception as exc:
            assert False, f"Exception raised: {exc}"

    def test_all_ok(self):
        study_path = DATA_PATH / "all_ntc_ok"
        checker = ChronicleChecker(study_path, 820)
        try:
            checker.CheckChronicleConstraints()
        except Exception as exc:
            assert False, f"Exception raised: {exc}"

    @pytest.mark.parametrize(
        "study_path, expected_exception",
        [
            (DATA_PATH / "candidate_profile_mismatch", CandidateProfileColumnNumberMismatch),
            (DATA_PATH / "candidate_installed_profile_mismatch", CandidateInstalledProfileColumnNumberMismatch),
            (DATA_PATH / "candidate_profile_and_installed_dont_match", CandidateInstalledProfileColumnNumberMismatchWithProfile),
            (DATA_PATH / "ntc_mismatch", NTCColumnMismatch),
            (DATA_PATH / "ntc_mismatch_with_profile", NTC_And_Candidate_Mismatch),
        ]
    )
    def test_mismatch_column(self, study_path, expected_exception, tmp_path, request):
        checker = ChronicleChecker(study_path, 820)
        with pytest.raises(expected_exception) as exc:
            checker.CheckChronicleConstraints()

    def test_all_ok_no_installed_profile(self, tmp_path):
        study_path = DATA_PATH / "all_ntc_ok_no_installed_profile"
        checker = ChronicleChecker(study_path, 820)
        try:
            checker.CheckChronicleConstraints()
        except Exception as exc:
            assert False, f"Exception raised: {exc}"

if __name__ == '__main__':
    unittest.main()
