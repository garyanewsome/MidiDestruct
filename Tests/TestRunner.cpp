#include <cstdio>

#include <juce_core/juce_core.h>

int main (int, char**)
{
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.runAllTests();

    int numFailures = 0;

    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        const auto* result = runner.getResult (i);
        numFailures += result->failures;
    }

    std::printf ("\n%d failure(s) across %d test(s).\n", numFailures, runner.getNumResults());
    return numFailures == 0 ? 0 : 1;
}
