

from pathlib import Path

from antares_xpansion.logger import flushed_print, INFO_MSG


class SplitLinkProfile:
    def __init__(self, link_profile_file: str, capacity_dir: Path) -> None:

        if not capacity_dir.exists():
            raise SplitLinkProfile.CapacityDirNotFound(
                f"Capacity directory: {capacity_dir} was not found!")
        if not (capacity_dir / link_profile_file).exists():
            raise SplitLinkProfile.LinkProfileFileNotFound(
                f"link profile file: {(capacity_dir / link_profile_file)} was not found!")
        self._link_profile_file = capacity_dir / link_profile_file
        self.DIRECT_SUFFIX = "direct_"
        self.INDIRECT_SUFFIX = "indirect_"

    class CapacityDirNotFound(Exception):
        pass

    class LinkProfileFileNotFound(Exception):
        pass

    def split(self):
        two_profiles = self.two_profile()

        with open(self._link_profile_file, 'r') as profile_file:
            first_profile = []
            indirect_profile = []
            for idx, line in enumerate(profile_file):
                try:
                    line_vals = line.strip().split()
                    if (len(line_vals) == 1) and not two_profiles:
                        first_profile.append(float(line_vals[0]))
                    elif (len(line_vals) == 2) and two_profiles:
                        first_profile.append(float(line_vals[0]))
                        indirect_profile.append(float(line_vals[1]))
                    else:
                        print('Line %d in file %s is not valid.'
                              % (idx + 1, self._link_profile_file))
                        raise SplitLinkProfile.ProfileFileWrongNumberOfcolumns
                except ValueError:
                    print('Line %d in file %s is not valid: allowed float values in formats "X" or "X\tY".'
                          % (idx + 1, self._link_profile_file))
                    raise SplitLinkProfile.ProfileFileValueError
                if (first_profile[-1] < 0) or (two_profiles and indirect_profile[-1] < 0):
                    print('Line %d in file %s indicates a negative value'
                          % (idx + 1, self._link_profile_file))
                    raise SplitLinkProfile.ProfileFileNegativeValue

        if len(first_profile) != 8760:
            print('file %s does not have 8760 lines'
                  % self._link_profile_file)
            raise SplitLinkProfile.ProfileFileWrongNumberOfLines

        return self.write_splited_profile(
            two_profiles, first_profile, indirect_profile)

    def write_splited_profile(self, two_profiles, first_profile, indirect_profile):
        direct_file = self._link_profile_file.parent / \
            (self.DIRECT_SUFFIX+self._link_profile_file.name)
        indirect_file = self._link_profile_file.parent / \
            (self.INDIRECT_SUFFIX +
             self._link_profile_file.name) if two_profiles else direct_file

        flushed_print(
            f"{INFO_MSG} direct link profile: {direct_file.name} is created from link-profile file: {self._link_profile_file.name}")
        with open(direct_file, "w") as direct:
            direct.writelines(["%s\n" % item for item in first_profile])

        if (two_profiles):
            flushed_print(
                f"{INFO_MSG} indirect link profile: {indirect_file.name} is created from link-profile file: {self._link_profile_file.name}")
            with open(indirect_file, "w") as indirect:
                indirect.writelines(["%s\n" % item for item in indirect_profile]
                                    )

        return (direct_file, indirect_file)

    def two_profile(self):
        with open(self._link_profile_file, 'r') as profile_file:
            return (len(profile_file.readline().strip().split()) == 2)

    class ProfileFileWrongNumberOfLines(Exception):
        pass

    class ProfileFileWrongNumberOfcolumns(Exception):
        pass

    class ProfileFileValueError(Exception):
        pass

    class ProfileFileNegativeValue(Exception):
        pass
