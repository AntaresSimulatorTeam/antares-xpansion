#include <fstream>
#include <sstream>

#include "antares-xpansion/helpers/JsonXpansionReader.h"
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

    content_l =
        "\
{\n\
	\"antares\" : \n\
	{\n\
		\"name\" : \"unknown\",\n\
		\"version\" : \"unknown\"\n\
	},\n\
	\"antares_xpansion\" : \n\
	{\n\
		\"version\" : \"1.2.0\"\n\
	},\n\
	\"begin\" : \"20-11-2020 17:23:15\",\n\
	\"duration\" : 0.0,\n\
	\"end\" : \"20-11-2020 17:23:15\",\n\
	\"iterations\" : \n\
	{\n\
		\"1\" : \n\
		{\n\
			\"best_ub\" : 4.0,\n\
			\"candidates\" : \n\
			[\n\
				{\n\
					\"invest\" : 0.0,\n\
					\"max\" : 10.0,\n\
					\"min\" : 0.0,\n\
					\"name\" : \"x\"\n\
				}\n\
			],\n\
			\"duration\" : 0.0074719799999999996,\n\
			\"gap\" : 10000000004.0,\n\
			\"investment_cost\" : 0.0,\n\
			\"lb\" : -10000000000.0,\n\
			\"operational_cost\" : 4.0,\n\
			\"overall_cost\" : 4.0,\n\
			\"relative_gap\" : 2500000001.0,\n\
			\"ub\" : 4.0\n\
		},\n\
		\"2\" : \n\
		{\n\
			\"best_ub\" : 4.0,\n\
			\"candidates\" : \n\
			[\n\
				{\n\
					\"invest\" : 10.0,\n\
					\"max\" : 10.0,\n\
					\"min\" : 0.0,\n\
					\"name\" : \"x\"\n\
				}\n\
			],\n\
			\"duration\" : 0.0022926299999999999,\n\
			\"gap\" : 5.0,\n\
			\"investment_cost\" : 15.0,\n\
			\"lb\" : -1.0,\n\
			\"operational_cost\" : 0.0,\n\
			\"overall_cost\" : 15.0,\n\
			\"relative_gap\" : 1.0666666666666667,\n\
			\"ub\" : 15.0\n\
		},\n\
		\"3\" : \n\
		{\n\
			\"best_ub\" : 3.25,\n\
			\"candidates\" : \n\
			[\n\
				{\n\
					\"invest\" : 1.5,\n\
					\"max\" : 10.0,\n\
					\"min\" : 0.0,\n\
					\"name\" : \"x\"\n\
				}\n\
			],\n\
			\"duration\" : 0.001730494,\n\
			\"gap\" : 0.0,\n\
			\"investment_cost\" : 2.25,\n\
			\"lb\" : 3.25,\n\
			\"operational_cost\" : 1.0,\n\
			\"overall_cost\" : 3.25,\n\
			\"relative_gap\" : 0.0,\n\
			\"ub\" : 3.25\n\
		},\n\
        \"4\" : \n\
		{\n\
			\"best_ub\" : 3.25,\n\
			\"candidates\" : \n\
			[\n\
				{\n\
					\"invest\" : 10.0,\n\
					\"max\" : 10.0,\n\
					\"min\" : 0.0,\n\
					\"name\" : \"x\"\n\
				}\n\
			],\n\
			\"duration\" : 0.0022926299999999999,\n\
			\"gap\" : 5.0,\n\
			\"investment_cost\" : 15.0,\n\
			\"lb\" : -1.0,\n\
			\"operational_cost\" : 0.0,\n\
			\"overall_cost\" : 15.0,\n\
			\"relative_gap\" : 1.0666666666666667,\n\
			\"ub\" : 15.0\n\
		}\n\
	},\n\
	\"nbWeeks\" : 3,\n\
	\"options\" : \n\
	{\n\
		\"ACTIVECUTS\" : false,\n\
		\"AGGREGATION\" : false,\n\
		\"BASIS\" : true,\n\
		\"BOUND_ALPHA\" : true,\n\
		\"CSV_NAME\" : \"benders_output_trace\",\n\
		\"JSON_NAME\" : \"out\",\n\
		\"DELETE_CUT\" : false,\n\
		\"GAP\" : 9.9999999999999995e-07,\n\
		\"INPUTROOT\" : \".\",\n\
		\"LOG_LEVEL\" : 3,\n\
		\"MASTER_NAME\" : \"master\",\n\
		\"MAX_ITERATIONS\" : -1,\n\
		\"OUTPUTROOT\" : \".\",\n\
		\"RAND_AGGREGATION\" : 0,\n\
		\"SLAVE_WEIGHT_VALUE\" : 1.0,\n\
		\"STRUCTURE_FILE\" : \"structure.txt\",\n\
		\"THRESHOLD_AGGREGATION\" : 0,\n\
		\"THRESHOLD_ITERATION\" : 0,\n\
		\"TRACE\" : true\n\
	},\n\
	\"solution\" : \n\
	{\n\
		\"gap\" : 0.0,\n\
		\"investment_cost\" : 2.25,\n\
		\"iteration\" : 3,\n\
		\"operational_cost\" : 1.0,\n\
		\"overall_cost\" : 3.25,\n\
		\"problem_status\" : \"OPTIMAL\",\n\
		\"values\" : \n\
		{\n\
			\"x\" : 1.5\n\
		}\n\
	}\n\
}\n\
";

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
