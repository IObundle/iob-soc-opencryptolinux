#!/bin/sh 
set -x

# Needed to use Versat.
insmod phys.ko

IV="10987654321098765432109876543210"
EXPECTED_SHA="574874084239233FD9FF32BC890242F07E83357BE1A1237CE1C6BAC8AEE1B54F"
SEED="012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345"

# Alice generates a pair of private and public keys using McEliece. She storages the private key and publishes the public key
./crypto -P priv.txt -p pub.txt -s $SEED McEliece

# Bob wants to send this message to Alice encrypted
echo -n "Test string, contains multiple characters in order to test the block stream properly" > in.txt

# He calculates an hash of the content of the message
./crypto -i in.txt SHA256
echo ""

# Bob uses the public key to generate a session key as well as the chipertext that encapsulates that session key
BobSessionKey=$(./crypto -p pub.txt -o chiperText.txt -s $SEED McEliece)

# Alice receives the chipertext and decapsulates it to generate the same exact session key that Bob has
AliceSessionKey=$(./crypto -P priv.txt -i chiperText.txt -s $SEED McEliece)

# Bob uses the session key to encrypt the data using AES-256. He uses CBC mode since it's more secure.
./crypto -i in.txt -o out_256_cbc.txt -k $BobSessionKey -v $IV -e AES-256-CBC

# Alice receives the encrypted message and uses her own key to decrypt it
./crypto -i out_256_cbc.txt -o decryptedMessage.txt -k $AliceSessionKey -v $IV -d AES-256-CBC

# Alice calculates an hash of the content of the message. Since the hash matches, she knows that the content was not altered.
./crypto -i decryptedMessage.txt SHA256
echo ""
