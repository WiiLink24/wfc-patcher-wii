from multiprocessing.pool import ThreadPool, TimeoutError
from sys import argv
import os, csv, time, subprocess, hashlib

devkitppc = os.environ.get('DEVKITPPC')
path_cc = os.path.join(devkitppc, "bin", "powerpc-eabi-gcc")
path_objcopy = os.path.join(devkitppc, "bin", "powerpc-eabi-objcopy")

gct_print = False
extra_build_flags = []

def build(game):
    print(game["Title"])
    flags = []
    flags.append("-D" + game["Title"] + "=1")

    title_str = game["Title"]
    # pad to 9 bytes
    if len(title_str) == 7:
        title_str += "\\0\\0"

    flags.append("-DPAYLOAD=\"" + title_str + "\"")

    for key, value in game.items():
        if key != "Title":
            flags.append("-D" + key + "=" + value)

    flags += extra_build_flags

    out_path = os.path.join("build_tool", game["Title"])
    subprocess.run([path_cc, "-o" + out_path + ".o", "-xassembler-with-cpp", "wwfcLoaderTool.s", "-c", "-mregnames", "-I" + os.path.join("..", "include")] + flags).check_returncode()
    subprocess.run([path_objcopy, out_path + ".o", out_path + ".bin", "-O", "binary"])

    raw = open(out_path + ".bin", "rb").read()
    text = ""
    while True:
        assert(len(raw) >= 8)
        text += "{:02X}{:02X}{:02X}{:02X} {:02X}{:02X}{:02X}{:02X}\n".format(raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6], raw[7])

        raw = raw[8:]
        if len(raw) == 0:
            break

    if gct_print:
        print("$WiiLink WFC Payload Loader [" + game["Title"] + "]\n" + text)

    with open(out_path + ".txt", "w") as txtfile:
        txtfile.write(text)


if __name__ == "__main__":
    try:
        os.mkdir("build_tool")
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
        elif argv[i] == '-p':
            gct_print = True
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

