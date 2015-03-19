
#include "asynctest.h"

#include <assert.h>
#include <sched.h>

static std::function<void()>  s_async = nullptr;

void Test1()
{
    TEST_AreSame(1, 2, "this should fail");
    assert(false /*, "should never reach here" */);
}
TEST_REGISTER(Test1, "Test 1");

void Test2()
{
    TEST_AreSame(2, 2, "this should pass");
}
TEST_REGISTER(Test2, "Test 2");

void TestASyncPass()
{
    s_async = []() {
        asynctest::Test::Resume([]() {
            });
    };
    asynctest::Test::Wait();
}
TEST_REGISTER(TestASyncPass, "ASync passing test");

void TestASyncFail()
{
    s_async = []() {
        asynctest::Test::Resume([]() {
                TEST_AreSame(3, 2, "this should fail (async callback)");
            });
    };
    asynctest::Test::Wait();
}
TEST_REGISTER(TestASyncFail, "ASync failing test");

// -----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    // Simulate our frame loop

    while (!asynctest::Test::Tick())
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

    const bool result = asynctest::Test::ShowResults();
    printf("RESULT: %s\n", (result)?("passed"):("failed"));
    return (int )(!result);
}
