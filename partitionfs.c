#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <fuse.h>

#include <parted/parted.h>

typedef struct Partition_s
{
   off_t start;
   off_t size;
   uint8_t boot_indicator;
   uint8_t type;
} Partition;

static time_t timestamp = 0;
static PedDevice *device = NULL;
static PedDisk *disk = NULL;
static uint8_t *sector_buffer = NULL;

static PedPartition *partition_from_path(const char *path)
{
   if (path[0] == '/')
   {
      char *endptr;
      long int n;

      n = strtol(path+1, &endptr, 10);

      if ((endptr != (path+1)) && (*endptr == '\0'))
      {
         PedPartition *p = ped_disk_get_partition(disk, n);

         /* Include primary and logical partitions only */
         return ((p->type & ~PED_PARTITION_LOGICAL) == 0) ? p : NULL;
      }
   }

   // Invalid path
   return NULL;
}

static int getattr_impl(const char *path, struct stat *stbuf)
{
   memset(stbuf, 0, sizeof(struct stat));

   if (strcmp(path, "/") == 0)
   {
      stbuf->st_mode = S_IFDIR | 0755;
      stbuf->st_nlink = 2;
      stbuf->st_uid = getuid();
      stbuf->st_gid = getgid();
      stbuf->st_atime = timestamp;
      stbuf->st_mtime = timestamp;
      stbuf->st_ctime = timestamp;
      return 0;
   }
   else
   {
      PedPartition *p = partition_from_path(path);

      if (p != NULL)
      {
         stbuf->st_mode = S_IFREG | (device->read_only ? 0444 : 0644);
         stbuf->st_nlink = 1;
         stbuf->st_uid = getuid();
         stbuf->st_gid = getgid();
         stbuf->st_size = p->geom.length * device->sector_size;
         stbuf->st_blocks = (stbuf->st_size + 511) / 512;
         stbuf->st_atime = timestamp;
         stbuf->st_mtime = timestamp;
         stbuf->st_ctime = timestamp;
         return 0;
      }
   }

   return -ENOENT;
}

static int readdir_impl(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi)
{
   char szBuffer[10];
   int i;

   if (strcmp(path, "/") != 0)
   {
      return -ENOENT;
   }

   filler(buf, ".", NULL, 0);
   filler(buf, "..", NULL, 0);

   PedPartition *p = NULL;
   while ((p = ped_disk_next_partition(disk, p)) != NULL)
   {
      /* Include only primary and logical partitions in listing. */
      if ((p->type & ~PED_PARTITION_LOGICAL) == 0)
      {
         snprintf(szBuffer, sizeof(szBuffer), "%u", p->num);
         filler(buf, szBuffer, NULL, 0);
      }
   }

   return 0;
}

static int open_impl(const char *path, struct fuse_file_info *fi)
{
   PedPartition *p = partition_from_path(path);
   if (p)
   {
      fi->fh = (uint64_t)((uintptr_t)p);
      if (((fi->flags & O_WRONLY) != 0) || ((fi->flags & O_RDWR) != 0))
      {
         return device->read_only ? -EACCES : 0;
      }
      else
      {
         return 0;
      }
   }
   else
   {
      return -ENOENT;
   }
}

static int read_impl(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi)
{
   const PedPartition *p = (const PedPartition*)((uintptr_t)fi->fh);

   if (p)
   {
      PedSector sector_start  = p->geom.start + (offset / device->sector_size);
      long long sector_offset = offset % device->sector_size;
      size_t size_left = size;

      if (sector_start > p->geom.end)
         return 0;
      
      if (sector_offset != 0)
      {
         if (!ped_device_read(device, (void*)sector_buffer, sector_start, 1))
         {
            return -EIO;
         }

         long long read_amount = device->sector_size - sector_offset;

         if (read_amount > size_left)
            read_amount = size_left;

         memcpy(buf, sector_buffer + sector_offset, read_amount);

         buf += read_amount;
         size_left -= read_amount;
         sector_start++;
      }

      if ((size_left >= device->sector_size) && (sector_start <= p->geom.end))
      {
         PedSector sectors_left = size_left / device->sector_size;

         if ((sector_start + sectors_left) > p->geom.end)
         {
            sectors_left = p->geom.end - sector_start + 1;
         }

         if (!ped_device_read(device, (void*)buf, sector_start, sectors_left))
         {
            return -EIO;
         }

         sector_start += sectors_left;

         long long read_amount = sectors_left * device->sector_size;
         buf += read_amount;
         size_left -= read_amount;
      }

      if ((size_left != 0) && (sector_start <= p->geom.end))
      {
         if (!ped_device_read(device, (void*)sector_buffer, sector_start, 1))
         {
            return -EIO;
         }

         memcpy(buf, sector_buffer, size_left);
         size_left = 0;
      }

      return size - size_left;
   }
   else
   {
      return -ENOENT;
   }
}

