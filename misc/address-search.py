from sys import argv
import os, struct, csv
import re

# A messy script for searching through a list of DOL files for addresses
# Creates the gamedefs.csv file

games = []

def dol_to_real(hdr, offset):
    for i in range(18):
        sect = i * 4
        sect_off = struct.unpack('>I', hdr[sect:sect+4])[0]
        sect_size = struct.unpack('>I', hdr[sect+0x48+0x48:sect+0x48+0x48+4])[0]
        if offset >= sect_off and offset - sect_off < sect_size:
            sect_addr = struct.unpack('>I', hdr[sect+0x48:sect+0x48+4])[0]
            return sect_addr + (offset - sect_off)
    panic('invalid dol section (1)')

def real_to_dol(hdr, offset):
    for i in range(18):
        sect = i * 4
        sect_addr = struct.unpack('>I', hdr[sect+0x48:sect+0x48+4])[0]
        sect_size = struct.unpack('>I', hdr[sect+0x48+0x48:sect+0x48+0x48+4])[0]
        if offset >= sect_addr and offset - sect_addr < sect_size:
            sect_off = struct.unpack('>I', hdr[sect:sect+4])[0]
            return sect_off + (offset - sect_addr)
    exit('invalid dol section (2)')

def decode_bl(dol, index):
    hdr = dol[:0x100]

    branch = struct.unpack('>I', dol[index:index+4])[0]
    assert((branch & 0xFC000000) == 0x48000000)
    srcOffs = dol_to_real(hdr, index)

    branch &= 0x03FFFFFC
    if branch & 0x2000000:
        branch &= ~0x2000000
        branch = (0x10000000 - branch) & 0x1FFFFFC
        branch = 0 - branch

    return srcOffs + branch



