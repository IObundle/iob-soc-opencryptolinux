#!/bin/sh 

KEY="0123456789012345678901234567890123456789012345678901234567890123"
SEED="000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000008" # Guarantees one iteration, needed because software only is slow

# 
# Testing correctness
#

echo "Testing SHA with different buffer sizes"
./crypto -b 101 -i large.txt SHA256
./crypto -b 167 -i large.txt SHA256
./crypto -b 397 -i large.txt SHA256
./crypto -i large.txt SHA256

echo -e "\n"

echo "Testing AES with different buffer sizes"
./crypto -b 101 -i large.txt -o AES_101.txt -k $KEY -e AES-256-ECB
./crypto -b 167 -i large.txt -o AES_167.txt -k $KEY -e AES-256-ECB
./crypto -b 397 -i large.txt -o AES_397.txt -k $KEY -e AES-256-ECB
./crypto -i large.txt -o AES_DEFAULT.txt -k $KEY -e AES-256-ECB 

xxd -p AES_101.txt | head -c 48
echo ""
xxd -p AES_167.txt | head -c 48
echo ""
xxd -p AES_397.txt | head -c 48
echo ""
xxd -p AES_DEFAULT.txt | head -c 48
echo ""

echo -e "\n"

# 
# Testing performance compared to software only
#

# Testing performance
echo "SHA Versat time"
time ./crypto -i large.txt SHA256

echo -e "\n"

echo "SHA Software time"
time ./crypto -t -i large.txt SHA256

echo -e "\n"

echo "AES Versat time and result"
time ./crypto -i large.txt -o AES1.txt -k $KEY -e AES-256-ECB 
echo ""
xxd -p AES1.txt | head -c 48
echo ""

echo -e "\n"

echo "AES software time and result"
time ./crypto -t -i large.txt -o AES2.txt -k $KEY -e AES-256-ECB 
echo ""
xxd -p AES2.txt | head -c 48

echo -e "\n"

echo "McEliece Versat time and result"
time ./crypto -P priv1.txt -p pub1.txt -s $SEED McEliece
echo ""
xxd -p priv1.txt | head -c 48
echo ""
xxd -p priv1.txt | tail -c 48
echo ""
xxd -p pub1.txt  | head -c 48
echo ""

echo -e "\n"

echo -e "McEliece software time and result"
time ./crypto -t -P priv2.txt -p pub2.txt -s $SEED McEliece
echo ""
xxd -p priv2.txt | head -c 48
echo ""
xxd -p priv2.txt | tail -c 48
echo ""
xxd -p pub2.txt  | head -c 48
echo ""

echo -e "\n"
