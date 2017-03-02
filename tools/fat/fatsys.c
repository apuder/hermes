/******************************************************************************
 * FATFORMAT                                                                  *
 ******************************************************************************/

//#include <fs.h>
#include <stdio.h>
#include <stdlib.h>

void print_usage ()
{
  printf("USAGE: fatsys <image file> <boot_sector>");
}

main (int argc, char* argv[])
{
//  TOS_FAT_Device device;

//  unsigned int file_size, i;
//  unsigned char size[10] = "";

  unsigned char* buffer;
  char temp;
//  BPB_FAT bpb_fat;
//  BPB_FAT16 bpb_fat16;
//  BPB_FAT32 bpb_fat32;

  if (!(argc == 3))
    {
      print_usage();
      return;
    }
  int i=0;
  printf("%s\n\n", argv[2]);
  //printf("%c\n", argv[1][0]);
  //Variable to be used as the image file
  
  FILE *image_file;
  
  //Variable to be used as the boot sector
  FILE *boot_sector;

  boot_sector = fopen(argv[2], "r");
  image_file = fopen(argv[1], "r+");
  
  while (i < 3){
     temp = getc(boot_sector);
     fputc(temp, image_file);
     i++;
     printf("%i", i);
  }
  
  while(i < 62){
     temp = getc(image_file);
     fputc(temp, image_file);
     i++;
  }
  
  
  fseek(boot_sector, 62, SEEK_SET);
  
  while(temp != EOF){
  
     temp = getc(boot_sector);
     fputc(temp, image_file);
    
  }
  
  fclose(image_file);
  fclose(boot_sector);
  

  
  

/*      if (strcmp(size, "DISC") == 0)
	file_size = 1474560;
      else
	file_size = atoi(size);

      buffer = (char*) malloc(file_size);

      for (i = 0; i < file_size; i++)
	buffer[i] = '\0';

      if (strcmp(size, "DISC") == 0)
	{
	  bpb_fat.bs_jmp_boot[0] = 0xEB;
	  bpb_fat.bs_jmp_boot[1] = 0x3D;
	  bpb_fat.bs_jmp_boot[2] = 0x90;

	  strncpy(bpb_fat.bs_oem_name, "MSWIN4.1", 8);

	  bpb_fat.bpb_byts_per_sec = 512;
	  bpb_fat.bpb_sec_per_clus = 1;
	  bpb_fat.bpb_rsvd_sec_cnt = 1;
	  bpb_fat.bpb_num_fats = 2;
	  bpb_fat.bpb_root_ent_cnt = 224;
	  bpb_fat.bpb_tot_sec16 = 2880;
	  bpb_fat.bpb_media = 0xF0;
	  bpb_fat.bpb_fat_sz16 = 9;
	  bpb_fat.bpb_sec_per_trk = 18;
	  bpb_fat.bpb_num_heads = 2;
	  bpb_fat.bpb_hidd_sec = 0;
	  bpb_fat.bpb_tot_sec32 = 0;

	  bpb_fat16.bs_drv_num = 0;
	  bpb_fat16.bs_reserved1 = 0;
	  bpb_fat16.bs_boot_sig = 0x29;
	  bpb_fat16.bs_vol_id = 0;
	  
	  strncpy(bpb_fat16.bs_vol_lab, "NO NAME    ", 11);
	  strncpy(bpb_fat16.bs_fil_sys_type, "FAT12   ", 8);

	  bpb_fat.u.fat16 = bpb_fat16;

	  memcpy(buffer, &bpb_fat, 62);
	}

      buffer[510] = 0x55;
      buffer[511] = 0xAA;

      if (!(fp = fopen(argv[1], "w")))
	{
	  printf("EXCEPTION: Image file could not be written!\n");
	  return;
	}

      fwrite(buffer, file_size, 1, fp);

      if (fclose(fp) == EOF)
	{
	  printf("EXCEPTION: Image file could not be closed!\n");
	  return;
	}

      free (buffer);
    }
  else
    {
      fseek(fp, 0, SEEK_END);
      file_size = ftell(fp);
      printf("FILESIZE = %d\n", file_size);
    }
}

*/
}