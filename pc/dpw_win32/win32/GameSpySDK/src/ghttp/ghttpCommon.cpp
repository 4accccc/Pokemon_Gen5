/*
GameSpy GHTTP SDK 
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com
*/

#include "ghttpCommon.h"

// Disable compiler warnings for issues that are unavoidable.
/////////////////////////////////////////////////////////////
#if defined(_MSC_VER) // DevStudio
// Level4, "conditional expression is constant". 
// Occurs with use of the MS provided macro FD_SET
#pragma warning ( disable: 4127 )
#endif // _MSC_VER

#ifdef WIN32
// A lock.
//////////
typedef void * GLock;

// The lock used by ghttp.
//////////////////////////
static GLock ghiGlobalLock;
#endif

// Proxy server.
////////////////
char * ghiProxyAddress;
unsigned short ghiProxyPort;

// Throttle settings.
/////////////////////
int ghiThrottleBufferSize = 125;
gsi_time ghiThrottleTimeDelay = 250;

// Number of connections
/////////////////////
extern int ghiNumConnections;


#ifdef WIN32
// Creates a lock.
//////////////////
static GLock GNewLock(void)
{
	CRITICAL_SECTION * criticalSection;

	criticalSection = (CRITICAL_SECTION *)gsimalloc(sizeof(CRITICAL_SECTION));
	if(!criticalSection)
		return NULL;

	InitializeCriticalSection(criticalSection);

	return (GLock)criticalSection;
}

// Frees a lock.
////////////////
static void GFreeLock(GLock lock)
{
	CRITICAL_SECTION * criticalSection = (CRITICAL_SECTION *)lock;

	if(!lock)
		return;

	DeleteCriticalSection(criticalSection);

	gsifree(criticalSection);
}

// Locks a lock.
////////////////
static void GLockLock(GLock lock)
{
	CRITICAL_SECTION * criticalSection = (CRITICAL_SECTION *)lock;

	if(!lock)
		return;

	EnterCriticalSection(criticalSection);
}

// Unlocks a lock.
//////////////////
static void GUnlockLock(GLock lock)
{
	CRITICAL_SECTION * criticalSection = (CRITICAL_SECTION *)lock;

	if(!lock)
		return;

	LeaveCriticalSection(criticalSection);
}
#endif

// Creates the ghttp lock.
//////////////////////////
void ghiCreateLock(void)
{
#ifdef WIN32
	// We shouldn't already have a lock.
	////////////////////////////////////
	assert(!ghiGlobalLock);

	// Create the lock.
	///////////////////
	ghiGlobalLock = GNewLock();
#endif
}

// Frees the ghttp lock.
////////////////////////
void ghiFreeLock(void)
{
#ifdef WIN32
	if(!ghiGlobalLock)
		return;

	GFreeLock(ghiGlobalLock);
	ghiGlobalLock = NULL;
#endif
}

// Locks the ghttp lock.
////////////////////////
void ghiLock
(
	void
)
{
#ifdef WIN32
	if(!ghiGlobalLock)
		return;

	GLockLock(ghiGlobalLock);
#endif
}

// Unlocks the ghttp lock.
//////////////////////////
void ghiUnlock
(
	void
)
{
#ifdef WIN32
	if(!ghiGlobalLock)
		return;

	GUnlockLock(ghiGlobalLock);
#endif
}

// Logs traffic.
////////////////
#ifdef HTTP_LOG
void ghiLog(char * buffer, int len)
{
	FILE * file;

	if(!buffer || !len)
		return;

	file = fopen("http.log", "ab");
	if(file)
	{
		fwrite(buffer, 1, len, file);
		fclose(file);
	}
}
#endif

// Appends encrypted data to decodeBuffer
// Appends decrypted data to recvBuffer
// Returns GHTTPFalse if there was a fatal error
////////////////////////////////////////////////
static GHTTPBool ghiDecryptReceivedData(struct GHIConnection * connection)
{
	// Decrypt data from decodeBuffer to recvBuffer
	GHIEncryptionResult aResult = GHIEncryptionResult_None;

	// data to be decrypted
	char* aReadPos  = NULL;
	char* aWritePos = NULL;
	int   aReadLen  = 0;
	int   aWriteLen = 0;

	// Call the decryption func
	do 
	{
		aReadPos  = connection->decodeBuffer.data + connection->decodeBuffer.pos;
		aReadLen  = connection->decodeBuffer.len  - connection->decodeBuffer.pos; 
		aWritePos = connection->recvBuffer.data + connection->recvBuffer.len;
		aWriteLen = connection->recvBuffer.size - connection->recvBuffer.len;    // the amount of room in recvbuffer

		aResult = (connection->encryptor.mDecryptFunc)(connection, &connection->encryptor, 
			aReadPos, &aReadLen, aWritePos, &aWriteLen);
		if (aResult == GHIEncryptionResult_BufferTooSmall)
		{
			// Make some more room
			if (GHTTPFalse == ghiResizeBuffer(&connection->recvBuffer, connection->recvBuffer.sizeIncrement))
				return GHTTPFalse; // error
		}
	} while (aResult == GHIEncryptionResult_BufferTooSmall && aWriteLen == 0);

	// Adjust GHIBuffer sizes so they account for transfered data
	connection->decodeBuffer.pos += aReadLen;
	connection->recvBuffer.len   += aWriteLen;

	// Discard data from the decodedBuffer in chunks
	if (connection->decodeBuffer.pos > 0xFF)
	{
		int bytesToKeep = connection->decodeBuffer.len - connection->decodeBuffer.pos;
		if (bytesToKeep == 0)
			ghiResetBuffer(&connection->decodeBuffer);
		else
		{
			memmove(connection->decodeBuffer.data,
					connection->decodeBuffer.data + connection->decodeBuffer.pos,
					(size_t)bytesToKeep);
			connection->decodeBuffer.pos = 0;
			connection->decodeBuffer.len = bytesToKeep;
		}
	}

	if (aResult == GHIEncryptionResult_Error)
	{
		connection->completed = GHTTPTrue;
		connection->result = GHTTPEncryptionError;
		return GHTTPFalse;
	}

	return GHTTPTrue; 
}

