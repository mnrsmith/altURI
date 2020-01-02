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
// CARDroneNavDataDecoder.cpp : Decodes ARDrone NavData
//

#include "stdafx.h"
#include "CARDroneNavDataDecoder.h"



/********************************************************************
 * @fn ardrone_navdata_compute_cks:
 * @param nv Data to calculate the checksum.
 * @param size Size of data calculate as follow : size-sizeof(navdata_cks_t).
 * @return Retrieves the checksum from the navdata nv.
 *******************************************************************/
int CARDroneNavDataDecoder :: ComputeChecksum( char* nv, int iSize )
{
	int total = 0;

	for( int i = 0; i < iSize; i++ )
		total += nv[i];

	return total;
}