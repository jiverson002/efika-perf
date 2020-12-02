`efika-perf` is the performance benchmark framework for project Efika. The
benchmark framework is built on the
[Celero](https://github.com/DigitalInBlue/Celero) C++ benchmarking library. Its
primary purpose is twofold. First, it is intended to allow for direct comparison
of competing implementations of data structures and algorithms in the other
components of project Efika. Second, it is intended to be applied over a subset
of a component's Git history to identify performance regressions which are not
easily noticed when looking at performance diffs of one commit to the next.
Another tool, [efika-stat](https://github.com/jiverson002/efika-stat), is under
development to visualize the metrics output by this benchmark to study the
performance of a component over time.

Like the other components of project Efika, this repository is developed and
maintained separately from the other components. As such, the code contained
herein depends on well defined APIs in the other Efika components with which it
interacts. For example, in the component which implements fixed-radius nearest
neighbor algorithms ([efika-apss](https://github.com/jiverson002/efika-apss)),
not all of the currently available algorithms have been implemented from the
initial commit. Normally this would mean that the evolution of the benchmark
should be synchronized with the component; as algorithms are added and removed,
the benchmark is updated. Instead, the benchmark checks during configuration for
the availability of functions of interest and automatically includes them in the
benchmark if they are.

#### Rationale
The reason for this benchmark design originated from my desire to apply later
versions of my benchmarking program to earlier versions of the components being
benchmarked. It is quite common in my experience for a benchmark program to
evolve and become more robust over time, much like other parts of a project.
However, when the development of the benchmark itself is part of the history of
each of the components, then retroactively applying the current benchmark to
multiple versions of other parts of the code base can be tedious. With the
solution that has been implemented here, it is just a matter of writing a script
that iterates through the Git history of a component and recompiles the
benchmark program against that revision.
