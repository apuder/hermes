/*************************************************
 * FATCOPY                                       *
 *************************************************/

#include <fs.h>

main(int argc, char** argv)
{
  unsigned char* buffer;
  unsigned int file_size, i;

  FILE* source_file;

  TOS_Int32 error;

  TOS_FAT_Device device;
  FAT_FD fd;

  DIR_ENTRY_INFO dir_entry_info;

  FAT_DIR_ENTRY dir_entry;

  if (argc != 4)
    {
      printf("USAGE: fatcopy <image file> <source file> <destination file on image\n");
      return;
    }

  if (!(fp = fopen(argv[1], "r+")))
    {
      printf("EXCEPTION: Image file could not be opened!\n");
      return;
    }

  if (!(source_file = fopen(argv[2], "r+")))
    {
      printf("EXCEPTION: Source file could not be opened!\n");
      return;
    }

  fs_init(&device);

  // read source file

  fseek(source_file, 0, SEEK_END);
  file_size = ftell(source_file);
  fseek(source_file, 0, SEEK_SET);

  buffer = (char*) malloc(file_size);

  fread(buffer, file_size, 1, source_file);

  // write destination file

  if ((fd = fs_open(&device, argv[3], TOS_FS_OPEN_MODE_WRITE)) < 0)
    {
      printf("EXCEPTION: Destination file could not be opened!\n");
      return;
    }

  if (fs_write(fd, buffer, file_size) < 0)
    {
      printf("EXCEPTION: Destination file could not be written!\n");
      return;
    }

  if (fs_close(fd) < 0)
    {
      printf("EXCEPTION: Destination file could not be closed!\n");
      return;
    }

  free(buffer);

  if (fclose(fp) == EOF)
    {
      printf("EXCEPTION: Image file could not be closed!\n");
      return;
    }

  if (fclose(source_file) == EOF)
    {
      printf("EXCEPTION: Source file could not be closed!\n");
      return;
    }
}
