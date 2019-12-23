#!/usr/bin/env python

from ledgerblue.comm import getDongle
import argparse
from base import parse_bip32_path
import binascii

parser = argparse.ArgumentParser()
parser.add_argument('--path', help="BIP32 path to retrieve. e.g. \"44'/195'/0'/0/0\".")
args = parser.parse_args()

if args.path == None:
	args.path = "44'/195'/0'/0/0"

donglePath = parse_bip32_path(args.path)


# Create APDU message.
# a vote
transactionRaw = '0a029f9122082fc729fb70fb41514098d7d790f32d5a860108041281010a30747970652e676f6f676c65617069732e636f6d2f70726f746f636f6c2e566f74655769746e657373436f6e7472616374124d0a1541340967e825557559dc46bbf0eabe5ccf99fd134e12190a1541f16412b9a17ee9408646e2a21e16478f72ed1e95100312190a1541f1a0466076c57c9f6d07decc86021ddbf8bae0b2100570c392d490f32d'

signatureCheck = "cd01fcd0a4f0bb9a55a43d57ae1c955374f2540ff931307029e4c1fb80a6dc91"   \
                     "3185edd8a4dfda2d03aad569b856cfb3ed9db387b6589451797ff01c9c353583"\
                     "01"

apduMessage = "E0041000" + '{:02x}'.format(int(len(donglePath) / 2) + 1 + int(len(transactionRaw) / 2)) + '{:02x}'.format(int(len(donglePath) / 4 / 2)) + donglePath + transactionRaw
apdu = bytearray.fromhex(apduMessage)

print("-= Tron Ledger =-")
print("Sign Transaction")

dongle = getDongle(True)
print(apduMessage.strip())
result = dongle.exchange(bytearray.fromhex(apduMessage))
print(binascii.hexlify(result[0:65]))
if binascii.hexlify(result[0:65]).decode()==signatureCheck:
	print("Signature Validated!")
else:
	print("Signature Error!")