def parse_file(file_path, title_name):
    dol = open(file_path, 'rb').read()
    hdr = dol[:0x100]
    # print(title_name)

    index = dol.find(bytes([0x3D,0x20,0x67,0x45,0x3D,0x00,0xEF,0xCE,0x39,0x40,0x00,0x00]))
    if index == -1:
        # skip missing MD5Digest
        return
    ADDRESS_MD5Digest = dol_to_real(hdr, index - 0x14)

    if dol[0xE0:0xE4] != bytes([0x00,0x00,0x00,0x00]):
        # get r2 and r13
        init_registers = real_to_dol(hdr, decode_bl(dol, real_to_dol(hdr, int.from_bytes(dol[0xE0:0xE4], byteorder='big'))))

        hi = struct.unpack('>H', dol[init_registers+0x7E:init_registers+0x80])[0]
        lo = struct.unpack('>H', dol[init_registers+0x82:init_registers+0x84])[0]
        ADDRESS_R2_BASE = (hi << 16) | lo

        hi = struct.unpack('>H', dol[init_registers+0x86:init_registers+0x88])[0]
        lo = struct.unpack('>H', dol[init_registers+0x8A:init_registers+0x8C])[0]
        ADDRESS_R13_BASE = (hi << 16) | lo

    index = dol.find(bytes([0x7C,0x03,0x03,0x78,0x4E,0x80,0x00,0x20,0x54,0x80,0x07,0xBE]))
    if index == -1:
        exit('missing strcmp :( ' + title_name)
    if dol[index-0x10:index-0xC] != bytes([0x88,0xA3,0x00,0x00]):
        exit('missing strcmp :( ' + title_name)
    ADDRESS_strcmp = dol_to_real(hdr, index - 0x10)

    # find OSGetTime
    index = dol.find(b'\x7C\x6D\x42\xE6\x7C\x8C\x42\xE6\x7C\xAD\x42\xE6\x7C\x03\x28\x00\x40\x82\xFF\xF0\x4E\x80\x00\x20')
    if index == -1:
        exit('missing OSGetTime :( ' + title_name)
    ADDRESS_OSGetTime = dol_to_real(hdr, index)

    # OSDisableInterrupts
    # 7C 60 00 A6 54 64 04 5E 7C 80 01 24 54 63 8F FE 4E 80 00 20
    index = dol.find(b'\x7C\x60\x00\xA6\x54\x64\x04\x5E\x7C\x80\x01\x24\x54\x63\x8F\xFE\x4E\x80\x00\x20')
    if index == -1:
        exit('missing OSDisableInterrupts :( ' + title_name)
    ADDRESS_OSDisableInterrupts = dol_to_real(hdr, index)

    # OSRestoreInterrupts
    # 2C 03 00 00 7C 80 00 A6 41 82 00 0C 60 85 80 00 48 00 00 08 54 85 04 5E 7C A0 01 24 54 83 8F FE 4E 80 00 20
    index = dol.find(b'\x2C\x03\x00\x00\x7C\x80\x00\xA6\x41\x82\x00\x0C\x60\x85\x80\x00\x48\x00\x00\x08\x54\x85\x04\x5E\x7C\xA0\x01\x24\x54\x83\x8F\xFE\x4E\x80\x00\x20')
    if index == -1:
        exit('missing OSRestoreInterrupts :( ' + title_name)
    ADDRESS_OSRestoreInterrupts = dol_to_real(hdr, index)

    index = dol.find('https://naswii.nintendowifi.net/ac'.encode('ascii'))
    if index == -1:
        exit('missing naswii url :( ' + title_name)
    ADDRESS_NASWII_AC_URL = dol_to_real(hdr, index)
    if dol[index+0x50:index+0x54] != ADDRESS_NASWII_AC_URL.to_bytes(4, byteorder='big'):
        exit('missing naswii url pointer :( ' + title_name)
    ADDRESS_NASWII_AC_URL_POINTER = dol_to_real(hdr, index+0x50)

    index = dol.find('https://naswii.nintendowifi.net/pr'.encode('ascii'))
    ADDRESS_NASWII_PR_URL = 0
    ADDRESS_NASWII_PR_URL_POINTER = 0
    if index != -1:
        ADDRESS_NASWII_PR_URL = dol_to_real(hdr, index)
        if dol[index+0x50:index+0x54] != ADDRESS_NASWII_PR_URL.to_bytes(4, byteorder='big'):
            exit('missing naswii pr url pointer :( ' + title_name)
        ADDRESS_NASWII_PR_URL_POINTER = dol_to_real(hdr, index+0x50)


    index = dol.find(b'\x7C\x00\x18\xAC\x38\x63\x00\x20\x42\x00\xFF\xF8\x44\x00\x00\x02')
    if index == -1:
        exit('missing dcflushrange :( ' + title_name)
    ADDRESS_DCFlushRange = dol_to_real(hdr, index-0x1C)

    index = dol.find(b'\x7C\x00\x1F\xAC\x38\x63\x00\x20\x42\x00\xFF\xF8\x7C\x00\x04\xAC\x4C\x00\x01\x2C')
    if index == -1:
        exit('missing icinvalidaterange :( ' + title_name)
    ADDRESS_ICInvalidateRange = dol_to_real(hdr, index-0x1C)

    index = dol.find(b'\x38\xA0\x00\x00\x90\xA3\x00\x20\x38\x00\x00\x01\x80\x81\x00\x08')
    if index == -1:
        index = dol.find(b'\x38\xA0\x00\x00\x38\x00\x00\x01\x90\xA3\x00\x20\x80\x81\x00\x08')
        if index == -1:
            exit('missing IOS_Open :( ' + title_name)

    assert(dol[index-0x5C:index-0x58] == b'\x94\x21\xFF\xE0')
    ADDRESS_IOS_Open = dol_to_real(hdr, index - 0x5C)

    index = dol.find(b'\x38\xA0\x00\x00\x90\xA3\x00\x20\x38\x00\x00\x02\x80\x81\x00\x08')
    if index == -1:
        index = dol.find(b'\x38\xA0\x00\x00\x38\x00\x00\x02\x90\xA3\x00\x20\x80\x81\x00\x08')
        if index == -1:
            exit('missing IOS_Close :( ' + title_name)

    assert(dol[index-0x50:index-0x4C] == b'\x94\x21\xFF\xE0')
    ADDRESS_IOS_Close = dol_to_real(hdr, index - 0x50)

    index = dol.find(b'\x38\xA0\x00\x00\x38\x00\x00\x03\x90\xA3\x00\x20\x80\x81\x00\x08')
    if index == -1:
        index = dol.find(b'\x38\xA0\x00\x00\x90\xA3\x00\x20\x38\x00\x00\x03\x80\x81\x00\x08')
        if index == -1:
            exit('missing IOS_Read :( ' + title_name)

    assert(dol[index-0x60:index-0x5C] == b'\x94\x21\xFF\xE0')
    ADDRESS_IOS_Read = dol_to_real(hdr, index - 0x60)

    index = dol.find(b'\x40\x82\x00\x38\x80\x61\x00\x08\x7F\x64\xDB\x78\x7F\x85\xE3\x78')
    if index == -1:
        exit('missing IOS_Ioctlv :( ' + title_name)
    assert(dol[index-0x88:index-0x84] == b'\x94\x21\xFF\xD0')
    ADDRESS_IOS_Ioctlv = dol_to_real(hdr, index - 0x88)

    index = dol.find(b'\x54\x63\x00\xBE\x7C\x7A\x03\xA6\x7C\x60\x00\xA6\x54\x63\x07\x32\x7C\x7B\x03\xA6\x4C\x00\x00\x64')
    if index == -1:
        exit('missing RealMode :( ' + title_name)
    ADDRESS_RealMode = dol_to_real(hdr, index)


    # (offset+0x44)
    versionA = bytes([0x7C,0xFF,0x3B,0x78,0x3B,0x86,0x51,0xC3,0x38,0xA6,0x41,0xC3,0x38,0xE4])

    # (offset+0x48)
    versionBC = bytes([0x38,0x7E,0x00,0x8C,0x54,0x00,0x10,0x3A,0x7E,0xC3,0x00,0x2E,0x48,0x00,0x00,0x14,0x80,0x0D])

    # (offset+0x28)
    versionD = bytes([0x80,0x7E,0x03,0x80,0x7C,0x97,0x23,0x78,0x7C,0xB8,0x2B,0x78,0x7C,0xFA,0x3B,0x78,0x80,0x03,0x59,0xC8,0x7D,0x19,0x43,0x78,0x3B,0xBD])

    found = False
    version = ''

    ADDRESS_DWCi_Auth_SendRequest = 0
    ADDRESS_DWCi_Auth_HandleResponse = 0
    ADDRESS_DWC_AUTH_ADD_MACADDR = 0
    ADDRESS_DWC_AUTH_ADD_CSNUM = 0
    ADDRESS_DWC_ERROR = 0

    index = -1

    if title_name == 'RPBJD00' or title_name == 'RPBJD01' or title_name == 'RPBJD02':
        if title_name == 'RPBJD00':
            ADDRESS_DWCi_Auth_SendRequest = 0x803238B8
            ADDRESS_DWCi_Auth_HandleResponse = 0x80324180
            ADDRESS_DWC_ERROR = 0x805E1A60
        else:
            ADDRESS_DWCi_Auth_SendRequest = dol_to_real(hdr, 0x31F7F8)
            ADDRESS_DWCi_Auth_HandleResponse = dol_to_real(hdr, 0x3200C0)
            ADDRESS_DWC_ERROR = 0x805E1DA0

        auth_func_size = 0x8C8

        ADDRESS_DWC_AUTH_ADD_MACADDR = ADDRESS_DWCi_Auth_SendRequest + (0x80323CF8 - 0x803238B8)
        ADDRESS_DWC_AUTH_ADD_CSNUM = ADDRESS_DWCi_Auth_SendRequest + (0x80323EEC - 0x803238B8)

        version = 'PB'
        index = real_to_dol(hdr, ADDRESS_DWCi_Auth_SendRequest)
        found = True

    if index == -1:
        index = dol.find(versionBC)
        if index != -1:
            index -= 0x48
            ADDRESS_DWCi_Auth_SendRequest = dol_to_real(hdr, index)

            # Supports /pr endpoint for profanity

            if dol[index+0xB3C:index+0xB40] == bytes([0x38,0x21,0x01,0xB0]) and dol[index+0xB28:index+0xB2A] == bytes([0x90,0x65]):
                # Example RMCPD00 0x800ED6E8
                version = 'B'
                ADDRESS_DWC_AUTH_ADD_MACADDR = dol_to_real(hdr, index+0x79C)
                ADDRESS_DWC_AUTH_ADD_CSNUM = dol_to_real(hdr, index+0x9B0)
                STRUCT_REQ_OFFSET = struct.unpack('>H', dol[index+0xB2A:index+0xB2C])[0]
                auth_func_size = 0xB44
            elif dol[index+0xB3C:index+0xB40] == bytes([0x38,0x21,0x01,0xB0]) and dol[index+0xB24:index+0xB26] == bytes([0x90,0x65]):
                # Example RUUED00 0x8033DD24
                version = 'B'
                ADDRESS_DWC_AUTH_ADD_MACADDR = dol_to_real(hdr, index+0x79C)
                ADDRESS_DWC_AUTH_ADD_CSNUM = dol_to_real(hdr, index+0x9B0)
                STRUCT_REQ_OFFSET = struct.unpack('>H', dol[index+0xB26:index+0xB28])[0]
                auth_func_size = 0xB44
            elif dol[index+0xB38:index+0xB3C] == bytes([0x38,0x21,0x01,0xB0]) and dol[index+0xB20:index+0xB24] == bytes([0x90,0x65,0x59,0xC4]):
                # Example R4QKD00 0x80475820
                # Only difference from B is the param_1 comparison at 0x1B0
                # Other than that, struct layouts seem to be different
                version = 'C'
                ADDRESS_DWC_AUTH_ADD_MACADDR = dol_to_real(hdr, index+0x798)
                ADDRESS_DWC_AUTH_ADD_CSNUM = dol_to_real(hdr, index+0x9AC)
                auth_func_size = 0xB40
            else:
                exit('BAD VERSION B/C? ' + title_name + " " + dol[index+0xB18:index+0xB2C].hex())

            hi = struct.unpack('>H', dol[index+0x82:index+0x84])[0]
            lo = struct.unpack('>H', dol[index+0x92:index+0x94])[0]
            if lo & 0x8000:
                hi -= 1
            ADDRESS_DWCi_Auth_HandleResponse = (hi << 16) | lo

            found = True

    if index == -1:
        index = dol.find(versionA)
        if index != -1:
            # Example R4QJD00
            # Example HDMEN0001 0x80210AD4
            index -= 0x44
            version = 'A'
            ADDRESS_DWCi_Auth_SendRequest = dol_to_real(hdr, index)

            if dol[index+0x928:index+0x92C] != bytes([0x38,0x21,0x01,0xB0]):
                print('R1 WHAT WHAAWWATTT')
                exit()

            auth_func_size = 0x930


            if dol[index+0x914:index+0x918] != bytes([0x90,0x65,0x55,0xD0]):
                print('R2 WHAT WHAAWWATTT')
                exit()

            hi = struct.unpack('>H', dol[index+0x42:index+0x44])[0]
            lo = struct.unpack('>H', dol[index+0x52:index+0x54])[0]
            if lo & 0x8000:
                hi -= 1
            ADDRESS_DWCi_Auth_HandleResponse = (hi << 16) | lo

            ADDRESS_DWC_AUTH_ADD_MACADDR = dol_to_real(hdr, index+0x5BC)
            ADDRESS_DWC_AUTH_ADD_CSNUM = dol_to_real(hdr, index+0x7B0)

            found = True

    if index == -1:
        index = dol.find(versionD)
        if index != -1:
            # example ST7ED00 0x803DBB50
            version = 'D'
            index -= 0x24
            ADDRESS_DWCi_Auth_SendRequest = dol_to_real(hdr, index)

            if dol[index+0xB3C:index+0xB40] != bytes([0x38,0x21,0x01,0xC0]):
                exit('R2 WHAT WHAAWWATTT')
            found = True

            auth_func_size = 0xB44

            hi = struct.unpack('>H', dol[index+0x8A:index+0x8C])[0]
            lo = struct.unpack('>H', dol[index+0x9A:index+0x9C])[0]
            if lo & 0x8000:
                hi -= 1
            ADDRESS_DWCi_Auth_HandleResponse = (hi << 16) | lo

            ADDRESS_DWC_AUTH_ADD_MACADDR = dol_to_real(hdr, index+0x7A0)
            ADDRESS_DWC_AUTH_ADD_CSNUM = dol_to_real(hdr, index+0x9B4)

    if found == False:
        print(title_name + ' ####### FOUND NOTHING')
        return

    # 0x180 type with B
    response = ''

    # print('{} 0x{:08X} 0x{:08X} 0x{:08X} 0x{:08X}'.format(version, ADDRESS_DWCi_Auth_SendRequest, ADDRESS_DWCi_Auth_HandleResponse, ADDRESS_DWC_AUTH_ADD_MACADDR, ADDRESS_DWC_AUTH_ADD_CSNUM))

    # do auth response function
    index = real_to_dol(hdr, ADDRESS_DWCi_Auth_HandleResponse)

    if title_name == 'RPBJD00' or title_name == 'RPBJD01' or title_name == 'RPBJD02':
        response = 'PB'
        ADDRESS_AUTH_HANDLERESP_HOOK = ADDRESS_DWCi_Auth_HandleResponse + (0x80324284 - 0x80324180)
        ADDRESS_AUTH_HANDLERESP_CONTINUE = ADDRESS_AUTH_HANDLERESP_HOOK + 0x4
        ADDRESS_AUTH_HANDLERESP_ERROR = ADDRESS_DWCi_Auth_HandleResponse + (0x803246D8 - 0x80324180)
        ADDRESS_AUTH_HANDLERESP_OUT = ADDRESS_DWCi_Auth_HandleResponse + (0x80324740 - 0x80324180)

    elif dol[index+0x130:index+0x134] == bytes([0x40,0x81,0x06,0x08]):
        # Example HDMEN0001 0x80211404
        # print('response A ' + title_name)
        response = 'A'
        if dol[index+0x810:index+0x814] != bytes([0x80,0x01,0x01,0x74]):
            exit('NOWAY ' + title_name)

        ADDRESS_AUTH_HANDLERESP_HOOK = ADDRESS_DWCi_Auth_HandleResponse + 0x134
        ADDRESS_AUTH_HANDLERESP_CONTINUE = ADDRESS_DWCi_Auth_HandleResponse + 0x138

        if dol[index+0x79C:index+0x7A0] != bytes([0x20,0x15,0xB1,0xE0]):
            exit('NOWAY2 ' + title_name)
        ADDRESS_AUTH_HANDLERESP_ERROR = ADDRESS_DWCi_Auth_HandleResponse + 0x7A0
        ADDRESS_AUTH_HANDLERESP_OUT = ADDRESS_DWCi_Auth_HandleResponse + 0x808

    elif dol[index+0x178:index+0x17C] == bytes([0x40,0x81,0x06,0x80]):
        if dol[index+0x930:index+0x934] == bytes([0x80,0x01,0x01,0x74]):
            # Example RMCPD00 0x800EE22C
            # print('response B ' + title_name)
            response = 'B'

            if dol[index+0x8D0:index+0x8D4] != bytes([0x20,0x15,0xB1,0xE0]):
                exit('NOWAY2 ' + title_name)
            ADDRESS_AUTH_HANDLERESP_ERROR = ADDRESS_DWCi_Auth_HandleResponse + 0x8D4
            ADDRESS_AUTH_HANDLERESP_OUT = ADDRESS_DWCi_Auth_HandleResponse + 0x928

        elif dol[index+0x92C:index+0x930] == bytes([0x80,0x01,0x01,0x74]):
            # Example RXPJD00 0x80298394
            # In fact that's the only example I'm aware of
            # print('response C ' + title_name)
            response = 'C'

            if dol[index+0x8CC:index+0x8D0] != bytes([0x20,0x15,0xB1,0xE0]):
                exit('NOWAY2 ' + title_name)
            ADDRESS_AUTH_HANDLERESP_ERROR = ADDRESS_DWCi_Auth_HandleResponse + 0x8D0
            ADDRESS_AUTH_HANDLERESP_OUT = ADDRESS_DWCi_Auth_HandleResponse + 0x924

        ADDRESS_AUTH_HANDLERESP_HOOK = ADDRESS_DWCi_Auth_HandleResponse + 0x17C
        ADDRESS_AUTH_HANDLERESP_CONTINUE = ADDRESS_DWCi_Auth_HandleResponse + 0x194

    elif dol[index+0x17C:index+0x180] == bytes([0x40,0x81,0x06,0x9C]):
        # Example E5XJN0000 0x80094070
        # example ST7ED00 0x803DC6A0
        # print('response D ' + title_name)
        response = 'D'
        if dol[index+0x950:index+0x954] != bytes([0x80,0x01,0x01,0x84]):
            exit('NOWAY ' + title_name)

        ADDRESS_AUTH_HANDLERESP_HOOK = ADDRESS_DWCi_Auth_HandleResponse + 0x180
        ADDRESS_AUTH_HANDLERESP_CONTINUE = ADDRESS_DWCi_Auth_HandleResponse + 0x198

        if dol[index+0x8F0:index+0x8F4] != bytes([0x20,0x13,0xB1,0xE0]):
            exit('NOWAY2 ' + title_name)
        ADDRESS_AUTH_HANDLERESP_ERROR = ADDRESS_DWCi_Auth_HandleResponse + 0x8F4
        ADDRESS_AUTH_HANDLERESP_OUT = ADDRESS_DWCi_Auth_HandleResponse + 0x948
    else:
        print('not found ' + title_name)

    d = real_to_dol(hdr, ADDRESS_AUTH_HANDLERESP_HOOK)
    AUTH_HANDLERESP_UNPATCH = struct.unpack('>I', dol[d:d+4])[0]

    if ADDRESS_DWC_ERROR == 0:
        hi = struct.unpack('>H', dol[index+0x16:index+0x18])[0]
        lo = struct.unpack('>H', dol[index+0x1E:index+0x20])[0]
        if lo & 0x8000:
            hi -= 1
        ADDRESS_DWC_ERROR = (hi << 16) | lo

    # find DWCi_SetError
    ADDRESS_DWCi_SetError = 0
    index = dol.find(b'\x2C\x00\x00\x09\x4D\x82\x00\x20\x90\x6D')
    if index != -1:
        ADDRESS_DWCi_SetError = dol_to_real(hdr, index - 0x4)

    if index == -1:
        index = dol.find(b'\x2C\x03\x00\x09\x4C\x82\x00\x20\x3C\x80')
        if index != -1:
            ADDRESS_DWCi_HandleGPError = dol_to_real(hdr, index - 0x14)

    if index == -1:
        index = dol.find(b'\x2C\x00\x00\x09\x4D\x82\x00\x20\x3C\xA0\x80')
        if index != -1:
            ADDRESS_DWCi_SetError = dol_to_real(hdr, index - 0x8)

    if index == -1:
        print('missing DWCi_SetError :( ' + title_name)

    # find DWCi_HandleGPError
    ADDRESS_DWCi_HandleGPError = 0
    index = dol.find(b'\x2C\x1D\x00\x03\x41\x82\x00\x3C\x40\x80\x00\x14\x2C\x1D\x00\x01\x41\x82\x00\x18')
    if index != -1:
        ADDRESS_DWCi_HandleGPError = dol_to_real(hdr, index - 0x44)

    if index == -1:
        index = dol.find(b'\x2C\x1D\x00\x01\x41\x82\x00\x20\x2C\x1D\x00\x02\x41\x82\x00\x24\x2C\x1D\x00\x03\x41\x82\x00\x28\x2C\x1D\x00\x04\x41\x82\x00\x2C\x48\x00\x00\x30\x3B\xC0\x00\x09')
        if index != -1:
            ADDRESS_DWCi_HandleGPError = dol_to_real(hdr, index - 0x44)

    if index == -1:
        print('missing DWCi_HandleGPError :( ' + title_name)

        

    if version == 'A':
        addrOf = real_to_dol(hdr, ADDRESS_DWCi_Auth_SendRequest)
        addr = struct.unpack('>H', dol[addrOf+0x90A:addrOf+0x90C])[0]

        if addr & 0x8000:
            addr = (0x8000 - addr) & 0x7FFF
            addrStr = '-0x{:04X}'.format(addr & ~0x8000)
        else:
            addrStr = '0x{:04X}'.format(addr & ~0x8000)

        LOAD_AUTH_REQ_INST_01 = "lwz %r5, "+addrStr+"(%r13)"
        LOAD_AUTH_REQ_INST_02 = "addi %r5, %r5, 0x55D0"
        LOAD_AUTH_REQ_INST_01_P = LOAD_AUTH_REQ_INST_01
    elif version == 'B':
        addrOf = real_to_dol(hdr, ADDRESS_DWCi_Auth_SendRequest)
        addr = struct.unpack('>H', dol[addrOf+0xB1A:addrOf+0xB1C])[0]

        if addr & 0x8000:
            addr = (0x8000 - addr) & 0x7FFF
            addrStr = '-0x{:04X}'.format(addr & ~0x8000)
        else:
            addrStr = '0x{:04X}'.format(addr & ~0x8000)

        LOAD_AUTH_REQ_INST_01 = "lwz %r5, "+addrStr+"(%r13)"
        LOAD_AUTH_REQ_INST_02 = "addi %r5, %r5, 0x{:04X}".format(STRUCT_REQ_OFFSET)
        LOAD_AUTH_REQ_INST_01_P = LOAD_AUTH_REQ_INST_01
    elif version == 'C':
        addrOf = real_to_dol(hdr, ADDRESS_DWCi_Auth_SendRequest)
        addr = struct.unpack('>H', dol[addrOf+0xB16:addrOf+0xB18])[0]

        if addr & 0x8000:
            addr = (0x8000 - addr) & 0x7FFF
            addrStr = '-0x{:04X}'.format(addr & ~0x8000)
        else:
            addrStr = '0x{:04X}'.format(addr & ~0x8000)


        LOAD_AUTH_REQ_INST_01 = "lwz %r5, "+addrStr+"(%r13)"
        LOAD_AUTH_REQ_INST_02 = "addi %r5, %r5, 0x59C4"
        LOAD_AUTH_REQ_INST_01_P = LOAD_AUTH_REQ_INST_01
    elif version == 'D' and response == 'D':
        LOAD_AUTH_REQ_INST_01 = "lwz %r5, 0x380(%r26)"
        LOAD_AUTH_REQ_INST_02 = "addi %r5, %r5, 0x59C4"

        d = real_to_dol(hdr, ADDRESS_DWCi_Auth_HandleResponse)
        ha = struct.unpack('>H', dol[d+0x16:d+0x18])[0]
        lo = struct.unpack('>H', dol[d+0x1E:d+0x20])[0]
        if lo & 0x8000:
            ha -= 1
        auth_work = ((ha << 16) | lo) + 0x380

        lo_str = '0x{:04X}'.format(auth_work & 0xFFFF)
        if auth_work & 0x8000:
            lo_str = '-0x{:04X}'.format(0x10000 - (auth_work & 0xFFFF))

        LOAD_AUTH_REQ_INST_01_P = "lis %r5, 0x{:04X}; lwz %r5, {}(%r5)".format((auth_work + 0x8000) >> 16, lo_str)

    elif version == 'PB':
        LOAD_AUTH_REQ_INST_01 = "lwz %r5, -0x5064(%r13)"
        LOAD_AUTH_REQ_INST_02 = "addi %r5, %r5, 0x55D4"
        LOAD_AUTH_REQ_INST_01_P = LOAD_AUTH_REQ_INST_01
    else:
        exit('NO AUTH RESPONSE COMPATIBILITY ' + version)

    ADDRESS_BMST_RSO_LOCATOR = 0
    ADDRESS_GH_ALLOC_FUNCTION = 0
    ADDRESS_PES_ALLOC_FUNCTION = 0
    ADDRESS_HBM_ALLOCATOR = 0
    # --- FIND HBM ALLOCATOR
    # Fortune Street uses an RSO
    if title_name == 'ST7ED00':
        ADDRESS_BMST_RSO_LOCATOR = 0x8054CB0C
    elif title_name == 'ST7JD00':
        ADDRESS_BMST_RSO_LOCATOR = 0x8054CA0C
    elif title_name == 'ST7PD00':
        ADDRESS_BMST_RSO_LOCATOR = 0x8054CD0C
    # Battalion Wars uses an RSO loaded _in MEM2_, but it's luckily always in the same location
    elif title_name == 'RBWJD00' or title_name == 'RBWED00':
        ADDRESS_HBM_ALLOCATOR = 0x903FE054
    elif title_name == 'RBWPD00':
        ADDRESS_HBM_ALLOCATOR = 0x90438C54
    # Mario Sonic London Olympic uses RSO
    elif title_name == 'SIIED00' or title_name == 'SIIPD00' or title_name == 'SIIJD00':
        ADDRESS_HBM_ALLOCATOR = 0x900AC6FC
    elif title_name == 'SIIKD00':
        ADDRESS_HBM_ALLOCATOR = 0x900B483C
    # Sonic and the Black Knight
    elif title_name.startswith('REN'):
        ADDRESS_HBM_ALLOCATOR = 0x9006875C
    # Sonic Colors
    elif title_name.startswith('SNC'):
        ADDRESS_HBM_ALLOCATOR = 0x9015AA34
    # Guitar Hero Legends of Rock only allocates when the HBM is opened, so we need our own allocator
    elif title_name.startswith('RGH'):
        index = dol.find(b'\x38\xA0\x00\x00\x90\x01\x00\x14\x38\xC0\x00\x00\x93\xE1\x00\x0C\x7C\x7F\x1B\x78\x80\x64')
        if index == -1:
            exit('GH MISSING ALLOC FUNCTION ' + title_name)
        ADDRESS_GH_ALLOC_FUNCTION = dol_to_real(hdr, index-0xC)
        ADDRESS_HBM_ALLOCATOR = 0
    # Aerosmith same thing
    elif title_name.startswith('RGV'):
        index = dol.find(b'\x38\xA0\x00\x00\x90\x01\x00\x14\x38\xC0\x00\x00\x38\xE0\x00\x00\x93\xE1\x00\x0C\x7C\x7F\x1B\x78\x80\x64')
        if index == -1:
            exit('GH MISSING ALLOC FUNCTION ' + title_name)
        ADDRESS_GH_ALLOC_FUNCTION = dol_to_real(hdr, index-0xC)
        ADDRESS_HBM_ALLOCATOR = 0
    elif title_name.startswith('SUX') or title_name.startswith('SPV') or title_name.startswith('S2P') or title_name.startswith('S3I'):
        index = dol.find(b'\x7C\x7F\x1B\x78\x40\x82\x00\x40\x2C\x1D\x00\x19\x40\x82\x00\x1C')
        if index == -1:
            exit('PES MISSING ALLOC FUNCTION ' + title_name)

        assert(dol[index-0x34:index-0x30] == b'\x94\x21\xFF\xE0')
        ADDRESS_PES_ALLOC_FUNCTION = dol_to_real(hdr, index-0x34)
    elif title_name.startswith('DWE') or title_name.startswith('RWE') or title_name.startswith('R2W'):
        index = dol.find(b'\x7C\x64\x1B\x78\x80\x01\x00\x14\x7C\x83\x23\x78\x7C\x08\x03\xA6\x38\x21\x00\x10\x4E\x80\x00\x20\x94\x21\xFF\xF0')
        if index == -1:
            exit('OLD PES MISSING ALLOC FUNCTION ' + title_name)

        # Use the GH alloc function because the caller format is the same
        ADDRESS_GH_ALLOC_FUNCTION = dol_to_real(hdr, index+0x18)
    # Tales of Graces
    elif title_name == "STGJD00":
        # Use the GH alloc function because the caller format is the same
        ADDRESS_GH_ALLOC_FUNCTION = 0x802CF974
    else:
        # Other games with HBM in DOL
        index = dol.find('P1_Def.brlyt\0\0\0\0P2'.encode('ascii'))
        if index == -1:
            exit('MISSING P1_Def ' + title_name)

        if (index & 3) == 0 and dol[index-4] == 0x80:
            # print('P1_Def HBM ' + title_name)
            if (dol[index-1] & 7) != 0:
                exit('that\'s improper P1_Def ' + title_name)
            ADDRESS_HBM_ALLOCATOR = dol_to_real(hdr, index-4)
        else:
            # print('improper HBM ' + title_name)
            index = dol.find('N_Trans\0'.encode('ascii') + bytes([0x80]))
            if index == -1:
                exit('MISSING N_Trans ' + title_name)
            if (index & 7) == 0 and dol[index-4] == 0x80:
                ADDRESS_HBM_ALLOCATOR = dol_to_real(hdr, index-4)
            elif (index & 7) == 0 and dol[index-8] == 0x80:
                ADDRESS_HBM_ALLOCATOR = dol_to_real(hdr, index-8)
            else:
                exit('BAD N_Trans ' + title_name)

    # Super Smash Bros. Brawl only sets the HBM heap after pressing the home button for the first time
    # so we must set it ourselves manually
    ADDRESS_SSBB_GET_HEAP_FUNCTION = 0
    if title_name.startswith('RSB'):
        index = dol.find(b'\x38\x60\x00\x00\x7c\x84\x02\x14\x80\x84\x00\x04\x2c\x04\x00\x00\x4d\x82\x00\x20\x38\x64\x00\x40\x4e\x80\x00\x20')
        if index == -1:
            exit('SSBB MISSING GET HEAP FUNCTION ' + title_name)
        ADDRESS_SSBB_GET_HEAP_FUNCTION = dol_to_real(hdr, index-0xC)


    # ---- SEARCH NHTTP FUNCTIONS
    # NHTTPCreateRequestPattern = bytes([0x39,0x20,0x00,0x00,0x39,0x40,0x00,0x00,0x48,0x00,0x00,0x04])

    d = real_to_dol(hdr, ADDRESS_DWCi_Auth_SendRequest)
    index = dol[d:d+0x200].find(bytes([0x38,0xC0,0x10,0x00,0x39,0,0,0]))
    if index == -1:
        exit('CANNOT FIND ' + title_name)
    ADDRESS_NHTTPCreateRequest = decode_bl(dol, d+index+8)

    de = d + auth_func_size
    index = de - 0x34
    if dol[index:index+4] != bytes([0x7F,0xA3,0xEB,0x78]):
        if dol[index+4:index+8] == bytes([0x7F,0xA3,0xEB,0x78]) or dol[index+4:index+8] == b'\x7F\x83\xE3\x78':
            index += 4
        else:
            exit('BAD FIND ' + title_name)
    ADDRESS_NHTTPSendRequestAsync = decode_bl(dol, index+4)


    d = real_to_dol(hdr, ADDRESS_DWCi_Auth_HandleResponse)
    if title_name == 'RPBJD00' or title_name == 'RPBJD01' or title_name == 'RPBJD02':
        # weird, it's like the just decided to rewrite everything specifically for the
        # japanese version of pokemon battle revolution
        ADDRESS_NHTTPDestroyResponse = decode_bl(dol, d+(0x80324744 - 0x80324180))
    else:
        index = dol[d:d+0x60].find(b'\x3C\x60\x01\x00\x4C\xC6\x31\x82')
        if index == -1:
            exit('uhhhh ' + title_name)
        ADDRESS_NHTTPDestroyResponse = decode_bl(dol, d+index+0x10)

    # print("{} 0x{:08X} 0x{:08X} 0x{:08X}".format(title_name, ADDRESS_NHTTPCreateRequest, ADDRESS_NHTTPSendRequestAsync, ADDRESS_NHTTPDestroyResponse))

    # Search NHTTPStartup
    ADDRESS_NHTTPStartup = 0
    index = dol.find(b'\x7C\x60\x00\x34\x83\xE1\x00\x1C\x54\x00\xD9\x7E\x83\xC1\x00\x18\x83\xA1\x00\x14\x7C\x60\x00\xD0')
    if index != -1:
        assert(dol[index-0x60:index-0x5C] == b'\x94\x21\xFF\xE0')
        ADDRESS_NHTTPStartup = dol_to_real(hdr, index-0x60)

    if index == -1:
        index = dol.find(b'\x7C\x60\x00\x34\x83\xE1\x00\x1C\x54\x00\xD9\x7E\x83\xC1\x00\x18\x7C\x60\x00\xD0')
        if index != -1:
            assert(dol[index-0x38:index-0x34] == b'\x94\x21\xFF\xE0')
            ADDRESS_NHTTPStartup = dol_to_real(hdr, index-0x34)

    # RPBJ only
    if index == -1:
        index = dol.find(b'\x7C\x60\x00\x34\x54\x00\xD9\x7E\x7C\x60\x00\xD0\x80\x01\x00\x14\x7C\x08\x03\xA6\x38\x21\x00\x10')
        if index != -1:
            assert(dol[index-0x10:index-0xC] == b'\x94\x21\xFF\xF0')
            ADDRESS_NHTTPStartup = dol_to_real(hdr, index-0x10)

    if index == -1:
        exit("NHTTPStartup not found " + title_name)
        

    ADDRESS_gethostbyname = 0
    ADDRESS_SKIP_DNS_CACHE = 0
    ADDRESS_SKIP_DNS_CACHE_CONTINUE = 0

    index = dol.find(b"Failed to cache DNS query.")
    if index != -1:
        index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xDC\x38\x7F\x00\xF4')
        if index != -1:
            # print(title_name + " login ver R3BE "  + dol[index+8:index+12].hex())
            pass

        if index == -1:
            index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xE0\x38\x7E\x00\xD0')
            if index != -1:
                # print(title_name + " login ver R4QE "  + dol[index+8:index+12].hex())
                pass

        if index == -1:
            index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xD8\x38\x7F\x01\x10')
            if index != -1:
                #print(title_name + " login ver R4QE "  + dol[index+8:index+12].hex())
                pass

        if index == -1:
            index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xE0\x38\x7E\x00\xE0')
            if index != -1:
                # print(title_name + " login ver R4QP "  + dol[index+8:index+12].hex())
                pass

        if index == -1:
            index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xDC\x38\x7F\x01\x10')
            if index != -1:
                # print(title_name + " login ver RUUE "  + dol[index+8:index+12].hex())
                pass

        if index == -1:
            index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xDC\x38\x7F\x01\x08')
            if index != -1:
                # print(title_name + " login ver WL2E "  + dol[index+8:index+12].hex())
                pass

        if index == -1:
            index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xE0\x38\x7E\x00\x78')
            if index != -1:
                # print(title_name + " login ver RXPJ "  + dol[index+8:index+12].hex())
                pass

        if index == -1:
            index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xE0\x38\x7F\x01\x4C')
            if index != -1:
                # print(title_name + " login ver WL2E "  + dol[index+8:index+12].hex())
                pass

        if index == -1:
            index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xE0\x38\x7E\x03\x28')
            if index != -1:
                # print(title_name + " login ver RGSP "  + dol[index+8:index+12].hex())
                pass

        if index == -1:
            index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xE0\x38\x7E\x02\xC0')
            if index != -1:
                # print(title_name + " login ver RNRE "  + dol[index+8:index+12].hex())
                pass

        if index == -1:
            index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xE0\x38\x7E\x03\x18')
            if index != -1:
                # print(title_name + " login ver RTKE "  + dol[index+8:index+12].hex())
                pass

        if index == -1:
            print(title_name + " IS A LOST")
            index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xE0\x38')
            if index != -1:
                print(title_name + " BUT THEN FOUND E0 TYPE " + dol[index+8:index+12].hex() + " 0x{:08X}".format(dol_to_real(hdr, index)))
            index = dol.find(b'\x2C\x03\x00\x00\x41\x82\x01\xDC\x38')
            if index != -1:
                print(title_name + " BUT THEN FOUND DC TYPE " + dol[index+8:index+12].hex())
            exit()

        ADDRESS_gethostbyname = decode_bl(dol, index+12)
        d = real_to_dol(hdr, ADDRESS_gethostbyname)
        # print(dol[d:d+0x10].hex())
        ADDRESS_SKIP_DNS_CACHE = dol_to_real(hdr, index-4)

        next_index = dol[index:index+0x200].find(b"\x38\x60\x00\x08\x4C\xC6\x31\x82")
        if next_index == -1:
            exit(title_name + " WHAT DNS PRINT CACHE TGHING NOT FOUND")
        ADDRESS_SKIP_DNS_CACHE_CONTINUE = dol_to_real(hdr, index+next_index+0xC)

    # find SOSocket
    # 40 82 00 7C 2C 1C 00 17 40 82 00 0C
    index = dol.find(b'\x40\x82\x00\x7C\x2C\x1C\x00\x17\x40\x82\x00\x0C')
    ADDRESS_SOSocket = 0
    if index == -1:
        exit('Missing find SOSocket ' + title_name)
    if dol[index-0x54:index-0x50] == b'\x94\x21\xFF\xE0':
        ADDRESS_SOSocket = dol_to_real(hdr, index - 0x54)
    elif dol[index-0x38:index-0x34] == b'\x94\x21\xFF\xE0':
        ADDRESS_SOSocket = dol_to_real(hdr, index - 0x38)
    else:
        exit('Missing verify SOSocket ' + title_name)

    # find SOClose
    # 40 82 00 64 38 60 00 0C 38 80 00 20
    index2 = dol[index:].find(b'\x40\x82\x00\x64\x38\x60\x00\x0C\x38\x80\x00\x20')
    ADDRESS_SOClose = 0
    if index2 == -1:
        exit('Missing find SOClose ' + title_name)
    index = index + index2
    assert(dol[index-0x28:index-0x24] == b'\x94\x21\xFF\xE0')
    ADDRESS_SOClose = dol_to_real(hdr, index - 0x28)

    # SOiAlloc and SOiFree
    index -= 0x28
    ADDRESS_SOiAlloc = decode_bl(dol, index + 0x34)
    ADDRESS_SOiFree = decode_bl(dol, index + 0x7C)

    # find SOBind
    # 80 61 00 08 7F C5 F3 78 38 80 00 02 38 C0 00 24 38 E0 00 00 39 00 00 00
    index2 = dol[index:].find(b'\x80\x61\x00\x08\x7F\xC5\xF3\x78\x38\x80\x00\x02\x38\xC0\x00\x24\x38\xE0\x00\x00\x39\x00\x00\x00')
    ADDRESS_SOBind = 0
    if index2 == -1:
        exit('Missing find SOBind ' + title_name)
    index = index + index2
    if dol[index-0x90:index-0x8C] == b'\x94\x21\xFF\xE0':
        ADDRESS_SOBind = dol_to_real(hdr, index - 0x90)
    elif dol[index-0x8C:index-0x88] == b'\x94\x21\xFF\xE0':
        ADDRESS_SOBind = dol_to_real(hdr, index - 0x8C)
    else:
        exit('Missing verify SOBind ' + title_name)

    # find SOConnect
    # 80 61 00 08 7F C5 F3 78 38 80 00 04 38 C0 00 24 38 E0 00 00 39 00 00 00
    index2 = dol[index:].find(b'\x80\x61\x00\x08\x7F\xC5\xF3\x78\x38\x80\x00\x04\x38\xC0\x00\x24\x38\xE0\x00\x00\x39\x00\x00\x00')
    ADDRESS_SOConnect = 0
    if index2 == -1:
        exit('Missing find SOConnect ' + title_name)
    index = index + index2
    if dol[index-0x90:index-0x8C] == b'\x94\x21\xFF\xE0':
        ADDRESS_SOConnect = dol_to_real(hdr, index - 0x90)
    elif dol[index-0x8C:index-0x88] == b'\x94\x21\xFF\xE0':
        ADDRESS_SOConnect = dol_to_real(hdr, index - 0x8C)

    # find SOGetSockName
    # 89 1B 00 00 7F 87 E3 78 38 80 00 07 38 C0 00 04
    index2 = dol[index:].find(b'\x89\x1B\x00\x00\x7F\x87\xE3\x78\x38\x80\x00\x07\x38\xC0\x00\x04')
    ADDRESS_SOGetSockName = 0
    if index2 == -1:
        exit('Missing find SOGetSockName ' + title_name)
    index = index + index2
    if dol[index-0x98:index-0x94] == b'\x94\x21\xFF\xD0':
        ADDRESS_SOGetSockName = dol_to_real(hdr, index - 0x98)
    elif dol[index-0x94:index-0x90] == b'\x94\x21\xFF\xD0':
        ADDRESS_SOGetSockName = dol_to_real(hdr, index - 0x94)
    else:
        exit('Missing verify SOGetSockName ' + title_name)

    # find SORecvFrom
    # 7C 8A 23 78 7C A9 2B 78 7C C0 33 78 7C E8 3B 78 7C 64 1B 78 7D 45 53 78 7D 26 4B 78 7C 07 03 78 38 60 00 00
    index2 = dol[index:].find(b'\x7C\x8A\x23\x78\x7C\xA9\x2B\x78\x7C\xC0\x33\x78\x7C\xE8\x3B\x78\x7C\x64\x1B\x78\x7D\x45\x53\x78\x7D\x26\x4B\x78\x7C\x07\x03\x78\x38\x60\x00\x00')
    ADDRESS_SORecvFrom = 0
    if index2 == -1:
        exit('Missing find SORecvFrom ' + title_name)
    index = index + index2
    ADDRESS_SORecvFrom = dol_to_real(hdr, index)

    ADDRESS_SORecv = 0
    if dol[index+0x28:index+0x2C] == b'\x7C\x88\x23\x78':
        ADDRESS_SORecv = ADDRESS_SORecvFrom + 0x28
        index = index + 0x28
    elif dol[index+0x28:index+0x34] == b'\x00\x00\x00\x00\x00\x00\x00\x00\x7C\x88\x23\x78':
        ADDRESS_SORecv = ADDRESS_SORecvFrom + 0x30
        index = index + 0x30
    else:
        exit('Missing verify SORecv ' + title_name)

    ADDRESS_SOSendTo = 0
    if dol[index+0x24:index+0x28] == b'\x7C\x8A\x23\x78':
        ADDRESS_SOSendTo = ADDRESS_SORecv + 0x24
        index = index + 0x24
    elif dol[index+0x24:index+0x34] == b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7C\x8A\x23\x78':
        ADDRESS_SOSendTo = ADDRESS_SORecv + 0x30
        index = index + 0x30
    else:
        exit('Missing verify SOSendTo ' + title_name)

    ADDRESS_SOSend = 0
    if dol[index+0x28:index+0x2C] == b'\x7C\x88\x23\x78':
        ADDRESS_SOSend = ADDRESS_SOSendTo + 0x28
        index = index + 0x28
    elif dol[index+0x28:index+0x34] == b'\x00\x00\x00\x00\x00\x00\x00\x00\x7C\x88\x23\x78':
        ADDRESS_SOSend = ADDRESS_SOSendTo + 0x30
        index = index + 0x30
    else:
        exit('Missing verify SOSend ' + title_name)

    # find SOFcntl
    # 39 61 00 98 38 01 00 08 3D 80 02 00
    index2 = dol[index:].find(b'\x39\x61\x00\x98\x38\x01\x00\x08\x3D\x80\x02\x00')
    ADDRESS_SOFcntl = 0
    if index2 == -1:
        exit('Missing find SOFcntl ' + title_name)
    index = index + index2
    if dol[index-0x48:index-0x44] == b'\x94\x21\xFF\x70':
        ADDRESS_SOFcntl = dol_to_real(hdr, index - 0x48)
    elif dol[index-0x4C:index-0x48] == b'\x94\x21\xFF\x70':
        ADDRESS_SOFcntl = dol_to_real(hdr, index - 0x4C)
    else:
        exit('Missing verify SOFcntl ' + title_name)

    # find SOShutdown
    # 40 82 00 0C 3B E0 FF CF 48 00 00 3C 93 A3 00 00
    index2 = dol[index:].find(b'\x40\x82\x00\x0C\x3B\xE0\xFF\xCF\x48\x00\x00\x3C\x93\xA3\x00\x00')
    ADDRESS_SOShutdown = 0
    if index2 == -1:
        exit('Missing find SOShutdown ' + title_name)
    index = index + index2
    if dol[index-0x48:index-0x44] == b'\x94\x21\xFF\xE0':
        ADDRESS_SOShutdown = dol_to_real(hdr, index - 0x48)
    else:
        exit('Missing verify SOShutdown ' + title_name)

    # find SOPoll
    # 1F BA 00 0C 38 60 00 0C 38 1D 00 3F
    index2 = dol[index:].find(b'\x1F\xBA\x00\x0C\x38\x60\x00\x0C\x38\x1D\x00\x3F')
    ADDRESS_SOPoll = 0
    if index2 == -1:
        exit('Missing find SOPoll ' + title_name)
    index = index + index2
    if dol[index-0x48:index-0x44] == b'\x94\x21\xFF\xD0':
        ADDRESS_SOPoll = dol_to_real(hdr, index - 0x48)
    else:
        exit('Missing verify SOPoll ' + title_name)

    # find SOInetAtoN
    index2 = dol[index:].find(b'\x40\x82\x00\x0C\x3B\xC0\xFF\xE4\x48\x00\x00\xA4')
    ADDRESS_SOInetAtoN = 0
    if index == -1:
        exit('Missing find SOInetAtoN ' + title_name)
    index = index + index2
    assert(dol[index-0x38:index-0x34] == b'\x94\x21\xFF\xD0')
    ADDRESS_SOInetAtoN = dol_to_real(hdr, index - 0x38)

    # find SOGetAddrInfo
    index2 = dol[index:].find(b'\x38\x63\x00\x01\x38\x63\x00\x1F\x38\x1D\x00\x1F\x54\x64\x00\x34')
    ADDRESS_SOGetAddrInfo = 0
    if index == -1:
        exit('Missing find SOGetAddrInfo ' + title_name)
    index = index + index2
    assert(dol[index-0x6C:index-0x68] == b'\x94\x21\xFF\xC0')
    ADDRESS_SOGetAddrInfo = dol_to_real(hdr, index - 0x6C)

    # find SOFreeAddrInfo
    index = index + 0x278
    ADDRESS_SOFreeAddrInfo = 0
    if dol[index:index+4] == b'\x94\x21\xFF\xF0':
        ADDRESS_SOFreeAddrInfo = dol_to_real(hdr, index)
    # or 4E 80 00 20 00 00 00 00 00 00 00 00 94 21 FF F0
    elif dol[index:index+0x10] == b'\x4E\x80\x00\x20\x00\x00\x00\x00\x00\x00\x00\x00\x94\x21\xFF\xF0':
        ADDRESS_SOFreeAddrInfo = dol_to_real(hdr, index+0xC)
    else:
        exit('Missing verify SOFreeAddrInfo ' + title_name)

    # find SOSetSockOpt
    # 80 61 00 08 7F C5 F3 78 38 80 00 09 38 C0 00 24 38 E0 00 00 39 00 00 00
    index2 = dol[index:].find(b'\x80\x61\x00\x08\x7F\xC5\xF3\x78\x38\x80\x00\x09\x38\xC0\x00\x24\x38\xE0\x00\x00\x39\x00\x00\x00')
    ADDRESS_SOSetSockOpt = 0
    if index2 == -1:
        exit('Missing find SOSetSockOpt ' + title_name)
    index = index + index2
    if dol[index-0xB0:index-0xAC] == b'\x94\x21\xFF\xD0':
        ADDRESS_SOSetSockOpt = dol_to_real(hdr, index - 0xB0)
    elif dol[index-0xA8:index-0xA4] == b'\x94\x21\xFF\xD0':
        ADDRESS_SOSetSockOpt = dol_to_real(hdr, index - 0xA8)
    else:
        exit('Missing verify SOSetSockOpt ' + title_name)

    # find SOGetInterfaceOpt
    # 40 82 01 8C 38 19 EF FF 28 00 00 01 41 81 00 0C
    index2 = dol[index:].find(b'\x40\x82\x01\x8C\x38\x19\xEF\xFF\x28\x00\x00\x01\x41\x81\x00\x0C')
    ADDRESS_SOGetInterfaceOpt = 0
    if index2 == -1:
        exit('Missing find SOGetInterfaceOpt ' + title_name)
    index = index + index2
    if dol[index-0x38:index-0x34] == b'\x94\x21\xFF\xD0':
        ADDRESS_SOGetInterfaceOpt = dol_to_real(hdr, index - 0x38)
    else:
        exit('Missing verify SOGetInterfaceOpt ' + title_name)

    old_index = 0
    didskipent = False
    index = -1
    while True:
        next_index = dol[old_index:].find(b'%s.available.')
        if next_index == -1:
            print('no find available ' + title_name)
            break

        next_index = old_index + next_index
        e = next_index + len(b'%s.available.')
        old_index = next_index + 1

        skipent = False
        for i in range(80):
            if dol[e+i:e+i+3] == b'%s.':
                # print('skip ent 1 ' + title_name)
                skipent = True
                didskipent = True
                break

        if skipent:
            continue

        index = next_index
        break

    if index == -1:
        print('Missing find available ' + title_name)

    ADDRESS_AVAILABLE_URL = dol_to_real(hdr, index)

    d = real_to_dol(hdr, ADDRESS_DWC_AUTH_ADD_MACADDR)
    ADDRESS_DWC_Printf = decode_bl(dol, d+0x78)

    d = real_to_dol(hdr, ADDRESS_DWC_Printf)
    index = dol[d:d+0x200].find(b'\x00\x00\x4C\xC6\x31\x82')
    if index == -1:
        exit('Missing find OSReport ' + title_name)

    ADDRESS_OSReport = decode_bl(dol, d+index+6)

    ADDRESS_GHIPARSEURL_HTTPS_PATCH = 0
    # search for the ghiParseURL force http patch
    if dol.find(b'GameSpyHTTP/') != -1:
        #match = re.search(b'\x38\x00\x00\x01(\x38|\x39|\x3A|\x3B).\x00\x08\x90', dol, flags=re.DOTALL)
        #if match == None:
            #exit('Missing find ghiParseURL ' + title_name)

        #ADDRESS_GHIPARSEURL_HTTPS_PATCH = dol_to_real(hdr, match.start(0)+4)

        index = dol.find(b'\x40\x82\x00\x14\x38\x00\x00\x01\x90\x1F\x00\x28')
        if index != -1:
            ADDRESS_GHIPARSEURL_HTTPS_PATCH = dol_to_real(hdr, index+4)
        else:
            match = re.search(b'\\\x40\\\x82\\\x00\\\x14\x38\\\x00\\\x00\\\x01....\\\x90\\\x1F\\\x00\\\x28', dol, flags=re.DOTALL)
            if match == None:
                exit('Missing find ghiParseURL ' + title_name)

            ADDRESS_GHIPARSEURL_HTTPS_PATCH = dol_to_real(hdr, match.start(0)+4)

    # search for the NHTTP force http patch
    index = -1
    ADDRESS_NHTTP_HTTPS_PORT_PATCH = 0
    ADDRESS_NHTTPi_SocSSLConnect = 0
    if title_name.startswith('RPBJD'):
        index = dol.find(b'\x40\x82\x00\x10\x38\x00\x01\xBB\xB0\x1F\x00\x20')
        if index == -1:
            print('Missing find RPBJ NHTTP https patch ' + title_name)
            return
        ADDRESS_NHTTP_HTTPS_PORT_PATCH = dol_to_real(hdr, index+4)

        index = dol.find(b'\x7C\x7D\x1B\x78\x80\x63\x00\xC8\x80\x9D\x00\x18')
        if index == -1:
            print('Missing find RPBJ NHTTP https SSL patch ' + title_name)
            return
        ADDRESS_NHTTPi_SocSSLConnect = dol_to_real(hdr, index-0x20)
    else:
        index = dol.find(b'\x38\x60\x00\x01\x38\x00\x01\xBB\x90\x78\x00\x08\x3B\xA0\x00\x08\x90\x18\x00\x20')
        if index != -1:
            index += 4

        if index == -1:
            index = dol.find(b'\x38\x60\x00\x01\x38\x00\x01\xBB\x90\x77\x00\x08\x3B\xA0\x00\x08\x90\x17\x00\x20')
            if index != -1:
                index += 4

        if index == -1:
            index = dol.find(b'\x38\x00\x00\x01\x90\x17\x00\x08\x38\x00\x01\xBB\x3B\xA0\x00\x08\x90\x17\x00\x20')
            if index == -1:
                print('Missing find NHTTP https port patch ' + title_name)
                return
            index += 8

        ADDRESS_NHTTP_HTTPS_PORT_PATCH = dol_to_real(hdr, index)

        index = dol.find(b'\x7C\x9B\x23\x78\x80\x65\x00\xCC\x7C\xBC\x2B\x78')
        if index == -1:
            print('Missing find NHTTP SSL patch ' + title_name)
            return
        assert(dol[index-0x18:index-0x14] == b'\x94\x21\xFF\xE0')

        ADDRESS_NHTTPi_SocSSLConnect = dol_to_real(hdr, index-0x18)

        #index2 = dol[index:index+0x400].find(b'\x54\x63\x0F\xFE\x38\x63\x00\x07')
        #if index2 == -1:
            #print('Missing find NHTTP https patch pt 2 ' + title_name)
            #return
        #ADDRESS_NHTTP_HTTPS_PATCH_02 = dol_to_real(hdr, index+index2+4)

        #index3 = dol[index:index+0x400].find(b'\x54\x04\x0F\xFE\x38\x04\x00\x07\x7C\x86\x02\x14')
        #if index3 == -1:
            #print('Missing find NHTTP https patch pt 3 ' + title_name)
            #return
        #ADDRESS_NHTTP_HTTPS_PATCH_03 = dol_to_real(hdr, index+index3+4)


    # here is the exploit searching
    # Match command exploit, more information in payload/wwfcSecurity.cpp

    ADDRESS_PATCH_SECURITY_GT2MATCHCOMMAND = 0

    # contains the GT2 exploit
    # this one can be exploited peer to peer so it's highest priority

    index = dol.find(b'\x48\x00\x00\x48\x38\x61\x00\x1C\x38\x9E\x00\x14')
    if index != -1:
        assert(dol[index+0x10:index+0x14] == b'\x88\xA1\x00\x10')
        assert(dol[index-0x14:index-0x10] == b'\x41\x82\x00\x18')
        ADDRESS_PATCH_SECURITY_GT2MATCHCOMMAND = dol_to_real(hdr, index+4)
    else:
        if dol.find(b'<GT2> RECV') != -1 or dol.find(b'GT2 command') != -1:
            exit('CANNOT FIND SECURITY GT2 RECV thing ' + title_name)

    ADDRESS_PATCH_SECURITY_QR2MATCHCOMMAND = 0

    # contains the QR2 exploit

    index = dol.find(b'\x48\x00\x00\x74\x88\xA1\x00\x11\x38\x61\x00\x1C\x38\x9C\x00\x14')
    if index != -1:
        assert(dol[index+0x14:index+0x18] == b'\x88\xA1\x00\x10')
        assert(dol[index-0x14:index-0x10] == b'\x41\x82\x00\x18')
        ADDRESS_PATCH_SECURITY_QR2MATCHCOMMAND = dol_to_real(hdr, index+4)
    else:
        index = dol.find(b'\x48\x00\x00\x78\x88\xA1\x00\x11\x38\x61\x00\x1C\x38\x9C\x00\x14')
        if index == -1:
            if dol.find(b'<SB> RECV') != -1 or dol.find(b'SBcommand') != -1:
                # some wiiware games such as W2FPN0005 have this weird handling of qr2 messages,
                # where like the strings for the function are there but the actual code isn't,
                # and there is this big function in place of the actual handling of sb commands
                # TODO: we should figure out what the purpose of this is
                if dol.find(b'\x88\x64\x00\x00\x2C\x03\x00\x3B\x40\x82\x00\x28\x81') == -1:
                    if dol.find(b'\x8B\xA4\x00\x00\x2C\x1D\x00\x3B\x40\x82\x00\x28\x81') == -1:
                        print('CANNOT FIND SECURITY QR2 RECV thing ' + title_name)
        else:
            assert(dol[index+0x14:index+0x18] == b'\x88\xA1\x00\x10')
            assert(dol[index-0x14:index-0x10] == b'\x41\x82\x00\x18')
            ADDRESS_PATCH_SECURITY_QR2MATCHCOMMAND = dol_to_real(hdr, index+4)

    # find gpiSendLogin
    match = re.search(b'\x7F\x83\xE3\x78\x38\x9E\x02\x10\x38...\x4B...\x39\x61\x02\xB0\x38\x60\x00\x00', dol, flags=re.DOTALL)
    if match == None:
        exit('Missing find gpiSendLogin ' + title_name)

    ADDRESS_PATCH_GPISENDLOGIN = dol_to_real(hdr, match.start(0))
    ADDRESS_gpiAppendStringToBuffer = decode_bl(dol, match.start(0)+0xC)
    ADDRESS_gpiAppendIntToBuffer = 0

    if dol[match.start(0)-0x38:match.start(0)-0x35].hex() == "38a000":
        ADDRESS_gpiAppendIntToBuffer = decode_bl(dol, match.start(0)-0x34)
    elif dol[match.start(0)-0x18:match.start(0)-0x15].hex() == "38a000":
        ADDRESS_gpiAppendIntToBuffer = decode_bl(dol, match.start(0)-0x14)
    else:
        exit(title_name + " GPISENDLOGIN " + dol[match.start(0)-0x34:match.start(0)-0x10].hex())

    # find gpiAddLocalInfo hook
    ADDRESS_gpiAddLocalInfo = 0
    index = dol.find(b'\x38\x00\x00\x00\x90\x1E\x05\xFC\x83\xE1\x00\x1C\x38\x60\x00\x00\x83\xC1\x00\x18')
    if index != -1:
        ADDRESS_PATCH_GPIADDLOCALINFO = dol_to_real(hdr, index + 8)

    if index == -1:
        index = dol.find(b'\x38\x00\x00\x00\x90\x1E\x05\xF8\x83\xE1\x00\x1C\x38\x60\x00\x00\x83\xC1\x00\x18')
        if index != -1:
            ADDRESS_PATCH_GPIADDLOCALINFO = dol_to_real(hdr, index + 8)

    if index == -1:
        index = dol.find(b'\x38\x00\x00\x00\x90\x1E\x04\x74\x83\xE1\x00\x1C\x38\x60\x00\x00\x83\xC1\x00\x18')
        if index != -1:
            ADDRESS_PATCH_GPIADDLOCALINFO = dol_to_real(hdr, index + 8)

    if index == -1:
        print('No find gpiAddLocalInfo ' + title_name)

    d = real_to_dol(hdr, ADDRESS_DWC_AUTH_ADD_MACADDR)
    ADDRESS_DWC_Base64Encode = decode_bl(dol, d + 0x48)

    ADDRESS_DWCi_GetUserData = 0
    index = 0
    while True:
        index2 = dol[index:].find(b'\x2C\x03\x00\x00\x41\x82\x00\x0C\x80\x63\x00\x1C\x4E\x80\x00\x20\x38\x60\x00\x00\x4E\x80\x00\x20')
        if index2 == -1:
            if ADDRESS_DWCi_GetUserData != 0:
                break
            exit('Missing DWCi_GetUserData ' + title_name)

        index = index2 + index

        if dol[index+0x18:index+0x1C] != b'\x94\x21\xFF\xF0':
            if dol[index+0x18:index+0x20] != b'\x00\x00\x00\x00\x94\x21\xFF\xF0':
                index += 1
                continue

        if dol[index-0x4:index-0x2] == b'\x80\x6D':
            if ADDRESS_DWCi_GetUserData != 0:
                exit('Duplicate DWCi_GetUserData ' + title_name)

            ADDRESS_DWCi_GetUserData = dol_to_real(hdr, index-4)
        elif dol[index-0x8:index-0x6] == b'\x3C\x60' and dol[index-0x4:index-0x2] == b'\x80\x63':
            if ADDRESS_DWCi_GetUserData != 0:
                exit('Duplicate DWCi_GetUserData ' + title_name)

            ADDRESS_DWCi_GetUserData = dol_to_real(hdr, index-8)

        index += 1

    ADDRESS_DWCi_GetGPBuddyAdditionalMsg = 0
    index = dol.find(b'\x7F\xFC\x18\x50\x7F\x63\xDB\x78\x7F\x84\xE3\x78\x7F\xE5\xFB\x78')
    if index != -1:
        assert(dol[index-0x8C:index-0x88] == b'\x94\x21\xFF\xE0')
        ADDRESS_DWCi_GetGPBuddyAdditionalMsg = dol_to_real(hdr, index-0x8C)

    # find OSInitSystemCall
    index = dol.find(b'\x7C\x00\x04\xAC\x38\x7F\x0C\x00\x38\x80\x01\x00')
    if index == -1:
        exit('Missing find OSInitSystemCall tail ' + title_name)

    ADDRESS_OSInitSystemCall_Tail = dol_to_real(hdr, index-0xC)

    # find gti2ReceiveMessages buffer
    ADDRESS_GTI2_BUFFER = 0

    # example jaderevo.elf (i think that's RGW? i don't actually remember)
    index = dol.find(b'\x3F\xA0\x00\x01\x48\x00\x01\xE4\x93\x61\x00\x08\x38\x9C')
    ha = 0
    lo = 0
    if index != -1:
        ha = struct.unpack('>H', dol[index-0x2:index])[0]
        lo = struct.unpack('>H', dol[index+0xE:index+0x10])[0]

    if index == -1:
        # example HDME
        index = dol.find(b'\x3F\x80\x00\x01\x48\x00\x01\xDC\x93\x41\x00\x08\x38\x9B')
        if index != -1:
            ha = struct.unpack('>H', dol[index-0x2:index])[0]
            lo = struct.unpack('>H', dol[index+0xE:index+0x10])[0]

    if index == -1:
        # example WL2EN0002
        index = dol.find(b'\x3F\xE0\x00\x01\x48\x00\x03\xA4\x93\xA1\x00\x08\x38\x9E')
        if index != -1:
            ha = struct.unpack('>H', dol[index-0x2:index])[0]
            lo = struct.unpack('>H', dol[index+0xE:index+0x10])[0]

    if index == -1:
        # example RUUED00
        index = dol.find(b'\x48\x00\x01\xE4\x93\x81\x00\x08\x38\x9D')
        if index != -1:
            assert(dol[index+0xC:index+0x10] == b'\x38\xE1\x00\x10')
            ha = struct.unpack('>H', dol[index-0x2:index])[0]
            lo = struct.unpack('>H', dol[index+0xA:index+0xC])[0]

    if index == -1:
        # example Mario Kart Wii (RMC)
        index = dol.find(b'\x48\x00\x01\xF4\x93\x81\x00\x08\x38\x9D')
        if index != -1:
            assert(dol[index+0xC:index+0x10] == b'\x38\xBC\xFF\xFF' or dol[index+0xC:index+0x10] == b'\x38\xE1\x00\x10')
            ha = struct.unpack('>H', dol[index-0x2:index])[0]
            lo = struct.unpack('>H', dol[index+0xA:index+0xC])[0]

    if index == -1:
        # example E5XJN0000
        index = dol.find(b'\x48\x00\x03\xA4\x93\xC1\x00\x08\x38\x9F')
        if index != -1:
            assert(dol[index+0xC:index+0x10] == b'\x38\xE1\x00\x10')
            ha = struct.unpack('>H', dol[index-0x2:index])[0]
            lo = struct.unpack('>H', dol[index+0xA:index+0xC])[0]

    if index == -1:
        # example HCFKN0200
        index = dol.find(b'\x48\x00\x03\xAC\x93\xC1\x00\x08\x38\x9F')
        if index != -1:
            assert(dol[index+0xC:index+0x10] == b'\x38\xE1\x00\x10')
            ha = struct.unpack('>H', dol[index-0x2:index])[0]
            lo = struct.unpack('>H', dol[index+0xA:index+0xC])[0]

    if index == -1:
        exit('Missing find gti2ReceiveMessages buffer ' + title_name)

    if lo & 0x8000:
        ha -= 1
    ADDRESS_GTI2_BUFFER = (ha << 16) | lo

    # find qr_process_client_message end
    index = dol.find(b'\x4E\x80\x04\x21\x48\x00\x00\x24\x81\x83\x00\xA4\x2C\x0C\x00\x00\x41\x82\x00\x18\x7C\x83\x23\x78\x7C\xA4\x2B\x78\x80\xBF\x01')
    if index == -1:
        print('Missing find qr_process_client_message end ' + title_name)
        ADDRESS_SBCM_RETURN = 0
    else:
        ADDRESS_SBCM_RETURN = dol_to_real(hdr, index+0x28)

    # find OSYieldThread
    index = dol.find(b'\x7C\x7F\x1B\x78\x38\x60\x00\x01\x4B\xFF\xFD\xA5\x7F\xE3\xFB\x78\x4B\xFF')
    if index == -1:
        # print('No find OSYieldThread ' + title_name)
        ADDRESS_OSYieldThread = 0
    else:
        assert(dol[index-0x14:index-0x10] == b'\x94\x21\xFF\xF0')
        ADDRESS_OSYieldThread = dol_to_real(hdr, index - 0x14)

    # find sendto
    index = dol.find(b'\x7C\xDE\x33\x78\x7C\xE4\x3B\x78\x7D\x1F\x43\x78\x38\x61\x00\x08\x38\xA0\x00\x08')
    if index == -1:
        print('No find sendto ' + title_name)
        ADDRESS_sendto = 0
    else:
        assert(dol[index-0x20:index-0x1C] == b'\x94\x21\xFF\xD0')
        ADDRESS_sendto = dol_to_real(hdr, index - 0x20)


    # find stpLoginCnt
    ADDRESS_stpLoginCnt = 0
    d = real_to_dol(hdr, ADDRESS_DWCi_GetUserData)
    if dol[d:d+2] == b'\x80\x6D':
        offset = struct.unpack('>H', dol[d+2:d+4])[0]
        if offset & 0x8000:
            offset -= 0x10000
        ADDRESS_stpLoginCnt = ADDRESS_R13_BASE + offset
    elif dol[d:d+2] == b'\x3C\x60' and dol[d+4:d+6] == b'\x80\x63':
        ha = struct.unpack('>H', dol[d+2:d+4])[0]
        lo = struct.unpack('>H', dol[d+6:d+8])[0]
        if lo & 0x8000:
            ha -= 1
        ADDRESS_stpLoginCnt = (ha << 16) | lo
    else:
        exit('bad stpLoginCnt access ' + title_name + " " + dol[d:d+12].hex())

    # find stpFriendCnt
    ADDRESS_stpFriendCnt = 0
    index = dol.find(b'\x48\x00\x00\x0C\x3B\xE0\x00\x06\x38\x60\xFF\xEC\x80\x0D')
    if index != -1:
        offset = struct.unpack('>H', dol[index+0x0E:index+0x10])[0]
        if offset & 0x8000:
            offset -= 0x10000
        ADDRESS_stpFriendCnt = ADDRESS_R13_BASE + offset

    if index == -1:
        index = dol.find(b'\x48\x00\x00\x0C\x3B\xA0\x00\x06\x38\x60\xFF\xEC\x3F\xC0\x80')
        if index != -1:
            ha = struct.unpack('>H', dol[index+0x0E:index+0x10])[0]
            lo = struct.unpack('>H', dol[index+0x12:index+0x14])[0]
            if lo & 0x8000:
                ha -= 1
            ADDRESS_stpFriendCnt = (ha << 16) | lo

    if index == -1:
        exit('No find stpFriendCnt ' + title_name)

    # get s_auth_result
    ADDRESS_AUTH_RESULT = 0
    d = real_to_dol(hdr, ADDRESS_DWCi_Auth_HandleResponse)
    if (dol[d+0x14:d+0x16] == b'\x3F\x40' and dol[d+0x1C:d+0x1E] == b'\x3B\x5A') or (dol[d+0x14:d+0x16] == b'\x3E\xC0' and dol[d+0x1C:d+0x1E] == b'\x3A\xD6'):
        ha = struct.unpack('>H', dol[d+0x16:d+0x18])[0]
        lo = struct.unpack('>H', dol[d+0x1E:d+0x20])[0]
        if lo & 0x8000:
            ha -= 1
        ADDRESS_AUTH_RESULT = (ha << 16) | lo
    elif title_name.startswith('RPBJD'):
        ha = struct.unpack('>H', dol[d+0x112:d+0x114])[0]
        lo = struct.unpack('>H', dol[d+0x122:d+0x124])[0]
        if lo & 0x8000:
            ha -= 1
        ADDRESS_AUTH_RESULT = (ha << 16) | lo
    else:
        print('bad s_auth_result access ' + title_name)

    # find natneg wait patch
    ADDRESS_NATNEG_SET_COMPLETED_DELAY = 0
    index = dol.find(b'\x38\x03\x13\x88\x90\x1F\x00\x2C\x48\x00\x00\xF4\x88\x1E\x00\x13')
    if index != -1:
        ADDRESS_NATNEG_SET_COMPLETED_DELAY = dol_to_real(hdr, index)
    else:
        index = dol.find(b'\x38\x03\x13\x88\x90\x1E\x00\x2C\x2C\x05\xFF\xFF\x41\x82\x01\x08')
        if index != -1:
            ADDRESS_NATNEG_SET_COMPLETED_DELAY = dol_to_real(hdr, index)
        else:
            # some games don't have this code I guess?
            # print('No find natneg wait patch ' + title_name)
            pass

    # find gti2CreateSocket
    if False:
        ADDRESS_gti2CreateSocket = 0
        index = dol.find(b'\x2C\x1C\x00\x00\x40\x82\x00\x08\x3F\x80\x00\x01\x2C\x1B\x00\x00\x40\x82\x00\x08\x3F\x60\x00\x01\x7F\xE3\xFB\x78\x38\x81\x00\x10\x38\xA1\x00\x08')
        if index != -1:
            if dol[index-0x30:index-0x2C] != b'\x94\x21\xFF\xC0':
                exit('bad gti2CreateSocket ' + title_name)
            ADDRESS_gti2CreateSocket = dol_to_real(hdr, index-0x30)
        else:
            exit('No find gti2CreateSocket ' + title_name)

    # find DWCi_GT2Startup patch
    ADDRESS_GT2_PORT_PATCH = 0
    GT2_PORT_PATCH_REG = 0
    match = re.search(b'\x38\xE7..(\x80\xBF\x00\x14\x80\xDF\x00\x18|\x80\xBE\x00\x14\x80\xDE\x00\x18)[\x48\x4B]', dol, flags=re.DOTALL)
    if match == None:
        exit('No find DWCi_GT2Startup patch ' + title_name)
    else:
        if match.group(1) == b'\x80\xBF\x00\x14\x80\xDF\x00\x18':
            GT2_PORT_PATCH_REG = 31
            
        elif match.group(1) == b'\x80\xBE\x00\x14\x80\xDE\x00\x18':
            GT2_PORT_PATCH_REG = 30

        assert(dol[match.start(0)+0x18:match.start(0)+0x20] == b'\x2C\x03\x00\x00\x41\x82\x00\x0C')
        ADDRESS_GT2_PORT_PATCH = dol_to_real(hdr, match.start(0)+4)

    ADDRESS_gt2AddressToString = decode_bl(dol, match.start(0)-0x10)
    ADDRESS_gt2CreateSocket = decode_bl(dol, match.start(0)+0xC)

    # get SCGetProductSN
    d = real_to_dol(hdr, ADDRESS_DWC_AUTH_ADD_CSNUM)
    ADDRESS_SCGetProductSN = decode_bl(dol, d+0x14)

    ADDRESS_ESP_FD = 0
    if dol.find(b'/dev/es') != -1:
        # find ES fd with ESP_LaunchTitle
        # 90 0C 00 04 38 E1 00 F0 39 41 00 20 81 2D
        index = dol.find(b'\x90\x0C\x00\x04\x38\xE1\x00\xF0\x39\x41\x00\x20\x81\x2D')
        if index == -1:
            # 90 0C 00 04 38 E1 00 F0 39 21 00 20 80 CD
            index = dol.find(b'\x90\x0C\x00\x04\x38\xE1\x00\xF0\x39\x21\x00\x20\x80\xCD')

        if index != -1:
            offset = struct.unpack('>H', dol[index+0xE:index+0x10])[0]
            if offset & 0x8000:
                offset -= 0x10000
            ADDRESS_ESP_FD = ADDRESS_R13_BASE + offset


    def fmthex(v):
        return "0x{:08X}".format(v)

    game = {
        "Title": title_name,
        "ADDRESS_MD5Digest":                 fmthex(ADDRESS_MD5Digest),
        "ADDRESS_strcmp":                    fmthex(ADDRESS_strcmp),
        "ADDRESS_OSGetTime":                 fmthex(ADDRESS_OSGetTime),
        "ADDRESS_OSDisableInterrupts":       fmthex(ADDRESS_OSDisableInterrupts),
        "ADDRESS_OSRestoreInterrupts":       fmthex(ADDRESS_OSRestoreInterrupts),
        "ADDRESS_DCFlushRange":              fmthex(ADDRESS_DCFlushRange),
        "ADDRESS_ICInvalidateRange":         fmthex(ADDRESS_ICInvalidateRange),
        "ADDRESS_IOS_Open":                  fmthex(ADDRESS_IOS_Open),
        "ADDRESS_IOS_Close":                 fmthex(ADDRESS_IOS_Close),
        "ADDRESS_IOS_Read":                  fmthex(ADDRESS_IOS_Read),
        "ADDRESS_IOS_Ioctlv":                fmthex(ADDRESS_IOS_Ioctlv),
        "ADDRESS_NASWII_AC_URL":             fmthex(ADDRESS_NASWII_AC_URL),
        "ADDRESS_NASWII_AC_URL_POINTER":     fmthex(ADDRESS_NASWII_AC_URL_POINTER),
        "ADDRESS_NASWII_PR_URL":             fmthex(ADDRESS_NASWII_PR_URL),
        "ADDRESS_NASWII_PR_URL_POINTER":     fmthex(ADDRESS_NASWII_PR_URL_POINTER),
        "ADDRESS_AVAILABLE_URL":             fmthex(ADDRESS_AVAILABLE_URL),
        "ADDRESS_DWCi_Auth_SendRequest":     fmthex(ADDRESS_DWCi_Auth_SendRequest),
        "ADDRESS_DWCi_Auth_HandleResponse":  fmthex(ADDRESS_DWCi_Auth_HandleResponse),
        "ADDRESS_DWC_AUTH_ADD_MACADDR":      fmthex(ADDRESS_DWC_AUTH_ADD_MACADDR),
        "ADDRESS_DWC_AUTH_ADD_CSNUM":        fmthex(ADDRESS_DWC_AUTH_ADD_CSNUM),
        "ADDRESS_AUTH_HANDLERESP_HOOK":      fmthex(ADDRESS_AUTH_HANDLERESP_HOOK),
        "ADDRESS_AUTH_HANDLERESP_CONTINUE":  fmthex(ADDRESS_AUTH_HANDLERESP_CONTINUE),
        "ADDRESS_AUTH_HANDLERESP_ERROR":     fmthex(ADDRESS_AUTH_HANDLERESP_ERROR),
        "ADDRESS_AUTH_HANDLERESP_OUT":       fmthex(ADDRESS_AUTH_HANDLERESP_OUT),
        "AUTH_HANDLERESP_UNPATCH":           fmthex(AUTH_HANDLERESP_UNPATCH),
        "LOAD_AUTH_REQ_INST_01":             LOAD_AUTH_REQ_INST_01,
        "LOAD_AUTH_REQ_INST_02":             LOAD_AUTH_REQ_INST_02,
        "LOAD_AUTH_REQ_INST_01_P":           LOAD_AUTH_REQ_INST_01_P,
        "ADDRESS_NHTTPCreateRequest":        fmthex(ADDRESS_NHTTPCreateRequest),
        "ADDRESS_NHTTPSendRequestAsync":     fmthex(ADDRESS_NHTTPSendRequestAsync),
        "ADDRESS_NHTTPDestroyResponse":      fmthex(ADDRESS_NHTTPDestroyResponse),
        "ADDRESS_NHTTPStartup":              fmthex(ADDRESS_NHTTPStartup),
        "ADDRESS_DWC_ERROR":                 fmthex(ADDRESS_DWC_ERROR),
        "ADDRESS_DWCi_SetError":             fmthex(ADDRESS_DWCi_SetError),
        "ADDRESS_DWCi_HandleGPError":        fmthex(ADDRESS_DWCi_HandleGPError),
        "ADDRESS_HBM_ALLOCATOR":             fmthex(ADDRESS_HBM_ALLOCATOR),
        "ADDRESS_BMST_RSO_LOCATOR":          fmthex(ADDRESS_BMST_RSO_LOCATOR),
        "ADDRESS_GH_ALLOC_FUNCTION":         fmthex(ADDRESS_GH_ALLOC_FUNCTION),
        "ADDRESS_PES_ALLOC_FUNCTION":        fmthex(ADDRESS_PES_ALLOC_FUNCTION),
        "ADDRESS_SSBB_GET_HEAP_FUNCTION":    fmthex(ADDRESS_SSBB_GET_HEAP_FUNCTION),
        "ADDRESS_SKIP_DNS_CACHE":            fmthex(ADDRESS_SKIP_DNS_CACHE),
        "ADDRESS_SKIP_DNS_CACHE_CONTINUE":   fmthex(ADDRESS_SKIP_DNS_CACHE_CONTINUE),
        "ADDRESS_gethostbyname":             fmthex(ADDRESS_gethostbyname),
        "ADDRESS_SOiAlloc":                  fmthex(ADDRESS_SOiAlloc),
        "ADDRESS_SOiFree":                   fmthex(ADDRESS_SOiFree),
        "ADDRESS_SOSocket":                  fmthex(ADDRESS_SOSocket),
        "ADDRESS_SOClose":                   fmthex(ADDRESS_SOClose),
        "ADDRESS_SOBind":                    fmthex(ADDRESS_SOBind),
        "ADDRESS_SOConnect":                 fmthex(ADDRESS_SOConnect),
        "ADDRESS_SOGetSockName":             fmthex(ADDRESS_SOGetSockName),
        "ADDRESS_SORecvFrom":                fmthex(ADDRESS_SORecvFrom),
        "ADDRESS_SORecv":                    fmthex(ADDRESS_SORecv),
        "ADDRESS_SOSendTo":                  fmthex(ADDRESS_SOSendTo),
        "ADDRESS_SOSend":                    fmthex(ADDRESS_SOSend),
        "ADDRESS_SOFcntl":                   fmthex(ADDRESS_SOFcntl),
        "ADDRESS_SOShutdown":                fmthex(ADDRESS_SOShutdown),
        "ADDRESS_SOPoll":                    fmthex(ADDRESS_SOPoll),
        "ADDRESS_SOInetAtoN":                fmthex(ADDRESS_SOInetAtoN),
        "ADDRESS_SOGetAddrInfo":             fmthex(ADDRESS_SOGetAddrInfo),
        "ADDRESS_SOFreeAddrInfo":            fmthex(ADDRESS_SOFreeAddrInfo),
        "ADDRESS_SOSetSockOpt":              fmthex(ADDRESS_SOSetSockOpt),
        "ADDRESS_SOGetInterfaceOpt":         fmthex(ADDRESS_SOGetInterfaceOpt),
        "ADDRESS_DWC_Printf":                fmthex(ADDRESS_DWC_Printf),
        "ADDRESS_OSReport":                  fmthex(ADDRESS_OSReport),
        "ADDRESS_GHIPARSEURL_HTTPS_PATCH":   fmthex(ADDRESS_GHIPARSEURL_HTTPS_PATCH),
        "ADDRESS_NHTTP_HTTPS_PORT_PATCH":    fmthex(ADDRESS_NHTTP_HTTPS_PORT_PATCH),
        "ADDRESS_NHTTPi_SocSSLConnect":      fmthex(ADDRESS_NHTTPi_SocSSLConnect),
        "ADDRESS_PATCH_SECURITY_GT2MATCHCOMMAND": fmthex(ADDRESS_PATCH_SECURITY_GT2MATCHCOMMAND),
        "ADDRESS_PATCH_SECURITY_QR2MATCHCOMMAND": fmthex(ADDRESS_PATCH_SECURITY_QR2MATCHCOMMAND),
        "ADDRESS_gpiAppendStringToBuffer":   fmthex(ADDRESS_gpiAppendStringToBuffer),
        "ADDRESS_gpiAppendIntToBuffer":      fmthex(ADDRESS_gpiAppendIntToBuffer),
        "ADDRESS_PATCH_GPISENDLOGIN":        fmthex(ADDRESS_PATCH_GPISENDLOGIN),
        "ADDRESS_PATCH_GPIADDLOCALINFO":     fmthex(ADDRESS_PATCH_GPIADDLOCALINFO),
        "ADDRESS_DWC_Base64Encode":          fmthex(ADDRESS_DWC_Base64Encode),
        "ADDRESS_DWCi_GetUserData":          fmthex(ADDRESS_DWCi_GetUserData),
        "ADDRESS_DWCi_GetGPBuddyAdditionalMsg": fmthex(ADDRESS_DWCi_GetGPBuddyAdditionalMsg),
        "ADDRESS_RealMode":                  fmthex(ADDRESS_RealMode),
        "ADDRESS_OSInitSystemCall_Tail":     fmthex(ADDRESS_OSInitSystemCall_Tail),
        "ADDRESS_GTI2_BUFFER":               fmthex(ADDRESS_GTI2_BUFFER),
        "ADDRESS_SBCM_RETURN":               fmthex(ADDRESS_SBCM_RETURN),
        "ADDRESS_OSYieldThread":             fmthex(ADDRESS_OSYieldThread),
        "ADDRESS_sendto":                    fmthex(ADDRESS_sendto),
        "ADDRESS_stpLoginCnt":               fmthex(ADDRESS_stpLoginCnt),
        "ADDRESS_stpFriendCnt":              fmthex(ADDRESS_stpFriendCnt),
        "ADDRESS_AUTH_RESULT":               fmthex(ADDRESS_AUTH_RESULT),
        "ADDRESS_NATNEG_SET_COMPLETED_DELAY": fmthex(ADDRESS_NATNEG_SET_COMPLETED_DELAY),
        # "ADDRESS_gti2CreateSocket":          fmthex(ADDRESS_gti2CreateSocket),
        "ADDRESS_GT2_PORT_PATCH":            fmthex(ADDRESS_GT2_PORT_PATCH),
        "GT2_PORT_PATCH_REG":                GT2_PORT_PATCH_REG,
        "ADDRESS_gt2AddressToString":        fmthex(ADDRESS_gt2AddressToString),
        "ADDRESS_gt2CreateSocket":           fmthex(ADDRESS_gt2CreateSocket),
        "ADDRESS_SCGetProductSN":            fmthex(ADDRESS_SCGetProductSN),
        "ADDRESS_ESP_FD":                    fmthex(ADDRESS_ESP_FD),
    }

    games.append(game)

