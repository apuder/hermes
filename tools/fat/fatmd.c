/******************************************************************************
 * FATMD                                                                      *
 ******************************************************************************/

#include <fs.h>

main(int argc, char** argv)
{
  TOS_FAT_Device device;

  if (argc != 4)
    {
      printf("USAGE: fatmd <image file> <parentdirectory> <directory>\n");
      return;
    }

  if (!(fp = fopen(argv[1], "r+")))
    {
      printf("EXCEPTION: Image file could not be opened!\n");
      return;
    }

  fs_init(&device);

  if (!fs_create_directory(&device, argv[2], argv[3]))
  {
    printf("EXCEPTION: Directory could not be created!\n");
    return;
  }

  if (fclose(fp) == EOF)
    {
      printf("EXCEPTION: Image file could not be closed!\n");
      return;
    }
}
