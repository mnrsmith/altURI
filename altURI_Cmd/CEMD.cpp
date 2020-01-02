/*
Copyright (c) 2012, Mark N R Smith, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer. Redistributions in binary form must
reproduce the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided with the
distribution. Neither the name of the author nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//=========================================================================
//	CEMD.h - Elementary Motion Detection in OpenCV
//=========================================================================

#include "stdafx.h"
#include "CEMD.h"
#include "XTrace.h"

// Tweaking values, see paper Iida & Lambrinos
const double CEMD :: A = 0.3;
const double CEMD :: B = 0.6;

const double CEMD :: RED = 0.2989;
const double CEMD :: GREEN = 0.5870;
const double CEMD :: BLUE = 0.1140;

//=========================================================================
IplImage* CEMD :: MakeEMD( const IplImage* img )
{
	IplImage* tmp_img = cvCreateImage( cvSize( img->width, img->height ), img->depth, img->nChannels );
				
	// Convert to HLS
	cvCvtColor(img, tmp_img, CV_RGB2HLS );

	if ( !CV_IS_IMAGE(m_ImagePrevData) )
	{
		m_ImagePrevData = cvCreateImage( cvSize( tmp_img->width, tmp_img->height ), tmp_img->depth, tmp_img->nChannels );
		cvZero(m_ImagePrevData);
	}
		
	int* highpass = new int[tmp_img->width];
	double* lowpass = new double[tmp_img->width];
	double* emd = new double[tmp_img->width];

	//int count = 0, sum = 0;

	for( int y = 0; y < tmp_img->height; y+=2 )
	{
		unsigned char* rowImage = &CV_IMAGE_ELEM( tmp_img, unsigned char, y, 0 );
		unsigned char* rowImagePlus = &CV_IMAGE_ELEM( tmp_img, unsigned char, y + 1, 0 );

		unsigned char* rowSaved = &CV_IMAGE_ELEM( m_ImagePrevData, unsigned char, y, 0 );
		unsigned char* rowSavedPlus = &CV_IMAGE_ELEM( m_ImagePrevData, unsigned char, y + 1, 0 );

		for( int x = 0; x < tmp_img->width*tmp_img->nChannels; x += tmp_img->nChannels )
		{
			// row[x] = H, row[x+1] = L, row[x+2] = S

			// 1. First High pass filter for each pixel is calculated
			highpass[y]			= rowImage[x + 1] - rowSaved[x + 1];
			highpass[y+1]		= rowImagePlus[x + 1] - rowSavedPlus[x + 1];

			// 2. Low pass filter is calculated for each pixel based on the highpass filter of prev_data (stored in hue-layer)
			lowpass[y]			= highpass[y] + m_CurrentOneMinusA * rowSaved[x];
			lowpass[y+1]		= highpass[y+1] + m_CurrentOneMinusA * rowSavedPlus[x];

			// 3. These are then multiplied
			emd[y]				= highpass[y+1] * lowpass[y];
			emd[y+1]			= highpass[y] * lowpass[y+1];

			// 4. Store the new high-pass-filter-answers in the prev_img hue layer and the current luminance layer to prev_img luminance layer
			rowSaved[x]			= highpass[y];
			rowSavedPlus[x]		= highpass[y + 1];

			rowSaved[x + 1]		= rowImage[x + 1];
			rowSavedPlus[x + 1]	= rowImagePlus[x + 1];

			// 5. Calculate EMD with a smoothing from previous EMD and store this in the saturation layer of the prev_img
			rowSaved[x + 2]		= m_CurrentOneMinusB * rowSaved[x + 2] + m_CurrentB * (emd[y] - emd[y+1]);
			rowSavedPlus[x + 2]	= m_CurrentOneMinusB * rowSavedPlus[x + 2] + m_CurrentB * (emd[y+1] - emd[y]);

			// 6. The final answer is stored in the luminance layer of the current image for display purposes
			// color = red for negative, else blue
			if ( rowSaved[x + 2] < 0 )
				rowImage[x] = 0;
			else
				rowImage[x] = 120;

			if ( rowSavedPlus[x + 2] < 0 )
				rowImagePlus[x] = 0;
			else
				rowImagePlus[x] = 120;

			rowImage[x+1] = abs(rowSaved[x + 2]*.2);
			rowImagePlus[x+1] =  abs(rowSavedPlus[x + 2]*.2);

			if (rowImage[x+1] > 100 ) rowImage[x+1] = 100;
			if (rowImagePlus[x+1] > 100 ) rowImagePlus[x+1] = 100;

			rowImage[x+2] = 100;
			rowImagePlus[x+2] = 100;

			//count++;
			//sum += abs(m_prev_data[i * tmp_img->widthStep + j*3 + 2]);
		}
	}

	cvCvtColor(tmp_img, tmp_img, CV_HLS2BGR );

	delete(emd);
	delete(lowpass);
	delete(highpass);

	//double avg = (double) sum / (double) count;

	return tmp_img;
}

//=========================================================================
IplImage* CEMD :: MakeEMD2( const IplImage* iplIn )
{
	const int OFFSET = 1;

	m_matImage = iplIn;

	if ( !m_matImage.empty() )
	{
		if ( m_matImage.rows != m_matGrayImagePrev.rows || m_matImage.cols != m_matGrayImagePrev.cols )
		{
			// ranges for EMD operations in each axis
			m_rcolShiftX = cv::Range( OFFSET, m_matImage.cols );
			m_rcolNormalX = cv::Range( 0, m_matImage.cols - OFFSET );

			m_rrowShiftY = cv::Range( OFFSET, m_matImage.rows );
			m_rrowNormalY = cv::Range( 0, m_matImage.rows - OFFSET );

			m_rAll = cv::Range::all();

			// setup grey image - same datatype as incoming but only one channel
			m_matGrayInitImage  = cv::Mat( m_matImage.rows , m_matImage.cols, CV_MAKETYPE( m_matImage.depth(), 1) );

			// setup double grey image
			m_matGrayImage  = cv::Mat( m_matImage.rows, m_matImage.cols, CV_32FC1 );

			// setup final image - same as incoming image
			m_matEMDRGB = cv::Mat( m_matImage.rows, m_matImage.cols, CV_32FC3 );

			m_matEMDRGB_ORG = cv::Mat( m_matImage.rows , m_matImage.cols, m_matImage.type() );

			// set up previous values for first call
			m_matGrayImagePrev = cv::Mat( m_matImage.rows , m_matImage.cols, CV_32FC1, cv::Scalar(0) );

			m_matHPFPrev = cv::Mat( m_matImage.rows , m_matImage.cols, CV_32FC1, cv::Scalar(0) );

			// slightly smaller result
			m_matEMDPrev = cv::Mat( m_matImage.rows - OFFSET, m_matImage.cols - OFFSET, CV_32FC1, cv::Scalar(0) );
		}
		
		// Convert to black and white image		
		cvtColor( m_matImage, m_matGrayInitImage, CV_RGB2GRAY, 1 );

		equalizeHist( m_matGrayInitImage, m_matGrayInitImage );
		//equalizeHist( m_matGrayImage, m_matGrayImage );

		// Convert to float
		m_matGrayInitImage.convertTo( m_matGrayImage, CV_32FC1, 1./255 );

		// make high pass filter: HPF = NewImage - PrevImage
		m_matHPF = m_matGrayImage - m_matGrayImagePrev;

		// save image for next time
		m_matGrayImage.copyTo( m_matGrayImagePrev );
		
		// make low pass filter: LPF	= (a x HPF) + ( (1 - a) x HPFPrev )
		m_matLPF = m_matHPF * m_CurrentA + m_matHPFPrev * m_CurrentOneMinusA;
		//addWeighted( m_matHPF, a, m_matHPFPrev, oneminusa, 0, m_matLPF );

		// save HPF for next time
		m_matHPF.copyTo( m_matHPFPrev );

		// make EMD in X: ( HPFNew x LPF_shift ) + ( HPFNew_shift x LPF ) 
		m_matEMDX = m_matHPF( m_rAll, m_rcolNormalX ).mul( m_matLPF( m_rAll, m_rcolShiftX ) ) + 
					m_matHPF( m_rAll, m_rcolShiftX ).mul( m_matLPF( m_rAll, m_rcolNormalX ) );

		// make EMD in Y
		m_matEMDY = m_matHPF( m_rrowNormalY, m_rAll ).mul( m_matLPF( m_rrowShiftY, m_rAll ) ) + 
					m_matHPF( m_rrowShiftY, m_rAll ).mul( m_matLPF( m_rrowNormalY, m_rAll ) );

		// pythagoras
		m_matEMDX = m_matEMDX.mul( m_matEMDX );
		m_matEMDY = m_matEMDY.mul( m_matEMDY );

		cv::sqrt( m_matEMDX( m_rrowNormalY, m_rcolNormalX ) + m_matEMDY( m_rrowNormalY, m_rcolNormalX ) , m_matEMD );

		// make full emd: EMD	= ( ( 1 - b ) x EMDPrev ) + ( b x EMD )
		m_matEMD = m_matEMDPrev * m_CurrentOneMinusB + m_matEMD * m_CurrentB;

		// save EMD for next time
		m_matEMD.copyTo( m_matEMDPrev );

		// convert mat EMD to colour ipl image
		cvtColor( m_matEMD, m_matEMDRGB, CV_GRAY2RGB, 3 );

		// convert back to original type
		m_matEMDRGB.convertTo( m_matEMDRGB_ORG, m_matImage.type(), 255 );

		m_ImageEMD = m_matEMDRGB_ORG;

		return &m_ImageEMD;
	}

	return NULL;
}

#define ELEM(type,start,step,size,xpos,ypos) *((type*)(start+step*(ypos)+(xpos)*size))

#define ELEM_CH(type,start,step,size,xpos,ypos,ichannel) \
            *((type*)(start+step*(ypos)+(xpos)*size+ichannel))

#define S_ELEMENT( type, start, offset ) *((type*)(start+offset))

//=========================================================================
inline void CEMD :: CalculateAndStoreHPFnLPF( size_t stSourceAddress, size_t stDestAddress )
{
	// calculate gray pixel  BGR not RGB.
	//uchar uPixel = cv::saturate_cast<uchar>(	S_ELEMENT( uchar, m_matImage.data, iAddressOrg ) * BLUE +
	//											S_ELEMENT( uchar, m_matImage.data, iAddressOrg + 1 ) * GREEN +
	//											S_ELEMENT( uchar, m_matImage.data, iAddressOrg + 2 ) * RED );

	uchar uPixel = S_ELEMENT( uchar, m_matImage.data, stSourceAddress ) * BLUE + S_ELEMENT( uchar, m_matImage.data, stSourceAddress + 1 ) * GREEN + S_ELEMENT( uchar, m_matImage.data, stSourceAddress + 2 ) * RED;

	// make high pass filter: HPF = NewImage - PrevImage
	S_ELEMENT( uchar, m_matHPF.data, stDestAddress ) = uPixel - S_ELEMENT( uchar, m_matGrayImagePrev.data, stDestAddress );
				
	//  save for next time
	S_ELEMENT( uchar, m_matGrayImagePrev.data, stDestAddress ) = uPixel;

	// make low pass filter: LPF	= (a x HPF) + ( (1 - a) x HPFPrev )
	//S_ELEMENT( uchar, m_matLPF.data, iAddress ) = cv::saturate_cast<uchar>( 
	//											a * S_ELEMENT( uchar, m_matHPF.data, iAddress ) + 
	//											oneminusa * S_ELEMENT( uchar, m_matHPFPrev.data, iAddress ) );

	S_ELEMENT( uchar, m_matLPF.data, stDestAddress ) = m_CurrentA * S_ELEMENT( uchar, m_matHPF.data, stDestAddress ) + m_CurrentOneMinusA * S_ELEMENT( uchar, m_matHPFPrev.data, stDestAddress );

	//  save for next time
	S_ELEMENT( uchar, m_matHPFPrev.data, stDestAddress ) = S_ELEMENT( uchar, m_matHPF.data, stDestAddress );
}

//=========================================================================
inline void CEMD :: AllocateImageMats( const int rows, const int cols, const int orgcode, const int newcode )
{
	// previous gray image
	m_matGrayImagePrev = cv::Mat::zeros( rows , cols, orgcode );

	// High Pass Filter storage
	m_matHPF = cv::Mat::zeros( rows, cols, newcode );

	// Low Pass Filter storage
	m_matLPF = cv::Mat::zeros( rows, cols, newcode );

	// Previous High Pass Filter storage
	m_matHPFPrev = cv::Mat::zeros( rows, cols, newcode );

	// Result - magnitude
	m_matEMD = cv::Mat::zeros( rows - OFFSET, cols, newcode );

	// Result - magnitude and theta using colour
	m_matEMDColour = cv::Mat::zeros( rows - OFFSET, cols, orgcode );

	// Previous result
	m_matEMDPrev = cv::Mat::zeros( rows - OFFSET, cols, newcode );
}

//=========================================================================
IplImage* CEMD :: MakeEMD3( const IplImage* iplIn )
{
	m_matImage = iplIn;

	if ( !m_matImage.empty() )
	{
		if ( m_matImage.rows != m_matGrayImagePrev.rows || m_matImage.cols != m_matGrayImagePrev.cols )
		{
			AllocateImageMats( m_matImage.rows, m_matImage.cols,  m_matImage.type(), CV_8U ); // slightly smaller working images

			TRACE( TEXT( "Source Image Rows=%d, Cols=%d\n" ), m_matImage.rows, m_matImage.cols );

			// step and size for original image
			m_stSourceStep = m_matImage.step;
			m_stSourceElementSize = m_matImage.elemSize();

			// step and size for working images
			m_stDestStep = m_matHPF.step;
			m_stDestElementSize = m_matHPF.elemSize();

			TRACE( TEXT( "m_stSourceStep=%d, m_stSourceElementSize=%d, m_stDestStep=%d, m_stDestElementSize=%d\n" ), m_stSourceStep, m_stSourceElementSize, m_stDestStep, m_stDestElementSize );
		}
		
		uchar uxEMD, uyEMD;
		//double dEMD;

		// mat.at(i, j) = mat.at(row, col) = mat.at(y, x)
		// addr(M_{ij})=M.data + M.step*i + j*M.elemSize()
		// addr(M_{row col})=M.data + M.step*row + col*M.elemSize()

		size_t stSourceRow, stDestRow;
		size_t stSourceRowCol, stSourceRowColShiftRow, stSourceRowColShiftCol, stDestRowCol, stDestRowColShiftRow, stDestRowColShiftCol;

		for(	stSourceRow = 0,
				stDestRow = 0
				;
				stSourceRow < ( m_matImage.rows - OFFSET - 1 ) * m_stSourceStep
				;
				stSourceRow += m_stSourceStep,
				stDestRow += m_stDestStep + OFFSET )
			{
			TRACE( TEXT(   "stSourceRow=%d, stDestRow=%d\n" ), stSourceRow,    stDestRow );
			for(	stSourceRowCol = stSourceRow,
					stSourceRowColShiftRow = stSourceRow + ( m_stSourceStep * OFFSET ),
					stSourceRowColShiftCol = stSourceRow + ( OFFSET * m_stSourceElementSize ),

					stDestRowCol = stDestRow,
					stDestRowColShiftRow = stDestRow + ( m_stDestStep * OFFSET ),
					stDestRowColShiftCol = stDestRow + ( OFFSET * m_stDestElementSize )
					;
					stSourceRowCol < stSourceRow + ( m_matImage.cols * m_stSourceElementSize )
					;
					stSourceRowCol +=			m_stSourceElementSize, 
					stSourceRowColShiftRow +=	m_stSourceElementSize,
					stSourceRowColShiftCol +=	m_stSourceElementSize,
	
					stDestRowCol +=				m_stDestElementSize,
					stDestRowColShiftRow +=		m_stDestElementSize,
					stDestRowColShiftCol +=		m_stDestElementSize )
		/*for(	iOffRowOrg = 0,
				iOffRowShiftOrg = m_iStepOrg * OFFSET,
				iOffRow = 0,
				iOffRowShift = m_iStep * OFFSET
				;
				iOffRowOrg < ( m_matImage.rows - OFFSET - 1 ) * m_iStepOrg
				;
				iOffRowOrg += m_iStepOrg,
				iOffRowShiftOrg += m_iStepOrg,
				iOffRow += m_iStep,
				iOffRowShift += m_iStep )

			{
			for(	iOffOrg = iOffRowOrg,
					iOffShiftColOrg = iOffRowOrg + OFFSET * m_iElementSizeOrg,
					iOffShiftRowOrg = iOffRowShiftOrg,

					iOff = iOffRow,
					iOffShiftCol = iOffRow + OFFSET * m_iElementSize,
					iOffShiftRow = iOffRowShift
					;
					iOffOrg < ( m_matImage.cols * m_iElementSizeOrg )
					;
					// iOffOrg += m_iElementSizeOrg, 
					iOffShiftColOrg += m_iElementSizeOrg,
					// iOffShiftRowOrg += m_iElementSizeOrg, 

					// iOff +=				m_iElementSize, 
					iOffShiftCol +=		m_iElementSize
					// ,iOffShiftRow +=		m_iElementSize )

			//for( int col = 0; col < m_matImage.cols; col++ )
			*/
			{
				// calculate element offsets for original image
				//int iOffOrg = m_iStepOrg * row + col * m_iElementSizeOrg;
				//int iOffShiftColOrg = m_iStepOrg * row + ( col + OFFSET) * m_iElementSizeOrg;
				//int iOffShiftRowOrg = m_iStepOrg * ( row + OFFSET) + col * m_iElementSizeOrg;

				// calculate element offsets for working images
				//int iOff = m_iStep * row + col * m_iElementSize;
				//int iOffShiftCol = m_iStep * row + ( col + OFFSET) * m_iElementSize;
				//int iOffShiftRow = m_iStep * ( row + OFFSET) + col * m_iElementSize;

				//TRACE( TEXT(   "stSourceRowCol=%d, stSourceRowColShiftRow=%d, stSourceRowColShiftCol=%d, stDestRowCol=%d, stDestRowColShiftRow=%d, stDestRowColShiftCol=%d\n" ),
				//				stSourceRowCol,    stSourceRowColShiftRow,    stSourceRowColShiftCol,    stDestRowCol,    stDestRowColShiftRow,    stDestRowColShiftCol );

				// only calculate current cell when not previously calculated i.e. first row is special case
				if ( stSourceRow == 0 )
				{
					// only calculate current cell for top left
					if ( stSourceRowCol == 0 )
						CalculateAndStoreHPFnLPF( stSourceRowCol, stDestRowCol );

					// calculate right cell unless in right most cell
					if ( stSourceRowCol < m_stSourceElementSize * ( m_matImage.cols - 1 ) )
						CalculateAndStoreHPFnLPF( stSourceRowColShiftCol, stDestRowColShiftCol );
				}

				// always calculate cell beneath (we don't iterate the last row)
				CalculateAndStoreHPFnLPF( stSourceRowColShiftRow, stDestRowColShiftRow );

				// make EMD in Col: ( HPFNew x LPF_shift ) + ( HPFNew_shift x LPF ) unless we are rightmost column
				if ( stSourceRowCol < m_stSourceElementSize * ( m_matImage.cols - 1 ) )
					uxEMD = S_ELEMENT( uchar, m_matHPF.data, stDestRowCol ) * S_ELEMENT( uchar, m_matLPF.data, stDestRowColShiftCol ) + S_ELEMENT( uchar, m_matHPF.data, stDestRowColShiftCol ) * S_ELEMENT( uchar, m_matLPF.data, stDestRowCol );
				else
					uxEMD = 0;

				// make EMD in Row
				uyEMD = S_ELEMENT( uchar, m_matHPF.data, stDestRowCol ) * S_ELEMENT( uchar, m_matLPF.data, stDestRowColShiftRow ) + S_ELEMENT( uchar, m_matHPF.data, stDestRowColShiftRow ) * S_ELEMENT( uchar, m_matLPF.data, stDestRowCol );

				// pythagoras = dEMD = sqrt( double( uxEMD * uxEMD + uyEMD * uyEMD ) ) ;

				// make full emd: EMD	= ( ( 1 - b ) x EMDPrev ) + ( b x EMD )
				S_ELEMENT( uchar, m_matEMD.data, stDestRowCol ) = S_ELEMENT( uchar, m_matEMDPrev.data, stDestRowCol ) * m_CurrentOneMinusB + m_CurrentB * sqrt( double( uxEMD * uxEMD + uyEMD * uyEMD ) );

				// save EMD for next time
				S_ELEMENT( uchar, m_matEMDPrev.data, stDestRowCol ) = S_ELEMENT( uchar, m_matEMD.data, stDestRowCol );

				// convert back to colour	* 255 ?????????????
				S_ELEMENT( uchar, m_matEMDRGB.data, stDestRowCol ) = S_ELEMENT( uchar, m_matEMD.data, stDestRowCol ) * BLUE;
				S_ELEMENT( uchar, m_matEMDRGB.data, stDestRowCol + 1 ) = S_ELEMENT( uchar, m_matEMD.data, stDestRowCol + 1 ) * GREEN;
				S_ELEMENT( uchar, m_matEMDRGB.data, stDestRowCol + 2 ) = S_ELEMENT( uchar, m_matEMD.data, stDestRowCol + 2 ) * RED;
			}
		}

		//m_matGrayImagePrev

		// convert mat EMD to colour ipl image
		//cvtColor( m_matEMD, m_matEMDRGB, CV_GRAY2RGB );

		cvtColor( m_matGrayImagePrev, m_matEMDRGB, CV_GRAY2BGR );

		// convert back to original type
		//m_matEMDRGB.convertTo( m_matEMDRGB_ORG, m_matImage.type() );
		
		//m_ImageEMD = m_matEMDRGB_ORG;

		m_ImageEMD = m_matEMDRGB;

		return &m_ImageEMD;
	}

	return NULL;
}

