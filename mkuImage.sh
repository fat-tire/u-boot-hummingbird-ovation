mkimage -A arm -O linux -T kernel -C none -a 0x83000000 -e 0x83000000 -n "2nd Bootloader" -d u-boot.bin uImage
