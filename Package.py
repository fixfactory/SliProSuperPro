#
# RSF Medals
# A browser extension for the Rally Sim Fans website
# Copyright 2024 Fixfactory
#
# This file is part of RSF Medals.
#
# RSF Medals is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or any later version.
#
# RSF Medals is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with RSF Medals. If not, see <http://www.gnu.org/licenses/>.
#

import os
import zipfile
import sys

# Get the version number from Version.h
major = ""
minor = ""
rev = ""

def get_val(line, token):
    i = line.find(token)
    if i != -1:   
        return line[i + len(token):-2]
    return ""

with open('.\\Source\\SliProSuperPro\\Version.h') as f:
    for line in f:
        if len(major) == 0:
            major = get_val(line, "kMajor = ")
        elif len(minor) == 0:
            minor = get_val(line, "kMinor = ")
        elif len(rev) == 0:
            rev = get_val(line, "kRevision = ")
        else:
            break
    if len(major) == 0 or len(minor) == 0 or len(rev) == 0:
        print("Couldn't find version number in Version.h")
        sys.exit(-1)

version = major + "." + minor + "." + rev
zipName = "SliProSuperPro-release-" + version + ".zip"

print("Packaging " + zipName)

# Zip the files
zf = zipfile.ZipFile(".\\Releases\\" + zipName, "w")
zf.write(".\\LICENSE.txt")
zf.write(".\\Docs\\README.txt", "README.txt")
zf.write(".\\Bin\\x64\\Release\\SliProSuperPro.exe", "SliProSuperPro.exe")
zf.write(".\\Bin\\x64\\Release\\RBR-NGP.Plugin.dll", "RBR-NGP.Plugin.dll")
zf.write(".\\Bin\\x64\\Release\\iRacing.Plugin.dll", "iRacing.Plugin.dll")
zf.write(".\\Bin\\x64\\Release\\ATS.Plugin.dll", "ATS.Plugin.dll")
zf.write(".\\Bin\\x64\\Release\\SPSP.ATS.Plugin.dll", "Game Plugins\\American Truck Simulator\\SPSP.ATS.Plugin.dll")
zf.write(".\\Data\\iRacing.Overrides.json", "iRacing.Overrides.json")
zf.close()
