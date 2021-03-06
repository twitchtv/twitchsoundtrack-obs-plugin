
# Copyright Twitch Interactive, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: GPL-2.0

from conans import ConanFile, CMake, tools
import os

class TwitchSoundtrackOBSPlugin(ConanFile):
    name = "twitchsoundtrack-obs-plugin"
    version = "0.1.11"
    license = "None"
    url = ""
    description = ""
    # set build specific options here
    settings = "os", "build_type"
    generators = "cmake"

    def package(self):
        self.copy("soundtrack-plugin-32.dll", src="package", dst="bin")
        self.copy("soundtrack-plugin-32.pdb", src="package", dst="bin")
        self.copy("soundtrack-plugin-64.dll", src="package", dst="bin")
        self.copy("soundtrack-plugin-64.pdb", src="package", dst="bin")
