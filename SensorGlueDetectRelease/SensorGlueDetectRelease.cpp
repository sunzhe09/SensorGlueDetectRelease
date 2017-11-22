// SensorGlueDetectRelease.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<ipp.h>
#include <stdio.h>
#include"mkl.h"
#include<algorithm>
#include<opencv2\opencv.hpp>
#include<vector>
#include<math.h>

#define THRESH_LOW    40.f /* Low threshold for edges detection */
#define THRESH_HIGHT  80.f /* Upper threshold for edges detection */
#define BORDER_VAL 0
#define PI 3.14159265

typedef struct _point_t
{
	int x;
	int y;
}_point;

using namespace std;
int RegionsSegement(unsigned char *src, int srcStep, IppiSize roiSize, const char *filename);
int MorphologyEroAndDiade(unsigned char* src, int srcStep, int srcWidth, int srcHeight, unsigned char *  dst);
void LigitEnhance(Ipp8u * src, int srcStep, IppiSize roiSize);

int main()
{

	clock_t start, finish;
	double totaltime;
	const char* filename = "C:/Users/bm00133/Desktop/点胶侧面图像/20171121图片/2017-11-21_16_41_45_281.bmp";

	IplImage *pImage = cvLoadImage(filename, 0);
	int Src_StepBytes = 0;
	
	IppiRect ROI =  { 500,1000 ,1600,600 };
	int relative = 15;

	IppiSize roi_size = IppiSize();
	roi_size.height = ROI.height;
	roi_size.width = ROI.width;

	//声明点
	int *data_x = NULL;
	int *data_y = NULL;
	int data_n = 0;

    //拷贝数据到源图像
	Ipp8u *pSrcImage = NULL;
	pSrcImage = ippiMalloc_8u_C1(ROI.width, ROI.height, &Src_StepBytes);
	ippiCopy_8u_C1R((Ipp8u*)pImage->imageData + ROI.y*pImage->widthStep + ROI.x, pImage->widthStep, pSrcImage, Src_StepBytes, roi_size);

	//IplImage *show = cvCreateImageHeader(cvSize(roi_size.width, roi_size.height), 8u, 1);
	//cvSetData(show, (uchar*)pSrcImage, Src_StepBytes);

	start = clock();

	 LigitEnhance(pSrcImage, Src_StepBytes, roi_size);


	int flag= RegionsSegement(pSrcImage, Src_StepBytes, roi_size, filename);

	//IplImage *show = cvCreateImageHeader(cvSize(roi_size.width, roi_size.height), 8u, 1);
	//cvSetData(show, (uchar*)pSrcImage, Src_StepBytes);

	/*******提取边缘*******/
	
	Ipp8u *pBuffer = NULL;

	IppStatus status = ippStsNoErr;
	int srcStep = 0, dstStep = 0; 
	int iTmpBufSize = 0;
	IppiDifferentialKernel filterType = ippFilterScharr;
	IppiMaskSize mask = ippMskSize3x3;
	IppiBorderType bodertype = ippBorderRepl;
	

	status = ippiCannyBorderGetSize(roi_size, filterType, ippMskSize3x3, ipp8u, &iTmpBufSize);
	pBuffer = ippsMalloc_8u(iTmpBufSize);
	status = ippiCannyBorder_8u_C1R(pSrcImage, Src_StepBytes, pSrcImage, Src_StepBytes, roi_size, filterType, ippMskSize3x3, ippBorderRepl, BORDER_VAL, THRESH_LOW, THRESH_HIGHT, ippNormL2, pBuffer);


	//为指针分配内存
	data_x = (int*)malloc(sizeof(int)*roi_size.width*roi_size.height);
	data_y = (int*)malloc(sizeof(int)*roi_size.width*roi_size.height);

	int tempMin= roi_size.width/2, tempMax = 0;
	int lefttop_index = 0,righttop_index = 0;
	for (int i = 0; i<roi_size.height; i++)
	{
		for (int j = 0; j < roi_size.width; j++)
		{
			if (pSrcImage[i*Src_StepBytes + j] != 0)
			{	

				data_x[data_n] = j;
				data_y[data_n] = i;
				data_n++;
				if (max(tempMax, j) > tempMax)
				{
					tempMax = j;
					righttop_index = data_n-1;
				}
				
				if(  min(tempMin, j)<tempMin)
				{
					tempMin = j;
					lefttop_index = data_n-1;
				}
				
			}
		}

	}

	
	int maxdist = 0;
	int row = 0, col = 0, row1 = 0, col1 = 0;

	for (int i = 0; i < data_n; ++i)
	{
		for (int j = i + 1; j < data_n; ++j)
		{
			if (data_y[j]<max(data_y[righttop_index],data_y[lefttop_index]) &&data_y[i]<max(data_y[righttop_index],data_y[lefttop_index])&&abs(data_y[i]-data_y[j])<=20 &&data_x[i] > tempMin + relative&&data_x[j] > tempMin + relative&&data_x[i] < tempMax - relative&&data_x[j] < tempMax - relative)
			{
				int distance = (data_x[i] - data_x[j])*(data_x[i] - data_x[j]) + (data_y[i] - data_y[j])*(data_y[i] - data_y[j]);
				if (distance > maxdist)
				{
					maxdist = distance;
					row = data_y[i];
					col = data_x[i];
					row1 = data_y[j];
					col1 = data_x[j];
				}
			}
		}

	}

	_point  pt1;
	pt1.x = col;
	pt1.y = row;
	_point  pt2;
	pt2.x = col1;
	pt2.y = row1;


	finish = clock();
	totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
	cout << "\n此程序的运行时间为" << totaltime << "秒！" << endl;

	cv::Mat input = cv::imread(filename, 1);
	cv::line(input, cv::Point(pt1.x + ROI.x, pt1.y + ROI.y), cv::Point(pt2.x + ROI.x, pt2.y + ROI.y), cv::Scalar(0, 0, 255));
	cv::line(input, cv::Point(data_x[lefttop_index] + ROI.x, data_y[lefttop_index] + ROI.y), cv::Point(data_x[righttop_index] + ROI.x, data_y[righttop_index] + ROI.y), cv::Scalar(0, 255, 255));


	//ippFree(pSrcImage);
	cvReleaseImage(&pImage);
	ippsFree(pBuffer);
	free(data_x);
	free(data_y);
	//cvReleaseImageHeader(&show);
	

    return status;
}