//=========================================================================
IplImage* CEMD :: MakeEMD4( const IplImage* iplImageIn )
{
	if ( CV_IS_IMAGE(iplImageIn) )
	{
		m_matImage = cv::Mat::Mat( iplImageIn );

		if ( m_matImage.rows != m_matGrayImagePrev.rows || m_matImage.cols != m_matGrayImagePrev.cols )
			AllocateImageMats( m_matImage.rows, m_matImage.cols,  m_matImage.type(), CV_8UC1 ); // slightly smaller working images;
		
		// mat.at(i, j) = mat.at(row, col) = mat.at(y, x)
		// addr(M_{ij})=M.data + M.step*i + j*M.elemSize()
		// addr(M_{row col})=M.data + M.step*row + col*M.elemSize()

		size_t stSourceRow, stDestRow;
		size_t stSourceRowCol, stSourceRowColShiftRow, stSourceRowColShiftCol, stDestRowCol, stDestRowColShiftRow, stDestRowColShiftCol;

		size_t stSrcStep = m_matImage.step;
		size_t stDestStep = m_matGrayImagePrev.step;

		size_t stSrcElemSize = m_matImage.elemSize();
		size_t stDestElemSize = m_matGrayImagePrev.elemSize();

		size_t stSourceRowShift, stDestRowShift;

		size_t stSourceRowLimit = ( m_matImage.rows - OFFSET - 2 ) * stSrcStep ;
		//size_t stSourceRowLimit =  ( m_matImage.rows - OFFSET ) * stSrcStep + 1;
		size_t stSourceRowColLimit;

		uchar uxEMD, uyEMD;

		for(	stSourceRow = 0,
				stDestRow = 0,
				stSourceRowShift = stSrcStep,
				stDestRowShift = stDestStep
				;
				stSourceRow < stSourceRowLimit
				;
				stSourceRow			+= stSrcStep,
				stDestRow			+= stDestStep,
				stSourceRowShift	+= stSrcStep,
				stDestRowShift		+= stDestStep )
			{
			//TRACE( TEXT(   "stSourceRow=%d, stDestRow=%d\n" ), stSourceRow,    stDestRow );
			
			stSourceRowColLimit = stSourceRow + stSrcStep;

			for(	stSourceRowCol = stSourceRow,
					stDestRowCol = stDestRow,
					stSourceRowColShiftRow = stSourceRowShift,
					stDestRowColShiftRow = stDestRowShift,
					stSourceRowColShiftCol = stSourceRow + stSrcElemSize,
					stDestRowColShiftCol = stDestRow + stDestElemSize
					;
					stSourceRowCol < stSourceRowColLimit
					;
					stSourceRowCol			+=	stSrcElemSize, 
					stDestRowCol			+=	stDestElemSize,
					stSourceRowColShiftRow	+=	stSrcElemSize, 
					stDestRowColShiftRow	+=	stDestElemSize,
					stSourceRowColShiftCol	+=	stSrcElemSize,
					stDestRowColShiftCol	+=	stDestElemSize
				)
			{
				//TRACE( TEXT(   "stSourceRowCol=%d, stSourceRowColShiftRow=%d, stSourceRowColShiftCol=%d, stDestRowCol=%d, stDestRowColShiftRow=%d, stDestRowColShiftCol=%d\n" ),
				//				stSourceRowCol,    stSourceRowColShiftRow,    stSourceRowColShiftCol,    stDestRowCol,    stDestRowColShiftRow,    stDestRowColShiftCol );

				// only calculate current cell when not previously calculated i.e. first row is special case
				if ( stSourceRow == 0 )
				{
					// only calculate current cell for top left
					if ( stSourceRowCol == 0 )
						CalculateAndStoreHPFnLPF( stSourceRowCol, stDestRowCol );

					// calculate right cell
					CalculateAndStoreHPFnLPF( stSourceRowColShiftCol, stDestRowColShiftCol );
				}

				// always calculate cell beneath (we don't iterate the last row)
				CalculateAndStoreHPFnLPF( stSourceRowColShiftRow, stDestRowColShiftRow );

				// make EMD in Col: ( HPFNew x LPF_shift ) + ( HPFNew_shift x LPF ) unless we are rightmost column
				uxEMD = S_ELEMENT( uchar, m_matHPF.data, stDestRowCol ) * S_ELEMENT( uchar, m_matLPF.data, stDestRowColShiftCol ) + S_ELEMENT( uchar, m_matHPF.data, stDestRowColShiftCol ) * S_ELEMENT( uchar, m_matLPF.data, stDestRowCol );

				// make EMD in Row
				uyEMD = S_ELEMENT( uchar, m_matHPF.data, stDestRowCol ) * S_ELEMENT( uchar, m_matLPF.data, stDestRowColShiftRow ) + S_ELEMENT( uchar, m_matHPF.data, stDestRowColShiftRow ) * S_ELEMENT( uchar, m_matLPF.data, stDestRowCol );

				// make full emd: EMD	= ( ( 1 - b ) x EMDPrev ) + ( b x EMD ) // with pythagoras
				S_ELEMENT( uchar, m_matEMD.data, stDestRowCol ) = S_ELEMENT( uchar, m_matEMDPrev.data, stDestRowCol ) * m_CurrentOneMinusB + m_CurrentB * sqrt( double( uxEMD * uxEMD + uyEMD * uyEMD ) );

				// save EMD for next time
				S_ELEMENT( uchar, m_matEMDPrev.data, stDestRowCol ) = S_ELEMENT( uchar, m_matEMD.data, stDestRowCol );
			}
		}

		m_ImageEMD = m_matEMD;
		return &m_ImageEMD;
	}

	return NULL;
}

