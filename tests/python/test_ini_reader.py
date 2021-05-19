import pytest

from src.python.antares_xpansion.general_data_reader import GeneralDataIniReader, IniFileNotFound


class TestGetNbActivatedYear:

    def _create_reader(self, tmp_path, content):
        file_path = tmp_path / 'test.ini'
        file_path.write_text(content)
        self.ini_reader = GeneralDataIniReader(file_path)

    @staticmethod
    def _create_nb_year_content(nb_years: int) -> str:
        content = "[general]\n" \
                  f"nbyears = {nb_years}\n"
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
        content = self._create_nb_year_content(nb_years)
        self._create_reader(tmp_path, content)
        assert self.ini_reader.get_nb_years() == nb_years

    @pytest.mark.parametrize(
        "nb_years", [1, 12, 14],
    )
    def test_read_nb_activated_year_no_playlist(self, tmp_path, nb_years):
        content = self._create_nb_year_content(nb_years)
        self._create_reader(tmp_path, content)
        assert self.ini_reader.get_nb_activated_year() == nb_years

    @pytest.mark.parametrize(
        "nb_years, playlist_reset, active_years, inactive_years, nb_activated_years",
        [
            (12, False, [], [], 0),
            (12, False, [6, 7], [], 2),
            (12, False, [6, 7, 8], [], 3),
            (12, False, [6, 7, 20], [], 2),
            (12, False, [6, 7], [1, 5], 2),
            (12, True, [], [], 12),
            (12, True, [0, 4, 5, 6], [], 12),
            (2, True, [0, 4, 5, 6, 40, 45, 50, 99], [], 2),
            (12, True, [], [0, 5], 10),
            (12, True, [], [0, 5, 15, 20], 10)
        ],
    )
    def test_read_nb_activated_year(self, tmp_path, nb_years, playlist_reset, active_years, inactive_years,
                                    nb_activated_years):
        content = self._create_nb_year_content(nb_years)
        content += self._create_playlist_section(playlist_reset)
        content += self._create_active_years_content(active_years)
        content += self._create_inactive_years_content(inactive_years)
        self._create_reader(tmp_path, content)
        assert self.ini_reader.get_nb_activated_year() == nb_activated_years
