partitionfs
===========

partitionfs is a tool for exposing the partitions set aside in a disk image file.  It's very useful for can-openering a virtual disk without the need for guestfish/qemu/etc. which can be slow on systems.  Derived from a public domain contribution to the FUSE mailing list in around 2008, this currently supports MSDOS partition tables in disk images.  Plans are for supporting whatever libparted enables us to support including GPT partition tables.