#define F_ELEMENT( data ) *((uchar*)(data))

//=========================================================================
inline void CEMD :: CalculateAndStoreHPFnLPF2( uchar* stSourceAddress, uchar* stDestAddress_matGrayImagePrev, uchar* stDestAddress_matHPF, uchar* stDestAddress_matLPF, uchar* stDestAddress_matHPFPrev )
{
	// calculate gray pixel  BGR not RGB.
	uchar uPixel = F_ELEMENT( stSourceAddress ) * BLUE + F_ELEMENT( stSourceAddress + 1 ) * GREEN + F_ELEMENT( stSourceAddress + 2 ) * RED;

	// make high pass filter: HPF = NewImage - PrevImage
	F_ELEMENT( stDestAddress_matHPF ) = uPixel - F_ELEMENT( stDestAddress_matGrayImagePrev );
				
	//  save for next time
	F_ELEMENT( stDestAddress_matGrayImagePrev ) = uPixel;

	// make low pass filter: LPF	= (a x HPF) + ( (1 - a) x HPFPrev )
	F_ELEMENT( stDestAddress_matLPF ) = m_CurrentA * F_ELEMENT( stDestAddress_matHPF ) + m_CurrentOneMinusA * F_ELEMENT( stDestAddress_matHPFPrev );

	//  save for next time
	F_ELEMENT( stDestAddress_matHPFPrev ) = F_ELEMENT( stDestAddress_matHPF );
}

