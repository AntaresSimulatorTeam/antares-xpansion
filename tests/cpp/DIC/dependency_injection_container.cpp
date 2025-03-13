#include <gtest/gtest.h>
#include <antares-xpansion/dic/DependencyInjectionContainer.h>

using namespace Xpansion;

TEST(dic, registeration_type)
{
    DependencyInjectionContainer::Instance().Register<int>("int", 5);
    EXPECT_EQ(DependencyInjectionContainer::Instance().get<int>("int"), 5);
}

class Foo { public: virtual int foo(){return 42;} };
class Bar: public Foo { public: int foo() override {return 43;} };

TEST(dic, make_object)
{
    DependencyInjectionContainer::Instance().Make<Bar>("foo");
    EXPECT_EQ(DependencyInjectionContainer::Instance().get<std::unique_ptr<Bar>>("foo")->foo(), 43);
}

TEST(dic, register_object)
{
    auto b = std::make_unique<Bar>();
    DependencyInjectionContainer::Instance().Register("foo", std::move(b));
    EXPECT_EQ(DependencyInjectionContainer::Instance().get<std::unique_ptr<Bar>>("foo")->foo(), 43);
}