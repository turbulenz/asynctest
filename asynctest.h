
#include <string>
#include <functional>

namespace asynctest
{

class Test
{
public:

    typedef void (*EntryPoint)();

    /// Keep calling this per-frame.  When it returns true, all tests
    /// have been run.
    static bool Tick();

    static size_t GetNumTestsRun();

    /// When all tests have completed, this returns a summary of the
    /// results and a global pass/fail flag (true means all tests
    /// passed).
    static bool ShowResults();

    /// Called from within tests to signify that a test will not
    /// complete until a later tick
    static void Wait();

    /// Resume a test.  Must correspond to a previous call to Wait.
    static void Resume(const std::function<void()> &fn);

    /// Registers a test.  In general, this should be called via the
    /// ASYNCTEST_REGISTER macro.
    static bool Register(EntryPoint fn, const char *name);

    /// Marks the current test as having failed.
    static void Fail(const char *file, int line, const char *message, ...);

protected:

    static Test *GetInstance();

};

#if !defined(_WIN32)
# define TEST_REGISTER( _entry, _name )                                 \
    __attribute__ ((constructor))                                       \
    static void _reg_##_entry()                                         \
    {                                                                   \
        asynctest::Test::Register( _entry, _name );                     \
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
            asynctest::Test::Fail(__FILE__, __LINE__, _message, ##__VA_ARGS__); \
        }                                                               \
    }

#define TEST_AreEqual TEST_AreSame

#define TEST_IsTrue( _cond, _message, ... )                             \
    {                                                                   \
        if (!(_cond))                                                   \
        {                                                               \
            asynctest::Test::Fail(__FILE__, __LINE__, _message, ##__VA_ARGS__); \
        }                                                               \
    }

} // namespace asynctest
