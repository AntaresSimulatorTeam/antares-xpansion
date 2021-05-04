from pathlib import Path
from typing import List


class XpansionStudyReader:

    @staticmethod
    def get_years_weight_from_file(file_path: Path) -> List[float]:
        result = []
        with open(file_path, 'r') as weights_file:
            for line in weights_file:
                if line.strip():
                    line_value = float(line.strip())
                    result.append(line_value)

        return result
