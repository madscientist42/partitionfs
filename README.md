partitionfs
===========

partitionfs is a FUSE filesystem based tool for exposing the partitions set aside in a disk image file.

Derived from a public domain contribution to the FUSE developer discussion list, the code provides a
fairly useful method to provide access to a disk image file's partition scheme so that someone can then
do FUSE mounts of the filesystems in the partitons.  This allows things like no-root/sudo access 
needed builds of disk images from start to finish when combined with other FUSE filesystems.  

Currently, the code is set up to work solely with a MSDOS partition table.  Future releases are planned
to support a larger range of partition types such as GPT- whatever libparted helps enable this filesystem
to expose, it will eventually support.

Building is simple.  You need FUSE, libparted, libblkid, and libuuid installed as a developer (i.e. 
headers and libraries) configuration as well as CMake version 2.8 or later as prerequisites- and just 
simply run "cmake ." on the git checkout directory followed by "make".

To install, it's just "sudo make install" or if you want to use the CPack specification to make a
.deb, it's just "make package" and then install the .deb accordingly.  The .deb will be for the 
active architechture of the build host or cross-compile sandbox.

It's been verified on Linux modern (as of 11/26/2013...) distributions and should just simply build.

In the current version, there's no help- to use, just simply run partitionfs <image file> <mountpoint>
where <image file> is the disk image file, and <mountpoint> is the directory to mount to.  In the 
mountpoint, there will be a numbered file that comprises the partition in the image file.  You can use
the large gamut of tools that understand working with a filesystem image file to format, mount, etc.
the filesystems in those virtual image files.  Care should be exercised- the filesystem's got very 
limited protections against multiple workers at the same time going and clobbering the whole disk
image.  You should do one set of operations at a time for now.

(Update 06-17-2016) There are now .deb and .rpm packagings for this project created from the 1.1.0
version tag.  They _*should*_ install and run properly on anything from the Ubuntu 15.10 timeframe
space and later- they may install and run on earlier, but that isn't as assured.  This is a pre-release
in the sense that it's ready for "normal" use if you build it and here's some pre-builts for what's
likely to be out there.
