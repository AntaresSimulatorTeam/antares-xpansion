#include "gtest/gtest.h"

#include <sstream>
#include <fstream>

#include "common_lpnamer.h"
#include "CandidatesInitializer.h"
#include "StudyUpdater.h"

class StudyUpdateTest : public ::testing::Test
{
protected:

    static Candidates* candidates_;

	static void SetUpTestCase()
	{
        //Change capacity folder to retrieve temp_profile.ini in the current location
        Candidate::_capacitySubfolder = "";
        candidates_ = new Candidates();

        // called before 1st test
        std::string content_l;
        std::ofstream file_l;

        // dummy interco tmp file name
        file_l.open("temp_interco.txt");
        content_l = "\
0 0 1\n\
1 0 2\n\
";
        file_l << content_l;
        file_l.close();

        // dummy area tmp file name
        file_l.open("temp_area.txt");
        content_l = "\
area1\n\
area2\n\
peak\n\
";
        file_l << content_l;
        file_l.close();

        // dummy candidates tmp file name
        file_l.open("temp_candidates.ini");
        content_l = "\
[1]\n\
name = peak\n\
investment-type = peak\n\
link = area1 - peak\n\
annual-cost-per-mw = 60000\n\
unit-size = 100\n\
max-units = 20\n\
link-profile = temp_profile.ini\n\
\n\
\n\
[2]\n\
name = transmission_line\n\
link = area1 - area2\n\
annual-cost-per-mw = 10000\n\
unit-size = 400\n\
max-units = 8\n\
already-installed-capacity = 100\n\
";
        file_l << content_l;
        file_l.close();

        // dummy link profile tmp file name
        file_l.open("temp_profile.ini");
        std::vector<double> directLinkprofile_l(8760, 1);
        directLinkprofile_l[0] = 0;
        directLinkprofile_l[1] = 0.5;
        std::vector<double> indirectLinkprofile_l(8760, 1);
        indirectLinkprofile_l[0] = 0.25;
        indirectLinkprofile_l[1] = 0.75;

        for(auto cnt_l = 0; cnt_l < directLinkprofile_l.size()-1 ; ++cnt_l)
        {
            file_l <<  directLinkprofile_l[cnt_l] << "\t" << indirectLinkprofile_l[cnt_l] << "\n";
        }
        file_l << directLinkprofile_l.back() << "\t" << indirectLinkprofile_l.back();
        file_l.close();


        //Uninitialized mpslist //initMPSList(mps_file_name);
        initIntercoMap("temp_interco.txt");
        initAreas("temp_area.txt");
        StudyUpdateTest::candidates_->getCandidatesFromFile("temp_candidates.ini");
    }

