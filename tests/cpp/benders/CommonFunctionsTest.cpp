#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "antares-xpansion/benders/benders_core/common.h"
#include <filesystem>
#include <fstream>
#include <cmath>
// Test fixture for common functions
class CommonFunctionsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for tests
        temp_dir = std::filesystem::temp_directory_path() / "benders_test_common";
        std::filesystem::create_directories(temp_dir);
    }
    void TearDown() override {
        // Clean up temporary directory
        if (std::filesystem::exists(temp_dir)) {
            std::filesystem::remove_all(temp_dir);
        }
    }
    std::filesystem::path temp_dir;
};
// Tests for norm_point function
TEST_F(CommonFunctionsTest, NormPointWithIdenticalPoints) {
    Point p1 = {{"x", 1.0}, {"y", 2.0}, {"z", 3.0}};
    Point p2 = {{"x", 1.0}, {"y", 2.0}, {"z", 3.0}};
    double distance = norm_point(p1, p2);
    EXPECT_DOUBLE_EQ(distance, 0.0);
}
TEST_F(CommonFunctionsTest, NormPointWithDifferentPoints) {
    Point p1 = {{"x", 0.0}, {"y", 0.0}, {"z", 0.0}};
    Point p2 = {{"x", 3.0}, {"y", 4.0}, {"z", 0.0}};
    double distance = norm_point(p1, p2);
    // Distance should be sqrt(3^2 + 4^2) = 5.0
    EXPECT_DOUBLE_EQ(distance, 5.0);
}
TEST_F(CommonFunctionsTest, NormPointWithNegativeValues) {
    Point p1 = {{"x", -1.0}, {"y", -2.0}};
    Point p2 = {{"x", 2.0}, {"y", 2.0}};
    double distance = norm_point(p1, p2);
    // Distance should be sqrt(3^2 + 4^2) = 5.0
    EXPECT_DOUBLE_EQ(distance, 5.0);
}
TEST_F(CommonFunctionsTest, NormPointWithSingleDimension) {
    Point p1 = {{"x", 1.0}};
    Point p2 = {{"x", 4.0}};
    double distance = norm_point(p1, p2);
    EXPECT_DOUBLE_EQ(distance, 3.0);
}
// Tests for mkdir function
TEST_F(CommonFunctionsTest, MkdirCreatesNewDirectory) {
    auto new_dir = temp_dir / "new_test_folder";
    EXPECT_FALSE(std::filesystem::exists(new_dir));
    bool result = mkdir(new_dir);
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(new_dir));
    EXPECT_TRUE(std::filesystem::is_directory(new_dir));
}
TEST_F(CommonFunctionsTest, MkdirWithExistingDirectory) {
    auto existing_dir = temp_dir / "existing_folder";
    std::filesystem::create_directory(existing_dir);
    EXPECT_TRUE(std::filesystem::exists(existing_dir));
    bool result = mkdir(existing_dir);
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(existing_dir));
}
TEST_F(CommonFunctionsTest, MkdirCreatesNestedDirectories) {
    auto nested_dir = temp_dir / "level1" / "level2" / "level3";
    EXPECT_FALSE(std::filesystem::exists(nested_dir));
    bool result = mkdir(nested_dir);
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(nested_dir));
}
// Tests for get_json_file_content function
TEST_F(CommonFunctionsTest, GetJsonFileContentWithValidJson) {
    auto json_file = temp_dir / "test.json";
    // Create a valid JSON file
    std::ofstream out(json_file);
    out << R"({
        "key1": "value1",
        "key2": 42,
        "key3": [1, 2, 3]
    })";
    out.close();
    Json::Value content = get_json_file_content(json_file);
    EXPECT_EQ(content["key1"].asString(), "value1");
    EXPECT_EQ(content["key2"].asInt(), 42);
    EXPECT_EQ(content["key3"].size(), 3);
}
TEST_F(CommonFunctionsTest, GetJsonFileContentWithEmptyJson) {
    auto json_file = temp_dir / "empty.json";
    // Create an empty JSON object file
    std::ofstream out(json_file);
    out << "{}";
    out.close();
    Json::Value content = get_json_file_content(json_file);
    EXPECT_TRUE(content.isObject());
    EXPECT_TRUE(content.empty());
}
TEST_F(CommonFunctionsTest, GetJsonFileContentWithNestedJson) {
    auto json_file = temp_dir / "nested.json";
    // Create a nested JSON file
    std::ofstream out(json_file);
    out << R"({
        "outer": {
            "inner": {
                "value": 123
            }
        }
    })";
    out.close();
    Json::Value content = get_json_file_content(json_file);
    EXPECT_EQ(content["outer"]["inner"]["value"].asInt(), 123);
}
TEST_F(CommonFunctionsTest, GetJsonFileContentWithArrays) {
    auto json_file = temp_dir / "arrays.json";
    // Create a JSON file with arrays
    std::ofstream out(json_file);
    out << R"({
        "numbers": [1, 2, 3, 4, 5],
        "strings": ["a", "b", "c"]
    })";
    out.close();
    Json::Value content = get_json_file_content(json_file);
    EXPECT_EQ(content["numbers"].size(), 5);
    EXPECT_EQ(content["strings"].size(), 3);
    EXPECT_EQ(content["numbers"][0].asInt(), 1);
    EXPECT_EQ(content["strings"][2].asString(), "c");
}
