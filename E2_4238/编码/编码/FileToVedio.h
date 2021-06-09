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
	//��ɫ��rgbֵ�Լ�������Ӧ�ı��������ֵ������棬string��ʾ��ɫ�����֣�int[0-2]Ϊrgbֵ��int[3]Ϊ��ɫ����Ӧ��2bit��ʮ����ֵ
	map<string, int[4]> value_of_color;		
	Mat img;
	VideoWriter vedio;
	//��ǰ���������ݱ�ţ�����ǰ��ȡ��vector<unsigned char> data�еڼ����ַ���
	int currentDataIndex;
	//��ǰͼƬ�ı��
	int currentPic;  
	// ����������ͼƬʱָʾ��ǰ��ȡ�е�λ��
	int currentRow;
	//����������ͼƬʱָʾ��ǰ��ȡ�е�λ��
	int currentCol;
	bool currentPrity = false;
	void Check_CalaCRC8(unsigned char* pdat, unsigned char len);
	void DrawBlock(int x, int y, int widthor[]);
	// ����λ��Ͷ����
	void InitialFormat(int height, int width); 
	//����ֵ��ʾ��ǰͼƬ�����ݱ������ݵ����ؿ����������ж���ʲôʱ��д�벹����
	int DrawHead(int height, int width);
	//����ɫ���ؿ�ʱȷ�����ؿ�λ�õĺ���
	void PositionOfPic(int row, int col, int color);	
	void DrawImage(int height, int width, int vedioTime);
	void ReadToMemory();
public:
	FileToVedio(const char* filePath);
	void GenerateVedio(const char* vedioPath, int height, int width, const char* vedioTime);
};