PROJECT(partitionfs)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Required software packages for this FUSE filesystem
FIND_PACKAGE(PkgConfig REQUIRED)
pkg_check_modules(FUSE REQUIRED fuse)
pkg_check_modules(LIBPARTED REQUIRED libparted)
# This is a bit of a kludge.  We rely on libparted MANDATING these...
FIND_LIBRARY(LIBBLKID blkid)
FIND_LIBRARY(LIBUUID uuid)

# We're going to include the ability to make packaging for this to
# make distribution teams happier with us...maybe even bundle us
# into things...  We'll still need something hook-ish to build
# DEBs and RPMs as part of their processes, but one step at a time.
INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "partitionfs - FUSE filesystem to expose the partition map of a disk image file as loopback files")
SET(CPACK_PACKAGE_VENDOR "Frank C. Earl <frank@earlconsult.com>")
SET(CPACK_PACKAGE_CONTACT "frank@earlconsult.com")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_STRIP_FILES "bin/partitionfs")
SET(CPACK_GENERATOR "DEB;RPM")

# Handle all the DEB specific declares...
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "fuse libparted blkid uuid")
SET(CPACK_DEBIAN_PACKAGE_SECTION "filesystems")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
# Ultimately, we'd like to package 32 AND 64-bit bundles.  For NOW, we're 
# bundling only what I'm building- which is 64-bits.  If you need
# to build/declare 32-bits, change this from amd64 to i386...
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE, "amd64")

# Handle all the RPM specific declares...
SET(CPACK_RPM_PACKAGE_REQUIRES "fuse libparted blkid uuid")
SET(CPACK_RPM_PACKAGE_PROVIDES "partitionfs")
SET(CPACK_RPM_PACKAGE_RELEASE, "1")
# Ultimately, we'd like to package 32 AND 64-bit bundles.  For NOW, we're 
# bundling only what I'm building- which is 64-bits.  If you need
# to build/declare 32-bits, change this from x86_64 to i386...
SET(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")

# Finally, bring in CPack proper which will do the rest appropriately...
INCLUDE(CPack)


# Seems CMake can't grok the notion of squashing a list
# into a string without a bit of help (Sigh...).
function(JOIN VALUES GLUE OUTPUT)
  string (REPLACE ";" "${GLUE}" _TMP_STR "${VALUES}")
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()
JOIN("${FUSE_CFLAGS}" " " PARTITIONFS_CFLAGS)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUSE_USE_VERSION=25 ${PARTITIONFS_CFLAGS}")

# Specify our application
ADD_EXECUTABLE(partitionfs partitionfs.c)
TARGET_LINK_LIBRARIES(partitionfs ${FUSE_LIBRARIES} ${LIBPARTED_LIBRARIES} ${LIBBLKID} ${LIBUUID})
INSTALL(PROGRAMS partitionfs DESTINATION bin)


