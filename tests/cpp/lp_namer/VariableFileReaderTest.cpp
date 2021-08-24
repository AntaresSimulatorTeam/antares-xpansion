#include "gtest/gtest.h"

#include <fstream>
#include "VariableFileReader.h"

const char* TEMP_FILE_NAME= "temp_variable.txt";

struct VariableFileLine{
    int id;
    std::string variable;
    int id_pays;
    int id_link;
    int time_step;
};

class VariableFileReaderTest : public ::testing::Test
{
protected:

    static std::vector<VariableFileLine> createVariableFileLineVec(const std::vector<int>& ids, const std::vector<std::string>& variable,
                                                                   const std::vector<int>& id_pays,
                                                                   const std::vector<int>& id_link, const std::vector<int>& time_step){
        std::vector<VariableFileLine> result;
        result.reserve(ids.size());
        assert(ids.size() == variable.size());
        assert(variable.size() == id_pays.size());
        assert(id_pays.size() == id_link.size());
        assert(id_link.size() == time_step.size());

        for (int i = 0; i< ids.size();i++) {
            VariableFileLine line;
            line.id = ids.at(i);
            line.variable = variable.at(i);
            line.id_pays = id_pays.at(i);
            line.id_link = id_link.at(i);
            line.time_step = time_step.at(i);
            result.push_back(line);
        }

        return result;
    }

    static void createVariableFile(const std::string &temp_variable_name,const std::vector<VariableFileLine> &variableFileLineVec) {
        std::ofstream file(temp_variable_name);

        for (const auto& fileLine : variableFileLineVec) {
            file << fileLine.id << " " << fileLine.variable << " " <<fileLine.id_pays <<" " <<fileLine.id_link<<" "<<fileLine.time_step << "\n";
        }
        file.close();
    }

    static void createVariableFile(const std::string &temp_variable_name,
                                   const std::vector<int>& ids,
                                   const std::vector<std::string>& variable,
                                   const std::vector<int>& id_pays,
                                   const std::vector<int>& id_link,
                                   const std::vector<int>& time_step) {
        createVariableFile(temp_variable_name, createVariableFileLineVec(ids,variable,id_pays,id_link,time_step));
    }

    static std::vector<std::string> createExpectedVariableName(const std::vector<std::string>& variable,
                                                               const std::vector<int>& id_pays,
                                                               const std::vector<int>& id_link,
                                                               const std::vector<int>& time_step) {

        std::vector<std::string> result;
        result.reserve(variable.size());
        assert(variable.size() == id_pays.size());
        assert(id_pays.size() == id_link.size());
        assert(id_link.size() == time_step.size());

        for (int i = 0; i< variable.size();i++) {
            std::ostringstream name;
            name << variable.at(i) << "_";
            name << id_pays.at(i) << "_";
            name << id_link.at(i) << "_";
            name << time_step.at(i) << "_";
            result.push_back(name.str());
        }
        return result;
    }

    static void SetUpTestCase()
    {

    }

    static void TearDownTestCase()
    {
        // called after last test

        //delete the created tmp file
        std::remove(TEMP_FILE_NAME);
    }

    void SetUp()
    {
        // called before each test
    }

    void TearDown()
    {
        // called after each test
    }

    std::vector<int> _ids;
    std::vector<std::string> _variable;
    std::vector<int> _id_pays;
    std::vector<int> _id_link;
    std::vector<int> _time_step;
};


TEST_F(VariableFileReaderTest, FileNotAvailable) {
    try {
        std::vector<ActiveLink> links;
        VariableFileReader varReader("not_available_file.txt", links, "");
        FAIL();
    }
    catch(const std::runtime_error& expected) {
        ASSERT_STREQ(expected.what(), "Unable to open 'not_available_file.txt'");
    }
}

