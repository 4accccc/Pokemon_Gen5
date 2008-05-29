///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// gSOAP Glue
#include "core.h"
#include "soap.h"
#include "stdsoap2.h"


// GAMESPY DEVELOPERS ->  Use gsiExecuteSoap


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define SOAP_DEFAULT_TIMEOUT	30000  // 30 second timeout


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Soap task delegates
static void gsiSoapTaskExecute(void* theTask);
static GSTaskResult gsiSoapTaskThink(void* theTask);
static void gsiSoapTaskCallback(void* theTask, GSTaskResult theResult);
static void gsiSoapTaskCleanup(void* theTask);

// Cleanup task delegates (cleans up soap thread)
static GSTaskResult gsiSoapTaskThreadCleanupThink(void* aTaskData);
static void gsiSoapTaskThreadCleanupCallback(void* aTaskData, GSTaskResult aResult);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Execute a soap function (this should be the only call made from other SDKs)
void gsiExecuteSoap(GSSoapFunc theFunc, const char* theURL,
					const char* theService, void* theNamespaces,
					void* theRequestData, void* theResponseData,
					 GSSoapCallbackFunc theCallbackFunc, 
					 gsi_time theTimeoutMs, void* theUserData)
{
	GSSoapTask* aSoapTask = NULL;
	GSTask* aCoreTask = NULL;

	aSoapTask = (GSSoapTask*)gsimalloc(sizeof(GSSoapTask));
	aSoapTask->mSoapFunc     = theFunc;
	aSoapTask->mCallbackFunc = theCallbackFunc;
	aSoapTask->mURL          = theURL;
	aSoapTask->mService      = theService;
	aSoapTask->mRequestData  = theRequestData;
	aSoapTask->mResponseData = theResponseData;
	aSoapTask->mNamespaces   = theNamespaces;
	aSoapTask->mUserData     = theUserData;
	aSoapTask->mSoapTimeoutMs = theTimeoutMs;
	aSoapTask->mIsRunning    = 0;
	aSoapTask->mIsCanceled   = 0;
	aSoapTask->mThreadID     = 0;
	aSoapTask->mSoapResult   = 0;
	aSoapTask->mShutdownSemaphoreID = gsiCreateSemaphore(0, 1, NULL);
	
	aCoreTask = gsiCoreCreateTask();
	aCoreTask->mCallbackFunc = gsiSoapTaskCallback;
	aCoreTask->mExecuteFunc  = gsiSoapTaskExecute;
	aCoreTask->mThinkFunc    = gsiSoapTaskThink;
	aCoreTask->mCleanupFunc  = gsiSoapTaskCleanup;
	aCoreTask->mCancelFunc   = NULL;
	aCoreTask->mTaskData     = (void*)aSoapTask;

	if (theTimeoutMs != 0)
		gsiCoreExecuteTask(aCoreTask, theTimeoutMs);
	else
		gsiCoreExecuteTask(aCoreTask, SOAP_DEFAULT_TIMEOUT);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Soap execute thread func, a new thread is spawned for each soap op
#if defined(_WIN32)
	static DWORD WINAPI gsiSoapThreadFunc(void* theArg)
#elif defined(_PS2)
	#define INFINITE ((unsigned int)-1)
	static void gsiSoapThreadFunc(void* theArg)
#endif
{
	GSSoapTask* aSoapTask = (GSSoapTask*)theArg;
	int aResult = 0;
	struct soap aSoap;

#ifdef SN_SYSTEMS
	int snResult = sockAPIregthr();
	if (snResult != 0)
	{
		// 101 = max threads reached  (increase number of socket threads)
		aSoapTask->mSoapResult = SOAP_SNSYSTEM_REGTHREADERROR;
		aSoapTask->mIsRunning = 0;
		gsiWaitForSemaphore(aSoapTask->mShutdownSemaphoreID, INFINITE);
		return;
	}
#endif

	soap_init(&aSoap);
	soap_set_namespaces(&aSoap, aSoapTask->mNamespaces);
	aSoap.recv_timeout = -((int)(aSoapTask->mSoapTimeoutMs * 1000)); // soap timeout is in usec
	
	aResult = (aSoapTask->mSoapFunc)(&aSoap, aSoapTask->mURL, aSoapTask->mService, 
		                              aSoapTask->mRequestData,aSoapTask->mResponseData);
	aSoapTask->mSoapResult = aResult;
	aSoapTask->mIsRunning = 0;
	
	// Because we allocate the soap struct on the stack,
	// we need to keep it around until the callback has been dispatched
	gsiWaitForSemaphore(aSoapTask->mShutdownSemaphoreID, INFINITE);
	soap_end(&aSoap);
	soap_destroy(&aSoap);
	soap_free(&aSoap);

#ifdef SN_SYSTEMS
	if (snResult != 0)
		sockAPIderegthr();
#endif

#if defined(_WIN32)
	return 0;
#endif
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
				//////////  SOAP EXECUTE TASK  //////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Checks to see if the soap task has completed
//   - return GSTaskResult_InProgress for "keep checking"
//   - return anything else           for "finished - trigger callback and delete"
static GSTaskResult gsiSoapTaskThink(void* theTask)
{
	// is the request still processing?
	GSSoapTask* aSoapTask = (GSSoapTask*)theTask;
	if (aSoapTask->mIsRunning)
		return GSTaskResult_InProgress;
	else
	{
		if (aSoapTask->mIsCanceled)
			return GSTaskResult_Canceled;
		else
			return GSTaskResult_Finished;
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Spawns the soap thread and begins execution
static void gsiSoapTaskExecute(void* theTask)
{
	GSSoapTask* aSoapTask = (GSSoapTask*)theTask;
	aSoapTask->mIsRunning = 1;

	// The soap object is created on the stack, it is slightly more than 8096bytes
	gsiStartThread(gsiSoapThreadFunc, (0x4000), (void*)theTask, &aSoapTask->mThreadID);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Called when the soap task completes or is cancelled/timed out
static void gsiSoapTaskCallback(void* theTask, GSTaskResult theResult)
{
	// Call the developer callback
	GSSoapTask* aSoapTask = (GSSoapTask*)theTask;
	if (theResult == GSTaskResult_Canceled)
	{
		(aSoapTask->mCallbackFunc)(SOAP_CANCELED, aSoapTask->mRequestData, 
			aSoapTask->mResponseData, aSoapTask->mUserData);
	} 
	else if (theResult == GSTaskResult_TimedOut)
	{
		(aSoapTask->mCallbackFunc)(SOAP_TIMEDOUT, aSoapTask->mRequestData, 
			aSoapTask->mResponseData, aSoapTask->mUserData);
	}	
	else 
	{
		(aSoapTask->mCallbackFunc)(aSoapTask->mSoapResult, aSoapTask->mRequestData, 
			aSoapTask->mResponseData, aSoapTask->mUserData);
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// After the soap call has completed, launch a separate cleanup event (see comments)
static void gsiSoapTaskCleanup(void *theTask)
{
	// Schedule the thread for termination and cleanup
	//   (Can't do this immediately since this callback may have been triggered
	//    from a blocked thread or other timeout condition)
	GSSoapTask* aSoapTask = (GSSoapTask*)theTask;

	GSTask* aCoreTask = gsiCoreCreateTask();
	aCoreTask->mCallbackFunc = gsiSoapTaskThreadCleanupCallback;
	aCoreTask->mThinkFunc    = gsiSoapTaskThreadCleanupThink;
	aCoreTask->mTaskData     = (void*)aSoapTask;

	gsiCoreExecuteTask(aCoreTask, INFINITE);

	// Release thread so it can shutdown
	gsiReleaseSemaphore(aSoapTask->mShutdownSemaphoreID, 1);
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
				//////////  POST-SOAP CLEANUP TASK  //////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Waits for thread to shutdown
GSTaskResult gsiSoapTaskThreadCleanupThink(void* aTaskData)
{
	GSSoapTask* aSoapTask = (GSSoapTask*)aTaskData;
	GSIThreadID aThreadID = aSoapTask->mThreadID;
	gsi_u32 aResult = gsiHasThreadShutdown(aThreadID); // poll state
	if (aResult == 0)
		return GSTaskResult_InProgress;
	else
		return GSTaskResult_Finished;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Called after thread closes to free system resources
void gsiSoapTaskThreadCleanupCallback(void* aTaskData, GSTaskResult aResult)
{
	GSSoapTask* aSoapTask = (GSSoapTask*)aTaskData;
	GSIThreadID aThreadID = aSoapTask->mThreadID;

	// If the thread won't die we have to kill it the hard way
	if (aResult == GSTaskResult_TimedOut)
		gsiCancelThread(aThreadID);

	gsiCleanupThread(aThreadID);
	gsifree(aSoapTask);

	GSI_UNUSED(aResult);
}
