#include <gtest/gtest.h>
#include <antares-xpansion/dic/DependencyInjectionContainer.h>

using namespace Xpansion;

TEST(dic, RegisterSimpleType)
{
    DependencyInjectionContainer::Instance().Register<int>("int", 5);
    EXPECT_EQ(DependencyInjectionContainer::Instance().get<int>("int"), 5);
}

class Foo { public: virtual int foo(){return 42;} };
class Bar: public Foo { public: int foo() override {return 43;} };

TEST(dic, CreateOwningObject)
{
    DependencyInjectionContainer::Instance().Make<Bar>("foo");
    EXPECT_EQ(DependencyInjectionContainer::Instance().get<std::unique_ptr<Bar>>("foo")->foo(), 43);
}

TEST(dic, RegisterObject)
{
    Bar b;
    DependencyInjectionContainer::Instance().Register("mybar", b);
    EXPECT_EQ(DependencyInjectionContainer::Instance().get<Bar>("mybar").foo(), 43);
}

TEST(dic, RegisterOwningObject)
{
    auto b = std::make_unique<Bar>();
    DependencyInjectionContainer::Instance().Register("foo2", std::move(b));
    EXPECT_EQ(DependencyInjectionContainer::Instance().get<std::unique_ptr<Bar>>("foo")->foo(), 43);
}

TEST(dic, registrationFailForAlreadyExistingKey)
{
    DependencyInjectionContainer::Instance().Register<int>("value", 5);
    EXPECT_THROW(DependencyInjectionContainer::Instance().Register<int>("value", 42), std::invalid_argument);
}

TEST(dic, makeObjectFailsForAlreadyExistingObject)
{
    DependencyInjectionContainer::Instance().Make<Bar>("Bar");
    EXPECT_THROW(DependencyInjectionContainer::Instance().Make<Bar>("Bar"), std::invalid_argument);
}

TEST(dic, RegistringObjectFailsForAlreadyExistingObject)
{
    const Bar b;
    DependencyInjectionContainer::Instance().Register("newbar", b);
    EXPECT_THROW(DependencyInjectionContainer::Instance().Register("newbar", b), std::invalid_argument);
}
