# Building

All DEPS are in tree, no need to do the equivalent of gclient sync.

    git clone https://github.com/sgraham/nope.git
    cd nope
    regen
    ninja -C out\Release content_shell
    out\Release\content_shell

# Run benchmarks

e.g.

    python tools/perf/run_benchmark --browser=exact --browser-executable=out\Release\content_shell.exe startup.warm.blank_page

TODO:
- setup + robocopy to standard config laptop for consistent testing
- hook startup_metric_utils and other telemetry helpers
- wrapper for run\_benchmark that runs + summarizes the set deemed important for
  each commit.

# Updating blink

Pull in upstream blink with `git subtree` (at least while it's in its own repo).

# Motto

![Nope](http://fc09.deviantart.net/fs70/f/2013/257/6/8/grumpy_cat__nope_by_imwithstoopid13-d624kvl.png "Nope")
