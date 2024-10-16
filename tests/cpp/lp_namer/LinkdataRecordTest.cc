#include <fstream>

#include "antares-xpansion/lpnamer/problem_modifier/LinkdataRecord.h"
#include "antares-xpansion/study-updater/StudyUpdater.h"
#include "gtest/gtest.h"

class LinkdataRecordTest : public ::testing::Test
{
protected:

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

TEST_F(LinkdataRecordTest, version700)
{
    LinkdataRecord::FileColumns aFileColumns {1000, 500, 1,1,1,1,1,1};
	LinkdataRecord record_l(aFileColumns);

    ASSERT_EQ(record_l.fileColumns.directCapacity_, 1000);
    ASSERT_EQ(record_l.fileColumns.indirectCapacity_, 500);
    ASSERT_EQ(record_l.fileColumns.directHurdlesCost_, 1);
    ASSERT_EQ(record_l.fileColumns.indirectHurdlesCost_, 1);
    ASSERT_EQ(record_l.fileColumns.impedances_, 1);
    ASSERT_EQ(record_l.fileColumns.loopFlow_, 1);
    ASSERT_EQ(record_l.fileColumns.pShiftMin_, 1);
    ASSERT_EQ(record_l.fileColumns.pShiftMax_, 1);

    record_l.reset();

    ASSERT_EQ(record_l.fileColumns.directCapacity_, 0);
    ASSERT_EQ(record_l.fileColumns.indirectCapacity_, 0);
    ASSERT_EQ(record_l.fileColumns.directHurdlesCost_, 0);
    ASSERT_EQ(record_l.fileColumns.indirectHurdlesCost_, 0);
    ASSERT_EQ(record_l.fileColumns.impedances_, 0);
    ASSERT_EQ(record_l.fileColumns.loopFlow_, 0);
    ASSERT_EQ(record_l.fileColumns.pShiftMin_, 0);
    ASSERT_EQ(record_l.fileColumns.pShiftMax_, 0);
}

TEST_F(LinkdataRecordTest, fromRow)
{
    LinkdataRecord linkdata_record;
    linkdata_record.fillFromRow("1000\t500\t1\t1\t1\t1\t1\t1");

    ASSERT_EQ(linkdata_record.fileColumns.directCapacity_, 1000);
    ASSERT_EQ(linkdata_record.fileColumns.indirectCapacity_, 500);
    ASSERT_EQ(linkdata_record.fileColumns.directHurdlesCost_, 1);
    ASSERT_EQ(linkdata_record.fileColumns.indirectHurdlesCost_, 1);
    ASSERT_EQ(linkdata_record.fileColumns.impedances_, 1);
    ASSERT_EQ(linkdata_record.fileColumns.loopFlow_, 1);
    ASSERT_EQ(linkdata_record.fileColumns.pShiftMin_, 1);
    ASSERT_EQ(linkdata_record.fileColumns.pShiftMax_, 1);
}



TEST_F(LinkdataRecordTest, toRow)
{
    LinkdataRecord::FileColumns aFileColumns {1000, 500, 1,1,1,1,1,1};
    LinkdataRecord linkdata_record(aFileColumns);
    ASSERT_EQ(linkdata_record.to_row("\t"), "1000.000000\t500.000000\t1.000000\t1.000000\t1.000000\t1.000000\t1.000000\t1.000000");
}


TEST_F(LinkdataRecordTest, modernAntaresVersionRecord)
{
    LinkdataRecord record_l;
    ASSERT_EQ(record_l.fileColumns.directCapacity_, 0);
    ASSERT_EQ(record_l.fileColumns.indirectCapacity_, 0);
    ASSERT_EQ(record_l.fileColumns.directHurdlesCost_, 0);
    ASSERT_EQ(record_l.fileColumns.indirectHurdlesCost_, 0);
    ASSERT_EQ(record_l.fileColumns.impedances_, 0);
    ASSERT_EQ(record_l.fileColumns.loopFlow_, 0);
    ASSERT_EQ(record_l.fileColumns.pShiftMin_, 0);
    ASSERT_EQ(record_l.fileColumns.pShiftMax_, 0);

    record_l.fillFromRow("1\t2\t3\t4\t5\t6\t7\t8");
    ASSERT_EQ(record_l.fileColumns.directCapacity_, 1);
    ASSERT_EQ(record_l.fileColumns.indirectCapacity_, 2);
    ASSERT_EQ(record_l.fileColumns.directHurdlesCost_, 3);
    ASSERT_EQ(record_l.fileColumns.indirectHurdlesCost_, 4);
    ASSERT_EQ(record_l.fileColumns.impedances_, 5);
    ASSERT_EQ(record_l.fileColumns.loopFlow_, 6);
    ASSERT_EQ(record_l.fileColumns.pShiftMin_, 7);
    ASSERT_EQ(record_l.fileColumns.pShiftMax_, 8);

    record_l.updateCapacities(10, 20);
    ASSERT_EQ(record_l.fileColumns.directCapacity_, 10);
    ASSERT_EQ(record_l.fileColumns.indirectCapacity_, 20);
}
