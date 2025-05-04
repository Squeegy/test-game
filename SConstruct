#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp-4.4/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["src/"])
VariantDir("build", "demo/src", duplicate=0)
sources = Glob("build/*.cpp")

env["optimize"] = "no"

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "demo/bin/libgdexample.{}.{}.framework/libgdexample.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
elif env["platform"] == "ios":
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            "demo/bin/libgdexample.{}.{}.simulator.a".format(env["platform"], env["target"]),
            source=sources,
        )
    else:
        library = env.StaticLibrary(
            "demo/bin/libgdexample.{}.{}.a".format(env["platform"], env["target"]),
            source=sources,
        )
else:
    library = env.SharedLibrary(
        "demo/bin/libgdexample{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

if env["platform"] == "windows":
    env.Append(CPPPATH=["C:/Users/willg/Documents/vcpkg/installed/x64-windows/include"])
    env.Append(LIBPATH=["C:/Users/willg/Documents/vcpkg/installed/x64-windows/lib"])
    env.Append(LIBS=["portaudio"])

print("CPPPATH:", env.get("CPPPATH"))

Default(library)
