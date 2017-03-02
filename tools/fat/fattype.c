/******************************************************************************
 * FATTYPE                                                                    *
 ******************************************************************************/

#include <fs.h>

main(int argc, char** argv)
{
  unsigned char* buffer;

  unsigned int i, size;

  FAT_FD fd;

  TOS_FAT_Device device;

  if (argc != 3)
    {
      printf("USAGE: fattype <image file> <file>\n");
      return;
    }

  if (!(fp = fopen(argv[1], "r+")))
    {
      printf("EXCEPTION: Image file could not be opened!\n");
      return;
    }

  fs_init(&device);

  if ((fd = fs_open(&device, argv[2], TOS_FS_OPEN_MODE_READ)) < 0)
    {
      printf("EXCEPTION: File could not be opened!\n");
      return;
    }

  if (fs_get_file_size(fd, &size) < 0)
    {
      printf("EXCEPTION: Bad file descriptor!\n");
      return;
    }

  buffer = (unsigned char*) malloc(size);

  fs_read(fd, buffer, size);

  if (fs_close(fd) < 0)
    {
      printf("EXCEPTION: File could not be closed!\n");
      free(buffer);
      return;
    }

  for (i = 0; i < size; i++)
    printf("%c", buffer[i]);

  printf("\n");

  free(buffer);

  if (fclose(fp) == EOF)
    {
      printf("EXCEPTION: Image file could not be closed!\n");
      return;
    }
}
