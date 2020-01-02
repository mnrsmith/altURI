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
#pragma once 

#include <fstream>
#include <iostream>

typedef uchar v3b[3];


// some support for the EMD parameters
struct EMDPARAMSTATS
{
	double	dStart;
	double	dEnd;
	int		iSteps;
	int		iRepeats;
};

class CEMDParamsStats
{
public:
	EMDPARAMSTATS sParamsStats[10];
};

// class for current EMD Param values that change according to above ParamStats value
class CEMDParams {

public:
	CEMDParams(){ };
	~CEMDParams(){ };
public:
		double EMD_Param_A1;	// 1 & 2 channel HPF
		double EMD_Param_A2;	// 1 & 2 channel HPF
		double EMD_Param_B1;	// 1 & 2 channel LPF
		double EMD_Param_B2;	// 1 & 2 channel LPF
		double EMD_Param_B3;	// 2 channel only LPF
		double EMD_Param_B4;	// 2 channel only LPF
		double EMD_Param_C1;	// 1 & 2 channel PREV EMD
		double EMD_Param_D1;	// 2 channel only HW+ channel 1
		double EMD_Param_D2;	// 2 channel only HW- channel 1 
		double EMD_Param_D3;	// 2 channel only HW+ channel 2
		double EMD_Param_D4;	// 2 channel only HW- channel 2
} ;


// EMD main classes
class CEMD
{
public:

	CEMD(){ SetEMDConstants(); m_llCounterThen = 0; };
	~CEMD(){ if ( m_fout.is_open() ) m_fout.close(); };

	IplImage* MakeEMD( const IplImage* iplImageIn );	

	IplImage* MakeEMD2( const IplImage* iplImageIn );	

	IplImage* MakeEMD3( const IplImage* iplImageIn );	

	IplImage* MakeEMD4( const IplImage* iplImageIn );	

	IplImage* MakeEMD5( const IplImage* iplImageIn );	

	IplImage* MakeEMD6( const IplImage* iplImageIn );	

	IplImage* MakeEMD7( const IplImage* iplImageIn );

	IplImage* MakeEMD8( const IplImage* iplImageIn );

	IplImage* MakeEMD9( const IplImage* iplImageIn );

	IplImage* Make2ChannelEMD( const IplImage* iplImageIn );

	IplImage* MakeEMDRegions( const IplImage* iplImageIn, double dHorizontal, double dVertical );

	IplImage* MakeEMDLog( const IplImage* iplImageIn, double dHorizontal, double dVertical );

	void SetEMDConstants( const double a = CEMD::A , const double b = CEMD::B ){ m_CurrentA = a; m_CurrentOneMinusA = 1. - a; m_CurrentB = b; m_CurrentOneMinusB = 1. - b; };

public:
	static const int OFFSET = 1;
	static const double A;
	static const double B;

private:

	inline void CalculateAndStoreHPFnLPF( size_t stSourceAddress, size_t stDestAddress );

	inline void CalculateAndStoreHPFnLPF2( uchar* stSourceAddress, uchar* stDestAddress_matGrayImagePrev, uchar* stDestAddress_matHPF, uchar* stDestAddress_matLPF, uchar* stDestAddress_matHPFPrev );

	inline void CalculateAndStoreHPFnLPF3( const v3b stSource, uchar& stDest_matGrayImagePrev, uchar& stDest_matHPF, uchar& stDest_matLPF, uchar& stDest_matHPFPrev );

	inline void CalculateAndStoreHPFnLPF4( const v3b stSource, uchar& stDest_matGrayImagePrev, ushort& stDest_matHPF, ushort& stDest_matLPF, ushort& stDest_matHPFPrev );

	inline void CalculateAndStoreHPFnLPF2Channel( const v3b stSource, uchar& stDest_matGrayImagePrev, ushort& stDest_matHPF_POS, ushort& stDest_matHPF_NEG, ushort& stDest_matLPF_POS, ushort& stDest_matLPF_NEG, ushort& stDest_matHPFPrev );

	inline void AllocateImageMats( const int rows, const int cols, const int orgcode, const int newcode);

	inline void CEMD :: AverageAndLabelRegion( cv::Mat matLabelledImage, cv::Mat matRegionX, cv::Mat matRegionY, int iColSlotWidth, int iRowSlotHeight, int iFontFace, double dFontScale, cv::Scalar cvSColour, int iThickness );

	//inline std::string sDoubleToString( const double d ){ std::stringstream s; if ( (s << std::setprecision(2) << std::setiosflags(std::ios_base::fixed) << d) ) return s.str(); else return ""; };
	inline std::string sDoubleToString( const double d ){ std::stringstream s; if ( (s << d) ) return s.str(); else return ""; };
	inline std::string sIntToString( const int i ){ std::stringstream s; if ( (s << i) ) return s.str(); else return ""; };
	inline std::string sLLToString( const LONGLONG ll ){ std::stringstream s; if ( (s << ll) ) return s.str(); else return ""; };
	inline std::string susToString( const ushort us ){ std::stringstream s; if ( (s << us) ) return s.str(); else return ""; };

