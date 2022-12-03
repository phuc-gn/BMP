#pragma once

#ifndef _BMP
#define _BMP

#pragma pack(push, 2)
struct BMP_Header //14 bytes
{
	char sign[2]; //"BM"
	int size; // size of bmp
	int reserved;
	int dataOffset; //address of image data
};

struct BMP_DIB //40 bytes or higher
{
	int size; //dib size
	int width;
	int height;
	short planes;
	short bpp;
	int compression;
	int imageSize; // image size
	int xres;
	int yres;
	int numColour;
	int numimColour;
};
#pragma pack(pop)

struct BMP
{
	BMP_Header Header;
	BMP_DIB DIB;
	char* rDIB; //point to remain
	char* colourTable;
	char* imgData; //point to image data
};

void printusage();
int choose(int, char*);
void cleanup(BMP&);

int BMPread(BMP&, char*); //read image data from input path
int BMPsave(BMP, char*); //write image data to ouput path

int conv(BMP, BMP&); //convert 24bpp or 32bpp to 8bpp
int zoom(BMP, BMP&, int); //downsampling

#endif