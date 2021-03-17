#include "gtest/gtest.h"

#include <sstream>
#include <fstream>

#include "common_lpnamer.h"
#include "CandidatesInitializer.h"
#include "StudyUpdater.h"

class LinkdataRecordTest : public ::testing::Test
{
protected:

    static Candidates* candidates_;

	static void SetUpTestCase()
	{
        // called before first test
    }

	static void TearDownTestCase()
	{
		// called after last test
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


TEST_F(LinkdataRecordTest, versionPre700)
{
	LinkdataRecord record_l(1000, 500, 1,1,1);

    ASSERT_EQ(record_l.modernAntaresVersion_, false);
    ASSERT_EQ(record_l.directCapacity_, 1000);
    ASSERT_EQ(record_l.indirectCapacity_, 500);
    ASSERT_EQ(record_l.directHurdlesCost_, 1);
    ASSERT_EQ(record_l.indirectHurdlesCost_, 1);
    ASSERT_EQ(record_l.impedances_, 1);
    ASSERT_EQ(record_l.loopFlow_, 0);
    ASSERT_EQ(record_l.pShiftMin_, 0);
    ASSERT_EQ(record_l.pShiftMax_, 0);

    record_l.reset();

    ASSERT_EQ(record_l.modernAntaresVersion_, false);
    ASSERT_EQ(record_l.directCapacity_, 0);
    ASSERT_EQ(record_l.indirectCapacity_, 0);
    ASSERT_EQ(record_l.directHurdlesCost_, 0);
    ASSERT_EQ(record_l.indirectHurdlesCost_, 0);
    ASSERT_EQ(record_l.impedances_, 0);
    ASSERT_EQ(record_l.loopFlow_, 0);
    ASSERT_EQ(record_l.pShiftMin_, 0);
    ASSERT_EQ(record_l.pShiftMax_, 0);


}

TEST_F(LinkdataRecordTest, version700)
{
	LinkdataRecord record_l(1000, 500, 1,1,1,1,1,1);

    ASSERT_EQ(record_l.modernAntaresVersion_, true);
    ASSERT_EQ(record_l.directCapacity_, 1000);
    ASSERT_EQ(record_l.indirectCapacity_, 500);
    ASSERT_EQ(record_l.directHurdlesCost_, 1);
    ASSERT_EQ(record_l.indirectHurdlesCost_, 1);
    ASSERT_EQ(record_l.impedances_, 1);
    ASSERT_EQ(record_l.loopFlow_, 1);
    ASSERT_EQ(record_l.pShiftMin_, 1);
    ASSERT_EQ(record_l.pShiftMax_, 1);

    record_l.reset();

    ASSERT_EQ(record_l.modernAntaresVersion_, true);
    ASSERT_EQ(record_l.directCapacity_, 0);
    ASSERT_EQ(record_l.indirectCapacity_, 0);
    ASSERT_EQ(record_l.directHurdlesCost_, 0);
    ASSERT_EQ(record_l.indirectHurdlesCost_, 0);
    ASSERT_EQ(record_l.impedances_, 0);
    ASSERT_EQ(record_l.loopFlow_, 0);
    ASSERT_EQ(record_l.pShiftMin_, 0);
    ASSERT_EQ(record_l.pShiftMax_, 0);
}

TEST_F(LinkdataRecordTest, fromRow)
{
    LinkdataRecord modernRecord_l(true);
    modernRecord_l.fillFromRow("1000\t500\t1\t1\t1\t1\t1\t1");

    ASSERT_EQ(modernRecord_l.modernAntaresVersion_, true);
    ASSERT_EQ(modernRecord_l.directCapacity_, 1000);
    ASSERT_EQ(modernRecord_l.indirectCapacity_, 500);
    ASSERT_EQ(modernRecord_l.directHurdlesCost_, 1);
    ASSERT_EQ(modernRecord_l.indirectHurdlesCost_, 1);
    ASSERT_EQ(modernRecord_l.impedances_, 1);
    ASSERT_EQ(modernRecord_l.loopFlow_, 1);
    ASSERT_EQ(modernRecord_l.pShiftMin_, 1);
    ASSERT_EQ(modernRecord_l.pShiftMax_, 1);

    LinkdataRecord record_l(false);
    record_l.fillFromRow("1000\t500\t1\t1\t1\t1\t1\t1");

    ASSERT_EQ(record_l.modernAntaresVersion_, false);
    ASSERT_EQ(record_l.directCapacity_, 1000);
    ASSERT_EQ(record_l.indirectCapacity_, 500);
    ASSERT_EQ(record_l.directHurdlesCost_, 1);
    ASSERT_EQ(record_l.indirectHurdlesCost_, 1);
    ASSERT_EQ(record_l.impedances_, 1);
    ASSERT_EQ(record_l.loopFlow_, 0);
    ASSERT_EQ(record_l.pShiftMin_, 0);
    ASSERT_EQ(record_l.pShiftMax_, 0);
}



TEST_F(LinkdataRecordTest, toRow)
{
    LinkdataRecord modernRecord_l(1000, 500, 1,1,1,1,1,1);
    ASSERT_EQ(modernRecord_l.to_row("\t"), "1000.000000\t500.000000\t1.000000\t1.000000\t1.000000\t1.000000\t1.000000\t1.000000");

    LinkdataRecord record_l(1000, 500, 1,1,1);
    ASSERT_EQ(record_l.to_row("\t"), "1000.000000\t500.000000\t1.000000\t1.000000\t1.000000");
}


TEST_F(LinkdataRecordTest, modernAntaresVersionRecord)
{
    LinkdataRecord record_l(true);
    ASSERT_EQ(record_l.directCapacity_, 0);
    ASSERT_EQ(record_l.indirectCapacity_, 0);
    ASSERT_EQ(record_l.directHurdlesCost_, 0);
    ASSERT_EQ(record_l.indirectHurdlesCost_, 0);
    ASSERT_EQ(record_l.impedances_, 0);
    ASSERT_EQ(record_l.loopFlow_, 0);
    ASSERT_EQ(record_l.pShiftMin_, 0);
    ASSERT_EQ(record_l.pShiftMax_, 0);

    record_l.fillFromRow("1\t2\t3\t4\t5\t6\t7\t8");
    ASSERT_EQ(record_l.directCapacity_, 1);
    ASSERT_EQ(record_l.indirectCapacity_, 2);
    ASSERT_EQ(record_l.directHurdlesCost_, 3);
    ASSERT_EQ(record_l.indirectHurdlesCost_, 4);
    ASSERT_EQ(record_l.impedances_, 5);
    ASSERT_EQ(record_l.loopFlow_, 6);
    ASSERT_EQ(record_l.pShiftMin_, 7);
    ASSERT_EQ(record_l.pShiftMax_, 8);

    record_l.updateCapacities(10, 20);
    ASSERT_EQ(record_l.directCapacity_, 10);
    ASSERT_EQ(record_l.indirectCapacity_, 20);
}


TEST_F(LinkdataRecordTest, oldAntaresVersionRecord)
{
    LinkdataRecord record_l(false);
    ASSERT_EQ(record_l.directCapacity_, 0);
    ASSERT_EQ(record_l.indirectCapacity_, 0);
    ASSERT_EQ(record_l.directHurdlesCost_, 0);
    ASSERT_EQ(record_l.indirectHurdlesCost_, 0);
    ASSERT_EQ(record_l.impedances_, 0);
    ASSERT_EQ(record_l.loopFlow_, 0);
    ASSERT_EQ(record_l.pShiftMin_, 0);
    ASSERT_EQ(record_l.pShiftMax_, 0);

    record_l.fillFromRow("1\t2\t3\t4\t5");
    ASSERT_EQ(record_l.directCapacity_, 1);
    ASSERT_EQ(record_l.indirectCapacity_, 2);
    ASSERT_EQ(record_l.directHurdlesCost_, 3);
    ASSERT_EQ(record_l.indirectHurdlesCost_, 4);
    ASSERT_EQ(record_l.impedances_, 5);
    ASSERT_EQ(record_l.loopFlow_, 0);
    ASSERT_EQ(record_l.pShiftMin_, 0);
    ASSERT_EQ(record_l.pShiftMax_, 0);

    record_l.fillFromRow("1\t2\t3\t4\t5\t6\t7\t8");
    ASSERT_EQ(record_l.directCapacity_, 1);
    ASSERT_EQ(record_l.indirectCapacity_, 2);
    ASSERT_EQ(record_l.directHurdlesCost_, 3);
    ASSERT_EQ(record_l.indirectHurdlesCost_, 4);
    ASSERT_EQ(record_l.impedances_, 5);
    ASSERT_EQ(record_l.loopFlow_, 0);
    ASSERT_EQ(record_l.pShiftMin_, 0);
    ASSERT_EQ(record_l.pShiftMax_, 0);

    record_l.updateCapacities(10, 20);
    ASSERT_EQ(record_l.directCapacity_, 10);
    ASSERT_EQ(record_l.indirectCapacity_, 20);
}

