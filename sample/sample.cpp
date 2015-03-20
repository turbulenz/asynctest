
#include "asynctest.h"

#include <assert.h>
#include <sched.h>

static std::function<void()>  s_async = nullptr;

class Test1 : public asynctest::ITest
{
public:
    void Test() override
    {
        TEST_AreSame(1, 2, "this should fail");
        assert(false /*, "should never reach here" */);
    }
};
TEST_REGISTER(Test1, "Test 1");


class Test2 : public asynctest::ITest
{
public:
    void Test() override
    {
        TEST_AreSame(2, 2, "this should pass");
    }
};
TEST_REGISTER(Test2, "Test 2");

class TestASyncPass : public asynctest::ITest
{
public:
    void Test() override
    {
        s_async = []() {
            asynctest::Resume([]() {
                    TEST_AreSame(3, 3, "should pass (async callback)");
                });
        };
        asynctest::Wait();
    }
};
TEST_REGISTER(TestASyncPass, "ASync passing test");

class TestASyncFail : public asynctest::ITest
{
public:
    void Test() override
    {
        s_async = []() {
            asynctest::Resume([]() {
                    TEST_AreSame(3, 2, "this should fail (async callback)");
                });
        };
        asynctest::Wait();
    }
};
TEST_REGISTER(TestASyncFail, "ASync failing test");

// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    // Simulate our frame loop

    while (!asynctest::Tick())
    {
        // printf("back from tick\n");

        if (s_async)
        {
            // printf("got async func\n");
            s_async();
            s_async = nullptr;
        }

        sched_yield();
    }

    printf("Finished.\n");

    // Show results

    const bool result = asynctest::ShowResults();
    printf("RESULT: %s\n", (result)?("passed"):("failed"));
    return (int )(!result);
}
