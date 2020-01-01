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

It's been verified on Linux modern (as of 01/01/2020...) distributions and should just simply build.

In the current version, there's no help- to use, just simply run partitionfs <image file> <mountpoint>
where <image file> is the disk image file, and <mountpoint> is the directory to mount to.  In the 
mountpoint, there will be a numbered file that comprises the partition in the image file.  You can use
the large gamut of tools that understand working with a filesystem image file to format, mount, etc.
the filesystems in those virtual image files.  Care should be exercised in use- _the filesystem's got very 
limited protections against multiple workers at the same time going and clobbering the whole disk
image or similar nasty things_.  You should do _one_ set of operations at a time for now _(Meaning you 
should only be doing things against a single thread of execution in a single script file or manually 
in a single window)_.  Biggest problem is libparted doesn't even contemplate this use and is not 
expecting to be leveraged in this manner- and it's not even close to threadsafe.  A re-work to handle 
a workqueue of sorts will mostly fix the problem and it's on the plans, but...

(Update 01/01/2020) This has been validated against at least Ubuntu/Debian/Devuan as of this date.  It has also been validated against cross-compile/native/nativesdk for Yocto Warrior as of this date.  Packaging support has been removed.  To the best of my knowledge nobody has adopted this yet (yet...) for major distributions.  That's not a reflection of the tool, mind, just that it's not been noticed or the value thereof has been realized.  As such, it's going to be easier if we just simply got out of the way and let the distributions themselves do the proper packaging for their distro themselves.  There is Yocto recipes for this in meta-pha's layer as an example thereof. 
