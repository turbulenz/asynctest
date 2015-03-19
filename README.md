Simple framework for testing code that has a concept of a 'frame' or a
'tick', e.g. tests that run in rendering frameworks, or tests that
perform a lot of asynchronous calls.

For each tick, no more than one test is executed.  Tests can be paused
(`Test::Wait()`) and resumed (`Test::Resume()`).  When a test fails,
no more test code is executed and the next test will run at the next
tick.

See `sample/sample.cpp` for a trivial example.