TEST_F(VariableFileReaderTest, ReadVariable) {

    _ids = {0,1,2,3,4,5,6};
    _variable = {"var_1", "var_2", "var_3", "var_4", "var_5", "var_6", "var_7"};
    _id_pays = {0,0,0,0,0,0,00};
    _id_link = {7,8,9,10,11,12,13};
    _time_step = {14,15,16,17,18,19,20};
    createVariableFile(TEMP_FILE_NAME, _ids,_variable,_id_pays,_id_link,_time_step);

    std::vector<ActiveLink> links;
    VariableFileReader varReader(TEMP_FILE_NAME, links,"");

    std::vector<std::string> expectedVariable = createExpectedVariableName(_variable,_id_pays, _id_link,_time_step);
    std::vector<std::string> variable = varReader.getVariables();

    ASSERT_EQ(variable, expectedVariable);
}

TEST_F(VariableFileReaderTest, ReadNtcColumnsWithoutActiveLink) {

    _ids = {0,1,2,3,4,5,6};
    _variable = {"var_ntc", "var_ntc", "var_3", "var_4", "var_5", "var_6", "var_7"};
    _id_pays = {0,0,0,0,0,0,00};
    _id_link = {7,8,9,10,11,12,13};
    _time_step = {14,15,16,17,18,19,20};
    createVariableFile(TEMP_FILE_NAME, _ids,_variable,_id_pays,_id_link,_time_step);

    std::vector<ActiveLink> links;
    VariableFileReader varReader(TEMP_FILE_NAME, links,"var_ntc");

    std::map<linkId,  ColumnsToChange> ntcVarColumns = varReader.getNtcVarColumns();
    std::map<linkId , ColumnsToChange> expectedNtcVarColumns;

    ASSERT_EQ(ntcVarColumns.size(), expectedNtcVarColumns.size());
}

TEST_F(VariableFileReaderTest, ReadNtcColumnsWithOneActiveLink) {

    _ids = {0,1,2,3,4,5,6};
    _variable = {"var_ntc", "var_ntc", "var_3", "var_4", "var_5", "var_6", "var_7"};
    _id_pays = {0,0,0,0,0,0,00};
    _id_link = {1,1,9,10,11,12,13};
    _time_step = {14,15,16,17,18,19,20};
    createVariableFile(TEMP_FILE_NAME, _ids,_variable,_id_pays,_id_link,_time_step);

    std::vector<ActiveLink> links;
    links.push_back(ActiveLink(1,"link"));
    VariableFileReader varReader(TEMP_FILE_NAME, links,"var_ntc");

    std::map<linkId,  ColumnsToChange> ntcVarColumns = varReader.getNtcVarColumns();
    std::map<linkId , ColumnsToChange> expectedNtcVarColumns;
    expectedNtcVarColumns[1] = { ColumnToChange(0,14), ColumnToChange(1,15)};

    ASSERT_EQ(ntcVarColumns,expectedNtcVarColumns);
}

TEST_F(VariableFileReaderTest, ReadNtcColumnsWithMultipleActiveLink) {

    _ids = {0,1,2,3,4,5,6};
    _variable = {"var_ntc", "var_ntc", "var_3", "var_4", "var_5", "var_ntc", "var_ntc"};
    _id_pays = {0,0,0,0,0,0,00};
    _id_link = {1,1,9,10,11,2,2};
    _time_step = {14,15,16,17,18,19,20};
    createVariableFile(TEMP_FILE_NAME, _ids,_variable,_id_pays,_id_link,_time_step);

    std::vector<ActiveLink> links;
    links.push_back(ActiveLink(1,"link"));
    links.push_back(ActiveLink(2,"link2"));
    VariableFileReader varReader(TEMP_FILE_NAME, links,"var_ntc");

    std::map<linkId,  ColumnsToChange> ntcVarColumns = varReader.getNtcVarColumns();
    std::map<linkId , ColumnsToChange> expectedNtcVarColumns;
    expectedNtcVarColumns[1] = { ColumnToChange(0,14), ColumnToChange(1,15)};
    expectedNtcVarColumns[2] = { ColumnToChange(5,19), ColumnToChange(6,20)};

    ASSERT_EQ(ntcVarColumns,expectedNtcVarColumns);
}


