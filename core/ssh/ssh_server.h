/*©mit**************************************************************************
*                                                                              *
* This file is part of FRIEND UNIFYING PLATFORM.                               *
* Copyright 2014-2017 Friend Software Labs AS                                  *
*                                                                              *
* Permission is hereby granted, free of charge, to any person obtaining a copy *
* of this software and associated documentation files (the "Software"), to     *
* deal in the Software without restriction, including without limitation the   *
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or  *
* sell copies of the Software, and to permit persons to whom the Software is   *
* furnished to do so, subject to the following conditions:                     *
*                                                                              *
* The above copyright notice and this permission notice shall be included in   *
* all copies or substantial portions of the Software.                          *
*                                                                              *
* This program is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
* MIT License for more details.                                                *
*                                                                              *
*****************************************************************************©*/

/** @file
 * 
 *  SSH server definition
 *
 *  @author PS (Pawel Stefanski)
 *  @date created 2015
 */

#ifndef __SSH_SSH_SERVER_H__
#define __SSH_SSH_SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <core/types.h>
#include <core/thread.h>
#include <network/socket.h>
//#include <network/sshsocket.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>

#include <libssh/libssh.h>
#include <system/systembase.h>
#include <system/auth/authmodule.h>

typedef struct SSHServer
{
	FThread 		*sshs_Thread;
	char				*sshs_FriendHome;
	char 			*sshs_RSAKeyHome;
	char				*sshs_DSAKeyHome;

	FBOOL 		sshs_Quit;
	void				*sshs_SB;
	//SSHSocket		*sshs_Socket;		// socket
	//int 			sshs_Epollfd;		// EPOLL - file descriptor
}SSHServer;

//
// SSH session for user
//

typedef struct SSHSession		// single session structure
{
	int					sshs_Authenticated;		// is user authenticated
	int 					sshs_Tries;				// check user login times
	int 					sshs_Error;				// error
	ssh_channel 		sshs_Chan;				// session channel
	ssh_session 		sshs_Session;			// session
	User 				*sshs_Usr;				// logged user
	char 				*sshs_DispText;			// display text for user
	char 				*sshs_Path;				// user path
	FBOOL 			sshs_Quit;				// session quit
	void					*sshs_SB;			//SystemBase
}SSHSession;


#define SSH_SERVER_PORT	"6505"

//
//
//

SSHServer *SSHServerNew( void *sb );

//
//
//

void SSHServerDelete( SSHServer *ts );

//
//
//

int SSHThread( FThread *ptr );

#endif //__SSH_SSH_SERVER_H__