static int write_impl(const char *path, const char *buf, size_t size,
                      off_t offset, struct fuse_file_info *fi)
{
   const PedPartition *p = (const PedPartition*)((uintptr_t)fi->fh);

   if (p)
   {
      PedSector sector_start  = p->geom.start + (offset / device->sector_size);
      long long sector_offset = offset % device->sector_size;
      size_t size_left = size;

      if (sector_start > p->geom.end)
         return 0;
      
      if (sector_offset != 0)
      {
         long long write_amount = device->sector_size - sector_offset;

         if (write_amount > size_left)
            write_amount = size_left;

         if (!ped_device_read(device, (void*)sector_buffer, sector_start, 1))
         {
            return -EIO;
         }

         memcpy(sector_buffer + sector_offset, buf, write_amount);

         if (!ped_device_write(device, (const void*)sector_buffer, sector_start, 1))
         {
            return -EIO;
         }

         buf += write_amount;
         size_left -= write_amount;
         sector_start++;
      }

      if ((size_left >= device->sector_size) && (sector_start <= p->geom.end))
      {
         PedSector sectors_left = size_left / device->sector_size;

         if ((sector_start + sectors_left) > p->geom.end)
         {
            sectors_left = p->geom.end - sector_start + 1;
         }

         if (!ped_device_write(device, (const void*)buf, sector_start, sectors_left))
         {
            return -EIO;
         }

         sector_start += sectors_left;

         long long write_amount = sectors_left * device->sector_size;
         buf += write_amount;
         size_left -= write_amount;
      }

      if ((size_left != 0) && (sector_start <= p->geom.end))
      {
         if (!ped_device_read(device, (void*)sector_buffer, sector_start, 1))
         {
            return -EIO;
         }

         memcpy(sector_buffer, buf, size_left);

         if (!ped_device_write(device, (const void*)sector_buffer, sector_start, 1))
         {
            return -EIO;
         }

         size_left = 0;
      }

      return size - size_left;
   }
   else
   {
      return -ENOENT;
   }
}

int truncate_impl(const char *path, off_t new_size)
{
   /* Fixed size; ignore truncate requests */
   return 0;
}

static struct fuse_operations callbacks = {
   .getattr  = getattr_impl,
   .readdir  = readdir_impl,
   .open     = open_impl,
   .read     = read_impl,
   .write    = write_impl,
   .truncate = truncate_impl,
};

static PedExceptionOption ped_handler_impl(PedException *ex)
{
   if (ex->options & PED_EXCEPTION_IGNORE)
   {
      /* Suppress anything that can be ignored. */
      return PED_EXCEPTION_IGNORE;
   }
   else
   {
      return PED_EXCEPTION_UNHANDLED;
   }
}

static void usage()
{
   fprintf(stderr, "usage: partitionfs source mount_point [options]\n\n");
   fprintf(stderr, "general options:\n");
   fprintf(stderr, "    -o opt[,opt...]         mount options\n");
   fprintf(stderr, "    -h     --help           print help\n");
   fprintf(stderr, "    -V     --version        print version\n\n");
   fprintf(stderr, "FUSE options:\n");
   fprintf(stderr, "    -d     -o debug         enable debug output (implies -f)\n");
   fprintf(stderr, "    -f                      foreground operation\n");
   fprintf(stderr, "    -s                      disable multi-threaded operation\n\n");
   fprintf(stderr, "    -o allow_other          allow access to other users\n");
   fprintf(stderr, "    -o allow_root           allow access to root\n");
   fprintf(stderr, "    -o nonempty             allow mounts over non-empty file/dir\n");
   fprintf(stderr, "    -o default_permissions  enable permission check by kernel\n");
   fprintf(stderr, "    -o max_read             set maximum size of read requests\n");
   fprintf(stderr, "    -o max_write            set maximum size of write requests\n");
   fprintf(stderr, "    -o async_read           perform reads asynchronously (default)\n");
   fprintf(stderr, "    -o sync_read            perform reads synchronously\n");
}

static int opt_proc_impl(void *data, const char *arg, int key,
                         struct fuse_args *outargs)
{
   if ((strcmp(arg, "--help") == 0) || (strcmp(arg, "-h") == 0))
   {
      usage();
      exit(0);
   }
   if ((strcmp(arg, "--version") == 0) || (strcmp(arg, "-V") == 0))
   {
      fprintf(stderr, "partitionfs version 0.5\n");
      exit(0);
   }
   if (arg[0] != '-' && (device == NULL))
   {
      device = ped_device_get(arg);
      return (device == NULL) ? -1 : 0;
   }

   return 1;
}

int main(int argc, char *argv[])
{
   struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

   ped_exception_set_handler(ped_handler_impl);
   atexit(ped_device_free_all);

   if (fuse_opt_parse(&args, NULL, NULL, opt_proc_impl) != 0)
   {
      usage();
      return 1;
   }

   if (device == NULL || !ped_device_open(device))
   {
      fprintf(stderr, "Failed to open input file.\n");
      return 2;
   }

   disk = ped_disk_new(device);

   if (disk == NULL)
   {
      fprintf(stderr, "Input file does not contain a partition table.\n");
      return 3;
   }

   sector_buffer = (uint8_t*)malloc(device->sector_size);

   if (sector_buffer == NULL)
   {
      fprintf(stderr, "Out of memory.\n");
      return 4;
   }

   umask(0);
   timestamp = time(NULL);
   return fuse_main(args.argc, args.argv, &callbacks);
}