// Receive some data.
/////////////////////
GHIRecvResult ghiDoReceive
(
	GHIConnection * connection,
	char buffer[],
	int * bufferLen
)
{
	int rcode;
	int socketError;
	int len;

	// How much to try and receive.
	///////////////////////////////
	len = (*bufferLen - 1);

	// Are we throttled?
	////////////////////
	if(connection->throttle)
	{
		unsigned long now;

		// Don't receive too often.
		///////////////////////////
		now = current_time();
		if(now < (connection->lastThrottleRecv + ghiThrottleTimeDelay))
			return GHINoData;

		// Update the receive time.
		///////////////////////////
		connection->lastThrottleRecv = (unsigned int)now;

		// Don't receive too much.
		//////////////////////////
		len = min(len, ghiThrottleBufferSize);
	}

	// Check for buffered recv data first
	if (connection->recvBuffer.pos < connection->recvBuffer.len)
	{
		// return previously received data first
		ghiReadDataFromBuffer(&connection->recvBuffer, buffer, bufferLen);
		if (connection->recvBuffer.pos == connection->recvBuffer.len)
		{
			connection->recvBuffer.len = connection->tempDataIndex;
			connection->recvBuffer.pos = connection->tempDataIndex;
			//ghiResetBuffer(&connection->recvBuffer);
		}
		return GHIRecvData;
	}

	// Receive some data.
	/////////////////////
	rcode = recv(connection->socket, buffer, len, 0);

	// There was an error.
	//////////////////////
	if(rcode == SOCKET_ERROR)
	{
		// Get the error code.
		//////////////////////
		socketError = GOAGetLastError(connection->socket);

		// Check for nothing waiting.
		/////////////////////////////
		if((socketError == WSAEWOULDBLOCK) || (socketError == WSAEINPROGRESS) || (socketError == WSAETIMEDOUT))
			return GHINoData;

		// There was a real error.
		//////////////////////////
		connection->completed = GHTTPTrue;
		connection->result = GHTTPSocketFailed;
		connection->socketError = socketError;
		connection->connectionClosed = GHTTPTrue;

		return GHIError;
	}

	// The connection was closed.
	/////////////////////////////
	if(rcode == 0)
	{
		connection->connectionClosed = GHTTPTrue;
		return GHIConnClosed;
	}

	
	// If using encryption we must decode the data before returning
	if (connection->encryptor.mEngine != GHTTPEncryptionEngine_None)
	{
		int recvBufferLen = 0;

		// Append new encrypted data to anything we've held over
		//    We have to do this because we can't decrypt partial SSL messages
		if (!ghiAppendDataToBuffer(&connection->decodeBuffer, buffer, rcode))
			return GHIError;

		// Decrypt as much as we can
		if (!ghiDecryptReceivedData(connection))
		{
			connection->completed = GHTTPTrue;
			connection->result = GHTTPEncryptionError;
			return GHIError;
		}
		
		// Return any newly buffered data
		recvBufferLen = connection->recvBuffer.len - connection->recvBuffer.pos;
		if (recvBufferLen <= 0)
		{
			buffer[0] = '\0';
			*bufferLen = 0;
			return GHINoData;
		}
		else
		{
			// copy the decrypted data into buffer
			rcode = (*bufferLen)-1;
			if (!ghiReadDataFromBuffer(&connection->recvBuffer, buffer, &rcode))
				return GHIError;
			if (connection->recvBuffer.pos == connection->recvBuffer.len)
			{
				// Don't reset the buffer, need to keep status and header's intact
				//ghiResetBuffer(&connection->recvBuffer);
				connection->recvBuffer.len = connection->tempDataIndex;
				connection->recvBuffer.pos = connection->tempDataIndex;
			}
				
			if (rcode <= 0)
				return GHINoData;
		}
	}

	// Cap the buffer.
	//////////////////
	buffer[rcode] = '\0';
	*bufferLen = rcode;

	// Notify app if there was data it should handle.
	///////////////
	if (rcode > 0)
		return GHIRecvData;
	else
		return GHINoData;
}