//=========================================================================
inline void CEMD :: CalculateAndStoreHPFnLPF3( const v3b stSource, uchar& stDest_matGrayImagePrev, uchar& stDest_matHPF, uchar& stDest_matLPF, uchar& stDest_matHPFPrev )
{
	// calculate gray pixel  BGR not RGB.
	uchar uPixel = cv::saturate_cast<uchar>( stSource[0] * BLUE + stSource[1] * GREEN + stSource[2] * RED );
	
	// make high pass filter: HPF = NewImage - PrevImage
	stDest_matHPF = uPixel - stDest_matGrayImagePrev;
				
	//  save for next time
	stDest_matGrayImagePrev = uPixel;

	// make low pass filter: LPF	= (a x HPF) + ( (1 - a) x HPFPrev )
	stDest_matLPF = cv::saturate_cast<uchar>( m_CurrentA * stDest_matHPF + m_CurrentOneMinusA * stDest_matHPFPrev );

	//  save for next time
	stDest_matHPFPrev = stDest_matHPF;
}

//=========================================================================
inline void CEMD :: CalculateAndStoreHPFnLPF4( const v3b stSource, uchar& stDest_matGrayImagePrev, ushort& stDest_matHPF, ushort& stDest_matLPF, ushort& stDest_matHPFPrev )
{
	// calculate gray pixel  BGR not RGB.
	uchar uPixel = stSource[0] * BLUE + stSource[1] * GREEN + stSource[2] * RED;
	
	// make high pass filter: HPF = NewImage - PrevImage
	stDest_matHPF = uPixel - stDest_matGrayImagePrev;
				
	//  save for next time
	stDest_matGrayImagePrev = uPixel;

	// make low pass filter: LPF	= (a x HPF) + ( (1 - a) x HPFPrev )
	stDest_matLPF = m_CurrentA * stDest_matHPF + m_CurrentOneMinusA * stDest_matHPFPrev;

	//  save for next time
	stDest_matHPFPrev = stDest_matHPF;
}


//=========================================================================
inline void CEMD :: CalculateAndStoreHPFnLPF2Channel( const v3b stSource, uchar& stDest_matGrayImagePrev, ushort& stDest_matHPF_POS, ushort& stDest_matHPF_NEG, ushort& stDest_matLPF_POS, ushort& stDest_matLPF_NEG, ushort& stDest_matHPFPrev )
{
	// calculate gray pixel  BGR not RGB.
	uchar uPixel = stSource[0] * BLUE + stSource[1] * GREEN + stSource[2] * RED;
	
	// make high pass filter: HPF = NewImage - PrevImage
	ushort stDest_matHPF = uPixel - stDest_matGrayImagePrev;

	//  save for next time
	stDest_matGrayImagePrev = uPixel;

	// set HPF appropriate value to half rectified and other to zero
	if ( stDest_matHPF > 0 )
	{
		stDest_matHPF_POS = stDest_matHPF;
		stDest_matHPF_NEG = 0;
	}
	else
	{
		stDest_matHPF_POS = 0;
		stDest_matHPF_NEG = stDest_matHPF;
	}

	// make low pass filter: LPF = (a x HPF) + ( (1 - a) x HPFPrev )
	ushort stDest_matLPF = m_CurrentA * stDest_matHPF + m_CurrentOneMinusA * stDest_matHPFPrev;

	// set LPF appropriate value to half rectified and other to zero
	if ( stDest_matLPF > 0 )
	{
		stDest_matLPF_POS = stDest_matLPF;
		stDest_matLPF_NEG = 0;
	}
	else
	{
		stDest_matLPF_POS = 0;
		stDest_matLPF_NEG = stDest_matLPF;
	}

	//  save for next time
	stDest_matHPFPrev = stDest_matHPF;
}

//=========================================================================
IplImage* CEMD :: MakeEMD5( const IplImage* iplIn )
{
	m_matImage = iplIn;

	if ( !m_matImage.empty() )
	{
		if ( m_matImage.rows != m_matGrayImagePrev.rows || m_matImage.cols != m_matGrayImagePrev.cols )
			AllocateImageMats( m_matImage.rows, m_matImage.cols,  m_matImage.type(), CV_8UC1 );

		uchar uxEMD, uyEMD;

		for(	int iRow = 0; iRow < m_matImage.rows - OFFSET; iRow++ )
		{
			for(	int iCol = 0; iCol <  m_matImage.cols - OFFSET; iCol++ )
			{
				// only calculate current cell when not previously calculated i.e. first row is special case
				if ( iRow == 0 )
				{
					// only calculate current cell for top left
					if ( iCol == 0 )
						CalculateAndStoreHPFnLPF2( m_matImage.ptr( iRow, iCol ), m_matGrayImagePrev.ptr( iRow, iCol ), m_matHPF.ptr( iRow, iCol ), m_matLPF.ptr( iRow, iCol ), m_matHPFPrev.ptr( iRow, iCol ) );

					// calculate right cell
					CalculateAndStoreHPFnLPF2( m_matImage.ptr( iRow, iCol + 1 ), m_matGrayImagePrev.ptr( iRow, iCol + 1 ), m_matHPF.ptr( iRow, iCol + 1 ), m_matLPF.ptr( iRow, iCol + 1 ), m_matHPFPrev.ptr( iRow, iCol + 1 ) );
				}

				// always calculate cell beneath (we don't iterate the last row)
				CalculateAndStoreHPFnLPF2( m_matImage.ptr( iRow + 1, iCol ), m_matGrayImagePrev.ptr( iRow + 1, iCol ), m_matHPF.ptr( iRow + 1, iCol ), m_matLPF.ptr( iRow + 1, iCol ), m_matHPFPrev.ptr( iRow + 1, iCol ) );

				// make EMD in Col: ( HPFNew x LPF_shift ) + ( HPFNew_shift x LPF ) unless we are rightmost column
				uxEMD = F_ELEMENT(  m_matHPF.ptr( iRow, iCol ) ) * F_ELEMENT( m_matLPF.ptr( iRow, iCol + 1 ) ) + F_ELEMENT(  m_matHPF.ptr( iRow, iCol + 1 ) ) * F_ELEMENT( m_matLPF.ptr( iRow, iCol ) );

				// make EMD in Row
				uyEMD = F_ELEMENT( m_matHPF.ptr( iRow, iCol ) ) * F_ELEMENT( m_matLPF.ptr( iRow + 1, iCol ) ) + F_ELEMENT( m_matHPF.ptr( iRow + 1, iCol ) ) * F_ELEMENT( m_matLPF.ptr( iRow, iCol ) );

				// make full emd magnitude: EMD	= ( ( 1 - b ) x EMDPrev ) + ( b x EMD ) // with pythagoras
				F_ELEMENT( m_matEMD.ptr( iRow, iCol ) ) = F_ELEMENT( m_matEMDPrev.ptr( iRow, iCol ) ) * m_CurrentOneMinusB + m_CurrentB * sqrt( double( uxEMD * uxEMD + uyEMD * uyEMD ) );

				// save EMD for next time
				F_ELEMENT( m_matEMDPrev.ptr( iRow, iCol ) ) = F_ELEMENT( m_matEMD.ptr( iRow, iCol ) );
			}
		}

		//cvtColor( m_matEMD, m_matEMDRGB, CV_GRAY2BGR );

		//m_ImageEMD = m_matEMDRGB;

		m_ImageEMD = m_matEMD;

		return &m_ImageEMD;
	}

	return NULL;
}

