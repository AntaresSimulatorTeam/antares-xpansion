#include "gtest/gtest.h"

#include <sstream>
#include <fstream>

#include "IntercoINIReader.h"

class IntercoINIReaderTest : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        // called before 1st test
        std::string interco_content_l = "0 0 1\n"
                                        "1 0 3\n"
                                        "2 0 5\n"
                                        "3 1 2\n"
                                        "4 1 4";

        // dummy interco tmp file name
        std::ofstream file_interco("temp_interco.txt");
        file_interco << interco_content_l;
        file_interco.close();

        std::string area_content_l = "area1\n"
                                     "area2\n"
                                     "flex\n"
                                     "peak\n"
                                     "pv\n"
                                     "semibase";

        // dummy area tmp file name
        std::ofstream file_area("temp_area.txt");
        file_area << area_content_l;
        file_area.close();

        std::string candidate_content_l = "[1]\n"
                                     "name = semibase\n"
                                     "investment-type = semibase\n"
                                     "link = area1 - semibase\n"
                                     "annual-cost-per-mw = 126000\n"
                                     "unit-size = 200\n"
                                     "max-units = 10\n"
                                     "\n"
                                     "[2]\n"
                                     "name = peak\n"
                                     "investment-type = peak\n"
                                     "link = area1 - peak\n"
                                     "annual-cost-per-mw = 60000\n"
                                     "unit-size = 100\n"
                                     "max-units = 20";

        // dummy area tmp file name
        std::ofstream file_candidate("temp_candidate.ini");
        file_candidate << candidate_content_l;
        file_candidate.close();
    }

    static void TearDownTestCase()
    {
        // called after last test

        //delete the created tmp file
        std::remove("temp_interco.txt");
        std::remove("temp_area.txt");
        std::remove("temp_candidate.ini");
    }

    void SetUp()
    {
        // called before each test
    }

    void TearDown()
    {
        // called after each test
    }
};

TEST_F(IntercoINIReaderTest, testReadIntero) {

    std::vector<IntercoFileData> intercoDataList = IntercoINIReader::ReadAntaresIntercoFile("temp_interco.txt");

    ASSERT_EQ(intercoDataList[0].index_interco, 0);
    ASSERT_EQ(intercoDataList[0].index_pays_origine, 0);
    ASSERT_EQ(intercoDataList[0].index_pays_extremite, 1);
}

TEST_F(IntercoINIReaderTest, testReadArea) {

    std::vector<std::string> areaList = IntercoINIReader::ReadAreaFile("temp_area.txt");

    ASSERT_EQ(areaList[0], "area1");
    ASSERT_EQ(areaList[1], "area2");
    ASSERT_EQ(areaList[2], "flex");
}

TEST_F(IntercoINIReaderTest, testReadCandidate)
{
    IntercoINIReader reader("temp_interco.txt","temp_area.txt");

    std::vector<CandidateData> candidates_data = reader.readCandidateData("temp_candidate.ini");

    ASSERT_EQ(candidates_data.size(), 2);
    ASSERT_EQ(candidates_data[0].name, "semibase");
    ASSERT_EQ(candidates_data[0].investment_type, "semibase");
    ASSERT_EQ(candidates_data[0].link, "area1 - semibase");
    ASSERT_EQ(candidates_data[0].link_id, 2);

    ASSERT_EQ(candidates_data[1].name, "peak");
    ASSERT_EQ(candidates_data[1].investment_type, "peak");
    ASSERT_EQ(candidates_data[1].link, "area1 - peak");
    ASSERT_EQ(candidates_data[1].link_id, 1);
}
