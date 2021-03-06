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
 *  Admin Web body
 *
 * All functions related to Remote User structure
 *
 *  @author PS (Pawel Stefanski)
 *  @date created 30/05/2017
 */

#include <core/types.h>
#include <core/nodes.h>
#include "admin_web.h"

#include <system/handler/device_handling.h>
#include <core/functions.h>
#include <util/md5.h>
#include <network/digcalc.h>
#include <network/mime.h>
#include <system/invar/invar_manager.h>
#include <system/application/applicationweb.h>
#include <system/user/user_manager.h>
#include <system/handler/fs_manager.h>
#include <system/handler/fs_manager_web.h>
#include <system/handler/fs_remote_manager_web.h>
#include <core/pid_thread_web.h>
#include <system/handler/device_manager_web.h>
#include <network/mime.h>
#include <hardware/usb/usb_device_web.h>
#include <system/handler/door_notification.h>

/**
 * Network handler
 *
 * @param m pointer to SystemBase
 * @param urlpath pointer to table with path entries
 * @param request http request
 * @param loggedSession user session
 * @param pointer to integer where error number will be returned
 * @return http response
 */
Http *AdminWebRequest( void *m, char **urlpath, Http **request, UserSession *loggedSession, int *result )
{
	SystemBase *l = (SystemBase *)m;
	Http *response = NULL;
	
	char *path = NULL;
	DEBUG("ADMIN\n");
	
	struct TagItem tags[] = {
		{ HTTP_HEADER_CONTENT_TYPE, (FULONG)StringDuplicateN( "text/html", 9 ) },
		{	HTTP_HEADER_CONNECTION, (FULONG)StringDuplicateN( "close", 5 ) },
		{TAG_DONE, TAG_DONE}
	};
	
	if( response != NULL )
	{
		FERROR("RESPONSE admin\n");
		HttpFree( response );
	}
	response = HttpNewSimple( HTTP_200_OK, tags );
	
	if( urlpath[ 1 ] == NULL )
	{
		DEBUG( "URL path is NULL!\n" );
		HttpAddTextContent( response, "ok<!--separate-->{\"response\":\"second part of url is null!\"}" );
		
		goto error;
		//return response;
	}
	
	//
	// Get FC information
	//
	
	if( strcmp( urlpath[ 1 ], "info" ) == 0 )
	{
		BufString *bs = NULL;
		
		FriendCoreManager *fcm = (FriendCoreManager *) l->fcm;
		
		if( fcm->fcm_FCI != NULL )
		{
			bs = FriendCoreInfoGet( fcm->fcm_FCI );
			
			HttpAddTextContent( response, bs->bs_Buffer );
			*result = 200;
			
			BufStringDelete( bs );
		}
		else
		{
			HttpAddTextContent( response, "fail<!--separate-->{\"response\":\"cannot get information from FriendCoreInfo.\"}" );
		}
	}
	
	//
	// Get information about connections
	//
	
	else if( strcmp( urlpath[ 1 ], "listcores" ) == 0 )
	{
		FBOOL uiadmin = FALSE;
		char temp[ 1024 ];
		int pos = 0;
		
		if( UMUserIsAdmin( l->sl_UM, (*request), loggedSession->us_User ) == TRUE )
		{
			BufString *bs = BufStringNew();
			
			BufStringAddSize( bs, "ok<!--separate-->[", 18 );
			
			// add other FC connections
			
			CommFCConnection *actCon = l->fcm->fcm_CommService->s_Connections;
			while( actCon != NULL )
			{
				int size;
				
				if( pos == 0 )
				{
					size = snprintf( temp, sizeof(temp), "{\"name\":\"%s\",\"id\":\"%s\",\"host\":\"%s\",\"type\":\"fcnode\"}", actCon->cfcc_Name, actCon->cffc_ID, actCon->cfcc_Address );
				}
				else
				{
					size = snprintf( temp, sizeof(temp), ",{\"name\":\"%s\",\"id\":\"%s\",\"host\":\"%s\",\"type\":\"fcnode\"}", actCon->cfcc_Name, actCon->cffc_ID, actCon->cfcc_Address );
				}
				
				BufStringAddSize( bs, temp, size );
				
				actCon = (CommFCConnection *)actCon->node.mln_Succ;
				pos++;
			}
			
			BufStringAddSize( bs, "]", 1 );
			
			HttpSetContent( response, bs->bs_Buffer, bs->bs_Size );
			bs->bs_Buffer = NULL;
			
			BufStringDelete( bs );
		}
		else
		{
			HttpAddTextContent( response, "fail<!--separate-->{\"result\":\"User dont have access to functionality\"}" );
		}
		
		*result = 200;
	}
	
	//
	// Get information about connections
	//
	
	else if( strcmp( urlpath[ 1 ], "connectionsinfo" ) == 0 )
	{
		BufString *bs = BufStringNew();
		FBOOL uiadmin = FALSE;
		char temp[ 1024 ];
		int pos = 0;
		
		BufStringAddSize( bs, "ok<!--separate-->[", 18 );
		
		if( UMUserIsAdmin( l->sl_UM, (*request), loggedSession->us_User ) == TRUE )
		{
			uiadmin = TRUE;
		}
		
		// add remote connections
		
		RemoteUser *ru = l->sl_UM->um_RemoteUsers;
		
		if( uiadmin == TRUE )
		{
			while( ru != NULL )
			{
				int size;
				
				if( pos == 0 )
				{
					size = snprintf( temp, sizeof(temp), "{\"name\":\"%s\",\"id\":\"%s\",\"host\":\"%s\",\"type\":\"userremote\"}", ru->ru_Name, ru->ru_SessionID, ru->ru_Host );
				}
				else
				{
					size = snprintf( temp, sizeof(temp), ",{\"name\":\"%s\",\"id\":\"%s\",\"host\":\"%s\",\"type\":\"userremote\"}", ru->ru_Name, ru->ru_SessionID, ru->ru_Host );
				}
				
				BufStringAddSize( bs, temp, size );
			
				ru = (RemoteUser *)ru->node.mln_Succ;
				pos++;
			}
			
			// add other FC connections
			
			CommFCConnection *actCon = l->fcm->fcm_CommService->s_Connections;
			while( actCon != NULL )
			{
				int size;
				
				if( pos == 0 )
				{
					size = snprintf( temp, sizeof(temp), "{\"name\":\"%s\",\"id\":\"%s\",\"host\":\"%s\",\"type\":\"fcnode\"}", actCon->cfcc_Name, actCon->cffc_ID, actCon->cfcc_Address );
				}
				else
				{
					size = snprintf( temp, sizeof(temp), ",{\"name\":\"%s\",\"id\":\"%s\",\"host\":\"%s\",\"type\":\"fcnode\"}", actCon->cfcc_Name, actCon->cffc_ID, actCon->cfcc_Address );
				}
				
				BufStringAddSize( bs, temp, size );
				
				actCon = (CommFCConnection *)actCon->node.mln_Succ;
				pos++;
			}
		}
		else		// user is not admin, we are getting information only for current user
		{
			// add remote connections
			
			while( ru != NULL )
			{
				if( strcmp( ru->ru_Name, loggedSession->us_User->u_Name ) == 0 )
				{
					int size;
					
					if( pos == 0 )
					{
						size = snprintf( temp, sizeof(temp), "{\"name\":\"%s\",\"id\":\"%s\",\"host\":\"%s\",\"type\":\"userremote\"}", ru->ru_Name, ru->ru_SessionID, ru->ru_Host );
					}
					else
					{
						size = snprintf( temp, sizeof(temp), ",{\"name\":\"%s\",\"id\":\"%s\",\"host\":\"%s\",\"type\":\"userremote\"}", ru->ru_Name, ru->ru_SessionID, ru->ru_Host );
					}
				}
				ru = (RemoteUser *)ru->node.mln_Succ;
				pos++;
			}
		}
		
		BufStringAddSize( bs, "]", 1 );
		
		HttpSetContent( response, bs->bs_Buffer, bs->bs_Size );
		bs->bs_Buffer = NULL;
		*result = 200;
		
		BufStringDelete( bs );
	}
	
	//
	// Send command to remote server
	//
	
	else if( strcmp( urlpath[ 1 ], "remotecommand" ) == 0 )
	{
		HashmapElement *el = NULL;
		char *host = NULL;
		char *remsession = NULL;
		
		el =  HashmapGet( (*request)->parsedPostContent, "remotehost" );
		if( el != NULL )
		{
			host = UrlDecodeToMem( ( char *)el->data );
		}
		
		el =  HashmapGet( (*request)->parsedPostContent, "remotesessionid" );
		if( el != NULL )
		{
			remsession = UrlDecodeToMem( ( char *)el->data );
		}
		
		if( host != NULL )
		{
			// deviceid , appname
			// overwrite sessionid
			
			HashmapPut( (*request)->parsedPostContent, StringDuplicate("sessionid"), remsession );
			
			DataForm *df = DataFormFromHttp( *request );
			if( df != NULL )
			{
				DEBUG( "Connect to server rhost %s\n", host );
				CommFCConnection *mycon = ConnectToServer( l->fcm->fcm_CommService, host );
				if( mycon != NULL )
				{
					CommServiceRegisterEvent( mycon, mycon->cfcc_Socket );
					DataForm *recvdf = CommServiceSendMsgDirect( mycon, df );
					if( recvdf != NULL )
					{
						DEBUG( "Remote command, data received size %lu\n", recvdf->df_Size );
						int allocsize = recvdf->df_Size - ((COMM_MSG_HEADER_SIZE<<1)+COMM_MSG_HEADER_SIZE);
						char *resppointer = FCalloc( allocsize, sizeof(FBYTE) );
						if( resppointer != NULL )
						{
							memcpy( resppointer, ( (char *)recvdf + ((COMM_MSG_HEADER_SIZE<<1)+COMM_MSG_HEADER_SIZE) ), allocsize );
							DEBUG( "Setting response %s\n", resppointer );
							
							HttpSetContent( response, resppointer, allocsize );
						}
						else
						{
							HttpAddTextContent( response, "fail<!--separate-->{\"response\":\"cannot allocate memory for response!\"}" );
						}
						DataFormDelete( recvdf );
					}
					else
					{
						HttpAddTextContent( response, "fail<!--separate-->{\"response\":\"response is empty!\"}" );
					}
				}
				else
				{
					char temp[ 512 ];
					snprintf( temp, sizeof(temp), "ok<!--separate-->{\"response\":\"cannot setup connection with server!: %s\"}", host );
					
					HttpAddTextContent( response, temp );
				}
				
				DataFormDelete( df );
			}
			else
			{
				HttpAddTextContent( response, "fail<!--separate-->{\"response\":\"cannot convert message!\"}" );
			}
			FFree( host );
		}
		else
		{
			HttpAddTextContent( response, "fail<!--separate-->{\"response\":\"'remotehost' parameter not found!\"}" );
		}
		/*
		 * if( remsession != NULL )
		 * {
		 *	FFree( remsession );
	}*/
	}
	
	//
	// send message to all sessions
	//
	
	else if( strcmp( urlpath[ 1 ], "servermessage" ) == 0 )
	{
		HashmapElement *el = NULL;
		char *msg = NULL;
		
		el =  HashmapGet( (*request)->parsedPostContent, "message" );
		if( el != NULL )
		{
			msg = UrlDecodeToMem( ( char *)el->data );
		}
		
		if( UMUserIsAdmin( l->sl_UM, (*request), loggedSession->us_User ) == TRUE )
		{
			DEBUG("Get active sessions\n");
			
			BufString *bs = BufStringNew();
			
			BufStringAdd( bs, "{\"userlist\":[");
			
			// we are going through users and their sessions
			// if session is active then its returned
			
			time_t  timestamp = time( NULL );
			
			int msgsndsize = 0; 
			int pos = 0;
			User *usr = l->sl_UM->um_Users;
			while( usr != NULL )
			{
				DEBUG("Going through users, user: %s\n", usr->u_Name );
				
				UserSessList  *usl = usr->u_SessionsList;
				while( usl != NULL )
				{
					UserSession *locses = (UserSession *)usl->us;
					if( locses != NULL )
					{
						DEBUG("Going through sessions, device: %s\n", locses->us_DeviceIdentity );
						
						if( ( (timestamp - locses->us_LoggedTime) < LOGOUT_TIME ) )
						{
							char tmp[ 512 ];
							int tmpsize = 0;
							
							//DEBUG("Active session found for user %s - deviceidentity %s\n", usr->u_Name, locses->us_DeviceIdentity );
							
							if( pos == 0 )
							{
								tmpsize = snprintf( tmp, sizeof(tmp), "{\"username\":\"%s\", \"deviceidentity\":\"%s\"}", usr->u_Name, locses->us_DeviceIdentity );
							}
							else
							{
								tmpsize = snprintf( tmp, sizeof(tmp), ",{\"username\":\"%s\", \"deviceidentity\":\"%s\"}", usr->u_Name, locses->us_DeviceIdentity );
							}
							
							char tmpmsg[ 2048 ];
							int lenmsg = sprintf( tmpmsg, "{\"type\":\"msg\",\"data\":{\"type\":\"server-notice\",\"data\":{\"username\":\"%s\",\"message\":\"%s\"}}}", 
												  loggedSession->us_User->u_Name , msg );
							//TODO add application name
							
							
							msgsndsize += WebSocketSendMessageInt( locses, tmpmsg, lenmsg );
							//int err = AppSessionSendPureMessage( as, loggedSession, tmp, len );
							
							BufStringAddSize( bs, tmp, tmpsize );
							
							pos++;
						}
					}
					usl = (UserSessList *)usl->node.mln_Succ;
				}
				usr = (User *)usr->node.mln_Succ;
			}
			
			BufStringAdd( bs, "]}");
			
			HttpSetContent( response, bs->bs_Buffer, bs->bs_Size );
			bs->bs_Buffer = NULL;
			
			BufStringDelete( bs );
		}
		else	//is admin
		{
			HttpAddTextContent( response, "ok<!--separate-->{\"response\":\"access denied\"}" );
		}
		
		if( msg != NULL )
		{
			FFree( msg );
		}
		
		*result = 200;
	}
	error:
	
	return response;
}
