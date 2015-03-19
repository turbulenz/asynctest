
#include "asynctest.h"

#include <assert.h>
#include <setjmp.h>
#include <vector>

namespace asynctest
{

struct TestAndResult
{
    Test::EntryPoint         mTestFunction;
    std::string              mTestName;
    int                      mWaiting;
    bool                     mFailed;
    std::string              mMessage;

    TestAndResult();
};

TestAndResult::TestAndResult()
    : mTestFunction(nullptr)
    , mTestName()
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

bool
Test::Tick()
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

    assert(!s_haveJumpPoint);
    if (0 == setjmp(s_jumpPoint))
    {
        s_haveJumpPoint = true;

        current->mTestFunction();
    }

    assert(s_haveJumpPoint);
    s_haveJumpPoint = false;

    if (0 == current->mWaiting)
    {
        ++s_currentIdx;
    }

    return false;
}

size_t
Test::GetNumTestsRun()
{
    return s_currentIdx;
}

bool
Test::ShowResults()
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

void
Test::Wait()
{
    TestAndResult *current = &GetTestList()[s_currentIdx];
    ++(current->mWaiting);
}

void
Test::Resume(const std::function<void()> &fn)
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

        if (0 == current->mWaiting)
        {
            ++s_currentIdx;
        }
    }
    assert(!s_haveJumpPoint);
    return;
}

void
Test::Fail(const char *file, int line, const char *message, ...)
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

bool
Test::Register(EntryPoint fn, const char *name)
{
    TestList &tests = GetTestList();
    TestAndResult t;
    t.mTestFunction = fn;
    t.mTestName = name;
    tests.push_back(t);
    return true;
}

} // namespace asynctest
