#include "FileToVedio.h"
#include <bitset>

int FileToVedio::DrawHead(int height, int width)
{
	//在图片中绘制奇偶校验位的像素块
	if (currentPic % 2 == 0) {
		PositionOfPic(height / BlockSize, width / BlockSize, 1);
		PositionOfPic(height / BlockSize, width / BlockSize, 2);
	}
	else {
		PositionOfPic(height / BlockSize, width / BlockSize, 2);
		PositionOfPic(height / BlockSize, width / BlockSize, 1);
	}
	// 这里向图片写数据的头部-图片数据对应的总像素块个数
	//默认总像素块个数为width/10*height/10-64*3-25-4-10-1,4表示结束码，10表示头部,1表示有2bit数据舍弃
	int DataBlockNum =width/10*height/10-232 ;
	//最后一张图的数据像素块总数
	if (data.size()*4 - currentDataIndex*4 < DataBlockNum) {		
		DataBlockNum = (data.size() - currentDataIndex)*4 ;
	}
	//用于存放数据区字节数的二进制值
	int ByteBinary[16] = { 0 };	
	//进行十进制转二进制
	int copy = DataBlockNum;
	for (int i = 0;i<16 && copy; ++i) {
		ByteBinary[15-i] = copy % 2;
		copy /= 2;
	}
	//计算所生成的像素块的颜色
	int color = 0;	
	for (int i = 0; i < 16; ++i) {
		if (i % 2 == 0) color += ByteBinary[i]*2;	
		else {
			//每取两个bit，根据它们计算出color（二进制转十进制），然后再调用彩色像素块生成函数
			color += ByteBinary[i] ;
			PositionOfPic(height/10,width/10,color);	
			color = 0;
		}
	}
	//返回值表示当前的图片中有多少像素块是拿来存放数据的
	return DataBlockNum;	
}

//彩色像素块生成函数
void FileToVedio::PositionOfPic(int row,int col,int color) {
	string ColorName = "";
	//这是根据color值找到对应的颜色，从而得到它们的rgb，进行彩色像素绘生成
	for (map<string, int[4]>::iterator it=value_of_color.begin();it!=value_of_color.end(); it++) {
		if (it->second[3] == color) ColorName = it->first;
	}
	//彩色色素块排列方式---判断是奇数行还是偶数行，以及判断是否在定位点和对齐点的区域内
	for (; currentRow < row; ++currentRow) {//
		for (; currentCol < col&&currentCol>=0;) {//
				if (currentRow <= 7 ) {
					if ((currentRow <= 7 && currentCol <= 7) || (currentRow <= 7 && currentCol >= col - 8 && currentCol <= col - 1)) {
						if(currentRow % 2 == 0) ++currentCol;
						else --currentCol;
						continue;
					}
					else if(currentRow%2==0){
						DrawBlock(currentRow, currentCol,value_of_color[ColorName]);
						++currentCol;
						return;
					}
					else if (currentRow % 2 == 1) {
						DrawBlock(currentRow, currentCol, value_of_color[ColorName]);
						--currentCol;
						return;
					}
				}
				else if (currentRow > 7 && currentRow < row - 8) {
					if (currentRow % 2 == 0) {
						DrawBlock(currentRow, currentCol, value_of_color[ColorName]);
						++currentCol;
						return;
					}
					else {
						DrawBlock(currentRow, currentCol, value_of_color[ColorName]);
						--currentCol;
						return;
					}
				}
				else {
					if ((currentRow >= row - 8 && currentRow <= row - 1 && currentCol <= 7) || (currentRow >= row - 6 && currentRow <= row - 2 && currentCol >= col - 6 && currentCol <= col - 2)) {
						if (currentRow % 2 == 0) ++currentCol;
						else --currentCol;
						continue;
					}
					else if (currentRow % 2 == 0) {
						DrawBlock(currentRow, currentCol, value_of_color[ColorName]);
						++currentCol;
						return;
					}
					else if (currentRow % 2 == 1) {
						DrawBlock(currentRow, currentCol, value_of_color[ColorName]);
						--currentCol;
						return;
					}
				}
		}
		if (currentRow % 2 == 0) --currentCol;
		else ++currentCol;
	}
}

