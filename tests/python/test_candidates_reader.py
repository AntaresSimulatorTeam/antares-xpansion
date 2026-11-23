import numpy as np

from antares_xpansion.candidates_reader import CandidatesReader


def test_empty_profile_file_is_a_zero_timeseries(tmp_path):
    profile_file = tmp_path / "empty-profile.ini"
    profile_file.touch()

    profile = CandidatesReader._read_or_create_link_profile_array_simple(
        str(profile_file)
    )

    assert profile.shape == (8760,)
    assert np.array_equal(profile, np.zeros(8760))


def test_missing_profile_uses_default_unity_timeseries():
    profile = CandidatesReader._read_or_create_link_profile_array_simple()

    assert profile.shape == (8760,)
    assert np.array_equal(profile, np.ones(8760))
