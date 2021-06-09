#pragma once
#include<string>
#include<iostream>
#include<fstream>
#include <map>
#include<bitset>
using namespace std;

class DataTranslator {
private:
	string outfile1;
	string outfile2;
	char**DataArray=nullptr;	//整体二维码矩阵
	int dx = 108;
	int dy = 192;
	int sum=0;
	int nowx=0, nowy=0,direction=1;
	int ByteNum=0;
	int testindex = 0;
	static int Parity;			//奇数张为1，偶数张为0
	int allbyte = 0;
	int error = 0;
	bool PictureChange;			//与前一张图片是否一样
	int BlockLength = 44;
	int TemporaryArray[44];
	int TemporaryIndex = 0;
	int getnextD();
	bool WhetherInArea(int x, int y);
	short InternetCheckSum(unsigned short*buf,int nwords);
	void Initialization(char **tes);		//Parity,DataArray进行初始化
	unsigned char*hanmingcode(unsigned char*p,bool &s);
	bool Check_checkCRC8(unsigned char* pdat, unsigned char len);
public:
	int getSum();
	void OutPutData(unsigned char*data, unsigned char*check,int length);
	DataTranslator(string a,string b);
	void DateTranslate(char**tes,unsigned char*out,unsigned char*vout);
	int geterror();
	int getall();
	void out()
	{
		cout << endl << "error:" << error << endl << "all:" << allbyte << endl;
	}
};