void FileToVedio::ReadToMemory()
{
	fileIn.seekg(0, ios::end);
	//文件长度
	int fileLength = fileIn.tellg(); 
	fileIn.seekg(0, ios::beg);
	unsigned char ch[11];
	for (int i = 0; i < fileLength / 10; ++i)
	{
		fileIn.read((char*)ch, 10);
		Check_CalaCRC8(ch, 10);
		for (int j = 0; j < 11; ++j)
		{
			data.push_back(ch[j]);
		}
	}
	if (fileIn.eof()) return;
	//当剩余长度不足15字节
	// 剩余长度
	int i = 0; 
	while (!fileIn.eof())
	{
		char temp;
		fileIn.read(&temp, 1);
		data.push_back(temp);
		++i;
	}
}

void FileToVedio::Check_CalaCRC8(unsigned char* pdat, unsigned char len)
{
	unsigned char crc = 0x00;
	for (int i = 0; i < len; i++)
	{
		//先将取当前指针指向的值，然后把指针 + 1
		crc ^= ((*pdat++) & 0xFF); 
		for (int j = 8; j > 0; j--)
		{
			if (crc & 0x80)
			{
				crc <<= 1;
				crc ^= 0x31; //x^8+x^5+x^4+1
			}
			else crc <<= 1;
		}
	}
	*pdat = crc & 0xFF;
}


void FileToVedio::DrawImage(int height, int width,int vedioTime)
{
	//总共生成的图片数
	int PicNum = data.size() / ((width/BlockSize * height/BlockSize - 232) / 4);		
	if (PicNum*((width/BlockSize * height/BlockSize - 232) / 4) < data.size()) ++PicNum;
	PicNum = (vedioTime * 20 / 1000) < PicNum ? vedioTime * 20 / 1000 : PicNum;		
	//当前图片数初始化为1
	currentPic = 1;		
	while (currentPic <= PicNum) {
		InitialFormat(height, width);
		//获得当前图片的数据像素块数
		int DataBlockNum = DrawHead(width, height);
		//保存unsigned char数据的二进制数
		int DataBinary[8] = { 0 };	
		do {
			//获取当前的字符
			unsigned char currerntChar = data[currentDataIndex];	
			//转化为二进制形式
			for (int i = 0; i < 8; ++i) {						
				DataBinary[7 - i] = currerntChar % 2;
				currerntChar /= 2;
			}
			//生成彩色像素块，如drawHead()
			int color = 0;
			for (int i = 0; i < 8; ++i) {
				if (i % 2 == 0) color += DataBinary[i] * 2;
				else {
					color += DataBinary[i];
					PositionOfPic(height/BlockSize, width/BlockSize, color);	
					color = 0;
				}
			}
			//准备获取下一个字符
			++currentDataIndex;		
			//判断当前读取字符所对应的数据块是否已超出当前图的总字符块，即判断是否应切换到下一页图
		} while (currentDataIndex<data.size()&&((currentDataIndex * 4) - (currentPic - 1)* (width/BlockSize * height/BlockSize - 232)) < DataBlockNum);	
		//舍弃2bit信息，将其置为00，然后进行结束码0000 0000的绘制（4个像素块）
		for (int i = 0; i < 5; ++i) {
			PositionOfPic(height/BlockSize, width/BlockSize, 0);
		}
		//往图片中写入补齐码
		if (DataBlockNum < width/BlockSize * height/BlockSize - 232) {
			for (int i = 0; i < (width/BlockSize * height/BlockSize - 230) - DataBlockNum; ++i) {
				switch (i % 8) {
				case 0:case 2:
					PositionOfPic(height/BlockSize, width/BlockSize, 3);
					break;
				case 1:
					PositionOfPic(height/BlockSize, width/BlockSize, 2);
					break;
				case 3:case 5:case 7:
					PositionOfPic(height/BlockSize, width/BlockSize, 1);
					break;
				case 4:case 6:
					PositionOfPic(height/BlockSize, width/BlockSize, 0);
					break;
				}
			}
		}
		++currentPic;
		vedio << img;
	}
	vedio.release();
}



