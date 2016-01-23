make distclean
make clean
make dragonboard_config
make -j16 ARCH=arm CROSS_COMPILE=aarch64-linux-gnu-
sh img.sh
