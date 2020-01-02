/* The MIT License
* 
* Copyright (c) 2008, Naotoshi Seo <sonots(at)sonots.com>
			(c) 2010, Mark N R Smith <mnsmith@lincoln.ac.uk>
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
// A wrapper class for some matlab interface functions
// Extracted from OpenCVX, simplified, wrapped in class and added double** method
// Also added run time dynamic linking to avoid matlab dependency if matlab not present

// resolve conflict in matrix.h(330): error C2371 for VC2010
#if (_MSC_VER >= 1600)
#include <yvals.h>
#define __STDC_UTF_16__
#endif

#include "matrix.h"
//#include "mex.h"

// for older matlabs
#ifndef mwSize
#define mwSize int
#endif

// typedefs for dynamic functions (hopefully compatible with many matlabs)
typedef void* (__cdecl* pfmxGetData)( const mxArray *pa );
typedef mxArray* (__cdecl* pfmxCreateNumericArray)( int, const int *dims, mxClassID classid, mxComplexity flag );
typedef mxArray* (__cdecl* pfmxCreateDoubleMatrix)( int m, int n, mxComplexity flag );

//typedef void* (__cdecl* pfmexMakeArrayPersistent)( const mxArray *pa );
//typedef void* (__cdecl* pfmxDestroyArray)( const mxArray *pa );

class CMxInterface
{
public:

	~CMxInterface(){ if ( m_matlabmxDLL ) FreeLibrary( m_matlabmxDLL ); };

	bool IsMxLoaded( void ){ return ( m_matlabmxDLL != 0 &&  m_matlabmexDLL != 0 ); };

	// run time dynamic linking - avoid matlab dependency if matlab not present
	bool OpenMxInterface( void )
	{
		m_matlabmxDLL = LoadLibrary( TEXT("libmx.dll") );
		//m_matlabmexDLL = LoadLibrary( TEXT("libmex.dll") );

		if ( m_matlabmxDLL )
		{
			m_pfmxGetData = (pfmxGetData)GetProcAddress( m_matlabmxDLL, "mxGetData" );
			m_pfmxCreateNumericArray = (pfmxCreateNumericArray)GetProcAddress( m_matlabmxDLL, "mxCreateNumericArray" );
			m_pfmxCreateDoubleMatrix = (pfmxCreateDoubleMatrix)GetProcAddress( m_matlabmxDLL, "mxCreateDoubleMatrix" );

			//m_pfmxDestroyArray = (pfmxDestroyArray)GetProcAddress( m_matlabmxDLL, "mxDestroyArray" );
			//m_pfmexMakeArrayPersistent = (pfmexMakeArrayPersistent)GetProcAddress( m_matlabmexDLL, "mexMakeArrayPersistent" );

			return ( m_pfmxGetData && m_pfmxCreateNumericArray && m_pfmxCreateDoubleMatrix );
			//return ( m_pfmxGetData && m_pfmxCreateNumericArray && m_pfmxCreateDoubleMatrix && m_pfmexMakeArrayPersistent && m_pfmxDestroyArray );
		}
		else
			return false;

	};

	mxArray* mxArrayFromIplImage( const IplImage *img )
	{
		if ( !CV_IS_IMAGE(img) ) return NULL;

		int row, col, ch;

		/*
		if ( !mxarr || nChannel != img->nChannels || nRow != img->height || nCol != img->width )
		{
			if ( mxarr ) m_pfmxDestroyArray( mxarr );

			nRow     = img->height;
			nCol     = img->width;
			nChannel = img->nChannels;
			nDim     = (nChannel > 1) ? 3 : 2;
		
			dims[0] = nRow;
			dims[1] = nCol;
			dims[2] = nChannel;
			classid = cvmxClassIDFromIplDepth( img->depth );
			mxarr   = m_pfmxCreateNumericArray( nDim, dims, classid, mxREAL );

			//m_pfmexMakeArrayPersistent( mxarr );
		}
		*/

		int nDim = (img->nChannels > 1) ? 3 : 2;
		
		mwSize dims[3] = { img->height, img->width, img->nChannels };

		mxClassID classid  = cvmxClassIDFromIplDepth( img->depth );

		mxArray* mxarr    = m_pfmxCreateNumericArray( nDim, dims, classid, mxREAL );
		
		if ( classid == mxUINT8_CLASS )
		{
			unsigned char *mxData = (unsigned char*)m_pfmxGetData(mxarr);
			for (ch = 0; ch < img->nChannels; ch++)
			{
				for (row = 0; row < dims[0]; row++)
				{
					for (col = 0; col < dims[1]; col++)
					{
						mxData[dims[0] * dims[1] * ch + dims[0] * col + row]
							= CV_IMAGE_ELEM(img, unsigned char, row, img->nChannels * col + ch);
					}
				}
			}
		} else if ( classid == mxDOUBLE_CLASS )
		{
			double *mxData = (double*)m_pfmxGetData(mxarr);
			for (ch = 0; ch < img->nChannels; ch++)
			{
				for (row = 0; row < dims[0]; row++)
				{
					for (col = 0; col < dims[1]; col++)
					{
						mxData[dims[0] * dims[1] * ch + dims[0] * col + row]
							= CV_IMAGE_ELEM(img, double, row, img->nChannels * col + ch);
					}
				}
			}
		} else if ( classid == mxSINGLE_CLASS )
		{
			float *mxData = (float*)m_pfmxGetData(mxarr);
			for (ch = 0; ch < img->nChannels; ch++)
			{
				for (row = 0; row < dims[0]; row++)
				{
					for (col = 0; col < dims[1]; col++)
					{
						mxData[dims[0] * dims[1] * ch + dims[0] * col + row]
							= CV_IMAGE_ELEM(img, float, row, img->nChannels * col + ch);
					}
				}
			}
		}

		return mxarr;
	};

	mxArray*  mxArrayFromDouble2DArray( double** dArray2D, const int iRows, const int iCols )
	{
		if ( !dArray2D ) return NULL;

		// create Matlab array
		mxArray* mxarr = m_pfmxCreateDoubleMatrix( iRows, iCols, mxREAL );

		// populate the real part of the created array
		double *mxData = (double*)m_pfmxGetData(mxarr);

		// copy data to matlab array
		for ( int i = 0; i < iRows; i++ )
			for ( int j = 0; j < iCols; j++ )
				mxData[iRows * j + i] = dArray2D[i][j];

		return mxarr;
	};


