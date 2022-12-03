#include <iostream>
#include <fstream>
#include <string.h>
#include "_BMP.h"
using namespace std;

void printusage()
{
	cout << "Usage:\n" << endl;
	cout << "-conv <input file path> <output file path>\t\t Convert 24bpp or 32bpp image to 8bpp image." << endl;
	cout << "-zoom <input file path> <output file path> <scale>\t Downsampling the input image." << endl;
}
int choose(int n, char* str)
{
	// string sstr = (string)str;
	if (n == 4 && !strcmp(str, "-conv"))
		return 1;
	if (n == 5 && !strcmp(str, "-zoom"))
		return 2;
	return 0;
}
void cleanup(BMP& bmp)
{
	delete[] bmp.rDIB;
	delete[] bmp.colourTable;
	delete[] bmp.imgData;

	bmp.rDIB = NULL;
	bmp.colourTable = NULL;
	bmp.imgData = NULL;
}

int BMPread(BMP& bmp, char* input)
{
	ifstream fin(input, ios::binary);
	if (fin.fail())
	{
		cout << "Failed to open.";
		return 0;
	}
	//point to begin of file
	fin.seekg(ios_base::beg);
	//read bmp
	fin.read((char*)&bmp.Header, 14);
	fin.read((char*)&bmp.DIB, 40);
	//dib check and read the remain of dib
	if (bmp.DIB.size > 40)
	{
		int r = bmp.DIB.size - 40;
		bmp.rDIB = new char[r];
		fin.read(bmp.rDIB, r);
	}
	// read colour data (optional)
	if (bmp.Header.dataOffset > (14 + bmp.DIB.size))
	{
		int r = bmp.Header.dataOffset - (14 + bmp.DIB.size);
		bmp.colourTable = new char[r];
		fin.read(bmp.colourTable, r);
	}
	//read image data
	bmp.imgData = new char[bmp.DIB.imageSize];
	fin.read(bmp.imgData, bmp.DIB.imageSize);
	fin.close();
	return 1;
}
int BMPsave(BMP bmp, char* output)
{
	ofstream fout(output, ios::binary);
	if (fout.fail())
	{
		cout << "Failed to write.";
		return 0;
	}
	//write bmp to output path
	fout.write((char*)&bmp.Header, 14);
	fout.write((char*)&bmp.DIB, 40);
	if (bmp.DIB.size > 40)
		fout.write(bmp.rDIB, bmp.DIB.size - 40);
	if (bmp.Header.dataOffset > (14 + bmp.DIB.size))
		fout.write(bmp.colourTable, bmp.Header.dataOffset - (14 + bmp.DIB.size));
	fout.write(bmp.imgData, bmp.DIB.imageSize);
	fout.close();
	return 1;
}

