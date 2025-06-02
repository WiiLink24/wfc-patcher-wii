from multiprocessing.pool import ThreadPool, TimeoutError
import time, os, csv, subprocess
from sys import argv

devkitppc = os.environ.get('DEVKITPPC')
path_cc = os.path.join(devkitppc, "bin", "powerpc-eabi-gcc")
path_objcopy = os.path.join(devkitppc, "bin", "powerpc-eabi-objcopy")

extra_build_flags = []

def build(game):
    print(game["Title"])

    flags = []
    flags.append("-D" + game["Title"] + "=1")

    title_str = game["Title"]
    flags.append("-DWWFC_PAYLOAD_NAME=\"" + title_str + "\"")
    flags.append("-DWWFC_GAME_ID=0x" + title_str[:4].encode("ascii").hex() + "")

    if title_str[4] == 'D':
        flags.append("-DWWFC_TITLE_TYPE=TITLE_TYPE_DISC")
    else:
        flags.append("-DWWFC_TITLE_TYPE=TITLE_TYPE_NAND")

    flags.append("-DWWFC_TITLE_VERSION=0x" + title_str[5:])

    for key, value in game.items():
        if key != "Title":
            flags.append("-D" + key + "=" + value)

    flags += ["-g", "-Os", "-fPIE", "-std=c++20", "-nostdinc", "-nostdinc++", "-Wall", "-Werror", "-fno-threadsafe-statics", "-Wsuggest-override", "-n", "-fno-rtti", "-fno-exceptions", "-fno-sized-deallocation", "-ffunction-sections", "-fdata-sections", "-fshort-wchar", "-Wl,--gc-sections", "-Wno-address-of-packed-member"]
    flags += extra_build_flags

    out_path = os.path.join("build", game["Title"])
    binary_path = os.path.join("binary", "payload." + game["Title"] + ".bin")
    include_path = os.path.join("..", "include")
    subprocess.run([path_cc, "-o" + out_path + ".elf", "wwfc.cpp", "-Tpayload.ld", "-I" + include_path, "-I."] + flags).check_returncode()
    subprocess.run([path_objcopy, out_path + ".elf", binary_path, "-O", "binary"]).check_returncode()

if __name__ == "__main__":
    try:
        os.mkdir("build")
    except:
        pass

    try:
        os.mkdir("binary")
    except:
        pass

    game_list = []
    with open(os.path.join("..", "gamedefs.csv")) as csv_file:
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
