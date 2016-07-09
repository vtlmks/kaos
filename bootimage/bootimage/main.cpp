#include <iostream>
#include <stdio.h>
#pragma warning(disable: 4996) // warning about using fopen; tells to use fopen_s

unsigned char diskimage[1440*1024];
FILE *tempfile;
long filesize = 0;

void readfile(const char *filename, unsigned int offset) {
	FILE *tempfile = fopen(filename, "rb");
	if (tempfile) {
		fseek(tempfile, 0, SEEK_END);
		int filesize = ftell(tempfile);
		rewind(tempfile);
		fread(diskimage + offset, 1, filesize, tempfile);
		fclose(tempfile);
	}
}

int main(int argc, char *argv[]) {
	std::cout << "B0071MA63 v0.47a Copyright (c)2016 mksys\n" << std::endl;
	readfile(argv[1], 0x0000);
	readfile(argv[2], 0x0200);
	readfile(argv[3], 0x2200);

	tempfile = fopen("floppy.img", "wb+");
	if(tempfile) {
		fwrite(diskimage, 1, 1440*1024, tempfile);
		fclose(tempfile);
	}
}