int conv(BMP srcBmp, BMP& dstBmp)
{
	//bpp check
	if (srcBmp.DIB.bpp != 24 && srcBmp.DIB.bpp != 32)
	{
		cout << "Invalid file input. Please try again." << endl;
		return 0;
	}
	//copy metadata
	dstBmp.Header = srcBmp.Header;
	dstBmp.DIB = srcBmp.DIB;
	dstBmp.Header.dataOffset = 14 + dstBmp.DIB.size + 1024;
	if (dstBmp.DIB.size > 40)
		dstBmp.rDIB = srcBmp.rDIB;
	//create colour table
	dstBmp.colourTable = new char[256 * 4];
	for (int i = 0; i < 256; i++)
	{
		dstBmp.colourTable[i * 4 + 0] = (char)i;
		dstBmp.colourTable[i * 4 + 1] = (char)i;
		dstBmp.colourTable[i * 4 + 2] = (char)i;
		dstBmp.colourTable[i * 4 + 3] = (char)i;
	}
	//change bpp
	dstBmp.DIB.bpp = 8;
	//setting up
	   //byte per pixel
	int srcBytePerPixel = srcBmp.DIB.bpp / 8;
	int dstBytePerPixel = dstBmp.DIB.bpp / 8;
	//extract some info
	int height = srcBmp.DIB.height;
	int width = srcBmp.DIB.width;
	//calculate padding byte(s)
	int srcPadding = (4 - (width * srcBytePerPixel % 4)) % 4;
	int dstPadding = (4 - (width * dstBytePerPixel % 4)) % 4;
	//update info
	dstBmp.DIB.imageSize = (width * dstBytePerPixel + dstPadding) * height;
	dstBmp.Header.size = dstBmp.Header.dataOffset + dstBmp.DIB.imageSize;
	//allocate memory for new bmp image data
	dstBmp.imgData = new char[dstBmp.DIB.imageSize];
	//byte per row
	int srcBytePerRow = width * srcBytePerPixel + srcPadding;
	int dstBytePerRow = width * dstBytePerPixel + dstPadding;
	//convert 24bpp or 32bpp to 8bpp
	char* pSrcRow = srcBmp.imgData;
	char* pDstRow = dstBmp.imgData;
	for (int y = 0; y < height; y++)
	{
		char* pSrcPix = pSrcRow;
		char* pDstPix = pDstRow;
		for (int x = 0; x < width; x++)
		{
			unsigned char A;
			unsigned char B, G, R;
			if (srcBmp.DIB.bpp == 24)
			{
				B = pSrcPix[0];
				G = pSrcPix[1];
				R = pSrcPix[2];
			}
			else
			{
				// A = pSrcPix[0]; // ABGR
				// B = pSrcPix[1];
				// G = pSrcPix[2];
				// R = pSrcPix[3];

				B = pSrcPix[0];
				G = pSrcPix[1];
				R = pSrcPix[2];
				A = pSrcPix[3]; // BGRA
			}
			pDstPix[0] = (unsigned char)((B + G + R) / 3);
			//move to next pixel
			pSrcPix += srcBytePerPixel;
			pDstPix += dstBytePerPixel;
		}
		//move to next row
		pSrcRow += srcBytePerRow;
		pDstRow += dstBytePerRow;
	}
	return 1;
}
int zoom(BMP srcBmp, BMP& dstBmp, int s)
{
	//bpp check
	if (srcBmp.DIB.bpp != 8 && srcBmp.DIB.bpp != 24 && srcBmp.DIB.bpp != 32)
	{
		cout << "Invalid file input. Please try again." << endl;
		return 0;
	}
	//copy metadata
	dstBmp.Header = srcBmp.Header;
	dstBmp.DIB = srcBmp.DIB;
	if (dstBmp.DIB.size > 40)
		dstBmp.rDIB = srcBmp.rDIB;
	if (dstBmp.Header.dataOffset > (14 + dstBmp.DIB.size))
		dstBmp.colourTable = srcBmp.colourTable;
	//setting up
	   //byte per pixel
	int srcBytePerPixel = srcBmp.DIB.bpp / 8;
	int dstBytePerPixel = dstBmp.DIB.bpp / 8;
	//extract some info
	int height = srcBmp.DIB.height;
	int width = srcBmp.DIB.width;
	int nheight = dstBmp.DIB.height = height / s;
	int nwidth = dstBmp.DIB.width = width / s;
	//calculate padding byte(s)
	int srcPadding = (4 - (width * srcBytePerPixel % 4)) % 4;
	int dstPadding = (4 - (nwidth * dstBytePerPixel % 4)) % 4;
	//byte per row
	int srcBytePerRow = width * srcBytePerPixel + srcPadding;
	int dstBytePerRow = nwidth * dstBytePerPixel + dstPadding;
	//update info
	dstBmp.DIB.imageSize = (nwidth * dstBytePerPixel + dstPadding) * nheight;
	dstBmp.Header.size = dstBmp.Header.dataOffset + dstBmp.DIB.imageSize;
	//allocate memory for new bmp image data
	dstBmp.imgData = new char[dstBmp.DIB.imageSize];
	//box sampling
	char* pSrcRow = srcBmp.imgData;
	char* pDstRow = dstBmp.imgData;
	for (int y = 0; y < nheight; y++)
	{
		char* pSrcPix = pSrcRow;
		char* pDstPix = pDstRow;
		for (int x = 0; x < nwidth; x++)
		{
			int a = 0, b = 0, g = 0, r = 0, grey = 0;
			for (int m = 0; m < s; m++)
			{
				for (int n = 0; n < s; n++)
				{
					if (srcBmp.DIB.bpp == 24)
					{
						b += (unsigned char)pSrcPix[m * srcBytePerRow + n * srcBytePerPixel + 0];
						g += (unsigned char)pSrcPix[m * srcBytePerRow + n * srcBytePerPixel + 1];
						r += (unsigned char)pSrcPix[m * srcBytePerRow + n * srcBytePerPixel + 2];
					}
					else if (srcBmp.DIB.bpp == 32)
					{
						a += (unsigned char)pSrcPix[m * srcBytePerRow + n * srcBytePerPixel + 0];
						b += (unsigned char)pSrcPix[m * srcBytePerRow + n * srcBytePerPixel + 1];
						g += (unsigned char)pSrcPix[m * srcBytePerRow + n * srcBytePerPixel + 2];
						r += (unsigned char)pSrcPix[m * srcBytePerRow + n * srcBytePerPixel + 3];
					}
					else
						grey += (unsigned char)pSrcPix[m * srcBytePerRow + n * srcBytePerPixel];
				}
			}
			//average
			int sqr = s * s;
			if (srcBmp.DIB.bpp == 24)
			{
				pDstPix[0] = b / sqr;
				pDstPix[1] = g / sqr;
				pDstPix[2] = r / sqr;
			}
			else if (srcBmp.DIB.bpp == 32)
			{
				pDstPix[0] = a / sqr;
				pDstPix[1] = b / sqr;
				pDstPix[2] = g / sqr;
				pDstPix[3] = r / sqr;
			}
			else
				pDstPix[0] = grey / sqr;
			//move to next pixel
			pSrcPix += s * srcBytePerPixel;
			pDstPix += dstBytePerPixel;
		}
		//move to next row
		pSrcRow += s * srcBytePerRow;
		pDstRow += dstBytePerRow;
	}
	return 1;
}