#include <iostream>
#include <fstream>
#include <string.h>
#include "_BMP.h"
using namespace std;

int main(int argc, char* argv[])
{
	BMP srcBmp;
	BMP dstBmp;

	int n = choose(argc, argv[1]);
	if (n == 0)
	{
		printusage();
		return 0;
	}

	int c = BMPread(srcBmp, argv[2]), d;

	if (n == 1 && c == 1)
	{
		d = conv(srcBmp, dstBmp);
		cout << "Convert successfully." << endl;
	}
	else if (n == 2 && c == 1)
	{
		d = zoom(srcBmp, dstBmp, (*argv[4] - '0'));
		cout << "Downsampling successfully." << endl;
	}
	else
	{
		cout << "Failed to process." << endl;
		return 0;
	}

	if (d == 1)
	{
		BMPsave(dstBmp, argv[3]);
		cout << "Saved." << endl;
	}

	cleanup(srcBmp);
	cleanup(dstBmp);
	return 1;
}