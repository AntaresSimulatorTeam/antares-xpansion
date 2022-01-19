#include "gtest/gtest.h"

#include "SensitivityWriter.h"
#include <cstdio>
#include <fstream>

class SensitivityWriterShould : public ::testing::Test
{
public:
    void SetUp() override
    {
        _fileName = std::tmpnam(nullptr);
    }

    void TearDown() override
    {
        std::remove(_fileName.c_str());
    }

    std::string _fileName;
};

TEST_F(SensitivityWriterShould, GenerateAValidFile)
{
    auto writer = SensitivityWriter(_fileName);
    writer.dump();

    std::ifstream fileStream(_fileName);
    fileStream.close();
    ASSERT_TRUE(fileStream.good());
}