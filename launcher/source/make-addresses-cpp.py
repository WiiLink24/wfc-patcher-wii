import os, csv
from sys import argv

if __name__ == "__main__":
    game_list = []
    with open(os.path.join("..", "..", "gamedefs.csv")) as csv_file:
        reader = csv.DictReader(csv_file, delimiter=",", dialect="excel")
        for game in reader:
            game_list.append(game)

    output = "#include \"GameAddresses.hpp\"\n\n"

    output += "const GameAddresses GameAddressesList[] = {\n"
    for game in game_list:
        if game["Title"][4] != "D":
            continue

        output += "    {\"" + game["Title"] + "\", "
        output += game["ADDRESS_DWCi_Auth_SendRequest"] + ", "
        output += game["ADDRESS_NHTTPCreateRequest"] + ", "
        output += game["ADDRESS_NHTTPSendRequestAsync"] + ", "
        output += game["ADDRESS_NHTTPDestroyResponse"] + ", "
        output += game["ADDRESS_DWC_ERROR"] + ",\n"
        output += "     GAME_ASM(\"" + game["LOAD_AUTH_REQ_INST_01_P"] + "\", \"" + game["LOAD_AUTH_REQ_INST_02"] + "\")},\n"
    
    output += "};\n\n"

    output += "const int GameAddressesListSize = sizeof(GameAddressesList) / sizeof(GameAddresses);\n"

    with open("GameAddresses.cpp", "w") as file:
        file.write(output)