//=========================================================================
IplImage* CEMD :: MakeEMD6( const IplImage* iplIn )
{
	if ( CV_IS_IMAGE(iplIn) )
	{
		m_matImage = iplIn;

		if ( m_matImage.rows != m_matGrayImagePrev.rows || m_matImage.cols != m_matGrayImagePrev.cols )
			AllocateImageMats( m_matImage.rows, m_matImage.cols,  m_matImage.type(), CV_8UC1 );

		// initialise for first row only
		m_ucp_matImageRow			= m_matImage.ptr<v3b>( 0 );
		m_ucp_matGreyImagePrevRow	= m_matGrayImagePrev.ptr( 0 );
		m_ucp_matHPFPrevRow			= m_matHPFPrev.ptr( 0 );
		// but we need these as well
		m_ucp_matHPFRow				= m_matHPF.ptr( 0 );
		m_ucp_matLPFRow				= m_matLPF.ptr( 0 );

		// calculate (0,0) cell
		CalculateAndStoreHPFnLPF3( m_ucp_matImageRow[ 0 ], m_ucp_matGreyImagePrevRow[ 0 ], m_ucp_matHPFRow[ 0 ], m_ucp_matLPFRow[ 0 ], m_ucp_matHPFPrevRow[ 0 ] );

		for( int iRow = 0; iRow < m_matEMD.rows; iRow++ )
		{
			m_ucp_matImageRowPlus			= m_matImage.ptr<v3b>( iRow + OFFSET );
			m_ucp_matGreyImagePrevRowPlus	= m_matGrayImagePrev.ptr( iRow + OFFSET );
			m_ucp_matHPFRow					= m_matHPF.ptr( iRow );
			m_ucp_matHPFRowPlus				= m_matHPF.ptr( iRow + OFFSET );
			m_ucp_matHPFPrevRowPlus			= m_matHPFPrev.ptr( iRow + OFFSET );
			m_ucp_matLPFRow					= m_matLPF.ptr( iRow );
			m_ucp_matLPFRowPlus				= m_matLPF.ptr( iRow + OFFSET );
			m_ucp_matEMDRow					= m_matEMD.ptr( iRow );
			m_ucp_matEMDPrevRow				= m_matEMDPrev.ptr( iRow );

			for( int iCol = 0; iCol < m_matImage.cols - OFFSET; iCol++ )
			{
				if ( iRow == 0 ) // calculate right cells for first row
					CalculateAndStoreHPFnLPF3( m_ucp_matImageRow[ iCol + OFFSET ], m_ucp_matGreyImagePrevRow[ iCol + OFFSET ], m_ucp_matHPFRow[ iCol + OFFSET ], m_ucp_matLPFRow[ iCol + OFFSET ], m_ucp_matHPFPrevRow[ iCol + OFFSET ] );
				
				// always calculate cell beneath (we don't iterate the last row)
				CalculateAndStoreHPFnLPF3( m_ucp_matImageRowPlus[ iCol ], m_ucp_matGreyImagePrevRowPlus[ iCol ], m_ucp_matHPFRowPlus[ iCol ], m_ucp_matLPFRowPlus[ iCol ], m_ucp_matHPFPrevRowPlus[ iCol ] );

				// make EMD in Col: ( HPFNew x LPF_shift ) + ( HPFNew_shift x LPF ) unless we are rightmost column
				m_uxEMD = m_ucp_matHPFRow[ iCol ] * m_ucp_matLPFRow[ iCol + OFFSET ] + m_ucp_matHPFRow[ iCol + OFFSET ] * m_ucp_matLPFRow[ iCol ];

				// make EMD in Row
				m_uyEMD = m_ucp_matHPFRow[ iCol ] * m_ucp_matLPFRowPlus[ iCol ] + m_ucp_matHPFRowPlus[ iCol ] * m_ucp_matLPFRow[ iCol ];

				// make full emd magnitude: EMD	= ( ( 1 - b ) x EMDPrev ) + ( b x EMD ) // with pythagoras
				m_ucp_matEMDRow[ iCol ] = m_ucp_matEMDPrevRow[ iCol ] * m_CurrentOneMinusB + m_CurrentB * sqrt( double( m_uxEMD * m_uxEMD + m_uyEMD * m_uyEMD ) );

				// save EMD for next time
				m_ucp_matEMDPrevRow[ iCol ] = m_ucp_matEMDRow[ iCol ];
			}
		}

		m_ImageEMD = m_matEMD;
		return &m_ImageEMD;
	}

	return NULL;
}

//=========================================================================
IplImage* CEMD :: MakeEMD7( const IplImage* iplIn )
{
	const int OFFSET = 1;

	if ( CV_IS_IMAGE(iplIn) )
	{
		m_matImage = iplIn;

		if ( m_matImage.rows != m_matGrayImagePrev.rows || m_matImage.cols != m_matGrayImagePrev.cols )
			AllocateImageMats( m_matImage.rows, m_matImage.cols,  m_matImage.type(), CV_8UC1 );

		// initialise for first row only
		m_ucp_matImageRow			= m_matImage.ptr<v3b>( 0 );
		m_ucp_matGreyImagePrevRow	= m_matGrayImagePrev.ptr( 0 );
		m_ucp_matHPFPrevRow			= m_matHPFPrev.ptr( 0 );
		// but we need these as well
		m_ucp_matHPFRow				= m_matHPF.ptr( 0 );
		m_ucp_matLPFRow				= m_matLPF.ptr( 0 );

		// calculate (0,0) cell
		CalculateAndStoreHPFnLPF3( m_ucp_matImageRow[ 0 ], m_ucp_matGreyImagePrevRow[ 0 ], m_ucp_matHPFRow[ 0 ], m_ucp_matLPFRow[ 0 ], m_ucp_matHPFPrevRow[ 0 ] );

		for( int iRow = 0; iRow < m_matEMD.rows; iRow++ )
		{
			m_ucp_matImageRowPlus			= m_matImage.ptr<v3b>( iRow + OFFSET );
			m_ucp_matGreyImagePrevRowPlus	= m_matGrayImagePrev.ptr( iRow + OFFSET );
			m_ucp_matHPFRow					= m_matHPF.ptr( iRow );
			m_ucp_matHPFRowPlus				= m_matHPF.ptr( iRow + OFFSET );
			m_ucp_matHPFPrevRowPlus			= m_matHPFPrev.ptr( iRow + OFFSET );
			m_ucp_matLPFRow					= m_matLPF.ptr( iRow );
			m_ucp_matLPFRowPlus				= m_matLPF.ptr( iRow + OFFSET );
			m_ucp_matEMDRow					= m_matEMD.ptr( iRow );
			m_ucp_matEMDColourRow			= m_matEMDColour.ptr<v3b>( iRow );
			m_ucp_matEMDPrevRow				= m_matEMDPrev.ptr( iRow );

			for( int iCol = 0; iCol < m_matImage.cols - OFFSET; iCol++ )
			{
				if ( iRow == 0 ) // calculate right cells for first row
					CalculateAndStoreHPFnLPF3( m_ucp_matImageRow[ iCol + OFFSET ], m_ucp_matGreyImagePrevRow[ iCol + OFFSET ], m_ucp_matHPFRow[ iCol + OFFSET ], m_ucp_matLPFRow[ iCol + OFFSET ], m_ucp_matHPFPrevRow[ iCol + OFFSET ] );
				
				// always calculate cell beneath (we don't iterate the last row)
				CalculateAndStoreHPFnLPF3( m_ucp_matImageRowPlus[ iCol ], m_ucp_matGreyImagePrevRowPlus[ iCol ], m_ucp_matHPFRowPlus[ iCol ], m_ucp_matLPFRowPlus[ iCol ], m_ucp_matHPFPrevRowPlus[ iCol ] );

				// make EMD in Col: ( HPFNew x LPF_shift ) + ( HPFNew_shift x LPF ) unless we are rightmost column
				m_dxEMD = m_ucp_matHPFRow[ iCol ] * m_ucp_matLPFRow[ iCol + OFFSET ] + m_ucp_matHPFRow[ iCol + OFFSET ] * m_ucp_matLPFRow[ iCol ];

				// make EMD in Row
				m_dyEMD = m_ucp_matHPFRow[ iCol ] * m_ucp_matLPFRowPlus[ iCol ] + m_ucp_matHPFRowPlus[ iCol ] * m_ucp_matLPFRow[ iCol ];

				// make full emd magnitude: EMD	= ( ( 1 - b ) x EMDPrev ) + ( b x EMD ) // with pythagoras
				m_dEMD = m_ucp_matEMDPrevRow[ iCol ] * m_CurrentOneMinusB + m_CurrentB * sqrt( m_dxEMD * m_dxEMD + m_dyEMD * m_dyEMD );

				// save EMD for next time
				m_ucp_matEMDPrevRow[ iCol ] = m_dEMD;

				// EMD direction
				m_dEMDTheta = atan2( m_dyEMD, m_dxEMD );

				// set colour based on direction
				if ( m_dEMDTheta > 0 )
				{
					m_ucp_matEMDColourRow[ iCol ][0] = m_dEMD / BLUE;
					m_ucp_matEMDColourRow[ iCol ][2] = 0;
				}
				else
				{
					m_ucp_matEMDColourRow[ iCol ][0] = 0;
					m_ucp_matEMDColourRow[ iCol ][2] = m_dEMD / RED;
				}

				m_ucp_matEMDColourRow[ iCol ][1] = 0;	// green not used 
			}
		}

		m_ImageEMD = m_matEMDColour;
		return &m_ImageEMD;
	}

	return NULL;
}

