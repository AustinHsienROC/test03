#include "stdafx.h"
#include "OpencvPlug.h"

#if( ON == OPENCV_EN  )

//******************************************************************************
// M A C R O   D E F I N I T I O N S
//******************************************************************************
OpencvPlug* OpencvPlug::instance = NULL;

//******************************************************************************
// F U N C T I O N   B O D Y S
//******************************************************************************

/**
* @brief simply show the image on GUI
*/
void OpencvPlug::ShowImg(UCHAR *in_grayImg, unsigned int Width , unsigned int Height)
{

	Mat showimage = Mat::zeros(Height,Width, CV_8UC1); //gray_image
	for( int vert = 0; vert < showimage.rows; ++vert )
		for( int hori = 0; hori < showimage.cols; ++hori )
			for( int c = 0; c < 1; c++ )		//Channel series: BGR
				showimage.at<uchar>(vert,hori) = in_grayImg[vert*Width + hori]; //Vec3b

	imshow( "ShowImg window", showimage ); // Show our image inside it.
	cvWaitKey(1); // Wait for a keystroke in the window
}

void OpencvPlug::SaveImg(UCHAR *in_grayImg, unsigned int Width , unsigned int Height ,
	UINT32 startxi , UINT32 startyi , UINT32 ROIwidth, UINT32 ROIheight ,const CHAR *FileName )
{
	///1) in_grayImg -> showimage
	Mat showimage = Mat::zeros(Height,Width, CV_8UC1); //gray_image
	for( int vert = 0; vert < showimage.rows; ++vert )
		for( int hori = 0; hori < showimage.cols; ++hori )
			for( int c = 0; c < 1; c++ )		//Channel series: BGR
				showimage.at<uchar>(vert,hori) = in_grayImg[vert*Width + hori]; //Vec3b

	///2) showimage -> croppedImage
	cv::Rect myROI(startxi, startyi, ROIwidth, ROIheight);
	cv::Mat croppedImage = showimage(myROI);
	//imshow( "Crop window", croppedImage ); // Show our image inside it.
	//cvWaitKey(1);
	

	///3) Save it
	std::wstring FilePath = OFFLINE_TRAIN_FOLDER;

	unsigned len = FilePath.size() * 4 ;
	setlocale(LC_CTYPE, "" );
	char *p = new  char [len];
	wcstombs(p,FilePath.c_str(),len);
	string tmp04(p); //"C:\\CTA_OFFLINE_TRAIN_DATA\\"
	delete[] p;

	tmp04 += FileName;
	imwrite( tmp04.c_str() ,croppedImage);

	//imwrite( "C:\\CTA_OFFLINE_TRAIN_DATA\\deb01.jpg" ,croppedImage);
}
/**
* @brief Update the Img and push the current Img to Previous one
*/
void OpencvPlug::UpdateCurGrayImg(UCHAR *in_grayImg, unsigned int Width , unsigned int Height)
{
	grayimagePrev = grayimage.clone();

	grayimage = Mat::zeros(Height,Width, CV_8UC1); //gray_image
	for( int vert = 0; vert < grayimage.rows; ++vert )
		for( int hori = 0; hori < grayimage.cols; ++hori )
			for( int c = 0; c < 1; c++ )		//Channel series: BGR
				grayimage.at<uchar>(vert,hori) = in_grayImg[vert*Width + hori]; //Vec3b

	//imshow( "Display Src window", grayimage ); // Show our image inside it.
	//cvWaitKey(1); // Wait for a keystroke in the window
	Frameidx01++;
}

