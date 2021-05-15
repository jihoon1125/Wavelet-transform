#pragma warning(disable : 4996)

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include<cstdio>
using namespace std;

#define PIXEL_NUM1  3840*2160 
#define PIXEL_NUM2  1920*1080 //고정 픽셀 수
#define PIXEL_NUM3  832*480
#define PIXEL_NUM4  512*512
#define PIXEL_NUM5  256*256
#define M_ICH 3//채널 수

vector<vector<UCHAR>> m_ui8DeepestUniquantized_trans(3, vector<UCHAR>(PIXEL_NUM1));//transformed image(7-7-5-4 균등 양자화 버전)
vector<vector<UCHAR>> m_ui8DeepUniquantized_trans(3, vector<UCHAR>(PIXEL_NUM1));//transformed image(7-7-6-5 균등 양자화 버전)
vector<vector<UCHAR>> m_ui8Uniquantized_trans(3, vector<UCHAR>(PIXEL_NUM1));//transformed image(8bit 균등 양자화 버전)
vector<vector<SHORT>> m_ui16Nonquantized_trans(3, vector<SHORT>(PIXEL_NUM1));//transformed image(비양자화 버전)
vector<vector<UCHAR>> m_ui8Origin(3, vector<UCHAR>(PIXEL_NUM1));//원본 데이터
vector<vector<UCHAR>> m_ui8Comp(3, vector<UCHAR>(PIXEL_NUM1*2));//파일로부터 읽어온 데이터 8-bit버전
vector<vector<UCHAR>> m_ui8Out(3, vector<UCHAR>(PIXEL_NUM1));// reconstructed image
vector<vector<SHORT>> m_ui16Unquantized(3, vector<SHORT>(PIXEL_NUM1));//읽어올 transformed image(비양자화 버전)
vector<vector<UCHAR>> m_ui8DeepestUniquantized(3, vector<UCHAR>(PIXEL_NUM1));//읽어올 transformed image(7-7-5-4 균등 양자화 버전)
vector<vector<UCHAR>> m_ui8DeepUniquantized(3, vector<UCHAR>(PIXEL_NUM1));//읽어올 transformed image(7-7-6-5 균등 양자화 버전)
vector<vector<UCHAR>> m_ui8Uniquantized(3, vector<UCHAR>(PIXEL_NUM1));//읽어올 transformed image(8bit 균등 양자화 버전)



int m1_size[3] = { PIXEL_NUM1  , PIXEL_NUM1 / 4 , PIXEL_NUM1 / 4 };//각 채널별 픽셀 수
int m2_size[3] = { PIXEL_NUM2 , PIXEL_NUM2 / 4 , PIXEL_NUM2 / 4 };
int m3_size[3] = { PIXEL_NUM3 , PIXEL_NUM3 / 4 , PIXEL_NUM3 / 4 };
int m4_size[3] = { PIXEL_NUM4 , PIXEL_NUM4 / 4 , PIXEL_NUM4 / 4 };
int m5_size[3] = { PIXEL_NUM5 , PIXEL_NUM5 / 4 , PIXEL_NUM5 / 4 };


int width1[3] = { 3840, 1920, 1920 }; int height1[3] = { 2160, 1080, 1080 };
int width2[3] = { 1920, 960, 960 };	  int height2[3] = { 1080, 540, 540 };
int width3[3] = { 832, 416, 416 };	  int height3[3] = { 480, 240, 240 };
int width4[3] = { 512, 256, 256 };	  int height4[3] = { 512, 256, 256 };
int width5[3] = { 256, 128, 128 };	  int height5[3] = { 256, 128, 128 };


void readOneFrame(FILE* file, int resolution, int m_ibit, int quant_mode)//하나의 프레임을 읽어오는 함수
{
	int* m_size=NULL;
	int bitfactor = (m_ibit <= 8) ? 1 : 2;		//10-bit면 2, 8-bit면 1

	if (resolution == 1)
		m_size = m1_size;
	else if(resolution==2)
		m_size = m2_size;
	else if (resolution == 3)
		m_size = m3_size;
	else if (resolution == 4)
		m_size = m4_size;
	else if (resolution == 5)
		m_size = m5_size;
	
	for (int ch = 0; ch < M_ICH; ch++)
	{
		fread(&m_ui8Comp[ch][0], sizeof(UCHAR), m_size[ch] * bitfactor, file);// 10-bit면 8-bit씩 각각 두번 읽어야함


		if (m_ibit == 16) {
			for (int i = 0; i < m_size[ch]; i++)
				m_ui16Unquantized[ch][i] = (m_ui8Comp[ch][i * 2] + (m_ui8Comp[ch][i * 2 + 1] << 8));//10-bit의 하나의 데이터로 정제
		}
			
		else if(quant_mode==0){
			m_ui8Origin[ch] = m_ui8Comp[ch];
		}

		else if (quant_mode == 1) {
			m_ui8Uniquantized[ch] = m_ui8Comp[ch];
		}

		else if (quant_mode == 2) {
			m_ui8DeepUniquantized[ch] = m_ui8Comp[ch];
		}

		else if (quant_mode == 3) {
			m_ui8DeepestUniquantized[ch] = m_ui8Comp[ch];
		}
	}

}