//=========================================================================
IplImage* CEMD :: MakeEMD8( const IplImage* iplIn )
{
	const int OFFSET = 1;

	if ( CV_IS_IMAGE(iplIn) )
	{
		m_matImage = iplIn;

		if ( m_matImage.rows != m_matGrayImagePrev.rows || m_matImage.cols != m_matGrayImagePrev.cols )
		{
			AllocateImageMats( m_matImage.rows, m_matImage.cols,  m_matImage.type(), CV_8UC1 );
			// initilise for averages - arrays of double
			m_matEMDX = cv::Mat( m_matImage.rows - OFFSET, m_matImage.cols, CV_8UC1 );		// needs to be CV_16S - short
			m_matEMDY = cv::Mat( m_matImage.rows - OFFSET, m_matImage.cols, CV_8UC1 );
		}

		// initialise for first row only
		m_ucp_matImageRow			= m_matImage.ptr<v3b>( 0 );
		m_ucp_matGreyImagePrevRow	= m_matGrayImagePrev.ptr( 0 );
		m_ucp_matHPFPrevRow			= m_matHPFPrev.ptr( 0 );
		// but we need these as well
		m_ucp_matHPFRow				= m_matHPF.ptr( 0 );
		m_ucp_matLPFRow				= m_matLPF.ptr( 0 );

		// calculate (0,0) cell
		CalculateAndStoreHPFnLPF3( m_ucp_matImageRow[ 0 ], m_ucp_matGreyImagePrevRow[ 0 ], m_ucp_matHPFRow[ 0 ], m_ucp_matLPFRow[ 0 ], m_ucp_matHPFPrevRow[ 0 ] );

		for( int iRow = 0; iRow < m_matEMD.rows; iRow++ )
		{
			m_ucp_matImageRowPlus			= m_matImage.ptr<v3b>( iRow + OFFSET );
			m_ucp_matGreyImagePrevRowPlus	= m_matGrayImagePrev.ptr( iRow + OFFSET );
			m_ucp_matHPFRow					= m_matHPF.ptr( iRow );
			m_ucp_matHPFRowPlus				= m_matHPF.ptr( iRow + OFFSET );
			m_ucp_matHPFPrevRowPlus			= m_matHPFPrev.ptr( iRow + OFFSET );
			m_ucp_matLPFRow					= m_matLPF.ptr( iRow );
			m_ucp_matLPFRowPlus				= m_matLPF.ptr( iRow + OFFSET );
			m_ucp_matEMDRow					= m_matEMD.ptr( iRow );
			m_ucp_matEMDColourRow			= m_matEMDColour.ptr<v3b>( iRow );
			m_ucp_matEMDPrevRow				= m_matEMDPrev.ptr( iRow );
			m_ucp_matEMDX					= m_matEMDX.ptr( iRow );
			m_ucp_matEMDY					= m_matEMDY.ptr( iRow );

			for( int iCol = 0; iCol < m_matImage.cols - OFFSET; iCol++ )
			{
				if ( iRow == 0 ) // calculate right cells for first row
					CalculateAndStoreHPFnLPF3( m_ucp_matImageRow[ iCol + OFFSET ], m_ucp_matGreyImagePrevRow[ iCol + OFFSET ], m_ucp_matHPFRow[ iCol + OFFSET ], m_ucp_matLPFRow[ iCol + OFFSET ], m_ucp_matHPFPrevRow[ iCol + OFFSET ] );
				
				// always calculate cell beneath (we don't iterate the last row)
				CalculateAndStoreHPFnLPF3( m_ucp_matImageRowPlus[ iCol ], m_ucp_matGreyImagePrevRowPlus[ iCol ], m_ucp_matHPFRowPlus[ iCol ], m_ucp_matLPFRowPlus[ iCol ], m_ucp_matHPFPrevRowPlus[ iCol ] );

				// make EMD in Col: ( HPFNew x LPF_shift ) + ( HPFNew_shift x LPF ) unless we are rightmost column
				uchar vvm_dxEMD = cv::saturate_cast<uchar>(m_ucp_matHPFRow[ iCol ] * m_ucp_matLPFRow[ iCol + OFFSET ] + m_ucp_matHPFRow[ iCol + OFFSET ] * m_ucp_matLPFRow[ iCol ]);

				// make EMD in Row
				uchar vvm_dyEMD = cv::saturate_cast<uchar>(m_ucp_matHPFRow[ iCol ] * m_ucp_matLPFRowPlus[ iCol ] + m_ucp_matHPFRowPlus[ iCol ] * m_ucp_matLPFRow[ iCol ]);

				// save for averages
				m_ucp_matEMDX[ iCol ] = vvm_dxEMD;
				m_ucp_matEMDY[ iCol ] = vvm_dyEMD;

				// make full emd magnitude: EMD	= ( ( 1 - b ) x EMDPrev ) + ( b x EMD ) // with pythagoras
				//m_dEMD =  m_ucp_matEMDPrevRow[ iCol ] * m_CurrentOneMinusB + m_CurrentB * sqrt( m_dxEMD * m_dxEMD + m_dyEMD * m_dyEMD ) ;
				m_dEMD = cv::saturate_cast<uchar>(m_ucp_matEMDPrevRow[ iCol ] * m_CurrentOneMinusB + m_CurrentB * sqrt( (double)( (vvm_dxEMD * vvm_dxEMD) + (vvm_dyEMD * vvm_dyEMD) ) ));

				// save EMD for next time
				m_ucp_matEMDPrevRow[ iCol ] = m_dEMD;

				// EMD direction
				//m_dEMDTheta = atan2( m_dyEMD, m_dxEMD );
				m_dEMDTheta = atan2( (double)vvm_dyEMD, (double)vvm_dxEMD );

				// set colour based on direction
				if ( m_dEMDTheta > 0 )
				{
					m_ucp_matEMDColourRow[ iCol ][0] = m_dEMD / BLUE;
					m_ucp_matEMDColourRow[ iCol ][2] = 0;
				}
				else
				{
					m_ucp_matEMDColourRow[ iCol ][0] = 0;
					m_ucp_matEMDColourRow[ iCol ][2] = m_dEMD / RED;
				}

				m_ucp_matEMDColourRow[ iCol ][1] = 0;	// green not used 
			}
		}

		m_ImageEMD = m_matEMDColour;
		return &m_ImageEMD;
	}

	return NULL;
}

//=========================================================================
IplImage* CEMD :: MakeEMD9( const IplImage* iplIn )
{
	const int OFFSET = 1;

	if ( CV_IS_IMAGE(iplIn) )
	{
		m_matImage = iplIn;

		if ( m_matImage.rows != m_matGrayImagePrev.rows || m_matImage.cols != m_matGrayImagePrev.cols )
		{
			AllocateImageMats( m_matImage.rows, m_matImage.cols,  m_matImage.type(), CV_16S ); // CV_16S allows negative values for EMD calculations
			// initilise for averages - arrays of double
			m_matEMDX = cv::Mat::zeros( m_matImage.rows - OFFSET, m_matImage.cols, CV_16S );
			m_matEMDY = cv::Mat::zeros( m_matImage.rows - OFFSET, m_matImage.cols, CV_16S );
			m_matEMDXPrev = cv::Mat::zeros( m_matImage.rows - OFFSET, m_matImage.cols, CV_16S );
			m_matEMDYPrev = cv::Mat::zeros( m_matImage.rows - OFFSET, m_matImage.cols, CV_16S );
		}

		// initialise for first row only
		m_ucp_matImageRow			= m_matImage.ptr<v3b>( 0 );
		m_ucp_matGreyImagePrevRow	= m_matGrayImagePrev.ptr( 0 );
		m_sp_matHPFPrevRow			= m_matHPFPrev.ptr<ushort>( 0 );
		// but we need these as well
		m_sp_matHPFRow				= m_matHPF.ptr<ushort>( 0 );
		m_sp_matLPFRow				= m_matLPF.ptr<ushort>( 0 );

		// calculate (0,0) cell
		CalculateAndStoreHPFnLPF4( m_ucp_matImageRow[ 0 ], m_ucp_matGreyImagePrevRow[ 0 ], m_sp_matHPFRow[ 0 ], m_sp_matLPFRow[ 0 ], m_sp_matHPFPrevRow[ 0 ] );

		for( int iRow = 0; iRow < m_matEMD.rows; iRow++ )
		{
			m_ucp_matImageRowPlus			= m_matImage.ptr<v3b>( iRow + OFFSET );
			m_ucp_matGreyImagePrevRowPlus	= m_matGrayImagePrev.ptr( iRow + OFFSET );

			m_sp_matHPFRow					= m_matHPF.ptr<ushort>( iRow );
			m_sp_matHPFRowPlus				= m_matHPF.ptr<ushort>( iRow + OFFSET );
			m_sp_matHPFPrevRowPlus			= m_matHPFPrev.ptr<ushort>( iRow + OFFSET );

			m_sp_matLPFRow					= m_matLPF.ptr<ushort>( iRow );
			m_sp_matLPFRowPlus				= m_matLPF.ptr<ushort>( iRow + OFFSET );

			m_ucp_matEMDRow					= m_matEMD.ptr( iRow );
			m_ucp_matEMDColourRow			= m_matEMDColour.ptr<v3b>( iRow );
			m_ucp_matEMDPrevRow				= m_matEMDPrev.ptr( iRow );
			m_sp_matEMDX					= m_matEMDX.ptr<ushort>( iRow );
			m_sp_matEMDY					= m_matEMDY.ptr<ushort>( iRow );
			m_sp_matEMDXPrev				= m_matEMDXPrev.ptr<ushort>( iRow );
			m_sp_matEMDYPrev				= m_matEMDYPrev.ptr<ushort>( iRow );

			for( int iCol = 0; iCol < m_matImage.cols - OFFSET; iCol++ )
			{
				if ( iRow == 0 ) // calculate right cells for first row
					CalculateAndStoreHPFnLPF4( m_ucp_matImageRow[ iCol + OFFSET ], m_ucp_matGreyImagePrevRow[ iCol + OFFSET ], m_sp_matHPFRow[ iCol + OFFSET ], m_sp_matLPFRow[ iCol + OFFSET ], m_sp_matHPFPrevRow[ iCol + OFFSET ] );
				
				// always calculate cell beneath (we don't iterate the last row)
				CalculateAndStoreHPFnLPF4( m_ucp_matImageRowPlus[ iCol ], m_ucp_matGreyImagePrevRowPlus[ iCol ], m_sp_matHPFRowPlus[ iCol ], m_sp_matLPFRowPlus[ iCol ], m_sp_matHPFPrevRowPlus[ iCol ] );

				// make EMD in Col: ( HPFNew x LPF_shift ) + ( HPFNew_shift x LPF )
				ushort vvm_dxEMD = m_sp_matHPFRow[ iCol ] * m_sp_matLPFRow[ iCol + OFFSET ] - m_sp_matHPFRow[ iCol + OFFSET ] * m_sp_matLPFRow[ iCol ];

				// make EMD in Row
				ushort vvm_dyEMD = m_sp_matHPFRow[ iCol ] * m_sp_matLPFRowPlus[ iCol ] - m_sp_matHPFRowPlus[ iCol ] * m_sp_matLPFRow[ iCol ];

				// save for averages
				m_sp_matEMDX[ iCol ] = m_CurrentOneMinusB * m_sp_matEMDXPrev[ iCol ] + m_CurrentB * vvm_dxEMD;
				m_sp_matEMDY[ iCol ] = m_CurrentOneMinusB * m_sp_matEMDYPrev[ iCol ] + m_CurrentB * vvm_dyEMD;

				//m_sp_matEMDX[ iCol ] = vvm_dxEMD;
				//m_sp_matEMDY[ iCol ] = vvm_dyEMD;

				// make full emd magnitude: EMD	= ( ( 1 - b ) x EMDPrev ) + ( b x EMD ) // with pythagoras
				//m_dEMD = m_ucp_matEMDPrevRow[ iCol ] * m_CurrentOneMinusB + m_CurrentB * sqrt( (double)( (vvm_dxEMD * vvm_dxEMD) + (vvm_dyEMD * vvm_dyEMD) ) );
				m_dEMD = sqrt( (double)( (m_sp_matEMDX[ iCol ] * m_sp_matEMDX[ iCol ]) + (m_sp_matEMDY[ iCol ] * m_sp_matEMDY[ iCol ]) ) );

				// save EMD for next time
				//m_ucp_matEMDPrevRow[ iCol ] = m_dEMD;
				m_sp_matEMDXPrev[ iCol ] = m_sp_matEMDX[ iCol ];
				m_sp_matEMDYPrev[ iCol ] = m_sp_matEMDY[ iCol ];

				// EMD direction
				//m_dEMDTheta = atan2( m_dyEMD, m_dxEMD );
				m_dEMDTheta = atan2( (double)m_sp_matEMDY[ iCol ], (double)m_sp_matEMDX[ iCol ] );

				// set colour based on direction
				if ( m_dEMDTheta > 0 )
				{
					m_ucp_matEMDColourRow[ iCol ][0] = (m_dEMD + 65535L) / 256 / BLUE;
					m_ucp_matEMDColourRow[ iCol ][2] = 0;
				}
				else
				{
					m_ucp_matEMDColourRow[ iCol ][0] = 0;
					m_ucp_matEMDColourRow[ iCol ][2] = (m_dEMD + 65535L) / 256 / RED;
				}

				m_ucp_matEMDColourRow[ iCol ][1] = 0;	// green not used 
			}
		}

		m_ImageEMD = m_matEMDColour;
		return &m_ImageEMD;
	}

	return NULL;
}


