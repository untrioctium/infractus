# infractus
OpenGL/Lua/C++ fractal generator.

Uses GLSL to create interesting looking fractals and other patterns in real time. Initially created around 2010-2011ish, I recently dug it back up and got it to build on Windows instead of Ubuntu 10.4. It's pretty ugly in its current state and has a bunch of weird dependencies I'll need to list. The OpenGL code is pretty old and needs a rewrite, and tons of functionality needs to be gutted in favor of C++11/14 patterns. For now, though, it compiles and seems to work.

I at least need to get rid of all those Boost dependencies and XML files (in favor of JSON). I feel like I need to take a shower every time I see one.
