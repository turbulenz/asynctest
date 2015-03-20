
#include "asynctest.h"

#include <assert.h>
#include <setjmp.h>
#include <vector>

namespace asynctest
{

struct TestAndResult
{
    std::string              mTestName;
    ITest                 *(*mTestInitialize)();
    ITest                   *mTestClass;
    int                      mWaiting;
    bool                     mFailed;
    std::string              mMessage;

    TestAndResult(const char *name, ITest *(*initFn)());
};

TestAndResult::TestAndResult(const char *name, ITest *(*initFn)())
    : mTestName(name)
    , mTestInitialize(initFn)
    , mTestClass(nullptr)
    , mWaiting(0)
    , mFailed(false)
    , mMessage()
{
}

typedef std::vector<TestAndResult>  TestList;

static TestList  *s_testList = nullptr;
static size_t     s_currentIdx = 0;

static bool       s_haveJumpPoint = false;
static jmp_buf    s_jumpPoint;

TestList &GetTestList()
{
    if (nullptr == s_testList)
    {
        s_testList = new TestList();
    }
    return *s_testList;
}

// -----------------------------------------------------------------------------
// ITest class
// -----------------------------------------------------------------------------

ITest::~ITest()
{
}

void ITest::Startup()
{
}

void ITest::Shutdown()
{
}

void ITest::Wait()
{
    asynctest::Wait();
}

void ITest::Resume(const std::function<void()> &fn)
{
    asynctest::Resume(fn);
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

bool Tick()
{
    TestList &tests = GetTestList();

    if (s_currentIdx >= tests.size())
    {
        return true;
    }

    TestAndResult *current = &tests[s_currentIdx];
    if (0 != current->mWaiting)
    {
        return false;
    }

    assert(nullptr == current->mTestClass);
    current->mTestClass = current->mTestInitialize();
    assert(nullptr != current->mTestClass);

    assert(!s_haveJumpPoint);
    if (0 == setjmp(s_jumpPoint))
    {
        s_haveJumpPoint = true;

        current->mTestClass->Startup();
        current->mTestClass->Test();
    }

    assert(s_haveJumpPoint);
    s_haveJumpPoint = false;

    if (0 == current->mWaiting)
    {
        current->mTestClass->Shutdown();
        delete current->mTestClass;
        current->mTestClass = nullptr;

        ++s_currentIdx;
    }

    return false;
}

size_t GetNumTestsRun()
{
    return s_currentIdx;
}

bool ShowResults()
{
    TestList &tests = GetTestList();
    size_t numFailures = 0;
    for (size_t i = 0 ; i < tests.size() ; ++i)
    {
        TestAndResult *t = &tests[i];
        if (t->mFailed)
        {
            ++numFailures;
            printf("%20s : FAILED: %s\n",
                   t->mTestName.c_str(),
                   t->mMessage.c_str());
        }
        else
        {
            printf("%20s : PASSED\n", t->mTestName.c_str());
        }
    }
    if (0 != numFailures)
    {
        printf(" ******** %d Tests Failed ********\n", (int )numFailures);
        return false;
    }

    return true;
}

void Wait()
{
    TestAndResult *current = &GetTestList()[s_currentIdx];
    ++(current->mWaiting);
}

void Resume(const std::function<void()> &fn)
{
    TestAndResult *current = &GetTestList()[s_currentIdx];
    assert(0 < current->mWaiting);
    --(current->mWaiting);

    assert(!s_haveJumpPoint);
    if (fn)
    {
        if (0 == setjmp(s_jumpPoint))
        {
            s_haveJumpPoint = true;
            fn();
        }

        assert(s_haveJumpPoint);
        s_haveJumpPoint = false;

    }
    assert(!s_haveJumpPoint);

    if (0 == current->mWaiting)
    {
        current->mTestClass->Shutdown();
        delete current->mTestClass;
        current->mTestClass = nullptr;

        ++s_currentIdx;
    }

    return;
}

void Fail(const char *file, int line, const char *message, ...)
{
    va_list va;
    va_start(va, message);

    assert(s_haveJumpPoint);
    TestAndResult *current = &GetTestList()[s_currentIdx];
    if (!current->mFailed)
    {
        current->mFailed = true;

        if (nullptr != message)
        {
            static const size_t sTextBufSize = 1024;
            static char messageBuffer[sTextBufSize] = "";
            vsnprintf(messageBuffer, sizeof(messageBuffer), message, va);
            current->mMessage = messageBuffer;

            fprintf(stderr, "%s:%d: error: %s\n", file, line, messageBuffer);
        }
        else
        {
            current->mMessage = "";
            fprintf(stderr, "%s:%d: error: (unnamed)\n", file, line);
        }
    }

    va_end(va);

    longjmp(s_jumpPoint, 13);
}

bool Register(ITest *(*initFn)(), const char *name)
{
    TestList &tests = GetTestList();
    tests.push_back(TestAndResult(name, initFn));
    return true;
}

} // namespace asynctest