int RegionsSegement(unsigned char *pSrcMarker, int srcMarkerStep, IppiSize roiSize, const char *filename)
{

	IppStatus status = ippStsNoErr;
	IppiMorphAdvState *pState = NULL;

	int minLabel = 1;
	int maxLabel = 2000;
	int markersNum = 0;
	Ipp8u* pBuffer = NULL;
	int bufferSize = 0;


	/*Ipp8u *pBinImage = NULL;
	int Bin_StepBytes = 0;*/
	Ipp8u threshold = 0;

	//pBinImage = ippiMalloc_8u_C1(roiSize.width, roiSize.height, &Bin_StepBytes);
	ippiComputeThreshold_Otsu_8u_C1R(pSrcMarker, srcMarkerStep, roiSize, &threshold);
	ippiThreshold_LTValGTVal_8u_C1R(pSrcMarker, srcMarkerStep, pSrcMarker, srcMarkerStep, roiSize, threshold, 0, threshold, 255);


	/*IplImage *show = cvCreateImageHeader(cvSize(roiSize.width, roiSize.height), 8u, 1);
	cvSetData(show, (unsigned char*)pBinImage, Bin_StepBytes);*/
	//IppiBorderType
	//对图像进行腐蚀膨胀处理

	MorphologyEroAndDiade(pSrcMarker, srcMarkerStep, roiSize.width, roiSize.height, pSrcMarker);


	/*显示中间结果*/
	//IplImage *show = cvCreateImageHeader(cvSize(roiSize.width, roiSize.height), 8u, 1);
	//cvSetData(show, (unsigned char*)pBinImage, Bin_StepBytes);

	//传输进来的应该是二值化图像

	unsigned short *pMarker = NULL;//16u对应的就是两个字节的unsigned short 
	int markerStep = 0;
	pMarker = ippiMalloc_16u_C1(roiSize.width, roiSize.height, &markerStep);

	ippiConvert_8u16u_C1R(pSrcMarker, srcMarkerStep, pMarker, markerStep, roiSize);

	ippiLabelMarkersGetBufferSize_16u_C1R(roiSize, &bufferSize);

	pBuffer = ippsMalloc_8u(bufferSize);//

	ippiLabelMarkers_16u_C1IR(pMarker, markerStep, roiSize, minLabel, maxLabel, ippiNormL1, &markersNum, pBuffer);

	unsigned int *pixelNum = ippsMalloc_32u(markersNum);
	ippsSet_32s(0, (signed int*)pixelNum, markersNum);


	for (int row = 0; row < roiSize.height; ++row)
	{
		for (int col = 0; col < roiSize.width; ++col)
		{
			int  label = (int)pMarker[row*markerStep / sizeof(unsigned short) + col];

			if (label == 0)//顺便筛选掉row坐标不对的区域
			{

				continue;
			}
			else
			{

				pixelNum[label - 1] += 1;	//连通域的像素面积	

			}

		}
	}




	unsigned char *DeleteIndex = ippsMalloc_8u(markersNum);
	ippsSet_8u(0, DeleteIndex, markersNum);

	for (int i = 0; i < markersNum; i++)
	{
		//面积筛选，去除较大或者较小区域
		if (pixelNum[i] <= 25000)
		{
			DeleteIndex[i] = 1;

		}
	}

	for (int row = 0; row < roiSize.height; row++)
	{
		for (int col = 0; col < roiSize.width; col++)
		{
			int Label = (int)pMarker[col + row*(markerStep / sizeof(unsigned short))];
			if (Label == 0 || 1 == DeleteIndex[Label - 1])
			{
				//删除不合要求区域
				pSrcMarker[col + row*(srcMarkerStep / sizeof(unsigned char))] = 0;
				continue;
			}
			//给满足要求的区域赋值
			pSrcMarker[col + row*(srcMarkerStep / sizeof(unsigned char))] = 255;
		}
	}





	ippsFree(pBuffer);
	ippFree(pixelNum);
	ippFree(pMarker);
	ippFree(pSrcMarker);
	ippsFree(DeleteIndex);

	return  (int)status;



}

