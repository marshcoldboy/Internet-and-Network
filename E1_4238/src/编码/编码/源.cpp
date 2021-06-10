#include "FileToVedio.h"
#include <iostream>

using namespace std;

int main(int argc,char* argv[])
{
	FileToVedio file(argv[1]);
	file.GenerateVedio(argv[2], 1080, 1920,argv[3]);
	system("pause");
	return 0;
}