	static void TearDownTestCase()
	{
		// called after last test

		//delete the created tmp file
        std::remove("temp_interco.txt");
        std::remove("temp_area.txt");
        std::remove("temp_candidates.ini");
        std::remove("temp_profile.ini");

        delete candidates_;
        candidates_ = nullptr;
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

Candidates * StudyUpdateTest::candidates_ = nullptr;

/***
 Verify :
    * two candidates created :
    *   - peak
    *   - transmission_line
***/
TEST_F(StudyUpdateTest, candidatesInit)
{
	ASSERT_EQ(candidates_->size(), 2);
    ASSERT_EQ((*candidates_)["1"].str("name"), "peak");
    ASSERT_EQ((*candidates_)["2"].str("name"), "transmission_line");
}


/***
 Verify :
    * first candidate has a link profile read from file :
        - verify first 3 elements
        - verify last one (ie 8759)
        - invalid indices return 0
***/
TEST_F(StudyUpdateTest, linkprofile)
{
    //filename
    ASSERT_EQ(candidates_->begin()->second.str("link-profile"), "temp_profile.ini");

    //direct profile
	ASSERT_EQ(candidates_->begin()->second.profile(0, ".", true), 0);
    ASSERT_EQ(candidates_->begin()->second.profile(1, ".", true), 0.5);
    ASSERT_EQ(candidates_->begin()->second.profile(2, ".", true), 1);
    ASSERT_EQ(candidates_->begin()->second.profile(8759, ".", true), 1);

    //indirect profile
	ASSERT_EQ(candidates_->begin()->second.profile(0, ".", false), 0.25);
    ASSERT_EQ(candidates_->begin()->second.profile(1, ".", false), 0.75);
    ASSERT_EQ(candidates_->begin()->second.profile(2, ".", false), 1);
    ASSERT_EQ(candidates_->begin()->second.profile(8759, ".", false), 1);
}


TEST_F(StudyUpdateTest, defaultAntaresVersion)
{
    StudyUpdater studyupdater(".");
    ASSERT_EQ(studyupdater.getAntaresVersion(), 710);
}

TEST_F(StudyUpdateTest, antaresVersion700)
{
    std::string content_l;
    std::ofstream file_l("study.antares");

    // dummy study tmp file name
    content_l = "\
[antares]\n\
version = 700\n\
caption = test_case_7.1_structure\n\
created = 1455269769\n\
lastsave = 1592911186\n\
author = Unknown\n\
";
    file_l << content_l;
    file_l.close();

    StudyUpdater studyupdater(".");
    ASSERT_EQ(studyupdater.getAntaresVersion(), 700);

    std::remove("study.antares");
}

/***
 Verify :
    * returned locations of the link data files
    * (ie for each link ORIGIN - DESTINATION, they are in studypath/input/links/ORIGIN/DESTINATION.txt)
***/
TEST_F(StudyUpdateTest, LinkFilenames)
{
    StudyUpdater studyupdater(".");
    ASSERT_EQ(studyupdater.getLinkdataFilepath((*candidates_)["1"]), std::string(".") + PATH_SEPARATOR + "input" + PATH_SEPARATOR + "links" + PATH_SEPARATOR + "area1" + PATH_SEPARATOR + "peak.txt");
    ASSERT_EQ(studyupdater.getLinkdataFilepath((*candidates_)["2"]), std::string(".") + PATH_SEPARATOR + "input" + PATH_SEPARATOR + "links" + PATH_SEPARATOR + "area1" + PATH_SEPARATOR + "area2.txt");
}


/***
 Verify that the computed capacities are correct:
    * candidate 1 has no already-installed capacity
***/
TEST_F(StudyUpdateTest, computeNewCapacities)
{
    StudyUpdater studyupdater(".");

    //candidate 1 has a link profile
    ASSERT_EQ(studyupdater.computeNewCapacities(1000, (*candidates_)["1"], 0), std::make_pair(0.,250.));
    ASSERT_EQ(studyupdater.computeNewCapacities(1000, (*candidates_)["1"], 1), std::make_pair(500.,750.));
    ASSERT_EQ(studyupdater.computeNewCapacities(1000, (*candidates_)["1"], 2), std::make_pair(1000.,1000.));

    //candidate 2 has an already installed capacity of 100
    ASSERT_EQ(studyupdater.computeNewCapacities(0, (*candidates_)["2"], 0), std::make_pair(100.,100.));
    ASSERT_EQ(studyupdater.computeNewCapacities(0, (*candidates_)["2"], 1), std::make_pair(100.,100.));
    ASSERT_EQ(studyupdater.computeNewCapacities(0, (*candidates_)["2"], 2), std::make_pair(100.,100.));

    ASSERT_EQ(studyupdater.computeNewCapacities(1000, (*candidates_)["2"], 0), std::make_pair(1100.,1100.));
    ASSERT_EQ(studyupdater.computeNewCapacities(1000, (*candidates_)["2"], 1), std::make_pair(1100.,1100.));
    ASSERT_EQ(studyupdater.computeNewCapacities(1000, (*candidates_)["2"], 2), std::make_pair(1100.,1100.));
}
