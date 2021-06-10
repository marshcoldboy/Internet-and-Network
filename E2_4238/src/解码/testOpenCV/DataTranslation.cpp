#include"DataTranslation.h"
int DataTranslator::Parity = 0;

DataTranslator::DataTranslator(string a, string b)
{
	outfile1 = a;
	outfile2 = b;
}

bool DataTranslator::WhetherInArea(int x, int y)
{
		if ((x > 7 && x < (dx - 8) && y >= 0 && y < dy) || (y > 7 && y < (dy - 8) && x >= 0 && x <= dx))
			return 1;
		else
		{
			if (x >= (dx - 8) && y >= (dy - 8) && x < dx&&y < dy)
			{
				if (x > (dx - 7) && x<(dx - 1) && y>(dy - 7) && y < (dy - 1))
					return 0;
				else
					return 1;
			}
			else
				return 0;
		}
	
}

int DataTranslator::getnextD()
{
	while (WhetherInArea(nowx, nowy) == 0)
	{
		if (nowy >= dy)
		{
			nowy--;
			nowx++;
			direction = -1;
			
		}
		else if (nowy < 0)
		{
			nowy++;
			nowx++;
			direction = 1;
		
		}
		else
		{
			nowy += direction;
		}
	}	
	int j = DataArray[nowx][nowy];
	nowy+=direction;
	return j;
}

short DataTranslator::InternetCheckSum(unsigned short*buf, int nwords)
{
	unsigned long sum;
	for (sum = 0; nwords > 1; )
	{
		sum += *(unsigned short*)buf++;
		nwords -= 2;
	}
	if (nwords > 0)
		sum += *(unsigned char*)buf;
	while (sum >> 16)
	{
		sum = (sum >> 16) + sum & 0xffff;
	}
	cout << sum << endl;
	return ~sum;
}


bool DataTranslator::Check_checkCRC8(unsigned char* pdat, unsigned char len)
{
	unsigned int i;
	unsigned char j, sum;
	unsigned char crc = 0x00;
	for (i = 0; i < len; i++) {
		crc ^= ((*pdat++) & 0xFF);
		for (int j = 8; j > 0; j--)
		{
			if (crc & 0x80) {
				crc <<= 1;
				crc ^= 0x31;
			}
			else { crc <<= 1; }
		}
	}
	sum = *pdat;
	if (sum == crc)
	{
		//cout << 0 << endl;
		allbyte++;
		return true;
	}
	else
	{
		//cout << 1 << endl;
		allbyte++;
		error++;
		return false;
	}

}


unsigned char*DataTranslator::hanmingcode(unsigned char*ch, bool &s)
{
	bitset<128> checkedData;
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			checkedData[i * 8 + j] = (ch[i] >> (7 - j)) & 1;
		}
	}
	bitset<7> checkPosition;
	for (int i = 1; i < 128; ++i)
	{
		if (i & 1)checkPosition[0] = checkPosition[0] ^ checkedData[i];
		if (i & 2)checkPosition[1] = checkPosition[1] ^ checkedData[i];
		if (i & 4)checkPosition[2] = checkPosition[2] ^ checkedData[i];
		if (i & 8)checkPosition[3] = checkPosition[3] ^ checkedData[i];
		if (i & 16)checkPosition[4] = checkPosition[4] ^ checkedData[i];
		if (i & 32)checkPosition[5] = checkPosition[5] ^ checkedData[i];
		if (i & 64)checkPosition[6] = checkPosition[6] ^ checkedData[i];
	}
	allbyte++;
	int x = checkPosition.to_ulong();
	if (x)
	{
		s = 1;
		checkedData[x] = ~checkedData[x];
		error++;
	}
	unsigned char data[15];
	memset(data, 0, 15);
	for (int i = 1, skip = 0; i < 128; i++)
	{
		if (i == 1 || i == 2 || i == 4 || i == 8 || i == 16 || i == 32 || i == 64)
		{
			++skip;
			continue;
		}
		data[(i - skip - 1) / 8] += checkedData[i] << (7 - (i - skip - 1) % 8);
	}
	cout << x << endl;
	return data;
}

void DataTranslator::Initialization(char**tes)
{
	//判断奇偶是否符合
	DataArray = tes;
	if ((DataArray[0][8]==0&&DataArray[0][9]==7)||(DataArray[0][8]==7&&DataArray[0][9]==0))
	{
		if ((DataArray[0][8] == 7) && (Parity == 0))
		{
			Parity = 1;
			PictureChange = 1;
		}
		else if ((DataArray[0][8] == 0) && (Parity == 1))
		{
			Parity = 0;
			PictureChange = 1;
		}
		else
			PictureChange = 0;

	}
	else
	{
		PictureChange=0;
	}
	if (PictureChange)
	{
		int i = 16;
		nowx = 0, nowy = 10;
		ByteNum = 0;
		direction = 1;
		while (i != 0)		//读入16个bit 对Bytnum初始化
		{
			int j = getnextD();
			if (j == 0)		//黑色 01
			{
				ByteNum = ByteNum << 1;
				ByteNum = (ByteNum << 1) | 1;
			}	
			else if (j == 2)//绿色 11
			{
				ByteNum = (ByteNum << 1) | 1;
				ByteNum = (ByteNum << 1) | 1;
			}
			else if (j == 4)//红色 00
			{
				ByteNum = ByteNum << 1;
				ByteNum = ByteNum << 1;
			}
			else if (j == 7)//白色10
			{
				ByteNum = (ByteNum << 1) | 1;
				ByteNum = ByteNum << 1;
			}
			i-=2;
		}
	}
}

