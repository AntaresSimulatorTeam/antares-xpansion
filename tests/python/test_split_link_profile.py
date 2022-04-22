import pytest

from antares_xpansion.split_link_profile import SplitLinkProfile


class TestSplitLinkProfile:

    def setup_method(self):
        pass

    def test_with_non_existing_candidate_file_directory(self, tmp_path):
        with pytest.raises(SplitLinkProfile.CapacityDirNotFound):
            SplitLinkProfile("nothing.txt", tmp_path / "void")

    def test_with_non_existing_file(self, tmp_path):
        with pytest.raises(SplitLinkProfile.LinkProfileFileNotFound):
            SplitLinkProfile("nothing.txt", tmp_path)

    def test_output_files_exists(self, tmp_path):
        link_file = "OrigDest.txt"
        tmp_link_file = tmp_path / link_file
        with open(tmp_link_file, "a") as file:
            for i in range(0, 8760):
                file.write("1\t1\n")

        spliter = SplitLinkProfile(link_file, tmp_path)
        direct_link_file, indirect_link_file = spliter.split()

        assert direct_link_file.exists()
        assert indirect_link_file.exists()

    def test_output_files_content_two_profiles(self, tmp_path):
        link_file = "file.txt"
        tmp_link_file = tmp_path / link_file
        direct_ = 0.5
        indirect_ = 0.6
        with open(tmp_link_file, "a") as file:
            for i in range(0, 8760):
                file.write(f"{direct_}\t{indirect_}\n")

        spliter = SplitLinkProfile(link_file, tmp_path)
        direct_link_file, indirect_link_file = spliter.split()

        assert direct_link_file.exists()
        assert indirect_link_file.exists()

        with open(direct_link_file) as file:
            for line in file.readlines():
                assert float(line.strip()) == direct_

        with open(indirect_link_file) as file:
            for line in file.readlines():
                assert float(line.strip()) == indirect_

    def test_output_files_content_one_profile(self, tmp_path):
        link_file = "file_name.txt"
        tmp_link_file = tmp_path / link_file
        direct_ = 0.5
        with open(tmp_link_file, "a") as file:
            for i in range(0, 8760):
                file.write(f"{direct_}\n")

        spliter = SplitLinkProfile(link_file, tmp_path)
        direct_link_file, indirect_link_file = spliter.split()

        assert direct_link_file.exists()
        assert indirect_link_file.exists()
        assert indirect_link_file == direct_link_file

        with open(direct_link_file) as file:
            for line in file.readlines():
                assert float(line.strip()) == direct_