private:
	static mxClassID cvmxClassIDFromIplDepth( const int depth )
	{
		switch(depth)
		{
			case IPL_DEPTH_1U:
				return mxLOGICAL_CLASS;
			case IPL_DEPTH_8U:
				return mxUINT8_CLASS;
			case IPL_DEPTH_8S:
				return mxINT8_CLASS;
			case IPL_DEPTH_16U:
				return mxUINT16_CLASS;
			case IPL_DEPTH_16S:
				return mxINT16_CLASS;
			case IPL_DEPTH_32F:
				return mxSINGLE_CLASS;
			case IPL_DEPTH_64F:
				return mxDOUBLE_CLASS;
			default:
				return mxUNKNOWN_CLASS; 
		}
	}


private:
	HINSTANCE					m_matlabmxDLL;
	HINSTANCE					m_matlabmexDLL;
	pfmxGetData					m_pfmxGetData;
	pfmxCreateNumericArray		m_pfmxCreateNumericArray;
	pfmxCreateDoubleMatrix		m_pfmxCreateDoubleMatrix;

	//pfmexMakeArrayPersistent	m_pfmexMakeArrayPersistent;
	//pfmxDestroyArray			m_pfmxDestroyArray;

	// current image stats
	//int nChannel;
	//int nRow;
	//int nCol;
	//int nDim;

	//mwSize dims[3];
	//mxClassID classid;
	//static mxArray* mxarr;
};

