#include "FileToVedio.h"
#include <bitset>

int FileToVedio::DrawHead(int height, int width)
{
	//��ͼƬ�л�����żУ��λ�����ؿ�
	if (currentPic % 2 == 0) {
		PositionOfPic(height / BlockSize, width / BlockSize, 1);
		PositionOfPic(height / BlockSize, width / BlockSize, 2);
	}
	else {
		PositionOfPic(height / BlockSize, width / BlockSize, 2);
		PositionOfPic(height / BlockSize, width / BlockSize, 1);
	}
	// ������ͼƬд���ݵ�ͷ��-ͼƬ���ݶ�Ӧ�������ؿ����
	//Ĭ�������ؿ����Ϊwidth/10*height/10-64*3-25-4-10-1,4��ʾ�����룬10��ʾͷ��,1��ʾ��2bit��������
	int DataBlockNum =width/10*height/10-232 ;
	//���һ��ͼ���������ؿ�����
	if (data.size()*4 - currentDataIndex*4 < DataBlockNum) {		
		DataBlockNum = (data.size() - currentDataIndex)*4 ;
	}
	//���ڴ���������ֽ����Ķ�����ֵ
	int ByteBinary[16] = { 0 };	
	//����ʮ����ת������
	int copy = DataBlockNum;
	for (int i = 0;i<16 && copy; ++i) {
		ByteBinary[15-i] = copy % 2;
		copy /= 2;
	}
	//���������ɵ����ؿ����ɫ
	int color = 0;	
	for (int i = 0; i < 16; ++i) {
		if (i % 2 == 0) color += ByteBinary[i]*2;	
		else {
			//ÿȡ����bit���������Ǽ����color��������תʮ���ƣ���Ȼ���ٵ��ò�ɫ���ؿ����ɺ���
			color += ByteBinary[i] ;
			PositionOfPic(height/10,width/10,color);	
			color = 0;
		}
	}
	//����ֵ��ʾ��ǰ��ͼƬ���ж������ؿ�������������ݵ�
	return DataBlockNum;	
}

//��ɫ���ؿ����ɺ���
void FileToVedio::PositionOfPic(int row,int col,int color) {
	string ColorName = "";
	//���Ǹ���colorֵ�ҵ���Ӧ����ɫ���Ӷ��õ����ǵ�rgb�����в�ɫ���ػ�����
	for (map<string, int[4]>::iterator it=value_of_color.begin();it!=value_of_color.end(); it++) {
		if (it->second[3] == color) ColorName = it->first;
	}
	//��ɫɫ�ؿ����з�ʽ---�ж��������л���ż���У��Լ��ж��Ƿ��ڶ�λ��Ͷ�����������
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
	//�ļ�����
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
	//��ʣ�೤�Ȳ���15�ֽ�
	// ʣ�೤��
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
		//�Ƚ�ȡ��ǰָ��ָ���ֵ��Ȼ���ָ�� + 1
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
	//�ܹ����ɵ�ͼƬ��
	int PicNum = data.size() / ((width/BlockSize * height/BlockSize - 232) / 4);		
	if (PicNum*((width/BlockSize * height/BlockSize - 232) / 4) < data.size()) ++PicNum;
	PicNum = (vedioTime * 20 / 1000) < PicNum ? vedioTime * 20 / 1000 : PicNum;		
	//��ǰͼƬ����ʼ��Ϊ1
	currentPic = 1;		
	while (currentPic <= PicNum) {
		InitialFormat(height, width);
		//��õ�ǰͼƬ���������ؿ���
		int DataBlockNum = DrawHead(width, height);
		//����unsigned char���ݵĶ�������
		int DataBinary[8] = { 0 };	
		do {
			//��ȡ��ǰ���ַ�
			unsigned char currerntChar = data[currentDataIndex];	
			//ת��Ϊ��������ʽ
			for (int i = 0; i < 8; ++i) {						
				DataBinary[7 - i] = currerntChar % 2;
				currerntChar /= 2;
			}
			//���ɲ�ɫ���ؿ飬��drawHead()
			int color = 0;
			for (int i = 0; i < 8; ++i) {
				if (i % 2 == 0) color += DataBinary[i] * 2;
				else {
					color += DataBinary[i];
					PositionOfPic(height/BlockSize, width/BlockSize, color);	
					color = 0;
				}
			}
			//׼����ȡ��һ���ַ�
			++currentDataIndex;		
			//�жϵ�ǰ��ȡ�ַ�����Ӧ�����ݿ��Ƿ��ѳ�����ǰͼ�����ַ��飬���ж��Ƿ�Ӧ�л�����һҳͼ
		} while (currentDataIndex<data.size()&&((currentDataIndex * 4) - (currentPic - 1)* (width/BlockSize * height/BlockSize - 232)) < DataBlockNum);	
		//����2bit��Ϣ��������Ϊ00��Ȼ����н�����0000 0000�Ļ��ƣ�4�����ؿ飩
		for (int i = 0; i < 5; ++i) {
			PositionOfPic(height/BlockSize, width/BlockSize, 0);
		}
		//��ͼƬ��д�벹����
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
	//����ͼ�����ñ���ɫΪ��ɫ
	img.create(height,width, CV_8UC3);
	img.setTo(Scalar(255,255, 255));	
	currentRow = 0;
	currentCol = 0;
	for (int i = 1; i < 6; i++) {
		//���Ϸ���
		DrawBlock(1, i, value_of_color["black"]);
		DrawBlock(i, 1, value_of_color["black"]);
		DrawBlock(5, i, value_of_color["black"]);
		DrawBlock(i, 5, value_of_color["black"]);
		//���Ϸ���
		DrawBlock(1, i + width/10 - 7,value_of_color["black"]);
		DrawBlock(i, 1 + width/10 - 7,value_of_color["black"]);
		DrawBlock(5, i + width/10 - 7, value_of_color["black"]);
		DrawBlock(i, width/10 - 2, value_of_color["black"]);
		//���·���
		DrawBlock(height/10 - 6, i, value_of_color["black"]);
		DrawBlock(i + height/10 - 7, 1, value_of_color["black"]);
		DrawBlock(height/10 - 2, i, value_of_color["black"]);
		DrawBlock(i + height/10 - 7, 5, value_of_color["black"]);
	}
	for (int i = 0; i < 8; i++) {
		//��������
		DrawBlock(i, 7, value_of_color["black"]);
		DrawBlock(7, i, value_of_color["black"]);
		//��������
		DrawBlock(i, width/10 - 8, value_of_color["black"]);
		DrawBlock(7, i + width/10 - 8, value_of_color["black"]);
		//��������
		DrawBlock(i + height/10 - 8, 7, value_of_color["black"]);
		DrawBlock( height/10 - 8, i, value_of_color["black"]);
	}

	for (int i = 0; i < 3; i++) {
		//���¶����
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
	//���ɵ�ͼƬ���ĳ�ʼֵΪ0
	currentPic = 0;	
	//��ȡ���ݸ����ĳ�ʼֵΪ0
	currentDataIndex = 0;
	//��ɫ�ֵ�ĳ�ʼֵ
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

