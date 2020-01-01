PROJECT(partitionfs)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Required software packages for this FUSE filesystem
FIND_PACKAGE(PkgConfig REQUIRED)
pkg_check_modules(FUSE REQUIRED fuse)
pkg_check_modules(LIBPARTED REQUIRED libparted)
# This is a bit of a kludge.  We rely on libparted MANDATING these...
FIND_LIBRARY(LIBBLKID blkid)
FIND_LIBRARY(LIBUUID uuid)

# Establish current rules of engagement for FUSE...
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUSE_USE_VERSION=25")

# Specify our application
ADD_EXECUTABLE(partitionfs partitionfs.c)
TARGET_LINK_LIBRARIES(partitionfs ${FUSE_LIBRARIES} ${LIBPARTED_LIBRARIES} ${LIBBLKID} ${LIBUUID})
INSTALL(TARGETS partitionfs RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


