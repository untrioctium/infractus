# infractus
OpenGL/Lua/C++ fractal generator.

Uses GLSL to create interesting looking fractals and other patterns in real time. Initially created around 2010-2011ish, I recently dug it back up and got it to build on Debian 10 instead of Ubuntu 10. It's pretty ugly in its current state (BOOST_FOREACH, yuck); the OpenGL code is pretty old and needs a rewrite, and tons of functionality needs to be gutted in favor of C++1z patterns. For now, though, it compiles and seems to work.

Though I mostly have abandoned most work on this project in favor of a comprehensive rewrite (Refrakt), I have gone back and modernized the dependencies and code a bit. This was somewhat
"feature complete" at some point, and it would be a shame for it to stay an uncompilable chunk of code. Sol2 is now used instead of luabind, and SDL has been upgraded from v1.2 to v2. Some
of the documentation is incorrect (some parts refer to exceptions that are no longer thrown, etc.). Going to work on removing Boost completely (other than PropertyTree) and add back in
some safety/debugging that was originally done with boost::exception. Currently the Fractal Flame module does not work, but the rest do (but aren't as pretty or interactive as they once were).

To run, clone the project, and install the required dependencies. Building is done under clang via makefiles.
```
sudo apt install libglew-dev libsdl2-dev libsdl2-image-dev liblua5.3-dev libboost-dev
git clone https://github.com/untrioctium/infractus
cd infractus
make
sudo ln -s /usr/lib/libcompute.so ./libcompute/lib/libcompute.so
./infractus LifeLike
```
