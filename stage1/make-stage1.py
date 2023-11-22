import sys, os, struct, subprocess, string
from sys import argv

path_cc = "powerpc-eabi-gcc"
path_objcopy = "powerpc-eabi-objcopy"
path_build = "./build"

extra_build_flags = []

for i in range(len(argv)):
    if i == 0:
        continue

    if argv[i].startswith("-D"):
        extra_build_flags.append(argv[i])

try:
    os.mkdir(path_build)
except:
    pass

ccflags = "-Os -std=c++20 -fno-rtti -ffreestanding -nodefaultlibs -nostdlib -fno-unwind-tables -fno-exceptions -fmerge-all-constants -ffunction-sections -fdata-sections -fshort-enums -I../include -nodefaultlibs -nostdlib -lgcc -Wl,--gc-sections"

subprocess.run([path_cc, "./wwfcStage1.cpp"] + ccflags.split(" ") + ["-S", "-o" + path_build + "/build.s"] + extra_build_flags).check_returncode()

subprocess.run([path_cc, path_build + "/build.s"] + ccflags.split(" ") + ['-o' + path_build + "/build.elf", "-Tstage1.ld"]).check_returncode()

subprocess.run([path_objcopy, path_build + "/build.elf", path_build + "/stage1.bin", "-O", "binary"]).check_returncode()
