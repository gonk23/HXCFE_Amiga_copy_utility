/*
//
// This file is part of the HxCFloppyEmulator copy utility.
//
// It borrows heavily from the HxCFloppyEmulator file selector, so its notice
// is given below. Everything written in the notice below also applies to this file.
//
// Original HxCFloppyEmulator file selector notice:
// ---------------------------------------------------------------------------
// Copyright (C) 2009-2016 Jean-Fran�ois DEL NERO
//
// This file is part of the HxCFloppyEmulator file selector.
//
// HxCFloppyEmulator file selector may be used and distributed without restriction
// provided that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator file selector is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator file selector is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator file selector; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "hxcfeda.h"

#include "amiga_hw.h"
#include "amiga_regs.h"

#include "fat_opts.h"
#include "fat_misc.h"
#include "fat_defs.h"
#include "fat_filelib.h"


#define VERSION "0.1"
//#define DBGMODE 1
#define BUFFER_SIZE 16
#define PATH_SIZE 512

static unsigned long last_setlbabase;
static unsigned char sector[512];

volatile unsigned short io_floppy_timeout;

//-------------------------------------------------------------------
int setlbabase(unsigned long lba)
{
	int ret;
	unsigned char cmd_cnt;
	unsigned long lbatemp;

    direct_access_cmd_sector * dacs;
	direct_access_status_sector * dass;

	#ifdef DBGMODE
		printf("-- setlbabase E --\n");
	#endif

	dass=(direct_access_status_sector *)sector;
	dacs=(direct_access_cmd_sector  *)sector;

	memset(&sector,0,512);

	sprintf(dacs->DAHEADERSIGNATURE,"HxCFEDA");
	dacs->cmd_code=1;
	dacs->parameter_0=(lba>>0)&0xFF;
	dacs->parameter_1=(lba>>8)&0xFF;
	dacs->parameter_2=(lba>>16)&0xFF;
	dacs->parameter_3=(lba>>24)&0xFF;
	dacs->parameter_4=0xA5;

	ret=writesector( 0,(unsigned char *)&sector);
	if(!ret)
	{
		printf("ERROR: Write CTRL ERROR !\n");
		exit(1);
	}

	#ifdef DBGMODE
		printf("-- setlbabase L --\n");
	#endif

	return 0;
}

//-------------------------------------------------------------------
int test_floppy_if()
{
	unsigned char sector[512];
	direct_access_status_sector * dass;

	dass=(direct_access_status_sector *)sector;

	last_setlbabase = 2;
	do
	{
		setlbabase(last_setlbabase);
		if(!readsector(0,sector,1))
		{
			printf("read sector %d error !\n",last_setlbabase);
			exit(1);
		}

		#ifdef DBGMODE
			printf("       %.8X = %.8X ?\n", last_setlbabase, L_INDIAN(dass->lba_base));
		#endif

		if(last_setlbabase!=L_INDIAN(dass->lba_base))
		{
			printf("LBA Change Test Failed ! Write Issue ?\n");
			exit(1);
		}

		last_setlbabase--;
	}while(last_setlbabase);

	return 0;
}

//-------------------------------------------------------------------
int media_init()
{
	unsigned char ret;
	unsigned char sector[512];
	int i;
	direct_access_status_sector * dass;

	#ifdef DBGMODE
		printf("-- media_init E --\n");
	#endif

	last_setlbabase=0xFFFFF000;
	ret=readsector(0,(unsigned char*)&sector,1);

	if(ret)
	{
		dass=(direct_access_status_sector *)sector;
		if(!strcmp(dass->DAHEADERSIGNATURE,"HxCFEDA"))
		{
			//printf("Firmware %s\n" ,dass->FIRMWAREVERSION);

			test_floppy_if();

			dass= (direct_access_status_sector *)sector;
			last_setlbabase=0;
			setlbabase(last_setlbabase);

			#ifdef DBGMODE
				printf("-- media_init L --\n");
			#endif

			return 1;
		}

		printf("Bad signature - HxC Floppy Emulator not found!\n");

		#ifdef DBGMODE
			printf("-- media_init L --\n");
		#endif

		return 0;
	}

	printf("ERROR: Floppy Access error!  [%d]\n",ret);

	#ifdef DBGMODE
		printf("-- media_init L --\n");
	#endif

	return 0;
}

//-------------------------------------------------------------------
int media_read(unsigned long sector, unsigned char *buffer)
{
	int ret,retry;
	direct_access_status_sector * dass;

	dass= (direct_access_status_sector *)buffer;

	#ifdef DBGMODE
		printf("-- media_read E --\n");
	#endif

	ret=0;

	do
	{
		if((sector-last_setlbabase)>=8)
		{
			setlbabase(sector);
		}

		if(!readsector(0,buffer,0))
		{
			printf("ERROR: Read ERROR ! fsector %d\n",(sector-last_setlbabase)+1);
		}
		last_setlbabase = L_INDIAN(dass->lba_base);

	}while((sector-L_INDIAN(dass->lba_base))>=8);

	if(!readsector((sector-last_setlbabase)+1,buffer,0))
	{
		printf("ERROR: Read ERROR ! fsector %d\n",(sector-last_setlbabase)+1);
		exit(1);
	}

	#ifdef DBGMODE
		printf("-- media_read L --\n");
	#endif

	return 1;
}

//-------------------------------------------------------------------
int media_write(unsigned long sector, unsigned char *buffer)
{
	int ret,retry;
	direct_access_status_sector * dass;

	#ifdef DBGMODE
		printf("-- media_write E --\n");
	#endif

	if((sector-last_setlbabase)>=8)
	{
		last_setlbabase=sector;
		setlbabase(sector);
	}

	if(!writesector((sector-last_setlbabase)+1,buffer))
	{
		printf("ERROR: Write sector ERROR !\n");
		exit(1);
	}

	#ifdef DBGMODE
		printf("-- media_write L --\n");
	#endif

	return 1;
}

//-------------------------------------------------------------------
void usage(char *command_name)
{
    printf("Usage: %s <command> [<path>]\n", command_name);
    printf("\n");
    printf("<command> may be either:\n");
    printf("  dir  - Show FAT32 directory on HxC SD card.\n");
    printf("  copy - Copy FAT32 file/dir from HxC SD card.\n");
    printf("         The file or contents of dir are copied to current Amiga dir.\n");
    printf("         A dir is not created. Copy is not recursive.\n");
    printf("\n");
    printf("<path> uses the forward slash / as the path separator.\n");
}

//-------------------------------------------------------------------
void lowercase(char *s)
{
    int i;
    for (i=0; i < strlen(s); i++) s[i] = tolower(s[i]);
}

//-------------------------------------------------------------------
void normalise_path(char *new_path, char *old_path)
{
    if (old_path[0] == '/') old_path++;

    if (strlen(old_path)+1 > PATH_SIZE-1)
    {
        printf("Path length is too long (limit = %d)\n", PATH_SIZE-1);
        exit(1);
    }

    new_path[0] = '/';
    strcpy(++new_path, old_path);
}

//-------------------------------------------------------------------
void display_dir(char* path)
{
    struct fs_dir_list_status dirstat;

    if (fl_list_opendir(path, &dirstat))
    {
        struct fs_dir_ent dirent;

        while (fl_list_readdir(&dirstat, &dirent))
        {
            if (dirent.is_dir)
            {
                if ((strcmp(dirent.filename, ".")==0) || (strcmp(dirent.filename, "..")==0)) continue;
                printf("  %12s  %s\n", "<DIR>", dirent.filename);
            }
            else
            {
                printf("  %12d  %s\n", dirent.size, dirent.filename);
            }
        }
    }
}

//-------------------------------------------------------------------
void copy_file_from_hxc(char *filename_in)
{
    FL_FILE *fp_in;
    FILE *fp_out;
    char *filename_out;
    char buffer[BUFFER_SIZE];
    int bytes;

    filename_out = strrchr(filename_in, '/');
    if (filename_out == NULL)  //shouldn't happen!
    {
        filename_out = filename_in;
    }
    else
    {
        filename_out++;
    }

    if ((fp_in = fl_fopen(filename_in, "rb")) == NULL)
    {
        printf("Cannot open input file %s\n", filename_in);
        exit(1);
    }

    if ((fp_out = fopen(filename_out, "wb")) == NULL)
    {
        printf("Cannot open output file %s\n", filename_out);
        exit(1);
    }

    while (!fl_feof(fp_in))
    {
        if ((bytes = fl_fread(buffer, 1, BUFFER_SIZE, fp_in)) != BUFFER_SIZE)
        {
            if (!fl_feof(fp_in))  // There is no fl_ferror function
            {
                printf("Error while reading input file %s\n", filename_in);
                exit(1);
            }
        }
        if (fwrite(buffer, 1, bytes, fp_out) != bytes)
        {
            printf("Error while writing output file %s\n", filename_out);
            exit(1);
        }
    }

    fclose(fp_out);
    fl_fclose(fp_in);

    printf("Copied %s\n", filename_in);
}

//-------------------------------------------------------------------
void construct_full_filename(char *filename_full, char *path, char *filename)
{
    if (strlen(path)+1+strlen(filename) > PATH_SIZE-1)
    {
        printf("Full filename for path %s filename %s is too long (limit = %d)\n", path, filename, PATH_SIZE-1);
        exit(1);
    }

    strcpy(filename_full, path);
    if (filename_full[strlen(filename_full)-1] != '/') strcat(filename_full, "/");
    strcat(filename_full, filename);
}

//-------------------------------------------------------------------
void copy_dir_from_hxc(char* path)
{
    struct fs_dir_list_status dirstat;
    char filename_full[PATH_SIZE];

    if (fl_list_opendir(path, &dirstat))
    {
        struct fs_dir_ent dirent;

        while (fl_list_readdir(&dirstat, &dirent))
        {
            if (!dirent.is_dir)
            {
                construct_full_filename(filename_full, path, dirent.filename);
                copy_file_from_hxc(filename_full);
            }
        }
    }
}

//-------------------------------------------------------------------
int main(int argc, char* argv[])
{
	unsigned char bootdev;
    char path[PATH_SIZE];

    printf("HxC copy utility version %s\n", VERSION);

    if ((argc < 2) || (argc > 3))
    {
        usage(argv[0]);
        exit(1);
    }

    strcpy(path, "/");
    if (argc > 2) normalise_path(path, argv[2]);

	io_floppy_timeout = 0;

	bootdev = 0;
	while (bootdev < 4 && !test_drive(bootdev)) bootdev++;
	if(bootdev >= 4) bootdev = 0;
    printf("Found HxC on %d\n",bootdev);

	init_amiga_fdc(bootdev);

	if (media_init())
    {
		// Initialise File IO Library
		fl_init();

		// Attach media access functions to library
		if (fl_attach_media(media_read, media_write) != FAT_INIT_OK)
        {
			printf("ERROR: Media attach failed !");
			exit(1);
		}

        lowercase(argv[1]);
        if (strcmp(argv[1], "dir") == 0)
        {
            display_dir(path);
        }
        else if (strcmp(argv[1], "copy") == 0)
        {
            if (fl_is_dir(path))
            {
                copy_dir_from_hxc(path);
            }
            else
            {
                copy_file_from_hxc(path);
            }
        }
        else
        {
            usage(argv[0]);
            exit(1);
        }

        fl_shutdown();
	}

    jumptotrack(0);

    return 0;
}
