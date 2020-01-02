/*

Copyright (C) 2007-2011, PARROT SA, all rights reserved.

DISCLAIMER
The APIs is provided by PARROT and contributors "AS IS" and any express or implied warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose are disclaimed. In no event shall PARROT and
contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not
limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however
caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise)
arising in any way out of the use of this software, even if advised of the possibility of such damage.  

*/
//
// CARDroneImageDecoder.cpp : Converts ARDrone image stream to OpenCV format
//

// Thanks to:	Pongsak Suvanpong psksvp@robotvision2.com psksvp@gmail.com
//				JavaDrone http://code.google.com/p/javadrone/
//				droneController http://dronecontroller.codeplex.com/

#pragma once 

#include "CARDroneImageDecoderData.h"

class CARDroneImageDecoder
{
public:
	CARDroneImageDecoder() { RGB24OutputPixelData = NULL; InitializeCriticalSection( &m_cs ); m_lFrameRequired = 0; };
	~CARDroneImageDecoder(){ DeleteCriticalSection( &m_cs ); };

	bool decodeImage( unsigned char* stream, unsigned int streamLength );

	IplImage* GetIplImage( const int width, const int height, const int x, const int y );

private:
	void InverseTransform( int macroBlockIndex, int dataBlockIndex );
	int CountLeadingZeros( unsigned int value );

	inline unsigned short MakeRgb( int r, int g, int b ){ return (unsigned short)((r << 11) | (g << 5) | b); };
	inline int Saturate5( int x ) { if (x < 0) x = 0; x >>= 11; return (x > 0x1F) ? 0x1F : x; };
	inline int Saturate6( int x ){ if (x < 0) x = 0; x >>= 10; return x > 0x3F ? 0x3F : x; };

	void ComposeImageSlice();
	void AlignStreamData();
	inline int makeIntFromBytes( unsigned char* buffer, int index );
	unsigned int PeekStreamData( unsigned char* stream, int count );
	unsigned int ReadStreamData( int count );
	void DecodeFieldBytes( int& run, int& level, bool& last );

	void GetBlockBytes( bool acCoefficientsAvailable );
	void ReadHeader( void );
	void ProcessStream();

	inline void Lock() { EnterCriticalSection( &m_cs ); };
	inline void Unlock() { LeaveCriticalSection( &m_cs ); };

	cv::Mat& GetResizedMatCopy( const cv::Mat& matIn, const int width, const int height, const int x, const int y );

	IplImage* GetNewIplImageWhenRequired( IplImage* iplOld, const int width, const int height );

private:
	static const int kPictureFormatCIF = 1;
	static const int kPictureFormatVGA = 2;
    
	static const int CONST_BlockWidth = 8;
	static const int CONST_BlockSize = 64;
    
	static const int CONST_WidthCif = 88;
	static const int CONST_HeightCif = 72;
    
	static const int CONST_WidthVga = 160;
	static const int CONST_HeightVga = 120;
    
	static const int CONST_TableQuantization = 31;
    
	static const int FIX_0_298631336 = 2446;
	static const int FIX_0_390180644 = 3196;
	static const int FIX_0_541196100 = 4433;
	static const int FIX_0_765366865 = 6270;
	static const int FIX_0_899976223 = 7373;
	static const int FIX_1_175875602 = 9633;
	static const int FIX_1_501321110 = 12299;
	static const int FIX_1_847759065 = 15137;
	static const int FIX_1_961570560 = 16069;
	static const int FIX_2_053119869 = 16819;
	static const int FIX_2_562915447 = 20995;
	static const int FIX_3_072711026 = 25172;
    
	static const int CONST_BITS = 13;
	static const int PASS1_BITS = 1;
	static const int F1 = CONST_BITS - PASS1_BITS - 1;
	static const int F2 = CONST_BITS - PASS1_BITS;
	static const int F3 = CONST_BITS + PASS1_BITS + 3;

	RGB24BitsColor* RGB24OutputPixelData;
	short dataBlockBuffer[64]; 
	
    unsigned int StreamField;
    int StreamFieldBitIndex;
    int StreamIndex;
    
    bool PictureComplete;
    
    int PictureFormat;
    int Resolution;
    int PictureType;
    int QuantizerMode;
    int FrameIndex;
    
    int SliceCount;
    int SliceIndex;
    
    int BlockCount;
    
    int Width;
    int Height;
    
    int PixelRowSize;
    
    unsigned char* ImageStream;
    unsigned int ImageStreamLength;
    
    std::vector<unsigned short> PixelData;
	//std::vector<char> PixelData;

     
    ImageSlice AnImageSlice;

	CRITICAL_SECTION	m_cs;

	cv::Mat				m_matDroneImage;

	cv::Mat				m_matDroneImageResized;

	long				m_lFrameRequired;	// A frame has been requested 

	cv::Mat				m_matTemp;

	IplImage			m_Ipl_Resized;


};