//=========================================================================
IplImage* CEMD :: Make2ChannelEMD( const IplImage* iplIn )
{
	const int OFFSET = 1;

	if ( CV_IS_IMAGE(iplIn) )
	{
		m_matImage = iplIn;

		if ( m_matImage.rows != m_matGrayImagePrev.rows || m_matImage.cols != m_matGrayImagePrev.cols )
		{
			AllocateImageMats( m_matImage.rows, m_matImage.cols,  m_matImage.type(), CV_16S ); // CV_16S allows negative values for EMD calculations

			// High Pass Filter storage - for extra channel
			m_matHPFNEG = cv::Mat::zeros( m_matImage.rows, m_matImage.cols, CV_16S );

			// Low Pass Filter storage - for extra channel
			m_matLPFNEG = cv::Mat::zeros( m_matImage.rows, m_matImage.cols, CV_16S );

			// initilise for averages - arrays of double
			m_matEMDX = cv::Mat::zeros( m_matImage.rows - OFFSET, m_matImage.cols, CV_16S );
			m_matEMDY = cv::Mat::zeros( m_matImage.rows - OFFSET, m_matImage.cols, CV_16S );
			m_matEMDXPrev = cv::Mat::zeros( m_matImage.rows - OFFSET, m_matImage.cols, CV_16S );
			m_matEMDYPrev = cv::Mat::zeros( m_matImage.rows - OFFSET, m_matImage.cols, CV_16S );
		}

		// initialise for first row only
		m_ucp_matImageRow			= m_matImage.ptr<v3b>( 0 );
		m_ucp_matGreyImagePrevRow	= m_matGrayImagePrev.ptr( 0 );
		m_sp_matHPFPrevRow			= m_matHPFPrev.ptr<ushort>( 0 );
		// but we need these as well
		m_sp_matHPFRow				= m_matHPF.ptr<ushort>( 0 );
		m_sp_matLPFRow				= m_matLPF.ptr<ushort>( 0 );
		// and these for 2 channel
		m_sp_matHPFRowNEG			= m_matHPFNEG.ptr<ushort>( 0 );
		m_sp_matLPFRowNEG			= m_matLPFNEG.ptr<ushort>( 0 );

		// calculate (0,0) cell
		CalculateAndStoreHPFnLPF2Channel( m_ucp_matImageRow[ 0 ], m_ucp_matGreyImagePrevRow[ 0 ], m_sp_matHPFRow[ 0 ], m_sp_matHPFRowNEG[ 0 ], m_sp_matLPFRow[ 0 ], m_sp_matLPFRowNEG[ 0 ], m_sp_matHPFPrevRow[ 0 ] );

		for( int iRow = 0; iRow < m_matEMD.rows; iRow++ )
		{
			m_ucp_matImageRowPlus			= m_matImage.ptr<v3b>( iRow + OFFSET );
			m_ucp_matGreyImagePrevRowPlus	= m_matGrayImagePrev.ptr( iRow + OFFSET );

			m_sp_matHPFRow					= m_matHPF.ptr<ushort>( iRow );
			m_sp_matHPFRowNEG				= m_matHPFNEG.ptr<ushort>( iRow );
			m_sp_matHPFRowPlus				= m_matHPF.ptr<ushort>( iRow + OFFSET );
			m_sp_matHPFRowPlusNEG			= m_matHPFNEG.ptr<ushort>( iRow + OFFSET );
			m_sp_matHPFPrevRowPlus			= m_matHPFPrev.ptr<ushort>( iRow + OFFSET );

			m_sp_matLPFRow					= m_matLPF.ptr<ushort>( iRow );
			m_sp_matLPFRowNEG				= m_matLPFNEG.ptr<ushort>( iRow );
			m_sp_matLPFRowPlus				= m_matLPF.ptr<ushort>( iRow + OFFSET );
			m_sp_matLPFRowPlusNEG			= m_matLPFNEG.ptr<ushort>( iRow + OFFSET );

			m_ucp_matEMDRow					= m_matEMD.ptr( iRow );
			m_ucp_matEMDColourRow			= m_matEMDColour.ptr<v3b>( iRow );
			m_ucp_matEMDPrevRow				= m_matEMDPrev.ptr( iRow );
			m_sp_matEMDX					= m_matEMDX.ptr<ushort>( iRow );
			m_sp_matEMDY					= m_matEMDY.ptr<ushort>( iRow );
			m_sp_matEMDXPrev				= m_matEMDXPrev.ptr<ushort>( iRow );
			m_sp_matEMDYPrev				= m_matEMDYPrev.ptr<ushort>( iRow );

			for( int iCol = 0; iCol < m_matImage.cols - OFFSET; iCol++ )
			{
				if ( iRow == 0 ) // calculate right cells for first row
					CalculateAndStoreHPFnLPF2Channel( m_ucp_matImageRow[ iCol + OFFSET ], m_ucp_matGreyImagePrevRow[ iCol + OFFSET ], m_sp_matHPFRow[ iCol + OFFSET ], m_sp_matHPFRowNEG[ iCol + OFFSET ], m_sp_matLPFRow[ iCol + OFFSET ], m_sp_matLPFRowNEG[ iCol + OFFSET ], m_sp_matHPFPrevRow[ iCol + OFFSET ] );
				
				// always calculate cell beneath (we don't iterate the last row)
				CalculateAndStoreHPFnLPF2Channel( m_ucp_matImageRowPlus[ iCol ], m_ucp_matGreyImagePrevRowPlus[ iCol ], m_sp_matHPFRowPlus[ iCol ], m_sp_matHPFRowPlusNEG[ iCol ], m_sp_matLPFRowPlus[ iCol ], m_sp_matLPFRowPlusNEG[ iCol ], m_sp_matHPFPrevRowPlus[ iCol ] );

				// L1x = pos-half-wave LP * pos-half-wave HP_shift
				// L1y = pos-half-wave HP * pos-half-wave LP_shift

				// make EMD in Col: ( HPFNew x LPF_shift ) + ( HPFNew_shift x LPF )
				ushort vvm_dxEMDL1 = m_sp_matHPFRow[ iCol ] * m_sp_matLPFRow[ iCol + OFFSET ] - m_sp_matHPFRow[ iCol + OFFSET ] * m_sp_matLPFRow[ iCol ];

				// make EMD in Row
				ushort vvm_dyEMDL1 = m_sp_matHPFRow[ iCol ] * m_sp_matLPFRowPlus[ iCol ] - m_sp_matHPFRowPlus[ iCol ] * m_sp_matLPFRow[ iCol ];

				// L2x = neg-half-wave LP  * neg-half-wave HP_shift
				// L2y = neg-half-wave HP * neg-have-wave LP_shift

				// make EMD in Col: ( HPFNew x LPF_shift ) + ( HPFNew_shift x LPF )
				ushort vvm_dxEMDL2 = m_sp_matHPFRowNEG[ iCol ] * m_sp_matLPFRowNEG[ iCol + OFFSET ] - m_sp_matHPFRowNEG[ iCol + OFFSET ] * m_sp_matLPFRowNEG[ iCol ];

				// make EMD in Row
				ushort vvm_dyEMDL2 = m_sp_matHPFRowNEG[ iCol ] * m_sp_matLPFRowPlusNEG[ iCol ] - m_sp_matHPFRowPlusNEG[ iCol ] * m_sp_matLPFRowNEG[ iCol ];

				// save for averages
				m_sp_matEMDX[ iCol ] = m_CurrentOneMinusB * m_sp_matEMDXPrev[ iCol ] + m_CurrentB * ( vvm_dxEMDL1 + vvm_dxEMDL2 );
				m_sp_matEMDY[ iCol ] = m_CurrentOneMinusB * m_sp_matEMDYPrev[ iCol ] + m_CurrentB * ( vvm_dyEMDL1 + vvm_dyEMDL2 );

				//m_sp_matEMDX[ iCol ] = vvm_dxEMD;
				//m_sp_matEMDY[ iCol ] = vvm_dyEMD;

				// make full emd magnitude: EMD	= ( ( 1 - b ) x EMDPrev ) + ( b x EMD ) // with pythagoras
				//m_dEMD = m_ucp_matEMDPrevRow[ iCol ] * m_CurrentOneMinusB + m_CurrentB * sqrt( (double)( (vvm_dxEMD * vvm_dxEMD) + (vvm_dyEMD * vvm_dyEMD) ) );
				m_dEMD = sqrt( (double)( (m_sp_matEMDX[ iCol ] * m_sp_matEMDX[ iCol ]) + (m_sp_matEMDY[ iCol ] * m_sp_matEMDY[ iCol ]) ) );

				// save EMD for next time
				//m_ucp_matEMDPrevRow[ iCol ] = m_dEMD;
				m_sp_matEMDXPrev[ iCol ] = m_sp_matEMDX[ iCol ];
				m_sp_matEMDYPrev[ iCol ] = m_sp_matEMDY[ iCol ];

				// EMD direction
				//m_dEMDTheta = atan2( m_dyEMD, m_dxEMD );
				m_dEMDTheta = atan2( (double)m_sp_matEMDY[ iCol ], (double)m_sp_matEMDX[ iCol ] );

				// set colour based on direction
				if ( m_dEMDTheta > 0 )
				{
					m_ucp_matEMDColourRow[ iCol ][0] = (m_dEMD + 65535L) / 256 / BLUE;
					m_ucp_matEMDColourRow[ iCol ][2] = 0;
				}
				else
				{
					m_ucp_matEMDColourRow[ iCol ][0] = 0;
					m_ucp_matEMDColourRow[ iCol ][2] = (m_dEMD + 65535L) / 256 / RED;
				}

				m_ucp_matEMDColourRow[ iCol ][1] = 0;	// green not used 
			}
		}

		m_ImageEMD = m_matEMDColour;
		return &m_ImageEMD;
	}

	return NULL;
}

