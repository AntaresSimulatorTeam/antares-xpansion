#include "gtest/gtest.h"

#include <fstream>
#include "LinkProfileReader.h"

const auto valid_profile_name= "temp_profile.txt";
const auto invalid_profile_name = "temp_invalid_profile.txt";


class LinkProfileReaderTest : public ::testing::Test
{
protected:

    static void createProfileFile(const std::string &temp_profile_name, std::vector<double> &directLinkprofile_l,
                                  std::vector<double> &indirectLinkprofile_l) {
        std::ofstream file_profile(temp_profile_name);

        for(auto cnt_l = 0; cnt_l < directLinkprofile_l.size()-1 ; ++cnt_l)
        {
            file_profile <<  directLinkprofile_l[cnt_l] << "\t" << indirectLinkprofile_l[cnt_l] << "\n";
        }
        file_profile << directLinkprofile_l.back() << "\t" << indirectLinkprofile_l.back();
        file_profile.close();
    }

    static void SetUpTestCase()
    {

        std::vector<double> directLinkprofile_l(8760, 1);
        std::vector<double> indirectLinkprofile_l(8760, 1);

        directLinkprofile_l[0] = 0;
        directLinkprofile_l[1] = 0.5;
        indirectLinkprofile_l[0] = 0.25;
        indirectLinkprofile_l[1] = 0.75;

        createProfileFile(valid_profile_name, directLinkprofile_l, indirectLinkprofile_l);

        std::vector<double> invalid_directLinkprofile_l(100, 1);
        std::vector<double> invalid_indirectLinkprofile_l(100, 1);
        createProfileFile(invalid_profile_name, invalid_directLinkprofile_l, invalid_directLinkprofile_l);
    }

    static void TearDownTestCase()
    {
        // called after last test

        //delete the created tmp file
        std::remove(valid_profile_name);
        std::remove(invalid_profile_name);
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

TEST_F(LinkProfileReaderTest, ReadValidProfile) {

    LinkProfile profile = LinkProfileReader::ReadLinkProfile(valid_profile_name);

    ASSERT_EQ(profile.getDirectProfile(0), 0);
    ASSERT_EQ(profile.getIndirectProfile(0), 0.25);
    ASSERT_EQ(profile.getDirectProfile(1), 0.5);
    ASSERT_EQ(profile.getIndirectProfile(1), 0.75);
}

TEST_F(LinkProfileReaderTest, ReadInvalidProfile) {

    try {
        LinkProfile profile = LinkProfileReader::ReadLinkProfile(invalid_profile_name);
        FAIL();
    }
    catch(const std::runtime_error& expected) {
        ASSERT_STREQ(expected.what(), "error not enough line in link-profile temp_invalid_profile.txt");
    }
}

TEST_F(LinkProfileReaderTest, GetTimeStepLargerThan8760) {

    LinkProfile profile = LinkProfileReader::ReadLinkProfile(valid_profile_name);

    try {
        profile.getDirectProfile(8790);
        FAIL();
    }
    catch(const std::runtime_error& expected) {
        ASSERT_STREQ(expected.what(), "Link profiles can be requested between point 0 and 8759.");
    }

    try {
        profile.getIndirectProfile(8790);
        FAIL();
    }
    catch(const std::runtime_error& expected) {
        ASSERT_STREQ(expected.what(), "Link profiles can be requested between point 0 and 8759.");
    }
}


