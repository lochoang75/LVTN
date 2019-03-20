/***************************************************************************
Copyright (c) 2018, Xilinx, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors 
may be used to endorse or promote products derived from this software 
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/



#include "xf_headers.h"
#include "xf_erosion_config.h"

int main(int argc, char** argv)
{

	if(argc != 2)
	{
		fprintf(stderr,"Invalid Number of Arguments!\nUsage:\n");
		fprintf(stderr,"<Executable Name> <input image path> \n");
		return -1;
	}

	cv::Mat in_img,in_img1,out_img,ocv_ref;
	cv::Mat in_gray,in_gray1,diff;
#if GRAY
	// reading in the color image
	in_gray = cv::imread(argv[1], 0);
#endif


	if (in_gray.data == NULL)
	{
		fprintf(stderr,"Cannot open image at %s\n", argv[1]);
		return 0;
	}


#if GRAY
		// create memory for output images
		ocv_ref.create(in_gray.rows,in_gray.cols,CV_8UC1);
		out_img.create(in_gray.rows,in_gray.cols,CV_8UC1);
		diff.create(in_gray.rows,in_gray.cols,CV_8UC1);
#endif




	/////////////////////	End of OpenCV reference	 ////////////////
		cv::Mat element = cv::getStructuringElement( KERNEL_SHAPE,cv::Size(FILTER_SIZE, FILTER_SIZE), cv::Point(-1, -1));
#if __SDSCC__
		unsigned long long startC = 0;
		startC = sds_clock_counter();
#endif
		cv::erode(in_gray, ocv_ref, element, cv::Point(-1, -1), ITERATIONS, cv::BORDER_CONSTANT);
#if __SDSCC__
		 unsigned long long endC = 0;
		 endC = sds_clock_counter();
		 printf("The execution CPU clock on SW is : %d\n", endC - startC);
#endif
		cv::imwrite("out_ocv.jpg", ocv_ref);
	////////////////////	HLS TOP function call	/////////////////

	static xf::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgInput(in_gray.rows,in_gray.cols);
	static xf::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgOutput(in_gray.rows,in_gray.cols);


#if __SDSCC__
	unsigned char *structure_element=(unsigned char *)sds_alloc_non_cacheable(sizeof(unsigned char)*FILTER_SIZE*FILTER_SIZE);
#else
	unsigned char structure_element[FILTER_SIZE*FILTER_SIZE];
#endif


	for(int i=0;i<(FILTER_SIZE*FILTER_SIZE);i++)
	{
		structure_element[i]=element.data[i];
	}


	imgInput.copyTo(in_gray.data);

#if __SDSCC__
	startC = sds_clock_counter();
#endif

	erosion_accel(imgInput, imgOutput, structure_element);

#if __SDSCC__
 endC = sds_clock_counter();
 printf("The execution CPU clock on HW is : %d\n", endC - startC);
#endif


	// Write output image
	xf::imwrite("hls_out.jpg",imgOutput);


	//////////////////  Compute Absolute Difference ////////////////////

	xf::absDiff(ocv_ref, imgOutput, diff);
	//absdiff(ocv_ref, out_img, diff);
	cv::imwrite("out_error.jpg", diff);
//	FILE *fpp=fopen("cv_out.txt","w");
//	FILE *fpq=fopen("hls_out.txt","w");
//	FILE *fp2=fopen("fix_error.txt","w");
//		unsigned long int p=0,q=0;
//			for(int c=0;c<(2073600);c++)
//			{
//					fprintf(fpq,"%d\n",(unsigned char)imgOutput.data[p]);
//					p+=1;
//			}
//			FILE *f1=fopen("diff.txt","w");
//			for(int c=0;c<(2073600);c++)
//			{
//					short int diffpix=(unsigned char)imgOutput.data[q]-(unsigned char)ocv_ref.data[q];
//					fprintf(fpp,"%d\n",(unsigned char)ocv_ref.data[q]);
//					fprintf(f1,"%d\n",diffpix);
//
////					if((diffpix < -1)||(diffpix > 1))
////					{
////						fprintf(fp2,"%d (dut:%d, cv:%d),%d,(r-%d g-%d b-%d)\n",(short int)diffpix,(unsigned char)outputimg.data[q],(unsigned char)ocv_outputimg.data[q],c,(unsigned char)imgInput.data[c].range(7,0),(unsigned char)imgInput.data[c].range(15,8),(unsigned char)imgInput.data[c].range(23,16));
////					}
//					q+=1;
//
//			}
//
//	fclose(f1);
//
//	fclose(fpp);
//	fclose(fpq);
//	fclose(fp2);

	// Find minimum and maximum differences.
	double minval=256,maxval=0;
	int cnt = 0;
	for (int i=0; i<in_gray.rows; i++)
	{
		for(int j=0; j<in_gray.cols; j++)
		{
			uchar v = diff.at<uchar>(i,j);
			if (v>0)
				cnt++;
			if (minval > v)
				minval = v;
			if (maxval < v)
				maxval = v;
		}
	}
	float err_per = 100.0*(float)cnt/(in_gray.rows*in_gray.cols);
	fprintf(stderr,"Minimum error in intensity = %f\n Maximum error in intensity = %f\n Percentage of pixels above error threshold = %f\n",minval,maxval,err_per);

	if(err_per > 0.0f)
		return 1;


	return 0;
}