//=========================================================================
inline void CEMD :: AverageAndLabelRegion( cv::Mat matLabelledImage, cv::Mat matRegionX, cv::Mat matRegionY, int iColSlotWidth, int iRowSlotHeight, int iFontFace, double dFontScale, cv::Scalar cvSColour, int iThickness )
{
	cv::Scalar scTopMeanX = cv::sum( matRegionX );
	cv::Scalar scTopMeanY = cv::sum( matRegionY );

	double dTopMean = sqrt( scTopMeanX[0] * scTopMeanX[0] + scTopMeanY[0] * scTopMeanY[0] )/100000;

	std::string sText = sDoubleToString( dTopMean );

	int iBaseline;

	cv::Size cvsTextSize = cv::getTextSize( sText, iFontFace, dFontScale, iThickness, &iBaseline );

	cv::Point cvpTextOrg((matLabelledImage.cols - cvsTextSize.width)/2, (matLabelledImage.rows + cvsTextSize.height)/2);

	putText( matLabelledImage, sText, cvpTextOrg, iFontFace, dFontScale, cvSColour, iThickness );

	// Draw rectangle boundary
    cv::rectangle( matLabelledImage, cv::Rect( 0, 0, matLabelledImage.cols, matLabelledImage.rows ), cvSColour );

	// Draw arrow with preportional length and correct direction
	cv::Point cvpOrgChange( (cvpTextOrg.x + scTopMeanX[0]), (cvpTextOrg.y + scTopMeanY[0])  );

	cv::line( matLabelledImage, cvpTextOrg, cvpOrgChange, CV_RGB(0,255,0) );
}

//=========================================================================
IplImage* CEMD :: MakeEMDRegions( const IplImage* iplImageIn, double dHorizontal, double dVertical )
{

	if ( CV_IS_IMAGE(iplImageIn) )
	{
		m_matImage = iplImageIn;

		int iColSlotWidth = m_matImage.cols * dHorizontal;
		int iRowSlotHeight = m_matImage.rows * dVertical;

		int iFontFace = CV_FONT_HERSHEY_SIMPLEX;
		double dFontScale = 0.65;
		int iThickness = 2;
		
		cv::Rect rTopBand( 0, 0, m_matImage.cols, iRowSlotHeight );
		cv::Rect rBottomBand( 0, m_matImage.rows - iRowSlotHeight, m_matImage.cols, iRowSlotHeight );
		cv::Rect rLeftBand( 0, 0, iColSlotWidth, m_matImage.rows );
		cv::Rect rRightBand( m_matImage.cols - iColSlotWidth, 0, iColSlotWidth, m_matImage.rows );

		AverageAndLabelRegion( m_matImage( rTopBand ), m_matEMDX( rTopBand ),  m_matEMDY( rTopBand ), iColSlotWidth, iRowSlotHeight, CV_FONT_HERSHEY_SIMPLEX, dFontScale, CV_RGB( 0, 255, 0 ), iThickness );
		AverageAndLabelRegion( m_matImage( rBottomBand ), m_matEMDX( rBottomBand ),  m_matEMDY( rBottomBand ), iColSlotWidth, iRowSlotHeight, CV_FONT_HERSHEY_SIMPLEX, dFontScale, CV_RGB( 0, 255, 0 ), iThickness );
		AverageAndLabelRegion( m_matImage( rLeftBand ), m_matEMDX( rLeftBand ),  m_matEMDY( rLeftBand ), iColSlotWidth, iRowSlotHeight, CV_FONT_HERSHEY_SIMPLEX, dFontScale, CV_RGB( 0, 255, 0 ), iThickness );
		AverageAndLabelRegion( m_matImage( rRightBand ), m_matEMDX( rRightBand ),  m_matEMDY( rRightBand ), iColSlotWidth, iRowSlotHeight, CV_FONT_HERSHEY_SIMPLEX, dFontScale, CV_RGB( 0, 255, 0 ), iThickness );
		
		m_ImageEMD = m_matImage;
		return &m_ImageEMD;
	}

	return NULL;

}

//=========================================================================
IplImage* CEMD :: MakeEMDLog( const IplImage* iplImageIn, double dHorizontal, double dVertical )
{

	if ( CV_IS_IMAGE(iplImageIn) )
	{
		m_matImage = iplImageIn;

		std::string sLine;

		// start line with current clock ticks
		std::string sStart = sLLToString( GetFrameTicks() );

		// each line starts with clock ticks
		sLine = sStart;

		// save source gray scale line first
		for( int j = 0; j < m_matImage.cols; j++ )
			sLine = sLine + "," + sIntToString( m_matGrayImagePrev.at<uchar>( m_matImage.rows/4, j ) );

		// protect vector loading process
		EnterCriticalSection( &m_Logcs );
		{
			// add line to vector
			m_vLogLines.push_back( sLine );

			// then get three lines of data, 1/4, 1/2 and 3/4 of the way through the image
			for ( int i = m_matImage.rows/4; i < m_matImage.rows; i += m_matImage.rows/4 )
			{
				// each line starts with clock ticks
				sLine = sStart;

				for( int j = 0; j < m_matImage.cols; j++ )
					sLine = sLine + "," + susToString( m_matEMDX.at<ushort>( i, j ) );

				// add line to vector
				m_vLogLines.push_back( sLine );
			}
		}
		LeaveCriticalSection( &m_Logcs );
		
		m_ImageEMD = m_matImage;
		return &m_ImageEMD;
	}

	return NULL;

}
