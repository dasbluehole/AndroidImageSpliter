/* Android image spliter
 *(c)2019 Ashok Shankar Das ashok.s.das@gmail.com
 *Licensed under GNU LGPL v2 or latter
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//constants
unsigned char BOOT_MAGIC[] = "ANDROID!";
#define BOOT_MAGIC_SIZE  8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE  512
// Global variables to be filled.
unsigned int PAGE_SIZE = 0;
unsigned int KERNEL_SIZE = 0;
unsigned int RAMDISK_SIZE = 0;
unsigned int SECOND_SIZE = 0;

//Android image header
typedef struct boot_img_hdr
{
    unsigned char magic[BOOT_MAGIC_SIZE];

    unsigned kernel_size;  /* size in bytes */
    unsigned kernel_addr;  /* physical load addr */

    unsigned ramdisk_size; /* size in bytes */
    unsigned ramdisk_addr; /* physical load addr */

    unsigned second_size;  /* size in bytes */
    unsigned second_addr;  /* physical load addr */

    unsigned tags_addr;    /* physical addr for kernel tags */
    unsigned page_size;    /* flash page size we assume */
    unsigned unused[2];    /* future expansion: should be 0 */

    unsigned char name[BOOT_NAME_SIZE]; /* asciiz product name */

    unsigned char cmdline[BOOT_ARGS_SIZE];

    unsigned id[8]; /* timestamp / checksum / sha1 / etc */
}__attribute__((__packed__))boot_img_hdr;
//Utility functions
//source https://gist.github.com/ccbrown/9722406
void DumpHex(const void* data, size_t size) 
{
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

// read the header and stores it in different global variables.
void parse_header(FILE *fp)
{
	if(!fp)
	{
		printf("Error: Please Open android image file first\n");
		exit(0);
	}
	boot_img_hdr *bih=NULL;
	bih=(boot_img_hdr *)malloc(sizeof(boot_img_hdr));
	if(!bih)
	{
		printf("Error: Memory allocation error. Unable to allocate memory for boot image header\n");
		if(fp)		
			fclose(fp);
		exit(0);
	}
	if(fread(bih,sizeof(boot_img_hdr),1,fp)!=1)
	{
		printf("Error: Unable to read file\n");
		if(bih)
			free(bih);
		if(fp)
			fclose(fp);
		exit(0);
	}
	//we read the header
	// lets see if we have read it correctly
	char str[1024]="";
	strncpy(str,bih->magic,8);
	if(strcmp(BOOT_MAGIC,str))
	{
		printf("Error:Bad Magic!!!\n Wrong header or incorrect file.\n");
		DumpHex(bih,sizeof(boot_img_hdr));
		if(bih)
			free(bih);
		if(fp)
			fclose(fp);
		exit(0);
	}	
	printf("Found Magic!!!\n");
	printf("===========INFO===================\n");
	printf("Page size =     [%8d]   bytes\n",bih->page_size);
	printf("Kernel Size =   [%8d]   bytes\n",bih->kernel_size);
	printf("load address=   [0x%08x]       \n",bih->kernel_addr);
	printf("RAM disk size = [%8d]   bytes\n",bih->ramdisk_size);
	printf("load address =  [0x%08x]      \n",bih->ramdisk_addr);
	printf("DTB file size = [%8d]   bytes\n",bih->second_size);
	printf("load address =  [0x%08X]      \n",bih->second_addr);
	printf("KTAGs address = [0x%08X]      \n",bih->tags_addr);
	printf("(OPT)Product Name =  %s\n",bih->name);
	strncpy(str,bih->cmdline,BOOT_ARGS_SIZE);
	printf("(OPT)Comand Line =  %s\n",str);
	printf("==================================\n");
//populate the global variables.
	PAGE_SIZE = bih->page_size;
	KERNEL_SIZE = bih->kernel_size;
	RAMDISK_SIZE = bih->ramdisk_size;
	SECOND_SIZE = bih->second_size;

		if(bih)
			free(bih);
			
}
void dump_file(FILE *dest, FILE *src, int nPgs)
{
	unsigned char *buf =(unsigned char*)malloc(PAGE_SIZE);
	if(!buf)
	{
		printf("Error: Unable to allocate buffer of size %d\n Exiting...\n",PAGE_SIZE);
		if(dest)
			fclose(dest);
		if(src)
			fclose(src);		
		exit(0);
	} 
	while(nPgs)
	{
		if(!fread(buf,PAGE_SIZE,1,src))
		{
			printf("Error: reading file\n");
			return;
		}
		fwrite(buf,PAGE_SIZE,1,dest);
		nPgs--;
	}
	
}
void write_splited_files(FILE *fp)
{
	// we will attempt to split the input file in to constituents
	//though header is a structure it is given one page on disk
	// so skip 1 page from the begining to get the kernel.
	rewind(fp);
	fseek(fp,PAGE_SIZE,SEEK_SET); //skipped header
	// dump kernel
	FILE *kfp=fopen("kernel_boot.img","wb");
	if(!kfp)
	{
		printf("Error: Writing kernel image\n");
		return;
	}
	//calculate size in pages
	int kp = (int)((KERNEL_SIZE + PAGE_SIZE - 1) / PAGE_SIZE); // kernel pages
	int rp = (int)((RAMDISK_SIZE + PAGE_SIZE - 1) / PAGE_SIZE);// ramdisk pages
	int dp = (int)((SECOND_SIZE + PAGE_SIZE - 1) / PAGE_SIZE); //DTB pages
	dump_file(kfp,fp,kp);
	fclose(kfp);
	printf("[kernel_boot.img] Written\n"); 
	rewind(fp);
	fseek(fp,PAGE_SIZE*(1+kp),SEEK_SET); //skipped header+kernel
	FILE *rfp=fopen("ramdisk_boot.img","wb");
	if(!rfp)
	{
		printf("Error: Writing ramdisk image\n");
		return;
	}
	dump_file(rfp,fp,rp);
	fclose(rfp);
	printf("[ramdisk_boot.img] Written\n"); 
	rewind(fp);
	fseek(fp,PAGE_SIZE*(1+kp+rp),SEEK_SET); //skipped header+kernel
	FILE *dfp=fopen("dtb_boot.img","wb");
	if(!dfp)
	{
		printf("Error: Writing dtb image\n");
		return;
	}
	dump_file(dfp,fp,dp);
	fclose(dfp);
	printf("[dtb_boot.img] Written\n"); 
}
//main function
int main(int argc, char*argv[])
{
	if(argc!=2)
	{
		printf("USAGE: %s <romfile>\n",argv[0]);
		exit(0);
	}
	FILE *fp=NULL;
	fp=fopen(argv[1],"rb");
	if(!fp)
	{
		printf("Error: Unable to open file %s \n",argv[1]);
		exit(0);
	}
	//else file opened successfully
	parse_header(fp);
	write_splited_files(fp);
	if(fp)
		fclose(fp);
	return(0);
}