	inline LONGLONG GetFrameTicks( void ){ // HiRes Timer

		LARGE_INTEGER liNow;
		DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
		::QueryPerformanceCounter( &liNow );
		::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
		LONGLONG llTicks = liNow.QuadPart - m_llCounterThen;
		m_llCounterThen = liNow.QuadPart;

		return llTicks;
	}

public:
	std::string GetFileDateTimeString( void )
	{
		std::string DELIM = TEXT("_");

		time_t t = time( NULL );

		if (t == -1) return "";

		struct tm *tmp;
		tmp = localtime( &t );

		if (tmp == NULL) return "";

		return	sIntToString( tmp->tm_mday + 1 ) +
				DELIM + sIntToString( tmp->tm_mon + 1 ) +
				DELIM + sIntToString( tmp->tm_year + 1900 ) +
				DELIM + sIntToString( tmp->tm_hour ) +
				DELIM + sIntToString( tmp->tm_min ) +
				DELIM + sIntToString( tmp->tm_sec );
	}

	void WriteLogLines( const std::string sFilename ){
		
		// open new file
		if ( m_fout.is_open() )
		{
			// protect vector unloading process
			EnterCriticalSection( &m_Logcs );
			{
				// iterate thru vector writing lines
				for(std::vector<std::string>::iterator iter = m_vLogLines.begin(); iter != m_vLogLines.end(); iter++)
					m_fout << iter->c_str() << std::endl;

				// delete the lines
				m_vLogLines.clear();
			}
			LeaveCriticalSection( &m_Logcs );

			// if new filename close current
			if (sFilename.compare( m_sLogFileName ) != 0 )
			{
				m_fout.close();
				m_sLogFileName = sFilename;

				// open new file ready for next call
				m_fout.open( m_sLogFileName.c_str(), std::ios::out );
			}
		}
		else
			// open ready for next call
			m_fout.open( m_sLogFileName.c_str(), std::ios::out );
	}

private:
	// logging vars
	std::vector<std::string>	m_vLogLines;
	std::fstream				m_fout;
	std::string					m_sLogFileName;
	LONGLONG					m_llCounterThen;
	CRITICAL_SECTION			m_Logcs;

	static const double RED;
	static const double GREEN;
	static const double BLUE;

	double			m_CurrentA;
	double			m_CurrentOneMinusA;
	double			m_CurrentB;
	double			m_CurrentOneMinusB;

	IplImage*		m_ImagePrevData;

	// 
	IplImage*		m_ImageTemp;
	IplImage*		m_ImagePrev;
	IplImage*		m_ImageHPFPrev;
	IplImage*		m_ImageEMDPrev;

	cv::Range		m_rcolShiftX;
	cv::Range		m_rcolNormalX;
	cv::Range		m_rrowShiftY;
	cv::Range		m_rrowNormalY;
	cv::Range		m_rAll;

	cv::Mat			m_matImage;
	cv::Mat			m_matGrayInitImage;
	cv::Mat			m_matGrayImage;
	cv::Mat			m_matGrayImagePrev;
	cv::Mat			m_matHPF;
	cv::Mat			m_matLPF;
	cv::Mat			m_matHPFNEG;
	cv::Mat			m_matLPFNEG;
	cv::Mat			m_matHPFPrev;
	cv::Mat			m_matEMDX;
	cv::Mat			m_matEMDY;
	cv::Mat			m_matEMDXYSquared;
	cv::Mat			m_matEMD;
	cv::Mat			m_matEMDColour;
	cv::Mat			m_matEMDPrev;
	cv::Mat			m_matEMDRGB;
	cv::Mat			m_matEMDRGB_ORG;
	IplImage		m_ImageEMD;

	size_t			m_stSourceStep;
	size_t			m_stSourceElementSize;
	size_t			m_stDestStep;
	size_t			m_stDestElementSize;

	// latest
	const v3b* m_ucp_matImageRow;
	const v3b* m_ucp_matImageRowPlus;

	uchar* m_ucp_matGreyImagePrevRow;
	uchar* m_ucp_matGreyImagePrevRowPlus;

	uchar* m_ucp_matHPFRow;
	uchar* m_ucp_matHPFRowPlus;

	uchar* m_ucp_matHPFPrevRow;
	uchar* m_ucp_matHPFPrevRowPlus;

	uchar* m_ucp_matLPFRow;
	uchar* m_ucp_matLPFRowPlus;

	uchar* m_ucp_matEMDRow;
	v3b* m_ucp_matEMDColourRow;
	uchar* m_ucp_matEMDPrevRow;

	uchar* m_ucp_matEMDX;
	uchar* m_ucp_matEMDY;

	uchar m_uxEMD, m_uyEMD;
	double m_dxEMD, m_dyEMD;
	double m_dEMD, m_dEMDTheta;

	//short* m_sp_matGreyImagePrevRow;
	//short* m_sp_matGreyImagePrevRowPlus;

	ushort* m_sp_matHPFRow;
	ushort* m_sp_matHPFRowNEG;
	ushort* m_sp_matHPFRowPlus;
	ushort* m_sp_matHPFRowPlusNEG;

	ushort* m_sp_matLPFRow;
	ushort* m_sp_matLPFRowNEG;
	ushort* m_sp_matLPFRowPlus;
	ushort* m_sp_matLPFRowPlusNEG;

	ushort* m_sp_matHPFPrevRow;
	ushort* m_sp_matHPFPrevRowPlus;

	ushort* m_sp_matEMDX;
	ushort* m_sp_matEMDY;
	cv::Mat m_matEMDXPrev;
	cv::Mat m_matEMDYPrev;
	ushort* m_sp_matEMDXPrev;
	ushort* m_sp_matEMDYPrev;
};