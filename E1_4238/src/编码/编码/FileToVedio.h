#pragma once
#include <opencv2/opencv.hpp>
#include <fstream>
#include <list>
#include <map>
#include <string>
#include <vector>

#define BlockSize 10
#define FrameRate 20

using namespace std;
using namespace cv;


class FileToVedio
{
private:
	ifstream fileIn;
	vector<unsigned char> data;
	//颜色的rgb值以及它所对应的编码我用字典来储存，string表示颜色的名字，int[0-2]为rgb值，int[3]为颜色所对应的2bit的十进制值
	map<string, int[4]> value_of_color;		
	Mat img;
	VideoWriter vedio;
	//当前读到的数据编号，即当前读取到vector<unsigned char> data中第几个字符了
	int currentDataIndex;
	//当前图片的编号
	int currentPic;  
	// 用于在生成图片时指示当前读取行的位置
	int currentRow;
	//用于在生成图片时指示当前读取列的位置
	int currentCol;
	bool currentPrity = false;
	void Check_CalaCRC8(unsigned char* pdat, unsigned char len);
	void DrawBlock(int x, int y, int widthor[]);
	// 画定位点和对齐点
	void InitialFormat(int height, int width); 
	//返回值表示当前图片中数据保存数据的像素块数，用来判断是什么时候写入补齐码
	int DrawHead(int height, int width);
	//生彩色像素块时确定像素块位置的函数
	void PositionOfPic(int row, int col, int color);	
	void DrawImage(int height, int width, int vedioTime);
	void ReadToMemory();
public:
	FileToVedio(const char* filePath);
	void GenerateVedio(const char* vedioPath, int height, int width, const char* vedioTime);
};