void FileToVedio::DrawBlock(int x, int y,int color[]) //
{
	rectangle(
		img,
		Point(y*BlockSize+0.5, x*BlockSize+0.5),
		Point((y+1)*BlockSize-0.5, (x+1)*BlockSize-0.5),
		Scalar(color[0], color[1],color[2]),	
		FILLED,
		LINE_8
	);
}

void FileToVedio::InitialFormat(int height, int width)
{
	//生成图像并设置背景色为白色
	img.create(height,width, CV_8UC3);
	img.setTo(Scalar(255,255, 255));	
	currentRow = 0;
	currentCol = 0;
	for (int i = 1; i < 6; i++) {
		//左上方框
		DrawBlock(1, i, value_of_color["black"]);
		DrawBlock(i, 1, value_of_color["black"]);
		DrawBlock(5, i, value_of_color["black"]);
		DrawBlock(i, 5, value_of_color["black"]);
		//右上方框
		DrawBlock(1, i + width/10 - 7,value_of_color["black"]);
		DrawBlock(i, 1 + width/10 - 7,value_of_color["black"]);
		DrawBlock(5, i + width/10 - 7, value_of_color["black"]);
		DrawBlock(i, width/10 - 2, value_of_color["black"]);
		//左下方框
		DrawBlock(height/10 - 6, i, value_of_color["black"]);
		DrawBlock(i + height/10 - 7, 1, value_of_color["black"]);
		DrawBlock(height/10 - 2, i, value_of_color["black"]);
		DrawBlock(i + height/10 - 7, 5, value_of_color["black"]);
	}
	for (int i = 0; i < 8; i++) {
		//左上两边
		DrawBlock(i, 7, value_of_color["black"]);
		DrawBlock(7, i, value_of_color["black"]);
		//右上两边
		DrawBlock(i, width/10 - 8, value_of_color["black"]);
		DrawBlock(7, i + width/10 - 8, value_of_color["black"]);
		//左下两边
		DrawBlock(i + height/10 - 8, 7, value_of_color["black"]);
		DrawBlock( height/10 - 8, i, value_of_color["black"]);
	}

	for (int i = 0; i < 3; i++) {
		//右下对齐点
		DrawBlock(height/10 - 5, width/10 - 5 + i, value_of_color["black"]);
		DrawBlock(height/10 - 3, width/10 - 5 + i, value_of_color["black"]);
		DrawBlock(height/10 - 5 + i, width/10 - 5, value_of_color["black"]);
		DrawBlock(height/10 - 5 + i, width/10 - 3, value_of_color["black"]);
	}
}

void FileToVedio::GenerateVedio(const char* vedioPath, int height, int width,const char* vedioTime)
{
	int fourcc = vedio.fourcc('M', 'J', 'P', 'G');
	vedio.open(vedioPath, fourcc, FrameRate, Size(width, height), true);
	int vediotime = 0;
	for (int i = 0; i < strlen(vedioTime); ++i) {
		vediotime += (vedioTime[i] - '0')*pow(10, strlen(vedioTime) - i - 1);
	}
	ReadToMemory();
	DrawImage(height, width, vediotime);
}

FileToVedio::FileToVedio(const char* filePath)
{
	fileIn.open(filePath, ios::binary);
	//生成的图片数的初始值为0
	currentPic = 0;	
	//读取数据个数的初始值为0
	currentDataIndex = 0;
	//颜色字典的初始值
	value_of_color["black"][0] = 0;
	value_of_color["black"][1] = 0;
	value_of_color["black"][2] = 0;
	value_of_color["black"][3] =1;//
	value_of_color["white"][0] = 255;
	value_of_color["white"][1] = 255;
	value_of_color["white"][2] = 255;
	value_of_color["white"][3] = 2;//
	value_of_color["red"][0] = 0;
	value_of_color["red"][1] = 0;
	value_of_color["red"][2] = 255;
	value_of_color["red"][3] = 0;//
	value_of_color["green"][0] = 0;
	value_of_color["green"][1] = 255;
	value_of_color["green"][2] = 0;
	value_of_color["green"][3] = 3;//
}

