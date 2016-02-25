HxC Floppy Emulator SD copy utility
===================================

Allows files on the HxC SD card (FAT32) to be copied to the Amiga file system, without needing to use HFE files.

This is especially useful for copying files larger than 880kB (the Amiga floppy disk size).

Limitations:
* Requires KS/WB 2.x or later. This is a limitation of the build environment used.
* Slow! Takes about 5 mins to copy 1MB.
* Shorter filenames will be converted to lowercase.
* Only copies from HxC, i.e. doesn't copy to HxC.
* Doesn't do recursive (deep) copies.

Known bugs:
* Chip memory leak if utility fails due to sector read/write errors. I recommend you reboot if you encounter such errors to reclaim the memory.
* Amiga may crash if an SD card isn't inserted in the HxC.

Download and Installation
-------------------------

The "hxcfe" program is provided on a HFE image: https://github.com/gonk23/HXCFE_Amiga_copy_utility/raw/master/hxcfe.hfe.zip

Copy the "hxcfe" program in the provided HFE image to C: on the Amiga, i.e. assuming the HxC SD is connected as df1:

```
copy df1:hxcfe c:
```

Usage
-----

You can run "hxcfe" without any parameters to get usage instructions.

The idea is that you change to the destination folder on the Amiga, and you then use "hxcfe" to list directories and copy a file/folder from the HxC. E.g.:

```
cd work:
hxcfe dir
hxcfe copy myfile
hxcfe dir myfolder
hxcfe copy myfolder
```

I seriously don't recommend using this utility to try to copy to a floppy image mounted on the HxC!

Development
-----------

This utility heavily uses code from: https://github.com/jfdelnero/HXCFE_Amiga_file_selector by Jean-Francois DEL NERO.

The build environment used was: https://github.com/cahirwpz/amigaos-cross-toolchain .
