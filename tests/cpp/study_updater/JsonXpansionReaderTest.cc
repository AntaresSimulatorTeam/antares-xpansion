#include <fstream>
#include <sstream>

#include "JsonXpansionReader.h"
#include "gtest/gtest.h"

class JsonXpansionReaderTest : public ::testing::Test {
 protected:
  static JsonXpansionReader* jsonXpansionReader_;

  static void SetUpTestCase() {
    // called before 1st test
    std::string content_l;
    std::ofstream file_l;

    // dummy interco tmp file name
    file_l.open("temp_out.json");

    content_l = R"(
{
	"antares" : 
	{
		"name" : "unknown",
		"version" : "unknown"
	},
	"antares_xpansion" : 
	{
		"version" : "1.2.0"
	},
	"begin" : "20-11-2020 17:23:15",
	"duration" : 0.0,
	"end" : "20-11-2020 17:23:15",
	"iterations" : 
	{
		"1" : 
		{
			"best_ub" : 4.0,
			"candidates" : 
			[
				{
					"invest" : 0.0,
					"max" : 10.0,
					"min" : 0.0,
					"name" : "x"
				}
			],
			"duration" : 0.0074719799999999996,
			"gap" : 10000000004.0,
			"investment_cost" : 0.0,
			"lb" : -10000000000.0,
			"operational_cost" : 4.0,
			"overall_cost" : 4.0,
			"relative_gap" : 2500000001.0,
			"ub" : 4.0
		},
		"2" : 
		{
			"best_ub" : 4.0,
			"candidates" : 
			[
				{
					"invest" : 10.0,
					"max" : 10.0,
					"min" : 0.0,
					"name" : "x"
				}
			],
			"duration" : 0.0022926299999999999,
			"gap" : 5.0,
			"investment_cost" : 15.0,
			"lb" : -1.0,
			"operational_cost" : 0.0,
			"overall_cost" : 15.0,
			"relative_gap" : 1.0666666666666667,
			"ub" : 15.0
		},
		"3" : 
		{
			"best_ub" : 3.25,
			"candidates" : 
			[
				{
					"invest" : 1.5,
					"max" : 10.0,
					"min" : 0.0,
					"name" : "x"
				}
			],
			"duration" : 0.001730494,
			"gap" : 0.0,
			"investment_cost" : 2.25,
			"lb" : 3.25,
			"operational_cost" : 1.0,
			"overall_cost" : 3.25,
			"relative_gap" : 0.0,
			"ub" : 3.25
		},
        "4" : 
		{
			"best_ub" : 3.25,
			"candidates" : 
			[
				{
					"invest" : 10.0,
					"max" : 10.0,
					"min" : 0.0,
					"name" : "x"
				}
			],
			"duration" : 0.0022926299999999999,
			"gap" : 5.0,
			"investment_cost" : 15.0,
			"lb" : -1.0,
			"operational_cost" : 0.0,
			"overall_cost" : 15.0,
			"relative_gap" : 1.0666666666666667,
			"ub" : 15.0
		}
	},
	"nbWeeks" : 3,
	"options" : 
	{
		"ACTIVECUTS" : false,
		"AGGREGATION" : false,
		"BASIS" : true,
		"BOUND_ALPHA" : true,
		"CSV_NAME" : "benders_output_trace",
		"JSON_NAME" : "out",
		"DELETE_CUT" : false,
		"GAP" : 9.9999999999999995e-07,
		"INPUTROOT" : ".",
		"LOG_LEVEL" : 3,
		"MASTER_NAME" : "master",
		"MAX_ITERATIONS" : -1,
		"OUTPUTROOT" : ".",
		"RAND_AGGREGATION" : 0,
		"SLAVE_WEIGHT_VALUE" : 1.0,
		"STRUCTURE_FILE" : "structure.txt",
		"THRESHOLD_AGGREGATION" : 0,
		"THRESHOLD_ITERATION" : 0,
		"TRACE" : true
	},
	"solution" : 
	{
		"gap" : 0.0,
		"investment_cost" : 2.25,
		"iteration" : 3,
		"operational_cost" : 1.0,
		"overall_cost" : 3.25,
		"problem_status" : "OPTIMAL",
		"values" : 
		{
			"x" : 1.5
		}
	}
}
)";

    file_l << content_l;
    file_l.close();

    jsonXpansionReader_ = new JsonXpansionReader();
    jsonXpansionReader_->read("temp_out.json");
  }

  static void TearDownTestCase() {
    // called after last test

    // delete the created tmp file
    std::remove("temp_out.json");

    delete jsonXpansionReader_;
    jsonXpansionReader_ = nullptr;
  }

  void SetUp() {
    // called before each test
  }

  void TearDown() {
    // called after each test
  }
};

JsonXpansionReader* JsonXpansionReaderTest::jsonXpansionReader_ = nullptr;

TEST_F(JsonXpansionReaderTest, getBestIteration) {
  ASSERT_EQ(jsonXpansionReader_->getBestIteration(), 3);
}

TEST_F(JsonXpansionReaderTest, getLastIteration) {
  ASSERT_EQ(jsonXpansionReader_->getLastIteration(), 4);
}

TEST_F(JsonXpansionReaderTest, getSolutionPoint) {
  std::map<std::string, double> solution_l =
      jsonXpansionReader_->getSolutionPoint();

  ASSERT_EQ(solution_l["x"], 1.5);
}
