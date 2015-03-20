
#include <string>
#include <functional>

namespace asynctest
{

/// Class representing a single test.  Use this when there is startup
/// or shutdown code.
class ITest
{
public:
    virtual ~ITest();
    virtual void Startup();
    virtual void Shutdown();
    virtual void Test() = 0;

    void Wait();
    void Resume(const std::function<void()> &fn = nullptr);
};

typedef void (*EntryPoint)();

/// Keep calling this per-frame.  When it returns true, all tests
/// have been run.
bool Tick();

size_t GetNumTestsRun();

/// When all tests have completed, this returns a summary of the
/// results and a global pass/fail flag (true means all tests
/// passed).
bool ShowResults();

/// Called from within tests to signify that a test will not
/// complete until a later tick
void Wait();

/// Resume a test.  Must correspond to a previous call to Wait.
void Resume(const std::function<void()> &fn = nullptr);

/// Registers a test class.  In general, this should be called via the
/// ASYNCTEST_REGISTER macro.
bool Register(ITest *(*initFn)(), const char *name);

/// Marks the current test as having failed.
void Fail(const char *file, int line, const char *message, ...);

#if !defined(_WIN32)
# define TEST_REGISTER( _cls_, _name_ )                                 \
    __attribute__ ((constructor))                                       \
    static void _reg_##_cls_()                                          \
    {                                                                   \
        typedef asynctest::ITest* (*InitFn_t)();                        \
        InitFn_t initFn = static_cast<InitFn_t>                         \
            ([]() -> asynctest::ITest * {                               \
                return new _cls_ ();                                    \
            });                                                         \
        asynctest::Register(initFn, _name_ );                           \
    }
#else
# define TEST_REGISTER( _entry, _name )                                 \
    extern bool _entry##_reg_result =                                   \
        asynctest::Test::Register( _entry, _name )
#endif

#define TEST_AreSame( _expect, _actual, _message, ... )                 \
    {                                                                   \
        if ((_expect) != (_actual))                                     \
        {                                                               \
            asynctest::Fail(__FILE__, __LINE__, _message, ##__VA_ARGS__); \
        }                                                               \
    }

#define TEST_AreEqual TEST_AreSame

#define TEST_IsTrue( _cond, _message, ... )                             \
    {                                                                   \
        if (!(_cond))                                                   \
        {                                                               \
            asynctest::Fail(__FILE__, __LINE__, _message, ##__VA_ARGS__); \
        }                                                               \
    }

} // namespace asynctest
