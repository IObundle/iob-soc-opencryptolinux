#!/bin/sh 
set -x

insmod phys.ko
echo -n "Test string, contains multiple characters in order to test the block stream properly" > in.txt

echo -n "" > in2.txt
echo -n "ola" > in3.txt

./crypto -i in.txt SHA256
./crypto -i in2.txt SHA256
./crypto -i in3.txt SHA256

#./crypto -P priv.txt -p pub.txt -s 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345 McEliece
#./crypto -p pub.txt  -o chiperText.txt -s 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345 McEliece
#./crypto -P priv.txt -i chiperText.txt -s 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345 McEliece

#./crypto -i in.txt SHA256

#./crypto -i in.txt -o out_128_ecb.txt -k 01234567890123456789012345678901 -e AES-128-ECB
#hexdump out_128_ecb.txt
#./crypto -i out_128_ecb.txt -o out_128_ecb_decrypt.txt -k 01234567890123456789012345678901 -d AES-128-ECB
#hexdump out_128_ecb_decrypt.txt

#./crypto -i in.txt -o out_256_ecb.txt -k 0123456789012345678901234567890123456789012345678901234567890123 -e AES-256-ECB
#hexdump out_256_ecb.txt
#./crypto -i out_256_ecb.txt -o out_256_ecb_decrypt.txt -k 0123456789012345678901234567890123456789012345678901234567890123 -d AES-256-ECB
#hexdump out_256_ecb_decrypt.txt

#./crypto -i in.txt -o out_128_cbc.txt -k 01234567890123456789012345678901 -v 10987654321098765432109876543210 -e AES-128-CBC
#hexdump out_128_cbc.txt
#./crypto -i out_128_cbc.txt -o out_128_cbc_decrypt.txt -k 01234567890123456789012345678901 -v 10987654321098765432109876543210 -d AES-128-CBC
#hexdump out_128_cbc_decrypt.txt

#./crypto -i in.txt -o out_256_cbc.txt -k 0123456789012345678901234567890123456789012345678901234567890123 -v 10987654321098765432109876543210 -e AES-256-CBC
#hexdump out_256_cbc.txt
#./crypto -i out_256_cbc.txt -o out_256_cbc_decrypt.txt -k 0123456789012345678901234567890123456789012345678901234567890123 -v 10987654321098765432109876543210 -d AES-256-CBC
#hexdump out_256_cbc_decrypt.txt