if __name__ == '__main__':
    for filename in os.listdir('./dol2'):
        parse_file('./dol2/' + filename, filename[:-4])

    for filename in os.listdir('./dol'):
        parse_file('./dol/' + filename, filename[:-4])

    field_names = [
        "Title",
        "ADDRESS_MD5Digest",
        "ADDRESS_strcmp",
        "ADDRESS_OSGetTime",
        "ADDRESS_OSDisableInterrupts",
        "ADDRESS_OSRestoreInterrupts",
        "ADDRESS_DCFlushRange",
        "ADDRESS_ICInvalidateRange",
        "ADDRESS_IOS_Open",
        "ADDRESS_IOS_Close",
        "ADDRESS_IOS_Read",
        "ADDRESS_IOS_Ioctlv",
        "ADDRESS_NASWII_AC_URL",
        "ADDRESS_NASWII_AC_URL_POINTER",
        "ADDRESS_NASWII_PR_URL",
        "ADDRESS_NASWII_PR_URL_POINTER",
        "ADDRESS_AVAILABLE_URL",
        "ADDRESS_DWCi_Auth_SendRequest",
        "ADDRESS_DWCi_Auth_HandleResponse",
        "ADDRESS_DWC_AUTH_ADD_MACADDR",
        "ADDRESS_DWC_AUTH_ADD_CSNUM",
        "ADDRESS_AUTH_HANDLERESP_HOOK",
        "ADDRESS_AUTH_HANDLERESP_CONTINUE",
        "ADDRESS_AUTH_HANDLERESP_ERROR",
        "ADDRESS_AUTH_HANDLERESP_OUT",
        "AUTH_HANDLERESP_UNPATCH",
        "LOAD_AUTH_REQ_INST_01",
        "LOAD_AUTH_REQ_INST_02",
        "LOAD_AUTH_REQ_INST_01_P",
        "ADDRESS_NHTTPCreateRequest",
        "ADDRESS_NHTTPSendRequestAsync",
        "ADDRESS_NHTTPDestroyResponse",
        "ADDRESS_NHTTPStartup",
        "ADDRESS_DWC_ERROR",
        "ADDRESS_DWCi_SetError",
        "ADDRESS_DWCi_HandleGPError",
        "ADDRESS_HBM_ALLOCATOR",
        "ADDRESS_BMST_RSO_LOCATOR",
        "ADDRESS_GH_ALLOC_FUNCTION",
        "ADDRESS_PES_ALLOC_FUNCTION",
        "ADDRESS_SSBB_GET_HEAP_FUNCTION",
        "ADDRESS_SKIP_DNS_CACHE",
        "ADDRESS_SKIP_DNS_CACHE_CONTINUE",
        "ADDRESS_gethostbyname",
        "ADDRESS_SOiAlloc",
        "ADDRESS_SOiFree",
        "ADDRESS_SOSocket",
        "ADDRESS_SOClose",
        "ADDRESS_SOBind",
        "ADDRESS_SOConnect",
        "ADDRESS_SOGetSockName",
        "ADDRESS_SORecvFrom",
        "ADDRESS_SORecv",
        "ADDRESS_SOSendTo",
        "ADDRESS_SOSend",
        "ADDRESS_SOFcntl",
        "ADDRESS_SOShutdown",
        "ADDRESS_SOPoll",
        "ADDRESS_SOInetAtoN",
        "ADDRESS_SOGetAddrInfo",
        "ADDRESS_SOFreeAddrInfo",
        "ADDRESS_SOSetSockOpt",
        "ADDRESS_SOGetInterfaceOpt",
        "ADDRESS_DWC_Printf",
        "ADDRESS_OSReport",
        "ADDRESS_GHIPARSEURL_HTTPS_PATCH",
        "ADDRESS_NHTTP_HTTPS_PORT_PATCH",
        "ADDRESS_NHTTPi_SocSSLConnect",
        "ADDRESS_PATCH_SECURITY_GT2MATCHCOMMAND",
        "ADDRESS_PATCH_SECURITY_QR2MATCHCOMMAND",
        "ADDRESS_gpiAppendStringToBuffer",
        "ADDRESS_gpiAppendIntToBuffer",
        "ADDRESS_PATCH_GPISENDLOGIN",
        "ADDRESS_PATCH_GPIADDLOCALINFO",
        "ADDRESS_DWC_Base64Encode",
        "ADDRESS_DWCi_GetUserData",
        "ADDRESS_DWCi_GetGPBuddyAdditionalMsg",
        "ADDRESS_RealMode",
        "ADDRESS_OSInitSystemCall_Tail",
        "ADDRESS_GTI2_BUFFER",
        "ADDRESS_SBCM_RETURN",
        "ADDRESS_OSYieldThread",
        "ADDRESS_sendto",
        "ADDRESS_stpLoginCnt",
        "ADDRESS_stpFriendCnt",
        "ADDRESS_AUTH_RESULT",
        "ADDRESS_NATNEG_SET_COMPLETED_DELAY",
        # "ADDRESS_gti2CreateSocket",
        "ADDRESS_GT2_PORT_PATCH",
        "GT2_PORT_PATCH_REG",
        "ADDRESS_gt2AddressToString",
        "ADDRESS_gt2CreateSocket",
        "ADDRESS_SCGetProductSN",
        "ADDRESS_ESP_FD",
    ]

    with open('gamedefs.csv', 'w', newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=field_names, delimiter=',', dialect='excel')
        writer.writeheader()
        writer.writerows(games)
