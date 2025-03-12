#include <gtest/gtest.h>
#include <antares-xpansion/dic/DependencyInjectionContainer.h>

using namespace Xpansion;

TEST(dic, registeration_type)
{
    DependencyInjectionContainer::Instance().Register<int>("int", 5);
    EXPECT_EQ(DependencyInjectionContainer::Instance().get<int>("int"), 5);
}