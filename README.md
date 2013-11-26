partitionfs
===========

partitionfs is a FUSE filesystem based tool for exposing the partitions set aside in a disk image file.

Derived from a public domain contribution to the FUSE developer discussion list, the code provides a
fairly useful method to provide access to a disk image file's partition scheme so that someone can then
do FUSE mounts of the filesystems in the partitons.

Currently, the code is set up to work solely with a MSDOS partition table.  Future releases are planned
to support a larger range of partition types such as GPT- whatever libparted helps enable this filesystem
to expose, it will eventually support.

Building is simple.  You need FUSE, libparted, libblkid, and libuuid installed as a developer (i.e. 
headers and libraries) configuration as prerequisites- and just simply run make.  It's been verified
on Linux modern (as of 11/26/2013...) distributions and should just simply build.

In the current version, there's no help- to use, just simply run partitionfs <image file> <mountpoint>
where <image file> is the disk image file, and <mountpoint> is the directory to mount to.  In the 
mountpoint, there will be a numbered file that comprises the partition in the image file.  You can use
the large gamut of tools that understand working with a filesystem image file to format, mount, etc.
the filesystems in those virtual image files.
