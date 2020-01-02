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
// CARDroneImageDecoderData.cpp : structs and data for ARDrone Image to OpenCV format conversion
//

#pragma once 

struct RGB24BitsColor 
{
    unsigned char red, green, blue;
};
    
static short zigZagPositions[] =   
{
    0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63,
};
    
static short allzeros[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};
    
//Cfr. Handbook of Data Compression - Page 529
//David Salomon
//Giovanni Motta
    
static short quantizerValues[] = 
{  
    3,  5,  7,  9, 11, 13, 15, 17,
    5,  7,  9, 11, 13, 15, 17, 19,
    7,  9, 11, 13, 15, 17, 19, 21,
    9, 11, 13, 15, 17, 19, 21, 23,
    11, 13, 15, 17, 19, 21, 23, 25,
    13, 15, 17, 19, 21, 23, 25, 27,
    15, 17, 19, 21, 23, 25, 27, 29,
    17, 19, 21, 23, 25, 27, 29, 31
};
    
static unsigned char clzlut[] = 
{ 
    8,7,6,6,5,5,5,5, 
    4,4,4,4,4,4,4,4, 
    3,3,3,3,3,3,3,3, 
    3,3,3,3,3,3,3,3, 
    2,2,2,2,2,2,2,2, 
    2,2,2,2,2,2,2,2, 
    2,2,2,2,2,2,2,2, 
    2,2,2,2,2,2,2,2, 
    1,1,1,1,1,1,1,1, 
    1,1,1,1,1,1,1,1, 
    1,1,1,1,1,1,1,1, 
    1,1,1,1,1,1,1,1, 
    1,1,1,1,1,1,1,1, 
    1,1,1,1,1,1,1,1, 
    1,1,1,1,1,1,1,1, 
    1,1,1,1,1,1,1,1, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0 
};

struct MacroBlock
{
    short DataBlocks[8][64]; 
}; // MacroBlock
    
struct ImageSlice
{
	std::vector< MacroBlock > MacroBlocks;
      
	void setSize( int macroBlockCount )
	{
		MacroBlocks.resize( macroBlockCount );
	}
      
}; //ImageSlice