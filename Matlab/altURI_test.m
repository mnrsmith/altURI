%
% Copyright (c) 2010, Mark N R Smith, All rights reserved.
%
% Redistribution and use in source and binary forms, with or without modification,
% are permitted provided that the following conditions are met:

% Redistributions of source code must retain the above copyright notice, this list
% of conditions and the following disclaimer. Redistributions in binary form must
% reproduce the above copyright notice, this list of conditions and the following
% disclaimer in the documentation and/or other materials provided with the
% distribution. Neither the name of the author nor the names of its contributors
% may be used to endorse or promote products derived from this software without
% specific prior written permission.
%
% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
% ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
% WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
% DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
% FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
% DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
% SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
% CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
% OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
% OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
%

clc
pause on

if libisloaded( 'altURI' )
    unloadlibrary 'altURI'
end

addpath( 'D:\Visual Studio 2008\Projects\altURI\Release' )
addpath( 'D:\Visual Studio 2008\Projects\altURI\altURI_Cmd' )

[notfound,warnings] = loadlibrary( 'altURI_Cmd', 'altURI_Cmd.h', 'alias', 'altURI' );

%libfunctionsview altURI

result = calllib( 'altURI', 'SetupControl', 'D:\Visual Studio 2008\Projects\altURI\altURI_Cmd\airrobot.ini' );

result = calllib( 'altURI', 'SetupImages' );

% get this client IP address
sIP = calllib( 'altURI', 'GetClientIP' );

% set the robots name
sRobotName = 'altURI_Matlab_R1';

% create a robot
sInitRobot = [ 'INIT {ClassName USARBot.AirRobot} {Name ' sRobotName '} {Location 1.2,2.5,1.8}' ];

result = calllib( 'altURI', 'SendRobot', sInitRobot );

pause( 1 )

result = calllib( 'altURI', 'SetCatchMessages', true )

pause( 2 )

sensormessages = calllib( 'altURI', 'MatlabGetMessagesArray', 50, 0 )

% set camera to robot view
sSetCamera = [ 'SET {Type Camera} {Robot ' sRobotName '} {Client ' sIP '}' ];

result = calllib( 'altURI', 'SendRobot', sSetCamera  );

pause( 5 )

result = calllib( 'altURI', 'CommandDrive', 1, 10 );

pause( 5 )

result = calllib( 'altURI', 'CommandDriveAllStop' );

dummy = 0

robotimage = calllib( 'altURI', 'MatlabGetImage', dummy, dummy, dummy, dummy );

imshow( robotimage )


pause( 10 )

calllib( 'altURI', 'CloseImages' )

calllib( 'altURI', 'CloseControl' )


unloadlibrary 'altURI'