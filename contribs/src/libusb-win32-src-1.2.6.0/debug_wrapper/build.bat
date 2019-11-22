
gcc -O1 -std=gnu99 -Wformat -Wall -DVERSION_MAJOR=0 -DVERSION_MINOR=1 -DVERSION_MICRO=12 -DVERSION_NANO=1 -DINF_VERSION='0.1.2.2' -s -mdll -c libusb_dyn.c myprintf.c -I%DEVCI% 

gcc -i -o libusb0.dll libusb_dyn.o myprintf.o ../libusb0.def -s -mdll -Wl,--kill-at -Wl,--out-implib,libusb.a -Wl,--enable-stdcall-fixup -L. -lcfgmgr32 -lsetupapi

