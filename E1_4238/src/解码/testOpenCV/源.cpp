#include "detector.h"
#include "opencv2/opencv.hpp" //opencv 的头文件
#include "DataTranslation.h"
#include "slicer.h"
#include <bitset>
#include <cstdio>

using namespace std;
using namespace cv; //opencv 的命名空间


int main(int argc, char* argv[])
{
	static unsigned char temporary[3][10485760] = { 0 };
	static unsigned char checktemporary[3][10485760] = { 0 };
	int range[3];
	int start = getTickCount();
	Mat imgGray, res;
	Mat img;
	int count = 1;
	string inputfile = argv[count++];
	string outfile1 = argv[count++];
	string outfile2 = argv[count++];

	int ST = 0;
	int error = 0;
	int all = 0;
	for (int j = 0; j < 3; j++)
	{
		string a = "";
		string b = "";
		DataTranslator D(a, b);
		VedioSlicer GT(inputfile);
		int i = 0;
		while (1)
		{
			++i;

			img = GT.GetNextFrame();
			if (i % 3 != j)continue;
			if (img.empty())
			{
				break;
			}
			Detector d(img, 108, 192, 10);
			if (!d.Detect())throw;
			char x[2];
			d.picCount(x);
			D.DateTranslate(d.GetBinaryData(),temporary[j],checktemporary[j]);
		}
		range[j] = D.getSum();
		int nowall = D.getall();
		int nowerror = D.geterror();
		if (nowall > all)
		{
			ST = j;
			all = nowall;
			error = nowerror;
		}
		else if (nowall == all)
		{
			if (error > nowerror)
			{
				ST = j;
				all = nowall;
				error = nowerror;
			}
		}
	}

	DataTranslator DT(outfile1, outfile2);
	DT.OutPutData(temporary[ST], checktemporary[ST], range[ST]);
	//输出出错时校验的次数及校验的总次数
	cout << "error:" << error << endl << "all:" << all << endl;
	int end = getTickCount();
	int last = end - start;
	cout << "time consume: " << last / getTickFrequency() << 's' << endl;
	return 0;
}