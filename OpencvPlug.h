//1
/*
===================================================================
Name		: OpencvPlug.h
Author		: Austin
Version		: v0.0010
Copyright	: Copyright (C) 2014 By oToBrite Electronics Corp.
Description	: OpenCV PlugIn functions

==================================================================
History
--------------+-------------+-----------------------------------------+------------
DATE		  |VERSION      |DESCRIPTION  							  | Author
--------------+-------------+-----------------------------------------+------------
2014/07/29	  | v0.0010  	|First setup functions        	    	  | Austin
--------------+-------------+-----------------------------------------+------------
*/

//---------------------------------------------------------------------------

#ifndef _OPENCV_PLUGIN_H_
#define _OPENCV_PLUGIN_H_


//******************************************************************************
// I N C L U D E   F I L E S
//******************************************************************************
#include "CTA_Def.h"
#include <string>
//******************************************************************************
// M A C R O   D E F I N I T I O N S
//******************************************************************************

#define OPENCV_EN	OFF
    #define  SHOW_BV     OFF

#if( ON == OPENCV_EN ) 

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <imgproc/imgproc.hpp>

#include "cvaux.h" //cvGoodFeaturesToTrack

using namespace cv;

//******************************************************************************
// M A C R O   D E F I N I T I O N S
//******************************************************************************

#define OPENCV OpencvPlug::getInstance()
//******************************************************************************
// F U N C T I O N   B O D Y S
//******************************************************************************
class OpencvPlug
{
public:
	OpencvPlug(void);
	~OpencvPlug(void);
	static OpencvPlug *getInstance();
    static void ResetInstance();

	void ShowImg(UCHAR *in_grayImg, unsigned int Width , unsigned int Height);
	void SaveImg(UCHAR *in_grayImg, unsigned int Width , unsigned int Height ,
		UINT32 startxi , UINT32 startyi , UINT32 ROIwidth, UINT32 ROIheight ,const CHAR *FileName );
	
	void UpdateCurGrayImg(UCHAR *in_grayImg, unsigned int Width , unsigned int Height);

	void GoodFeatureToTrack();

	void FrameDiff();


private:
	Mat grayimage;
	Mat grayimagePrev;
	static OpencvPlug *instance;
	int Frameidx01;
};

#endif

#endif