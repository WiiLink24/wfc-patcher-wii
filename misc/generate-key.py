from sys import argv
import struct
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import rsa

def exit_usage():
    exit(
        "usage:\n" \
        "generate-key.py new    private-key-OUT.pem wwfcPayloadPublicKey-OUT.hpp\n" \
        "generate-key.py privin private-key-IN.pem  wwfcPayloadPublicKey-OUT.hpp\n" \
        "generate-key.py pubin  public-key-IN.pem   wwfcPayloadPublicKey-OUT.hpp" \
    )

if len(argv) < 4:
    exit_usage()

hpp_out = open(argv[3], "w")

if argv[1] == "new":
    private_key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
    private_bytes = private_key.private_bytes(serialization.Encoding.PEM, serialization.PrivateFormat.PKCS8, serialization.NoEncryption())
    open(argv[2], "wb").write(private_bytes)
    public_numbers = private_key.public_key().public_numbers()
elif argv[1] == "privin":
    private_key = serialization.load_pem_private_key(open(argv[2], "rb").read(), password=None)
    public_numbers = private_key.public_key().public_numbers()
elif argv[1] == "pubin":
    public_key = serialization.load_pem_public_key(open(argv[2], "rb").read())
    public_numbers = public_key.public_numbers()
else:
    exit_usage()

if public_numbers.e != 65537:
    exit("error: RSA key exponent must be 65537")

n = public_numbers.n
n_bytes = int.to_bytes(n, length=0x100)

b = pow(2, 32)
n0inv = -(pow(n, -1, b)) & 0xffffffff

data = struct.pack(">I", n0inv)

for i in range(len(n_bytes) >> 2):
    offset = len(n_bytes) - (i * 4) - 4
    data += n_bytes[offset:offset + 4]

r = pow(2, 2048)
r = pow(r, 2, n)
r_bytes = int.to_bytes(r, length=0x100)

for i in range(len(r_bytes) >> 2):
    offset = len(r_bytes) - (i * 4) - 4
    data += r_bytes[offset:offset + 4]

hpp_data = \
    "#pragma once\n" \
    "\n" \
    "namespace wwfc\n" \
    "{\n" \
    "\n" \
    "constexpr unsigned char PayloadPublicKey[] = {"

for i in range(len(data)):
    if (i % 12) == 0:
        hpp_data += "\n   "
    hpp_data += " 0x{:02x},".format(data[i])

hpp_data += \
    "\n" \
    "};\n" \
    "\n" \
    "} // namespace wwfc\n"

hpp_out.write(hpp_data)
hpp_out.close()
