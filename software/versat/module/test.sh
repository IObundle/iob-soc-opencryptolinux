insmod phys.ko
echo "ola" >> in.txt
./crypto -i in.txt -o out.txt -k 01234567890123456789012345678901 -e AES-128-ECB
hexdump out.txt
./crypto -i out.txt -o out2.txt -k 01234567890123456789012345678901 -d AES-128-ECB
hexdump out2.txt
