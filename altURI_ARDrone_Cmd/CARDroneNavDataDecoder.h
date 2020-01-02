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
// CARDroneNavDataDecoder.h : Decodes ARDrone NavData
//

#pragma once 

#pragma pack(1)

typedef struct _navdata_option_t {
	unsigned short	tag;
	unsigned short	size;
	char			*data;
} navdata_option_t;

/**
 * @brief Navdata structure sent over the network.
 */
typedef struct _navdata_t {
  unsigned int	header;				/*!< Always set to NAVDATA_HEADER */
  unsigned int	ardrone_state;		/*!< Bit mask built from def_ardrone_state_mask_t */
  unsigned int	sequence;			/*!< Sequence number, incremented for each sent packet */
  int			vision_defined;

  navdata_option_t  options[1];
} navdata_t;


/**
 * @struct _navdata_unpacked_t
 * @brief Decoded navigation data.
*/
typedef struct _navdata_unpacked_t
{
  unsigned int	ardrone_state;
  int			vision_defined;
  unsigned int	last_navdata_refresh;  /*! mask showing which block was refreshed when receiving navdata */

#define NAVDATA_OPTION_DEMO(STRUCTURE,NAME,TAG)  STRUCTURE NAME ;
#define NAVDATA_OPTION(STRUCTURE,NAME,TAG)       STRUCTURE NAME ;
#define NAVDATA_OPTION_CKS(STRUCTURE,NAME,TAG)

#include "navdata_keys.h"

} navdata_unpacked_t;

class CARDroneNavDataDecoder
{
public:
	CARDroneNavDataDecoder() { };
	~CARDroneNavDataDecoder(){ };
	
public: 
	bool decodeNavData( unsigned char* stream, unsigned int streamLength );

private:

	int ComputeChecksum( char* nv, int iSize );



};