/* filtering 작업 수행 및 transform 파일 저장 */
void filtering(int resolution)
{	
	int* width=NULL, * height=NULL, *m_size=NULL;
	vector<vector<SHORT>> m_ui8LL1(3, vector<SHORT>(PIXEL_NUM1 / 4));//LL1
	vector<vector<SHORT>> m_ui8HL1(3, vector<SHORT>(PIXEL_NUM1 / 4));//HL1
	vector<vector<SHORT>> m_ui8LH1(3, vector<SHORT>(PIXEL_NUM1 / 4));//LH1
	vector<vector<SHORT>> m_ui8HH1(3, vector<SHORT>(PIXEL_NUM1 / 4));//HH1
	vector<vector<SHORT>> m_ui8LL2(3, vector<SHORT>(PIXEL_NUM1 / 16));//LL2
	vector<vector<SHORT>> m_ui8HL2(3, vector<SHORT>(PIXEL_NUM1 / 16));//HL2
	vector<vector<SHORT>> m_ui8LH2(3, vector<SHORT>(PIXEL_NUM1 / 16));//LH2
	vector<vector<SHORT>> m_ui8HH2(3, vector<SHORT>(PIXEL_NUM1 / 16));//HH2
	vector<vector<SHORT>> m_ui8LL3(3, vector<SHORT>(PIXEL_NUM1 / 64));//LL3
	vector<vector<SHORT>> m_ui8HL3(3, vector<SHORT>(PIXEL_NUM1 / 64));//HL3
	vector<vector<SHORT>> m_ui8LH3(3, vector<SHORT>(PIXEL_NUM1 / 64));//LH3
	vector<vector<SHORT>> m_ui8HH3(3, vector<SHORT>(PIXEL_NUM1 / 64));//HH3
	
	if (resolution == 1)
	{
		width = width1; height = height1; m_size = m1_size;	
	}
	else if(resolution == 2)
	{
		width = width2; height = height2;   m_size = m2_size;		
	}
	else if (resolution == 3)
	{
		width = width3; height = height3;  m_size = m3_size;
	}
		
	else if (resolution == 4)
	{
		width = width4; height = height4;  m_size = m4_size;		
	}
	else if (resolution == 5)
	{
		width = width5; height = height5;  m_size = m5_size;
	}

	for (int ch = 0; ch < M_ICH; ch++)
	{
		vector<vector<INT>> m_ui8high(3, vector<INT>(m_size[ch]));
		vector<vector<INT>> m_ui8low(3, vector<INT>(m_size[ch]));

		/* LL1, LH1, HL1, HH1 */
		for (int h = 0; h < height[ch]; h++)
		{
			for (int w = 0; w < width[ch]/2; w++)
			{
				m_ui8low[ch][(width[ch]/2) * h + w] = ((SHORT)m_ui8Comp[ch][width[ch] * h + 2 * w] + (SHORT)m_ui8Comp[ch][width[ch] * h + (2 * w) + 1]);// 나누기 2 생략
				m_ui8high[ch][(width[ch]/2) * h + w] = ((SHORT)m_ui8Comp[ch][width[ch] * h + 2 * w] - (SHORT)m_ui8Comp[ch][width[ch] * h + (2 * w) + 1]);
			}
		}

		for (int i = 0; i < (width[ch]/2); i++)
		{
			for (int j = 0; j < (height[ch]/2); j++)
			{
				m_ui8LL1[ch][(width[ch] / 2) * j + i] = (m_ui8low[ch][(width[ch] / 2) * 2 * j + i] + m_ui8low[ch][(width[ch]/2) * (2*j+1) + i]) ;// 나누기 2 생략
				m_ui8LH1[ch][(width[ch] / 2) * j + i] = (m_ui8low[ch][(width[ch] / 2) * 2 * j + i] - m_ui8low[ch][(width[ch] / 2) * (2 * j + 1) + i]);
				m_ui8HL1[ch][(width[ch] / 2) * j + i] = (m_ui8high[ch][(width[ch] / 2) * 2 * j + i] + m_ui8high[ch][(width[ch] / 2) * (2 * j + 1) + i]);
				m_ui8HH1[ch][(width[ch] / 2) * j + i] = (m_ui8high[ch][(width[ch] / 2) * 2 * j + i] - m_ui8high[ch][(width[ch] / 2) * (2 * j + 1) + i]);
			}
		}

		/* LL2 HL2 LH2 HH2 */
		for (int i = 0; i < (height[ch]/2) ; i++)
		{
			for (int j = 0; j < (width[ch] / 4); j++)
			{
				m_ui8low[ch][width[ch] / 4 * i + j] = (m_ui8LL1[ch][width[ch]/2 * i + 2 * j] + m_ui8LL1[ch][width[ch]/2 * i + (2 * j) + 1]); // 나누기 2 생략
				m_ui8high[ch][width[ch] / 4 * i + j] = (m_ui8LL1[ch][width[ch]/2 * i + 2 * j] - m_ui8LL1[ch][width[ch]/2 * i + (2 * j) + 1]); //나누기 2 생략 
			}
		}		

		for (int i = 0; i < (width[ch] / 4); i++)
		{
			for (int j = 0; j < (height[ch] / 4); j++)
			{
				m_ui8LL2[ch][width[ch] / 4 * j + i] = (m_ui8low[ch][width[ch] / 4 * 2 * j + i] + m_ui8low[ch][width[ch] / 4 * (2 * j + 1) + i]);//나누기 2 생략
				m_ui8LH2[ch][width[ch] / 4 * j + i] = (m_ui8low[ch][width[ch] / 4 * 2 * j + i] - m_ui8low[ch][width[ch] / 4 * (2 * j + 1) + i]);
				m_ui8HL2[ch][width[ch] / 4 * j + i] = (m_ui8high[ch][width[ch] / 4 * 2 * j + i] + m_ui8high[ch][width[ch] / 4 * (2 * j + 1) + i]);
				m_ui8HH2[ch][width[ch] / 4 * j + i] = (m_ui8high[ch][width[ch] / 4 * 2 * j + i] - m_ui8high[ch][width[ch] / 4 * (2 * j + 1) + i]);
			}
		}

		/* LL3 HL3 LH3 HH3 */
		for (int i = 0; i < (height[ch] / 4); i++)
		{
			for (int j = 0; j < (width[ch] / 8); j++)
			{
				m_ui8low[ch][width[ch] / 8 * i + j] = (m_ui8LL2[ch][width[ch]/4 * i + 2 * j] + m_ui8LL2[ch][width[ch]/4 * i + (2 * j) + 1]);//나누기 2 생략
				m_ui8high[ch][width[ch] / 8 * i + j] = (m_ui8LL2[ch][width[ch]/4 * i + 2 * j] - m_ui8LL2[ch][width[ch]/4 * i + (2 * j) + 1]);//나누기 2 생략
			}
		}

		for (int i = 0; i < (width[ch] / 8); i++)
		{
			for (int j = 0; j < (height[ch] / 8); j++)
			{
				m_ui8LL3[ch][width[ch] / 8 * j + i] = (m_ui8low[ch][width[ch] / 8 * 2 * j + i] + m_ui8low[ch][width[ch] / 8 * (2 * j + 1) + i]);//나누기 2 생략
				m_ui8LH3[ch][width[ch] / 8 * j + i] = (m_ui8low[ch][width[ch] / 8 * 2 * j + i] - m_ui8low[ch][width[ch] / 8 * (2 * j + 1) + i]);//나누기 2생략
				m_ui8HL3[ch][width[ch] / 8 * j + i] = (m_ui8high[ch][width[ch] / 8 * 2 * j + i] + m_ui8high[ch][width[ch] / 8 * (2 * j + 1) + i]);//나누기 2 생략
				m_ui8HH3[ch][width[ch] / 8 * j + i] = (m_ui8high[ch][width[ch] / 8 * 2 * j + i] - m_ui8high[ch][width[ch] / 8 * (2 * j + 1) + i]);//나누기 2 ㅅ애략
			}
		}		



		for (int ch = 0; ch < M_ICH; ch++)
		{
			for (int i = 0; i < height[ch]; i++)
			{
				for (int j = 0; j < width[ch]; j++)
				{					
					/* LL3 */
					if ((j < width[ch]/8 && j>=0) && (i< height[ch]/8 && i>=0))
					{						
						m_ui16Nonquantized_trans[ch][width[ch] * i + j] = (m_ui8LL3[ch][width[ch] / 8 * i + j]);	
						m_ui8Uniquantized_trans[ch][width[ch] * i + j] = (m_ui8LL3[ch][width[ch] / 8 * i + j]) >> 6;//  /2^6	
						m_ui8DeepUniquantized_trans[ch][width[ch] * i + j] = (m_ui8LL3[ch][width[ch] / 8 * i + j]) >> 7;//  /2^7	
						m_ui8DeepestUniquantized_trans[ch][width[ch] * i + j] = (m_ui8LL3[ch][width[ch] / 8 * i + j]) >> 7;//  /2^6	
					}					

					/* HL3 */
					else if ((j < width[ch]/4 && j>= width[ch]/8) && (i < height[ch] / 8 && i>=0))
					{
						m_ui16Nonquantized_trans[ch][width[ch] * i + j] = (m_ui8HL3[ch][width[ch] / 8 * i + j - (width[ch] / 8)]);
						m_ui8Uniquantized_trans[ch][width[ch] * i + j] = (m_ui8HL3[ch][width[ch] / 8 * i + j - (width[ch] / 8)]) >> 6;//  / 2^6
						m_ui8DeepUniquantized_trans[ch][width[ch] * i + j] = (m_ui8HL3[ch][width[ch] / 8 * i + j - (width[ch] / 8)]) >> 7;//  / 2^7
						m_ui8DeepestUniquantized_trans[ch][width[ch] * i + j] = (m_ui8HL3[ch][width[ch] / 8 * i + j - (width[ch] / 8)]) >> 7;//  / 2^7
					}
					/* LH3 */
					else if ((j < width[ch]/8&&j>=0) && (i < height[ch] / 4 && i>=height[ch] / 8))
					{
						m_ui16Nonquantized_trans[ch][width[ch] * i + j] = (m_ui8LH3[ch][width[ch] / 8 * (i - height[ch]/8) + j]);
						m_ui8Uniquantized_trans[ch][width[ch] * i + j] = (m_ui8LH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j]) >> 6;//  / 2^6
						m_ui8DeepUniquantized_trans[ch][width[ch] * i + j] = (m_ui8LH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j]) >> 7;//  / 2^7
						m_ui8DeepestUniquantized_trans[ch][width[ch] * i + j] = (m_ui8LH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j]) >> 7;//  / 2^7
					}
					/* HH3 */
					else if ((j < width[ch] / 4 && j>=width[ch]/8) && (i < height[ch] / 4 && i>=height[ch] /8))
					{
						m_ui16Nonquantized_trans[ch][width[ch] * i + j] = (m_ui8HH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j - width[ch] / 8]);
						m_ui8Uniquantized_trans[ch][width[ch] * i + j] = (m_ui8HH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j - width[ch] / 8]) >> 6;//  / 2^6
						m_ui8DeepUniquantized_trans[ch][width[ch] * i + j] = (m_ui8HH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j - width[ch] / 8]) >> 7;//  / 2^7
						m_ui8DeepestUniquantized_trans[ch][width[ch] * i + j] = (m_ui8HH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j - width[ch] / 8]) >> 7;//  / 2^7
					}
					/* HL2 */
					else if ((j < width[ch] / 2&& j>=width[ch]/4) && (i < height[ch] / 4 && i>=0))
					{
						m_ui16Nonquantized_trans[ch][width[ch] * i + j] = (m_ui8HL2[ch][width[ch] / 4 * (i) + j - width[ch] / 4]);
						m_ui8Uniquantized_trans[ch][width[ch] * i + j] = (m_ui8HL2[ch][width[ch] / 4 * (i)+j - width[ch] / 4]) >> 4;//  / 2^4
						m_ui8DeepUniquantized_trans[ch][width[ch] * i + j] = (m_ui8HL2[ch][width[ch] / 4 * (i)+j - width[ch] / 4]) >> 6;//  / 2^6
						m_ui8DeepestUniquantized_trans[ch][width[ch] * i + j] = (m_ui8HL2[ch][width[ch] / 4 * (i)+j - width[ch] / 4]) >> 7;//  / 2^7
					}
					/* LH2 */
					else if ((j < width[ch] / 4 && j>=0) && (i < height[ch] / 2 && i>= height[ch]/4))
					{
						m_ui16Nonquantized_trans[ch][width[ch] * i + j] = (m_ui8LH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j]);
						m_ui8Uniquantized_trans[ch][width[ch] * i + j] = (m_ui8LH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j]) >> 4;//  / 2^4
						m_ui8DeepUniquantized_trans[ch][width[ch] * i + j] = (m_ui8LH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j]) >> 6;//  / 2^6
						m_ui8DeepestUniquantized_trans[ch][width[ch] * i + j] = (m_ui8LH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j]) >> 7;//  / 2^7
					}
					/* HH2 */
					else if ((j < width[ch] / 2 && j>= width[ch]/4) && (i < height[ch] / 2 && i>=height[ch]/4))
					{
						m_ui16Nonquantized_trans[ch][width[ch] * i + j] = (m_ui8HH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j - width[ch] / 4]);
						m_ui8Uniquantized_trans[ch][width[ch] * i + j] = (m_ui8HH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j - width[ch] / 4]) >> 4;//  / 2^4
						m_ui8DeepUniquantized_trans[ch][width[ch] * i + j] = (m_ui8HH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j - width[ch] / 4]) >> 6;//  / 2^6
						m_ui8DeepestUniquantized_trans[ch][width[ch] * i + j] = (m_ui8HH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j - width[ch] / 4]) >> 7;//  / 2^7
					}

					/* HL1 */
					else if ((i < height[ch] / 2 && j>= width[ch]/2))
					{
						m_ui16Nonquantized_trans[ch][width[ch] * i + j] = (m_ui8HL1[ch][width[ch] / 2 * (i)+j - width[ch] / 2]);
						m_ui8Uniquantized_trans[ch][width[ch] * i + j] = (m_ui8HL1[ch][width[ch] / 2 * (i)+j - width[ch] / 2]) >> 2;//  / 2^2
						m_ui8DeepUniquantized_trans[ch][width[ch] * i + j] = (m_ui8HL1[ch][width[ch] / 2 * (i)+j - width[ch] / 2]) >> 5;//  / 2^5
						m_ui8DeepestUniquantized_trans[ch][width[ch] * i + j] = (m_ui8HL1[ch][width[ch] / 2 * (i)+j - width[ch] / 2]) >> 6;//  / 2^6
					}
					/* LH1 */
					else if ((j < width[ch] / 2 && i>=height[ch]/2))
					{
						m_ui16Nonquantized_trans[ch][width[ch] * i + j] = (m_ui8LH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j]);
						m_ui8Uniquantized_trans[ch][width[ch] * i + j] = (m_ui8LH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j]) >> 2;//  / 2^2
						m_ui8DeepUniquantized_trans[ch][width[ch] * i + j] = (m_ui8LH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j]) >> 5;//  / 2^5
						m_ui8DeepestUniquantized_trans[ch][width[ch] * i + j] = (m_ui8LH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j]) >> 6;//  / 2^6
					}
					/* HH1 */
					else if(j>=width[ch]/2 && i>=height[ch]/2)
					{						
						m_ui16Nonquantized_trans[ch][width[ch] * i + j] = (m_ui8HH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j - width[ch] / 2]);
						m_ui8Uniquantized_trans[ch][width[ch] * i + j] = (m_ui8HH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j - width[ch] / 2]) >> 2;//  / 2^2
						m_ui8DeepUniquantized_trans[ch][width[ch] * i + j] = (m_ui8HH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j - width[ch] / 2]) >> 5;//  / 2^5
						m_ui8DeepestUniquantized_trans[ch][width[ch] * i + j] = (m_ui8HH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j - width[ch] / 2]) >> 6;//  / 2^6
					}		
				}
			}
		}



	}
}