void LigitEnhance(Ipp8u * src, int srcStep, IppiSize roiSize)
{

	Ipp8u mulFacorValue = 13;

	ippiMulC_8u_C1RSfs(src, srcStep, mulFacorValue, src, srcStep, roiSize, 0);

	//IplImage *show = cvCreateImageHeader(cvSize(roiSize.width, roiSize.height), 8u, 0);
	//cvSetData(show, (uchar*)src, srcStep);


}

int MorphologyEroAndDiade(unsigned char* src, int srcStep, int srcWidth, int srcHeight, unsigned char*dst)
{
	IppStatus status = ippStsNoErr;

	Ipp8u pMask[3 * 3] = { 1,1,1,1,0,1,1,1,1 };
	//Ipp8u pMask[5 * 5] = { 1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1 };

	IppiSize maskSize = { 3,3 };
	//IppiSize maskSize = { 5,5 };

	int pSpecSize;
	int pBufferSize;

	int dstStep = srcStep;
	IppiSize roiSize = { srcWidth,srcHeight };
	IppiBorderType borderType = ippBorderRepl;
	IppiMorphState *pMorphSpec;
	Ipp8u *pBuffer = NULL;

	Ipp8u *Sub = NULL;
	Sub = ippiMalloc_8u_C1(roiSize.width, roiSize.height, &srcStep);
	memset(Sub, 0, roiSize.width*roiSize.height);

	status = ippiMorphologyBorderGetSize_8u_C1R(roiSize, maskSize, &pSpecSize, &pBufferSize);
	if (status != ippStsNoErr)
	{

		return -1;
	}
	pMorphSpec = (IppiMorphState*)ippsMalloc_8u(pSpecSize);
	pBuffer = (Ipp8u*)ippsMalloc_8u(pBufferSize);

	status = ippiMorphologyBorderInit_8u_C1R(roiSize, pMask, maskSize, pMorphSpec, pBuffer);

	if (status != ippStsNoErr)
	{
		ippsFree(pMorphSpec);
		ippsFree(pBuffer);
		return -1;
	}

	//腐蚀
	status = ippiErodeBorder_8u_C1R(src, srcStep, Sub, srcStep, roiSize, borderType, 0, pMorphSpec, pBuffer);
	//膨胀
	status = ippiDilateBorder_8u_C1R(Sub, srcStep, dst, srcStep, roiSize, borderType, 0, pMorphSpec, pBuffer);

	//IplImage *show = cvCreateImageHeader(cvSize(roiSize.width, roiSize.height), 8u, 1);
	//cvSetData(show, (unsigned char*)dst, srcStep);

	return (int)status;

}

