from multiprocessing.pool import ThreadPool, TimeoutError
import time
import os
import csv
import subprocess
from sys import argv

cc = "powerpc-eabi-gcc"
objcopy = "powerpc-eabi-objcopy"

extra_build_flags = []

def build(game):
    print(game["Title"])

    flags = []
    flags.append("-D" + game["Title"] + "=1")

    title_str = game["Title"]
    flags.append("-DPAYLOAD_NAME=\"" + title_str + "\"")
    flags.append("-DGAME_ID=0x" + title_str[:4].encode("ascii").hex() + "")

    if title_str[4] == 'D':
        flags.append("-DTITLE_TYPE=TITLE_TYPE_DISC")
    else:
        flags.append("-DTITLE_TYPE=TITLE_TYPE_NAND")

    flags.append("-DTITLE_VERSION=0x" + title_str[5:])

    for key, value in game.items():
        if key != "Title":
            flags.append("-D" + key + "=" + value)

    flags += ["-g", "-Os", "-fPIE", "-std=c++20", "-n", "-ffunction-sections", "-fdata-sections", "-Wl,--gc-sections"]
    flags += extra_build_flags

    out_path = "./build/" + game["Title"]
    subprocess.run([cc, "-o" + out_path + ".elf", "./wwfc.cpp", "-T./payload.ld", "-I../include", "-I."] + flags).check_returncode()
    subprocess.run([objcopy, out_path + ".elf", './binary/payload.' + game["Title"] + ".bin", "-O", "binary"]).check_returncode()

if __name__ == "__main__":
    try:
        os.mkdir("./build")
    except:
        pass

    try:
        os.mkdir("./binary")
    except:
        pass

    game_list = []
    with open("../gamedefs.csv") as csv_file:
        reader = csv.DictReader(csv_file, delimiter=",", dialect="excel")
        for game in reader:
            game_list.append(game)

    pool_count = -1
    title_id = ""

    for i in range(len(argv)):
        if i == 0:
            continue

        if argv[i].startswith("-j"):
            if len(argv[i]) == 2:
                pool_count = 0
            else:
                pool_count = int(argv[i][2:])
        elif argv[i].startswith("-g") or argv[i].startswith("-t"):
            title_id = argv[i][2:]
        elif argv[i].startswith("-D"):
            extra_build_flags.append(argv[i])

    if title_id != "":
        map_game_list = []
        for game in game_list:
            if game["Title"].startswith(title_id):
                map_game_list.append(game)
        if map_game_list == []:
            print("No title for " + title_id)
    else:
        map_game_list = game_list

    if pool_count == -1:
        for game in map_game_list:
            build(game)
        exit()

    if pool_count == 0:
        pool = ThreadPool()
    else:
        pool = ThreadPool(pool_count)
    pool.map(build, map_game_list)
    pool.close()