void inverse_transform(int resolution, int quant_mode)
{
	int* width=NULL, * height=NULL, * m_size=NULL;
	vector<vector<SHORT>> m_ui8LL1(3, vector<SHORT>(PIXEL_NUM1 / 4));//LL1
	vector<vector<SHORT>> m_ui8HL1(3, vector<SHORT>(PIXEL_NUM1 / 4));//HL1
	vector<vector<SHORT>> m_ui8LH1(3, vector<SHORT>(PIXEL_NUM1 / 4));//LH1
	vector<vector<SHORT>> m_ui8HH1(3, vector<SHORT>(PIXEL_NUM1 / 4));//HH1
	vector<vector<SHORT>> m_ui8LL2(3, vector<SHORT>(PIXEL_NUM1 / 16));//LL2
	vector<vector<SHORT>> m_ui8HL2(3, vector<SHORT>(PIXEL_NUM1 / 16));//HL2
	vector<vector<SHORT>> m_ui8LH2(3, vector<SHORT>(PIXEL_NUM1 / 16));//LH2
	vector<vector<SHORT>> m_ui8HH2(3, vector<SHORT>(PIXEL_NUM1 / 16));//HH2
	vector<vector<SHORT>> m_ui8LL3(3, vector<SHORT>(PIXEL_NUM1 / 64));//LL3
	vector<vector<SHORT>> m_ui8HL3(3, vector<SHORT>(PIXEL_NUM1 / 64));//HL3
	vector<vector<SHORT>> m_ui8LH3(3, vector<SHORT>(PIXEL_NUM1 / 64));//LH3
	vector<vector<SHORT>> m_ui8HH3(3, vector<SHORT>(PIXEL_NUM1 / 64));//HH3

	if (resolution == 1)
	{
		width = width1; height = height1; m_size = m1_size;
	}
	else if (resolution == 2)
	{
		width = width2; height = height2;   m_size = m2_size;
	}
	else if (resolution == 3)
	{
		width = width3; height = height3;  m_size = m3_size;
	}

	else if (resolution == 4)
	{
		width = width4; height = height4;  m_size = m4_size;
	}
	else if (resolution == 5)
	{
		width = width5; height = height5;  m_size = m5_size;
	}


	for (int ch = 0; ch < M_ICH; ch++)
	{
		for (int i = 0; i < height[ch]; i++)
		{
			for (int j = 0; j < width[ch]; j++)
			{
				/* LL3 */
				if ((j < width[ch] / 8) && (i < height[ch] / 8))
				{
					if(quant_mode==0)//non uniquantized
						m_ui8LL3[ch][width[ch] / 8 * i + j] = m_ui16Unquantized[ch][width[ch] * i + j];
					else if (quant_mode == 1)//8-bit uniquantized
					{
						m_ui8LL3[ch][width[ch] / 8 * i + j] = m_ui8Uniquantized[ch][width[ch] * i + j];
					}
					else if (quant_mode == 2)//7-7-6-5 uniquantized
					{
						m_ui8LL3[ch][width[ch] / 8 * i + j] = m_ui8DeepUniquantized[ch][width[ch] * i + j] << 1;
					}
					else if (quant_mode == 3)//7-7-5-4 uniquantized
					{
						m_ui8LL3[ch][width[ch] / 8 * i + j] = m_ui8DeepestUniquantized[ch][width[ch] * i + j] << 1;
					}

				}
				/* HL3 */
				else if (j < width[ch] / 4 && i < height[ch] / 8)
				{
					if (quant_mode == 0)//non uniquantized
						m_ui8HL3[ch][width[ch] / 8 * i + j - width[ch] / 8] = m_ui16Unquantized[ch][width[ch] * i + j];
					else if (quant_mode == 1)//8-bit uniquantized
					{
						m_ui8HL3[ch][width[ch] / 8 * i + j - width[ch] / 8] = m_ui8Uniquantized[ch][width[ch] * i + j];
					}
					else if (quant_mode == 2)//7-7-6-5 uniquantized
					{
						m_ui8HL3[ch][width[ch] / 8 * i + j - width[ch] / 8] = m_ui8DeepUniquantized[ch][width[ch] * i + j] << 1;
					}
					else if (quant_mode == 3)//7-7-5-4 uniquantized
					{
						m_ui8HL3[ch][width[ch] / 8 * i + j - width[ch] / 8] = m_ui8DeepestUniquantized[ch][width[ch] * i + j] << 1;
					}
				}
				/* LH3 */
				else if (j < width[ch] / 8 && i < height[ch] / 4)
				{
					if (quant_mode == 0)//non uniquantized
						m_ui8LH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j] = m_ui16Unquantized[ch][width[ch] * i + j];
					else if (quant_mode == 1)//8-bit uniquantized
					{
						m_ui8LH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j] = m_ui8Uniquantized[ch][width[ch] * i + j];
					}
					else if (quant_mode == 2)//7-7-6-5 uniquantized
					{
						m_ui8LH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j] = m_ui8DeepUniquantized[ch][width[ch] * i + j] << 1;
					}
					else if (quant_mode == 3)//7-7-5-4 uniquantized
					{
						m_ui8LH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j] = m_ui8DeepestUniquantized[ch][width[ch] * i + j] << 1;
					}
				}
				/* HH3 */
				else if (j < width[ch] / 4 && i < height[ch] / 4)
				{
					if (quant_mode == 0)//non uniquantized
						m_ui8HH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j - width[ch] / 8] = m_ui16Unquantized[ch][width[ch] * i + j];
					else if (quant_mode == 1)//8-bit uniquantized
					{
						m_ui8HH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j - width[ch] / 8] = m_ui8Uniquantized[ch][width[ch] * i + j];
					}
					else if (quant_mode == 2)//7-7-6-5 uniquantized
					{
						m_ui8HH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j - width[ch] / 8] = m_ui8DeepUniquantized[ch][width[ch] * i + j]<<1;
					}
					else if (quant_mode == 3)//7-7-5-4 uniquantized
					{
						m_ui8HH3[ch][width[ch] / 8 * (i - height[ch] / 8) + j - width[ch] / 8] = m_ui8DeepestUniquantized[ch][width[ch] * i + j]<<1;
					}
				}
				/* HL2 */
				else if (j < width[ch] / 2 && i < height[ch] / 4)
				{
					if (quant_mode == 0)//non uniquantized
						m_ui8HL2[ch][width[ch] / 4 * (i)+j - width[ch] / 4] = m_ui16Unquantized[ch][width[ch] * i + j];
					else if (quant_mode == 1)//8-bit uniquantized
					{
						m_ui8HL2[ch][width[ch] / 4 * (i)+j - width[ch] / 4] = m_ui8Uniquantized[ch][width[ch] * i + j];
					}
					else if (quant_mode == 2)//7-7-6-5 uniquantized
					{
						m_ui8HL2[ch][width[ch] / 4 * (i)+j - width[ch] / 4] = m_ui8DeepUniquantized[ch][width[ch] * i + j] << 2;
					}
					else if (quant_mode == 3)//7-7-5-4 uniquantized
					{
						m_ui8HL2[ch][width[ch] / 4 * (i)+j - width[ch] / 4] = m_ui8DeepestUniquantized[ch][width[ch] * i + j] << 3;
					}
				}	
				/* LH2 */
				else if (j < width[ch] / 4 && i < height[ch] / 2)
				{
					if (quant_mode == 0)//non uniquantized
						m_ui8LH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j] = m_ui16Unquantized[ch][width[ch] * i + j];
					else if (quant_mode == 1)//8-bit uniquantized
					{
						m_ui8LH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j] = m_ui8Uniquantized[ch][width[ch] * i + j];
					}
					else if (quant_mode == 2)//7-7-6-5 uniquantized
					{
						m_ui8LH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j] = m_ui8DeepUniquantized[ch][width[ch] * i + j] << 2;
					}
					else if (quant_mode == 3)//7-7-5-4 uniquantized
					{
						m_ui8LH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j] = m_ui8DeepestUniquantized[ch][width[ch] * i + j] << 3;
					}
				}
				/* HH2 */
				else if (j < width[ch] / 2 && i < height[ch] / 2)
				{
					if (quant_mode == 0)//non uniquantized
						m_ui8HH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j - width[ch] / 4] = m_ui16Unquantized[ch][width[ch] * i + j];
					else if (quant_mode == 1)//8-bit uniquantized
					{
						m_ui8HH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j - width[ch] / 4] = m_ui8Uniquantized[ch][width[ch] * i + j];
					}
					else if (quant_mode == 2)//7-7-6-5 uniquantized
					{
						m_ui8HH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j - width[ch] / 4] = m_ui8DeepUniquantized[ch][width[ch] * i + j] << 2;
					}
					else if (quant_mode == 3)//7-7-5-4 uniquantized
						m_ui8HH2[ch][width[ch] / 4 * (i - height[ch] / 4) + j - width[ch] / 4] = m_ui8DeepestUniquantized[ch][width[ch] * i + j] << 3;
					{
					}
				}

				/* HL1 */
				else if (i < height[ch] / 2)
				{
					if (quant_mode == 0)//non uniquantized
						m_ui8HL1[ch][width[ch] / 2 * (i)+j - width[ch] / 2] = m_ui16Unquantized[ch][width[ch] * i + j];
					else if (quant_mode == 1)//8-bit uniquantized
					{
						m_ui8HL1[ch][width[ch] / 2 * (i)+j - width[ch] / 2] = m_ui8Uniquantized[ch][width[ch] * i + j];
					}
					else if (quant_mode == 2)//7-7-6-5 uniquantized
					{
						m_ui8HL1[ch][width[ch] / 2 * (i)+j - width[ch] / 2] = m_ui8DeepUniquantized[ch][width[ch] * i + j] << 3;
					}
					else if (quant_mode == 3)//7-7-5-4 uniquantized
					{
						m_ui8HL1[ch][width[ch] / 2 * (i)+j - width[ch] / 2] = m_ui8DeepestUniquantized[ch][width[ch] * i + j] << 4;
					}
				}
				/* LH1 */
				else if (j < width[ch] / 2)
				{
					if (quant_mode == 0)//non uniquantized
						m_ui8LH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j] = m_ui16Unquantized[ch][width[ch] * i + j];
					else if (quant_mode == 1)//8-bit uniquantized
					{
						m_ui8LH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j] = m_ui8Uniquantized[ch][width[ch] * i + j];
					}
					else if (quant_mode == 2)//7-7-6-5 uniquantized
					{
						m_ui8LH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j] = m_ui8DeepUniquantized[ch][width[ch] * i + j] << 3;
					}
					else if (quant_mode == 3)//7-7-5-4 uniquantized
					{
						m_ui8LH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j] = m_ui8DeepestUniquantized[ch][width[ch] * i + j] << 4;
					}
				}
				/* HH1 */
				else
				{
					if (quant_mode == 0)//non uniquantized
						m_ui8HH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j - width[ch] / 2] = m_ui16Unquantized[ch][width[ch] * i + j];
					else if (quant_mode == 1)//8-bit uniquantized
					{
						m_ui8HH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j - width[ch] / 2] = m_ui8Uniquantized[ch][width[ch] * i + j];
					}
					else if (quant_mode == 2)//7-7-6-5 uniquantized
					{
						m_ui8HH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j - width[ch] / 2] = m_ui8DeepUniquantized[ch][width[ch] * i + j] << 3;
					}
					else if (quant_mode == 3)//7-7-5-4 uniquantized
					{
						m_ui8HH1[ch][width[ch] / 2 * (i - height[ch] / 2) + j - width[ch] / 2] = m_ui8DeepestUniquantized[ch][width[ch] * i + j] << 4;
					}
				}
			}
		}
	}



	for (int ch = 0; ch < M_ICH; ch++)
	{
		vector<vector<INT>> m_ui8high(3, vector<INT>(m_size[ch]));
		vector<vector<INT>> m_ui8low(3, vector<INT>(m_size[ch]));

		/* reconstruct LL2*/
		for (int i = 0; i < width[ch] / 8; i++)
		{
			for (int j = 0; j < height[ch] / 8; j++)
			{
				m_ui8high[ch][width[ch] / 8 * (2 * j) + i] = m_ui8LL3[ch][width[ch] / 8 * (j)+i] + m_ui8LH3[ch][width[ch] / 8 * (j)+i];
				m_ui8high[ch][width[ch] / 8 * (2 * j + 1) + i] = m_ui8LL3[ch][width[ch] / 8 * j + i] - m_ui8LH3[ch][width[ch] / 8 * j + i];
				m_ui8low[ch][width[ch] / 8 * (2 * j) + i] = m_ui8HL3[ch][width[ch] / 8 * (j)+i] + m_ui8HH3[ch][width[ch] / 8 * (j)+i];
				m_ui8low[ch][width[ch] / 8 * (2 * j + 1) + i] = m_ui8HL3[ch][width[ch] / 8 * j + i] - m_ui8HH3[ch][width[ch] / 8 * j + i];
			}
		}

		for (int i = 0; i < height[ch] / 4; i++)
		{
			for (int j = 0; j < width[ch] / 8; j++)
			{
				if (quant_mode == 0) {
					m_ui8LL2[ch][i * width[ch] / 4 + 2 * j] = (m_ui8high[ch][i * width[ch] / 8 + j] + m_ui8low[ch][i * width[ch] / 8 + j]) >> 2;
					m_ui8LL2[ch][i * width[ch] / 4 + 2 * j + 1] = (m_ui8high[ch][i * width[ch] / 8 + j] - m_ui8low[ch][i * width[ch] / 8 + j]) >> 2;
				}
				else {
					m_ui8LL2[ch][i * width[ch] / 4 + 2 * j] = (m_ui8high[ch][i * width[ch] / 8 + j] + m_ui8low[ch][i * width[ch] / 8 + j]);
					m_ui8LL2[ch][i * width[ch] / 4 + 2 * j + 1] = (m_ui8high[ch][i * width[ch] / 8 + j] - m_ui8low[ch][i * width[ch] / 8 + j]);
				}
			}
		}

		/* reconstruct LL1*/
		for (int i = 0; i < width[ch] / 4; i++)
		{
			for (int j = 0; j < height[ch] / 4; j++)
			{
				m_ui8high[ch][width[ch] / 4 * (2 * j) + i] = m_ui8LL2[ch][width[ch] / 4 * (j)+i] + m_ui8LH2[ch][width[ch] / 4 * (j)+i];
				m_ui8high[ch][width[ch] / 4 * (2 * j + 1) + i] = m_ui8LL2[ch][width[ch] / 4 * j + i] - m_ui8LH2[ch][width[ch] / 4 * j + i];
				m_ui8low[ch][width[ch] / 4 * (2 * j) + i] = m_ui8HL2[ch][width[ch] / 4 * (j)+i] + m_ui8HH2[ch][width[ch] / 4 * (j)+i];
				m_ui8low[ch][width[ch] / 4 * (2 * j + 1) + i] = m_ui8HL2[ch][width[ch] / 4 * j + i] - m_ui8HH2[ch][width[ch] / 4 * j + i];
			}
		}

		for (int i = 0; i < height[ch] / 2; i++)
		{
			for (int j = 0; j < width[ch] / 4; j++)
			{
				if (quant_mode == 0) {
					m_ui8LL1[ch][i * width[ch] / 2 + 2 * j] = (m_ui8high[ch][i * width[ch] / 4 + j] + m_ui8low[ch][i * width[ch] / 4 + j]) >> 2;
					m_ui8LL1[ch][i * width[ch] / 2 + 2 * j + 1] = (m_ui8high[ch][i * width[ch] / 4 + j] - m_ui8low[ch][i * width[ch] / 4 + j]) >> 2;
				}
				else {
					m_ui8LL1[ch][i * width[ch] / 2 + 2 * j] = (m_ui8high[ch][i * width[ch] / 4 + j] + m_ui8low[ch][i * width[ch] / 4 + j]);
					m_ui8LL1[ch][i * width[ch] / 2 + 2 * j + 1] = (m_ui8high[ch][i * width[ch] / 4 + j] - m_ui8low[ch][i * width[ch] / 4 + j]);
				}
			}
		}


		/* reconstruct Outputimage*/
		for (int i = 0; i < width[ch] / 2; i++)
		{
			for (int j = 0; j < height[ch] / 2; j++)
			{
				m_ui8high[ch][width[ch] / 2 * (2 * j) + i] = m_ui8LL1[ch][width[ch] / 2 * (j)+i] + m_ui8LH1[ch][width[ch] / 2 * (j)+i];
				m_ui8high[ch][width[ch] / 2 * (2 * j + 1) + i] = m_ui8LL1[ch][width[ch] / 2 * j + i] - m_ui8LH1[ch][width[ch] / 2 * j + i];
				m_ui8low[ch][width[ch] / 2 * (2 * j) + i] = m_ui8HL1[ch][width[ch] / 2 * (j)+i] + m_ui8HH1[ch][width[ch] / 2 * (j)+i];
				m_ui8low[ch][width[ch] / 2 * (2 * j + 1) + i] = m_ui8HL1[ch][width[ch] / 2 * j + i] - m_ui8HH1[ch][width[ch] / 2 * j + i];
			}
		}

		for (int i = 0; i < height[ch]; i++)
		{
			for (int j = 0; j < width[ch] / 2; j++)
			{
				if (quant_mode == 0) {
					m_ui8Out[ch][i * width[ch] + 2 * j] = (m_ui8high[ch][i * width[ch] / 2 + j] + m_ui8low[ch][i * width[ch] / 2 + j]) >> 2;
					m_ui8Out[ch][i * width[ch] + 2 * j + 1] = (m_ui8high[ch][i * width[ch] / 2 + j] - m_ui8low[ch][i * width[ch] / 2 + j]) >> 2;
				}
				else {
					m_ui8Out[ch][i * width[ch] + 2 * j] = (m_ui8high[ch][i * width[ch] / 2 + j] + m_ui8low[ch][i * width[ch] / 2 + j]);
					m_ui8Out[ch][i * width[ch] + 2 * j + 1] = (m_ui8high[ch][i * width[ch] / 2 + j] - m_ui8low[ch][i * width[ch] / 2 + j]);
				}
			}
		}



	}

}



