#include "gtest/gtest.h"

#include <sstream>
#include <fstream>

#include "LinkProfileReader.h"

class LinkProfileReaderTest : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        // dummy profile tmp file name
        std::ofstream file_profile("temp_profile.txt");
        std::vector<double> directLinkprofile_l(8760, 1);
        std::vector<double> indirectLinkprofile_l(8760, 1);

        directLinkprofile_l[0] = 0;
        directLinkprofile_l[1] = 0.5;
        indirectLinkprofile_l[0] = 0.25;
        indirectLinkprofile_l[1] = 0.75;

        for(auto cnt_l = 0; cnt_l < directLinkprofile_l.size()-1 ; ++cnt_l)
        {
            file_profile <<  directLinkprofile_l[cnt_l] << "\t" << indirectLinkprofile_l[cnt_l] << "\n";
        }
        file_profile << directLinkprofile_l.back() << "\t" << indirectLinkprofile_l.back();
        file_profile.close();
    }

    static void TearDownTestCase()
    {
        // called after last test

        //delete the created tmp file
        std::remove("temp_profile.txt");
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

TEST_F(LinkProfileReaderTest, testReadProfile) {

    LinkProfile profile = LinkProfileReader::ReadLinkProfile("temp_profile.txt");

    ASSERT_EQ(profile.getDirectProfile(0), 0);
    ASSERT_EQ(profile.getIndirectProfile(0), 0.25);
    ASSERT_EQ(profile.getDirectProfile(1), 0.5);
    ASSERT_EQ(profile.getIndirectProfile(1), 0.75);
}


