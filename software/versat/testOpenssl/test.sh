#!/bin/sh 
set -x

echo -n "Test string, contains multiple characters in order to test the block stream properly" > in.txt

openssl aes-128-ecb -in in.txt -K 01234567890123456789012345678901 -e > out_128_ecb.txt
hexdump out_128_ecb.txt
openssl aes-128-ecb -in out_128_ecb.txt -K 01234567890123456789012345678901 -d -nopad > out_128_ecb_decrypt.txt
hexdump out_128_ecb_decrypt.txt

openssl aes-256-ecb -in in.txt -K 0123456789012345678901234567890123456789012345678901234567890123 -e > out_256_ecb.txt
hexdump out_256_ecb.txt
openssl aes-256-ecb -in out_256_ecb.txt -K 0123456789012345678901234567890123456789012345678901234567890123 -d -nopad > out_256_ecb_decrypt.txt
hexdump out_256_ecb_decrypt.txt

openssl aes-128-cbc -in in.txt -K 01234567890123456789012345678901 -iv 10987654321098765432109876543210 -e > out_128_cbc.txt
hexdump out_128_cbc.txt
openssl aes-128-cbc -in out_128_cbc.txt -K 01234567890123456789012345678901 -iv 10987654321098765432109876543210 -d -nopad > out_128_cbc_decrypt.txt
hexdump out_128_cbc_decrypt.txt

openssl aes-256-cbc -in in.txt -K 0123456789012345678901234567890123456789012345678901234567890123 -iv 10987654321098765432109876543210 -e > out_256_cbc.txt
hexdump out_256_cbc.txt
openssl aes-256-cbc -in out_256_cbc.txt -K 0123456789012345678901234567890123456789012345678901234567890123 -iv 10987654321098765432109876543210 -d -nopad > out_256_cbc_decrypt.txt
hexdump out_256_cbc_decrypt.txt