int ghiDoSend
(
	struct GHIConnection * connection,
	const char * buffer,
	int len
)
{
	int rcode;

	// Do the send.
	///////////////
	rcode = send(connection->socket, buffer, len, 0);

	// Check for an error.
	//////////////////////
	if(rcode == SOCKET_ERROR)
	{
		int error;

		// Would block just means 0 bytes sent.
		///////////////////////////////////////
		error = GOAGetLastError(connection->socket);
		if((error == WSAEWOULDBLOCK) || (error == WSAEINPROGRESS) || (error == WSAETIMEDOUT))
			return 0;

		connection->completed = GHTTPTrue;
		connection->result = GHTTPSocketFailed;
		connection->socketError = error;

		return SOCKET_ERROR;
	}

	if(connection->state == GHTTPPosting)
		connection->postingState.bytesPosted += rcode;

	return rcode;
}

GHITrySendResult ghiTrySendThenBuffer
(
	GHIConnection * connection,
	const char * buffer,
	int len
)
{
	int rcode = 0;

	// If we already have something buffered, don't send.
	/////////////////////////////////////////////////////
	if(!connection->sendBuffer.len)
	{
		// Try and send.
		////////////////
		rcode = ghiDoSend(connection, buffer, len);
		if(rcode == SOCKET_ERROR)
			return GHITrySendError;

		// Was it all sent?
		///////////////////
		if(rcode == len)
			return GHITrySendSent;
	}
	
	// Buffer whatever wasn't sent.
	///////////////////////////////
	if(!ghiAppendDataToBuffer(&connection->sendBuffer, buffer + rcode, len - rcode))
		return GHITrySendError;

	return GHITrySendBuffered;
}

static GHTTPBool ghiParseProxyServer
(
	const char * server,
	char ** proxyAddress,       // [out] the proxy address
	unsigned short * proxyPort  // [out] the proxy port
)
{
	char * strPort;

	// Make sure each pointer is valid as well as what it points to
	assert(server && *server);
	assert(proxyAddress && !*proxyAddress);
	assert(proxyPort);

	// Copy off the server address.
	///////////////////////////////
	*proxyAddress = goastrdup(server);
	if(!*proxyAddress)
		return GHTTPFalse;

	// Check for a port.
	////////////////////
	if((strPort = strchr(*proxyAddress, ':')) != NULL)
	{
		*strPort++ = '\0';

		// Try getting the port.
		////////////////////////
		*proxyPort = (unsigned short)atoi(strPort);
		if(!*proxyPort)
		{
			gsifree(*proxyAddress);
			*proxyAddress = NULL;
			return GHTTPFalse;
		}
	}
	else
	{
		*proxyPort = GHI_DEFAULT_PORT;
	}
	return GHTTPTrue;
}

GHTTPBool ghiSetProxy
(
	const char * server
)
{
	// Free any existing proxy address.
	///////////////////////////////////
	if(ghiProxyAddress)
	{
		gsifree(ghiProxyAddress);
		ghiProxyAddress = NULL;
	}
	ghiProxyPort = 0;

	// If a server was supplied, try to parse it
	if(server && *server)
		return ghiParseProxyServer(server, &ghiProxyAddress, &ghiProxyPort);

	// No server supplied results in proxy being cleared
	return GHTTPTrue;
}

GHTTPBool ghiSetRequestProxy
(
	GHTTPRequest request,
	const char * server
)
{
	// Obtain the connection for this request
	GHIConnection* connection = ghiRequestToConnection(request);
	if (connection == NULL)
		return GHTTPFalse;

	// Free any existing proxy address.
	///////////////////////////////////
	if(connection->proxyOverrideServer)
	{
		gsifree(connection->proxyOverrideServer);
		connection->proxyOverrideServer = NULL;
		connection->proxyOverridePort = GHI_DEFAULT_PORT;
	}

	// If a server was supplied, try to parse it
	if(server && *server)
		return ghiParseProxyServer(server, &connection->proxyOverrideServer, &connection->proxyOverridePort);
	
	// No server supplied results in proxy being cleared
	return GHTTPTrue;
}

void ghiThrottleSettings
(
	int bufferSize,
	gsi_time timeDelay
)
{
	ghiThrottleBufferSize = bufferSize;
	ghiThrottleTimeDelay = timeDelay;
}

#ifdef UNDER_CE
// CE doesn't have isspace().
/////////////////////////////
static int isspace(int c)
{
	if((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
		return 1;
	return 0;
}
#endif

// Re-enable previously disabled compiler warnings
///////////////////////////////////////////////////
#if defined(_MSC_VER)
#pragma warning ( default: 4127 )
#endif // _MSC_VER

