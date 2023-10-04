#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")
opts = Variables()

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["src/"])

opts.Add(PathVariable('resonance_audio', 'Resonance Audio path', '/home/nikolozka/Workspace/resonance-audio'))
opts.Add(PathVariable('target_path', 'The path where the lib is installed.', 'demo/bin/', PathVariable.PathAccept))
Help(opts.GenerateHelpText(env))
opts.Update(env)


resonance_audio_path = env["resonance_audio"] + "/resonance_audio"
resonance_audio_base_path = env["resonance_audio"]
resonance_audio_lib_path = env["resonance_audio"] + "/build/resonance_audio"
resonance_audio_common_lib_path = env["resonance_audio"] + "/build/platforms"

#resonance third party
resonance_audio_eigen_path = env["resonance_audio"] + "/third_party/eigen"
resonance_audio_pffft_path = env["resonance_audio"] + "/third_party/pffft"


#env.Append(CPPPATH=[resonance_audio_path])
env.Append(CPPPATH=[resonance_audio_path, resonance_audio_base_path])
env.Append(CPPPATH=[resonance_audio_eigen_path, resonance_audio_pffft_path])
env.Append(LIBPATH=[resonance_audio_lib_path])

env.Append(LIBS=["libResonanceAudioShared"])

env.Append(CCFLAGS=["-Wno-inconsistent-missing-override"])

sources = Glob("src/*.cpp")
sources.append(Glob(resonance_audio_path+"ambisonics/*.cc"))
sources.append(Glob(resonance_audio_path+"api/*.cc"))
sources.append(Glob(resonance_audio_path+"base/*.cc"))
sources.append(Glob(resonance_audio_path+"config/*.cc"))
sources.append(Glob(resonance_audio_path+"dsp/*.cc"))
sources.append(Glob(resonance_audio_path+"geometrical_acoustics/*.cc"))
sources.append(Glob(resonance_audio_path+"graph/*.cc"))
sources.append(Glob(resonance_audio_path+"node/*.cc"))
sources.append(Glob(resonance_audio_path+"utils/*.cc"))
sources.append(Glob(resonance_audio_base_path+"platforms/common/*.cc"))

#sources.append(Glob(resonance_audio_eigen_path+"common/*.cc"))




library = env.SharedLibrary(
    "cmsonic/bin/libgdresonance{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