void DataTranslator::OutPutData(unsigned char*data,unsigned char*check,int length)
{
	ofstream outFIle(outfile1, ios::app | ios::binary);
	ofstream checkFIle(outfile2, ios::app | ios::binary);
	for (int i = 0; i < length; i++)
	{
		outFIle.write((const char*)&data[i], sizeof(char));
		checkFIle.write((const char*)&check[i], sizeof(char));
	}
	outFIle.close();
	checkFIle.close();
}

void DataTranslator::DateTranslate(char**tes,unsigned char*out,unsigned char*vout)
{
	if (!tes)
	{
		Parity = Parity ^ 1;
		unsigned char a[466 * 10] = { 0 };
		unsigned char data[466 * 10] = { 0 };
		sum = 466 * 10;
		allbyte += 466;
		error += 466;
		for (int i = 0; i < 4660; i++)
		{
			out[testindex] = 0;
			vout[testindex] = 0;
			testindex++;
		}
	}
	else
	{
		int now = error;
		Initialization(tes);
		if (PictureChange)
		{
			bool sign = 0;
			if (ByteNum < 20504)
			{
				sign = 1;
			}
			unsigned char* test = new unsigned char[ByteNum / 4];//单张图片的数据
			unsigned char* check = new unsigned char[ByteNum / 4];//单张图片的数据
			unsigned char* DataCheckArray = new unsigned char[11];
			memset(DataCheckArray, 0, 11);
			int CheckArrayIndex = 0;
			int BlockNums = ByteNum;
			while (BlockNums != 0)
			{
				while (BlockLength != 0)
				{
					TemporaryArray[TemporaryIndex] = getnextD();
					BlockLength--;
					TemporaryIndex++;
					BlockNums--;
					if (BlockNums == 0)
					{
						break;
					}
				}
				if (BlockLength == 0)
				{
					memset(DataCheckArray, 0, 11);
					int ArrayIndex = 0;
					for (int i = 44; i > 0;)
					{
						for (int j = 0; j < 4; j++)
						{
							int m = TemporaryArray[ArrayIndex];
							ArrayIndex++;
							if (m == 0)//黑色 01
							{
								DataCheckArray[CheckArrayIndex] = DataCheckArray[CheckArrayIndex] << 1;
								DataCheckArray[CheckArrayIndex] = (DataCheckArray[CheckArrayIndex] << 1) | 1;
							}
							else if (m == 2)//绿色 11
							{
								DataCheckArray[CheckArrayIndex] = (DataCheckArray[CheckArrayIndex] << 1) | 1;
								DataCheckArray[CheckArrayIndex] = (DataCheckArray[CheckArrayIndex] << 1) | 1;
							}
							else if (m == 4)//红色 00
							{
								DataCheckArray[CheckArrayIndex] = DataCheckArray[CheckArrayIndex] << 1;
								DataCheckArray[CheckArrayIndex] = DataCheckArray[CheckArrayIndex] << 1;
							}
							else if (m == 7)//白色10
							{
								DataCheckArray[CheckArrayIndex] = (DataCheckArray[CheckArrayIndex] << 1) | 1;
								DataCheckArray[CheckArrayIndex] = DataCheckArray[CheckArrayIndex] << 1;
							}
						}
						CheckArrayIndex++;
						i -= 4;
					}
					//进行校验
					bool checkerror = Check_checkCRC8(DataCheckArray, 10);
					if (checkerror)
					{
						for (int i = 0; i < 10; i++)
						{
							out[testindex] = DataCheckArray[i];
							vout[testindex] = 255;
							testindex++;
						}
					}
					else
					{
						for (int i = 0; i < 10; i++)
						{
							out[testindex] = DataCheckArray[i];
							vout[testindex] = 0;
							testindex++;
						}
					}
					BlockLength = 44;
					TemporaryIndex = 0;
					CheckArrayIndex = 0;
					memset(DataCheckArray, 0, 11);
				}
				if (BlockLength != 0 && sign)
				{
					for (int i = 0; i < 11; i++)
					{
						DataCheckArray[i] = '\0';
					}
					int ArrayIndex = 0;
					for (int i = TemporaryIndex; i > 0; )
					{
						for (int j = 0; j < 4; j++)
						{
							int m = TemporaryArray[ArrayIndex];
							ArrayIndex++;
							if (m == 0)		//黑色 01
							{
								DataCheckArray[CheckArrayIndex] = DataCheckArray[CheckArrayIndex] << 1;
								DataCheckArray[CheckArrayIndex] = (DataCheckArray[CheckArrayIndex] << 1) | 1;
							}
							else if (m == 2)//绿色 11
							{
								DataCheckArray[CheckArrayIndex] = (DataCheckArray[CheckArrayIndex] << 1) | 1;
								DataCheckArray[CheckArrayIndex] = (DataCheckArray[CheckArrayIndex] << 1) | 1;
							}
							else if (m == 4)//红色 00
							{
								DataCheckArray[CheckArrayIndex] = DataCheckArray[CheckArrayIndex] << 1;
								DataCheckArray[CheckArrayIndex] = DataCheckArray[CheckArrayIndex] << 1;
							}
							else if (m == 7)//白色10
							{
								DataCheckArray[CheckArrayIndex] = (DataCheckArray[CheckArrayIndex] << 1) | 1;
								DataCheckArray[CheckArrayIndex] = DataCheckArray[CheckArrayIndex] << 1;
							}
							--i;
							if (i == 0)
							{
								break;
							}
						}
						CheckArrayIndex++;
					}
					for (int i = 0; i < CheckArrayIndex; i++)
					{
						out[testindex] = DataCheckArray[i];
						vout[testindex] = 255;
						testindex++;
					}
				}
			}
			sum = testindex;
		}
	}
}

int DataTranslator::getall()
{
	return allbyte;
}

int DataTranslator::geterror()
{
	return error;
}

int DataTranslator::getSum()
{
	return testindex;
}
