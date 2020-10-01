
# Copyright Twitch Interactive, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: GPL-2.0

from conans import ConanFile, CMake, tools
import os

#TODO change name of function
class SoundtrackPluginConan(ConanFile):
    name = "SoundtrackPlugin"
    version = "1.0.0"
    license = "None"
    url = "https://github.com/twitchtv/twitchsoundtrack-obs-plugin"
    description = ""
    # set build specific options here
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    requires = ("libsoundtrackutil/0.0.7",
    )
    # build_requires = "TODO: fill in build requirements if needed"
    #set package specific options here
    options = {"shared": [True, False]}
    default_options = "shared=False"
    exports_sources = "../cmakeUtils*", "TODO: Fill in sources if needed"

    @property
    def _is_win(self):
        return self.settings.os == "Windows"

    @property
    def _is_mac(self):
        return self.settings.os == "Macos"

    @property
    def _is_linux(self):
        return self.settings.os == "Linux"

    def _configure_cmake(self):
        cmake_build_type = self.settings.build_type
        if cmake_build_type == "Release":
            cmake_build_type = "RelWithDebInfo"
        cmake = CMake(self, build_type=cmake_build_type)
        cmake.definitions["ENABLE_CODE_FORMATTING"] = False
        cmake.definitions["BUILD_TESTING"] = False
        cmake.definitions["BUILD_SHARED_LIBS"] = self.options.shared

        # enable when automated builds have fixed the issue on windows
        #if self.settings.build_type == "Release":
        #    cmake.definitions["CMAKE_INTERPROCEDURAL_OPTIMIZATION"] = True

        if self._is_win:
            cmake.definitions["MSVC_DYNAMIC_RUNTIME"] = True
            if self.settings.compiler.runtime in ["MT", "MTd"]:
                cmake.definitions["MSVC_DYNAMIC_RUNTIME"] = False
        
        cmake.configure()
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)

    def _default_imports(self):
        #imports is called during package creation, so default to build_folder when the env variable is not set
        #env variable will be set, for the case where the user is pulling in dependencies to make a local build
        dest = os.getenv("CONAN_CMAKE_BINARY_DIR_PATH", None if not hasattr(self, "build_folder") else self.build_folder)

        if dest is None:
            raise RuntimeError("CONAN_CMAKE_BINARY_DIR_PATH not set")

    def imports(self):
        self._default_imports()
            
            


