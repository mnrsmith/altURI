/*
Copyright (c) 2010, Mark N R Smith, All rights reserved.

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
//	CDLLStart.h - simple thread class
//=========================================================================
#pragma once

#include "CThread.h"

extern LIBSPEC CDllState	g_DllState;
extern CServerManager		g_Server;

//=========================================================================
class CDLLStart : public CThread
{
	protected:
    virtual unsigned int ThreadProc()
    {
        // void *pData = GetData();

		if ( g_DllState.g_Graphics_Type == CDllState::NONE )
		{
			if ( g_DX11.AttachGraphXMode() == S_OK )
			{
				g_DllState.g_Graphics_Type = CDllState :: DX11;
			}
			else if ( g_DX10.AttachGraphXMode() == S_OK )
			{
				g_DllState.g_Graphics_Type = CDllState :: DX10;
			}
			else if ( g_DX9.AttachGraphXMode() == S_OK )
			{
				g_DllState.g_Graphics_Type = CDllState :: DX9;
			}
			else if ( g_DX8.AttachGraphXMode() == S_OK )
			{
				g_DllState.g_Graphics_Type = CDllState :: DX8;
			}
			else if ( g_OGL.AttachGraphXMode() == S_OK )
			{
				g_DllState.g_Graphics_Type = CDllState :: OGL;
			}

			if ( g_DllState.g_Graphics_Type != CDllState::NONE )
				g_Server.StartServer( 80 );
		}
        
        return 0;
    }
};