void PRINT_PSNR(int resolution)//PSNR 출력함수
{
	int MAX = 1023;
    double MSE;
	int* m_size=NULL;
	if (resolution == 1)
	{
		m_size = m1_size;
	}
	else if (resolution == 2)
	{
	   m_size = m2_size;
	}
	else if (resolution == 3)
	{
	   m_size = m3_size;
	}

	else if (resolution == 4)
	{
	   m_size = m4_size;
	}
	else if (resolution == 5)
	{
	   m_size = m5_size;
	}

	for (int ch = 0; ch < M_ICH; ch++)
	{
		MSE = 0;
		for (int i = 0; i < m_size[ch]; i++)//각 채널별로 MSE구하고 PSNR 출력
		{
			MSE += (m_ui8Origin[ch][i] - m_ui8Out[ch][i]) * (m_ui8Origin[ch][i] - m_ui8Out[ch][i]);
		}
		MSE /= m_size[ch];

		if (ch == 0)
			cout << "Y의 PSNR : ";
		else if (ch == 1)
			cout << "Cb의 PSNR : ";
		else
			cout << "Cr의 PSNR : ";

		cout << 10 * log10(MAX * MAX / (double)MSE) << endl;

	}
	cout << endl;
}
int main(void)
{
	FILE* fp_InputImg;
	FILE* fp_outputImg;
	
	/* 이거 각각 결과폴더 생성해주셔야 합니다 ㅠㅠ */
	const char* input_name[5] = { "./input/ParkRunning3_3840x2160_yuv420_8bit_frame0.yuv",
								  "./input/ParkScene_1920x1080_yuv420_8bit_frame200.yuv",								 
								  "./input/PartyScene_832x480_yuv420_8bit_frame0.yuv",
								  "./input/Couple_512x512_yuv400_8bit.raw",
								  "./input/Airplane_256x256_yuv400_8bit.raw"
							      };

	const char* output_name[20] = { "./Output/ParkRunning3_3840x2160_yuv420_16bit_frame0.yuv",
								  "./Output/ParkScene_1920x1080_yuv420_16bit_frame200.yuv",
								  "./Output/PartyScene_832x480_yuv420_16bit_frame0.yuv",
								  "./Output/Couple_512x512_yuv400_16bit.raw",
								  "./Output/Airplane_256x256_yuv400_16bit.raw",
								  "./Output/ParkRunning3_3840x2160_yuv420_8bit_frame0.yuv",
								  "./Output/ParkScene_1920x1080_yuv420_8bit_frame200.yuv",
								  "./Output/PartyScene_832x480_yuv420_8bit_frame0.yuv",
								  "./Output/Couple_512x512_yuv400_8bit.raw",
								  "./Output/Airplane_256x256_yuv400_8bit.raw",
								  "./Output/ParkRunning3_3840x2160_yuv420_7-7-6-5bit_frame0.yuv",
								  "./Output/ParkScene_1920x1080_yuv420_7-7-6-5bit_frame200.yuv",
								  "./Output/PartyScene_832x480_yuv420_7-7-6-5bit_frame0.yuv",
								  "./Output/Couple_512x512_yuv400_7-7-6-5bit.raw",
								  "./Output/Airplane_256x256_yuv400_7-7-6-5bit.raw",
								  "./Output/ParkRunning3_3840x2160_yuv420_7-7-5-4bit_frame0.yuv",
								  "./Output/ParkScene_1920x1080_yuv420_7-7-5-4bit_frame200.yuv",
								  "./Output/PartyScene_832x480_yuv420_7-7-5-4bit_frame0.yuv",
								  "./Output/Couple_512x512_yuv400_7-7-5-4bit.raw",
								  "./Output/Airplane_256x256_yuv400_7-7-5-4bit.raw"
	};
	


	const char* transform_name[20] = { "./Wavelet/ParkRunning3_3840x2160_yuv420_16bit_frame0.yuv",
								  "./Wavelet/ParkScene_1920x1080_yuv420_16bit_frame200.yuv",
								  "./Wavelet/PartyScene_832x480_yuv420_16bit_frame0.yuv",
								  "./Wavelet/Couple_512x512_yuv400_16bit.raw",
								  "./Wavelet/Airplane_256x256_yuv400_16bit.raw",
								  "./Wavelet/ParkRunning3_3840x2160_yuv420_8bit_frame0.yuv",
								  "./Wavelet/ParkScene_1920x1080_yuv420_8bit_frame200.yuv",
								  "./Wavelet/PartyScene_832x480_yuv420_8bit_frame0.yuv",
								  "./Wavelet/Couple_512x512_yuv400_8bit.raw",
								  "./Wavelet/Airplane_256x256_yuv400_8bit.raw",
								  "./Wavelet/ParkRunning3_3840x2160_yuv420_7-7-6-5bit_frame0.yuv",
								  "./Wavelet/ParkScene_1920x1080_yuv420_7-7-6-5bit_frame200.yuv",
								  "./Wavelet/PartyScene_832x480_yuv420_7-7-6-5bit_frame0.yuv",
								  "./Wavelet/Couple_512x512_yuv400_7-7-6-5bit.raw",
								  "./Wavelet/Airplane_256x256_yuv400_7-7-6-5bit.raw",
								  "./Wavelet/ParkRunning3_3840x2160_yuv420_7-7-5-4bit_frame0.yuv",
								  "./Wavelet/ParkScene_1920x1080_yuv420_7-7-5-4bit_frame200.yuv",
								  "./Wavelet/PartyScene_832x480_yuv420_7-7-5-4bit_frame0.yuv",
								  "./Wavelet/Couple_512x512_yuv400_7-7-5-4bit.raw",
								  "./Wavelet/Airplane_256x256_yuv400_7-7-5-4bit.raw"
	};

	for (int i = 0; i < 5; i++) {
		fp_InputImg = fopen(input_name[i], "rb");
		int* m_size=NULL, *width = NULL, *height = NULL;
		int resolution = 0;
		if (!fp_InputImg) {
			printf("Can not open file.");
		}

		resolution = i + 1;

		readOneFrame(fp_InputImg, resolution, 8, 0);//8-bit 이미지 데이터 저장

		if (resolution == 1)
		{
			width = width1; height = height1; m_size = m1_size;
		}
		else if (resolution == 2)
		{
			width = width2; height = height2;   m_size = m2_size;
		}
		else if (resolution == 3)
		{
			width = width3; height = height3;  m_size = m3_size;
		}

		else if (resolution == 4)
		{
			width = width4; height = height4;  m_size = m4_size;
		}
		else if (resolution == 5)
		{
			width = width5; height = height5;  m_size = m5_size;
		}

		fclose(fp_InputImg);

		filtering(resolution);
		
		
		fp_outputImg = fopen(transform_name[i], "wb"); // 비양자화 버전 트랜스폼 결과 저장
		for (int ch = 0; ch < M_ICH; ch++) {
				fwrite(&m_ui16Nonquantized_trans[ch][0], sizeof(SHORT), m_size[ch], fp_outputImg);
			}
		fclose(fp_outputImg);

		fp_outputImg = fopen(transform_name[i + 5], "wb"); // 8-bit 균일 양자화 버전 트랜스폼 결과 저장
		for (int ch = 0; ch < M_ICH; ch++) {
			fwrite(&m_ui8Uniquantized_trans[ch][0], sizeof(UCHAR), m_size[ch], fp_outputImg);
		}
		fclose(fp_outputImg);

		fp_outputImg = fopen(transform_name[i + 10], "wb"); //7-7-6-5bit 균일 양자화 버전 트랜스폼 결과 저장
		for (int ch = 0; ch < M_ICH; ch++) {
			fwrite(&m_ui8DeepUniquantized_trans[ch][0], sizeof(UCHAR), m_size[ch], fp_outputImg);
		}
		fclose(fp_outputImg);

		fp_outputImg = fopen(transform_name[i + 15], "wb"); // /7-7-5-4bit 균일 양자화 버전 트랜스폼 결과 저장
		for (int ch = 0; ch < M_ICH; ch++) {
			fwrite(&m_ui8DeepestUniquantized_trans[ch][0], sizeof(UCHAR), m_size[ch], fp_outputImg);
		}
		fclose(fp_outputImg);

		
		for (int j = 0; j < 4; j++) {
			fp_InputImg = fopen(transform_name[i + 5*j], "rb");

			if(j==0)
				readOneFrame(fp_InputImg, resolution, 16, j);
			else
				readOneFrame(fp_InputImg, resolution, 8, j);

			if (!fp_InputImg) {
				printf("Can not open file.");
			}

			inverse_transform(resolution, j );


			fp_outputImg = fopen(output_name[i + 5*j], "wb"); //결과 저장하기
			for (int ch = 0; ch < M_ICH; ch++) {
				fwrite(&m_ui8Out[ch][0], sizeof(UCHAR), m_size[ch], fp_outputImg);
			}
			fclose(fp_outputImg);

			cout << "<Reconstruction image>" << endl;
			cout << output_name[i + 5*j] << "의 PSNR 출력" << endl;
			PRINT_PSNR(resolution);//PSNR 출력	
		}
		
	}
	
	return 0;
}