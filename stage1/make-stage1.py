import sys, os, struct, subprocess, string
from sys import argv

devkitppc = os.environ.get('DEVKITPPC')
path_cc = os.path.join(devkitppc, "bin", "powerpc-eabi-gcc")
path_objcopy = os.path.join(devkitppc, "bin", "powerpc-eabi-objcopy")
path_build = "build"

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

ccflags = "-g -Os -std=c++20 -fno-rtti -ffreestanding -nodefaultlibs -nostdlib -fno-unwind-tables -fno-exceptions -fmerge-all-constants -ffunction-sections -fdata-sections -fshort-enums -nodefaultlibs -nostdlib -lgcc -Wl,--gc-sections -n "
ccflags += "-I" + os.path.join("..", "include")

subprocess.run([path_cc, "wwfcStage1.cpp"] + ccflags.split(" ") + ["-S", "-o" + os.path.join(path_build, "stage1.s")] + extra_build_flags).check_returncode()

subprocess.run([path_cc, os.path.join(path_build, "stage1.s")] + ccflags.split(" ") + ['-o' + os.path.join(path_build, "stage1.elf"), "-Tstage1.ld"]).check_returncode()

subprocess.run([path_objcopy, os.path.join(path_build, "stage1.elf"), os.path.join(path_build, "stage1.bin"), "-O", "binary"]).check_returncode()
