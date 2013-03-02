smooth
======

Architecture Assessed Coursework 2

Profiling
========
Unfortunately lab machines only have CUDA compute capability = 1.1 so many of
the advanced profiler features are not possible.
Furthermore lab machines only have Nvidia's command line profiler installed and
so we must make use of this.

In order to start profiling code
* ``source profile.sh`` - this will export the relevant shell variables
* Modify config.txt to contain any counters etc, you wish. Beware only a few
  counters should be added
  For a full list of counters see [this config
  file]: https://svn.ece.lsu.edu/svn/gp/cuda/stream/.cuda-profile-config
* Run code normally
* Find results in ``profile.dat``
* Parse with ``python python/profile_parser.py profile.dat`` to see useful
  results.

