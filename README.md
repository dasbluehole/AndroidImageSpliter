# AndroidImageSpliter
A small code to split android boot image.
There are several code pices available to split the android image to its components.

1) https://gist.github.com/jberkel/1087743
2) https://github.com/dianlujitao/split-appended-dtb
3) https://github.com/PabloCastellano/extract-dtb

There are several others too.
My code is written in the line of reference [1]

compilation is simple: gcc split_android.c -o split_android will do :P.

Usage :
	split_android <imagefile>

Example:

$./split_android ../boot.img
Found Magic!!!
===========INFO===================
Page size =     [    2048]   bytes
Kernel Size =   [ 7098387]   bytes
load address=   [0x01080000]       
RAM disk size = [ 1292577]   bytes
load address =  [0x01000000]      
DTB file size = [   88064]   bytes
load address =  [0x00F00000]      
KTAGs address = [0x00000100]      
(OPT)Product Name =  
(OPT)Comand Line =  
==================================
[kernel_boot.img] Written
[ramdisk_boot.img] Written
[dtb_boot.img] Written

$ 

Enjoy.
DasBluehole.
