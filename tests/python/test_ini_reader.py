import pytest

from antares_xpansion.general_data_reader import GeneralDataIniReader, IniFileNotFound


class TestGetNbActivatedYear:

    @staticmethod
    def _create_nb_year_content(nb_years: int, user_playlist: bool) -> str:
        playlist = "true" if user_playlist else "false"
        content = "[general]\n" \
                  f"nbyears = {nb_years}\n" \
                  f"user-playlist = {playlist}\n"
        return content

    @staticmethod
    def _create_playlist_section(playlist_reset: bool) -> str:
        content = "[playlist]\n" \
                  f"playlist_reset={playlist_reset}\n"
        return content

    @staticmethod
    def _create_active_years_content(active_year_list: []) -> str:
        content = ""
        for active_year in active_year_list:
            content += f"playlist_year += {active_year}\n"
        return content

    @staticmethod
    def _create_inactive_years_content(inactive_year_list: []) -> str:
        content = ""
        for inactive_year in inactive_year_list:
            content += f"playlist_year -= {inactive_year}\n"
        return content

    def test_empty_reader(self):
        with pytest.raises(IniFileNotFound):
            GeneralDataIniReader()

    @pytest.mark.parametrize(
        "nb_years", [1, 12, 14],
    )
    def test_read_nb_years(self, tmp_path, nb_years: int):
        content = self._create_nb_year_content(nb_years, True)
        file_path = tmp_path / 'test.ini'
        file_path.write_text(content)

        ini_reader = GeneralDataIniReader(file_path)
        assert ini_reader.get_nb_years() == nb_years

    @pytest.mark.parametrize(
        "nb_years", [1, 12, 14],
    )
    def test_read_nb_activated_year_no_playlist(self, tmp_path, nb_years):
        content = self._create_nb_year_content(nb_years, True)
        file_path = tmp_path / 'test.ini'
        file_path.write_text(content)

        ini_reader = GeneralDataIniReader(file_path)
        assert len(ini_reader.get_active_years()) == nb_years

    @pytest.mark.parametrize(
        "nb_years, playlist_reset, add_years, remove_years, nb_activated_years, active_years",
        [
            (2, False, [], [], 0, []),
            (4, False, [2, 3], [], 2, [3, 4]),
            (4, False, [2, 3, 4], [], 2, [3, 4]),
            (4, False, [2, 3], [2, 3], 2, [3, 4]),
            (2, True, [], [], 2, [1, 2]),
            (2, True, [1, 4], [], 2, [1, 2]),
            (4, True, [], [2, 4], 3, [1, 2, 4]),
        ],
    )
    def test_read_nb_activated_year(self, tmp_path, nb_years, playlist_reset, add_years, remove_years,
                                    nb_activated_years, active_years):
        content = self._create_nb_year_content(nb_years, True)
        content += self._create_playlist_section(playlist_reset)
        content += self._create_active_years_content(add_years)
        content += self._create_inactive_years_content(remove_years)
        file_path = tmp_path / 'test.ini'
        file_path.write_text(content)

        ini_reader = GeneralDataIniReader(file_path)
        assert ini_reader.get_active_years() == active_years

    @pytest.mark.parametrize(
        "nb_years, playlist_reset, add_years, remove_years, nb_activated_years, active_years",
        [
            (2, False, [], [], 2, [1, 2]),
            (4, False, [2, 4], [], 4, [1, 2, 3, 4]),
            (4, False, [2, 3], [2, 3], 4, [1, 2, 3, 4]),
            (2, True, [], [], 2, [1, 2]),
            (2, True, [1, 4], [], 2, [1, 2]),
            (4, True, [], [2, 3], 2, [1, 2, 3, 4]),
        ],
    )
    def test_active_years_is_unchanged_if_user_playlist_is_false(self, tmp_path, nb_years, playlist_reset, add_years, remove_years,
                                    nb_activated_years, active_years):
        content = self._create_nb_year_content(nb_years, False)
        content += self._create_playlist_section(playlist_reset)
        content += self._create_active_years_content(add_years)
        content += self._create_inactive_years_content(remove_years)
        file_path = tmp_path / 'test.ini'
        file_path.write_text(content)

        ini_reader = GeneralDataIniReader(file_path)
        assert ini_reader.get_active_years() == active_years