/**
* @brief Test cvGoodFeaturesToTrack to find the corner
* \n		Need grayimage as Input
*/
void OpencvPlug::GoodFeatureToTrack()
{
	if(Frameidx01<=122){return;}
	IplImage iplimage = grayimage;
	IplImage iplimagePrev = grayimagePrev;

	IplImage* img_temp = cvCreateImage(cvGetSize(&iplimage), 32, 1);  
	IplImage* img_eigen = cvCreateImage(cvGetSize(&iplimage), 32, 1);  

	// Create the array to store the points detected( <= 1000 )  
	int  count = 1000;  
	CvPoint2D32f* corners =  new  CvPoint2D32f[count];  

	// Find corners  
	//cvGoodFeaturesToTrack(&iplimage, img_eigen, img_temp, corners, &count, 0.1, 10);  
	cvGoodFeaturesToTrack(
		&iplimage,
		img_eigen,
		img_temp,
		corners,
		&count,
		0.1,
		5.0,
		0,
		3,
		0,
		0.04
		);

	/// LK Optical Flow
	const int MAX_CORNERS = 500;
	IplImage* imgA        = &iplimagePrev;
	IplImage* imgB        = &iplimage;
	int win_size          = 10;
	
	 //Sub Pixel
	/*
	 cvFindCornerSubPix(
		imgA,corners,
		count,
		cvSize(win_size,win_size),
		cvSize(-1,-1),
		cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03)
		);
	*/
	char features_found[MAX_CORNERS];
	float feature_errors[MAX_CORNERS];
	CvSize pyr_sz=cvSize(imgA->width+8,imgB->height/3);
	IplImage* pyrA=cvCreateImage(pyr_sz,IPL_DEPTH_32F,1);
	IplImage* pyrB=cvCreateImage(pyr_sz,IPL_DEPTH_32F,1);
	CvPoint2D32f* cornersB=new CvPoint2D32f[MAX_CORNERS];
	cvCalcOpticalFlowPyrLK(
		imgA,
		imgB,
		pyrA,
		pyrB,
		corners,
		cornersB,
		count,
		cvSize(win_size,win_size),
		5,
		features_found,
		feature_errors,
		cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.3),
		0
		);

	// Mark these corners on the original image  
	Mat tmpImg(&iplimage);
	Mat tmpImg2;
	cvtColor(tmpImg, tmpImg2, CV_GRAY2RGB);
	for ( int  i = 0; i<count ;i++)  
	{  
		int  thickness  =  3 ;
		int  lineType  =  8 ; 

		/*
		line (  tmpImg2 , Point(corners[i].x, corners[i].y) , Point(corners[i].x,corners[i].y) , 
			Scalar (  0 ,  255 ,  255  ), 
			thickness , 
			lineType  ); 
		*/
// Draw Optical Flow
		CvPoint p0=cvPoint(cvRound(corners[i].x) ,cvRound(corners[i].y));
		CvPoint p1=cvPoint(cvRound(cornersB[i].x),cvRound(cornersB[i].y));

		if( cvRound(corners[i].x) ==  cvRound(cornersB[i].x) )
		{
			if( cvRound(corners[i].y)==  cvRound(cornersB[i].y) )
				continue;
		}
		

		line (  tmpImg2 , p0, p1 , 
			Scalar (  0 ,  255 ,  255  ), 
			2 , 
			lineType  ); 
	}  

	imshow("cvGoodFeaturesToTrack", tmpImg2);  
	cvWaitKey(1);

	
	cvReleaseImage(&img_temp);
	cvReleaseImage(&img_eigen);
	delete [] corners;
	cvReleaseImage(&pyrA);
	cvReleaseImage(&pyrB);
	
}
void OpencvPlug::FrameDiff()
{
	if(Frameidx01<=2){return;}

	////////////////////////////Sobel Img//////////////////////////////////////////////

	Mat  grad , gradPrev ;
	{
		int  scale  =  1 ; 
		int  delta  =  0 ; 
		int  ddepth  =  CV_16S ;

		///創建grad_x和grad_y矩陣
		Mat  grad_x ,  grad_y ; 
		Mat  abs_grad_x ,  abs_grad_y ;

		///求X方向梯度 
		Sobel (  grayimage ,  grad_x ,  ddepth ,  1 ,  0 ,  3 ,  scale ,  delta ,  BORDER_DEFAULT  ); 
		convertScaleAbs (  grad_x ,  abs_grad_x  );

		///求Y方向梯度
		Sobel (  grayimage ,  grad_y ,  ddepth ,  0 ,  1 ,  3 ,  scale ,  delta ,  BORDER_DEFAULT  ); 
		convertScaleAbs (  grad_y ,  abs_grad_y  );

		///合併梯度(近似) 
		addWeighted (  abs_grad_x ,  0.5 ,  abs_grad_y ,  0.5 ,  0 ,  grad  );
	}
	{
		int  scale  =  1 ; 
		int  delta  =  0 ; 
		int  ddepth  =  CV_16S ;

		///創建grad_x和grad_y矩陣
		Mat  grad_x ,  grad_y ; 
		Mat  abs_grad_x ,  abs_grad_y ;

		///求X方向梯度 
		Sobel (  grayimagePrev ,  grad_x ,  ddepth ,  1 ,  0 ,  3 ,  scale ,  delta ,  BORDER_DEFAULT  ); 
		convertScaleAbs (  grad_x ,  abs_grad_x  );

		///求Y方向梯度
		Sobel (  grayimagePrev ,  grad_y ,  ddepth ,  0 ,  1 ,  3 ,  scale ,  delta ,  BORDER_DEFAULT  ); 
		convertScaleAbs (  grad_y ,  abs_grad_y  );

		///合併梯度(近似) 
		addWeighted (  abs_grad_x ,  0.5 ,  abs_grad_y ,  0.5 ,  0 ,  gradPrev  );
	}

	////////////////////////////Frame Diff//////////////////////////////////////////////
	Mat MatDiff;
	// Sobel Frame Diff
	absdiff( grad , gradPrev , MatDiff);

	// Frame Diff
	//absdiff( grayimage , grayimagePrev , MatDiff);

	///////////////////////////////////Threshold///////////////////////////////////////
  /* 0:二進制閾值    
	 1:反二進制閾值
	 2:截斷閾值
	 3: 0閾值
	 4:反0閾值
	 */
	int  threshold_value  =  30 ; 
	int  threshold_type  =  0 ; 
	int  const  max_value  =  255 ; 
	int  const  max_BINARY_value  =  255 ;
	Mat MatThd;
	threshold (  MatDiff ,  MatThd ,  threshold_value ,  max_BINARY_value , threshold_type  );

	imshow("FrameDiff1", MatThd);  
	cvWaitKey(1);
}

OpencvPlug::OpencvPlug(void)
{
	Frameidx01 = 0;
}
OpencvPlug::~OpencvPlug(void)
{
}
/**
* @brief Return Singleton
*/
OpencvPlug *OpencvPlug::getInstance()
{
	if(!instance)
		instance = new OpencvPlug;
	
	return instance;
}

void OpencvPlug::ResetInstance()
{
    delete instance; 
    instance = NULL;
}

#endif
