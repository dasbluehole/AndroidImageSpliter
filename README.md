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
	split_android imagefile

Enjoy.
DasBluehole.
