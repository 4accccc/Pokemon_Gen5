///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Core task/callback manager
#include "nonport.h"
#include "core.h"

// Move to common code
#if defined(_WIN32)
#define gsAssert(a) { if(!(a)) { _asm { int 3 } } }
#else
#define gsAssert(a)
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static GSCoreMgr* gsiGetStaticCore()
{
	static GSCoreMgr gStaticCore;
	return &gStaticCore;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsCoreInitialize()
{
	GSCoreMgr* aCore = gsiGetStaticCore();
	if (aCore->mIsInitialized == 1)
		return; // bail - already initalized

	// Reset to zero in case this core has been used before
	memset(aCore, 0, sizeof(GSCoreMgr));

	// perform initialization
	gsiInitializeCriticalSection(&aCore->mQueueCrit);

	// finished
	aCore->mIsInitialized = 1;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsiCoreTaskDispatchCallback(GSTask *theTask, GSTaskResult theResult)
{
	if (theTask->mIsCallbackPending)
	{
		theTask->mIsCallbackPending = 0;
		if (theTask->mCallbackFunc)
			(theTask->mCallbackFunc)(theTask->mTaskData, theResult);
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Optional maximum processing time
//    - Pass in 0 to process each task once
void gsCoreThink(gsi_time theMS)
{
	GSCoreMgr* aCore = gsiGetStaticCore();
	int i=0;
	gsi_time aStartTime = 0;
	gsi_i32 allTasksAreDead = 1;

	// enter queue critical section
	gsiEnterCriticalSection(&aCore->mQueueCrit);

	// start timing
	aStartTime = current_time();

	// process all tasks in the queue, dispatch callbacks
	// cancelled tasks continue processing until the cancel is acknowledge by the task
	for (i=0; i<GSICORE_MAXTASKS; i++)
	{
		if (aCore->mTaskArray[i] != NULL)
		{
			GSTask* aTask = (GSTask*)aCore->mTaskArray[i];
			GSTaskResult aResult = GSTaskResult_None;

			allTasksAreDead = 0;

			// If the task is running let it think (it may be cancelled and still running)
			if (aTask->mIsRunning && aTask->mThinkFunc)
				aResult = (aTask->mThinkFunc)(aTask->mTaskData);

			// Check for time out
			if ((!aTask->mIsCanceled) && (aResult == GSTaskResult_InProgress))
			{
				if ((aTask->mTimeout != 0) && (current_time() - aTask->mStartTime > aTask->mTimeout))
				{
					// Cancel the task...
					gsiCoreCancelTask(aTask);

					// ...but trigger callback immediately with "Timed Out"
					gsiCoreTaskDispatchCallback(aTask, GSTaskResult_TimedOut);
				}
				//else
				//    continue processing it
			}
			else if (aResult != GSTaskResult_InProgress)
			{
				// Call the callback (if exists)
				gsiCoreTaskDispatchCallback(aTask, aResult);
				aTask->mIsRunning = 0;

				// Call Cleanup hook and remove task
				if (aTask->mCleanupFunc)
					(aTask->mCleanupFunc)(aTask->mTaskData);
				
				aCore->mTaskArray[i] = NULL;
				gsifree(aTask);
			}
		}

		// Enough time to process another? (if not, break)
		if (theMS != 0 && (current_time()-aStartTime > theMS))
			break;
	}

	// shutting down?
	if (aCore->mIsShuttingDown && allTasksAreDead)
	{
		aCore->mIsShuttingDown = 0;
		aCore->mIsInitialized = 0;

		// leave queue critical section
		gsiLeaveCriticalSection(&aCore->mQueueCrit);
		gsiDeleteCriticalSection(&aCore->mQueueCrit);
	}
	else
	{
		// leave queue critical section
		gsiLeaveCriticalSection(&aCore->mQueueCrit);
	}
	
	GSI_UNUSED(aCore);
	GSI_UNUSED(theMS);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsCoreShutdown()
{
	GSCoreMgr* aCore = gsiGetStaticCore();
	int i=0;

	// If not initialized, just bail
	if (aCore->mIsInitialized == 0)
		return;

	gsiEnterCriticalSection(&aCore->mQueueCrit);

	aCore->mIsShuttingDown = 1;

	// Cancel all tasks
	for (i=0; i<GSICORE_MAXTASKS; i++)
	{
		if (aCore->mTaskArray[i] != NULL)
		{
			gsiCoreCancelTask(aCore->mTaskArray[i]);
		}
	}

	gsiLeaveCriticalSection(&aCore->mQueueCrit);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_i32 gsCoreIsShutdown()
{
	GSCoreMgr* aCore = gsiGetStaticCore();
	return !aCore->mIsInitialized;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Adds a GSCoreTask to the execution array
//   - Tasks may come from multiple threads
void gsiCoreExecuteTask(GSTask* theTask, gsi_time theTimeoutMs)
{
	GSCoreMgr* aCore = gsiGetStaticCore();
	int anInsertPos = -1;
	int i=0;

	// Bail, if the task has already started
	gsAssert(!theTask->mIsRunning);

	gsiEnterCriticalSection(&aCore->mQueueCrit);
	
	// Mark it as started and running
	theTask->mIsCallbackPending = 1;
	theTask->mIsStarted = 1;
	theTask->mIsRunning = 1;
	theTask->mTimeout = theTimeoutMs;
	theTask->mStartTime = current_time();	
	
	// Execute the task
	if (theTask->mExecuteFunc)
		(theTask->mExecuteFunc)(theTask->mTaskData);

	// add it to the process list
	for (i=0; i<GSICORE_MAXTASKS; i++)
	{
		if (aCore->mTaskArray[i] == NULL)
		{
			anInsertPos = i;
			break;
		}
	}
	gsAssert(anInsertPos != -1); // make sure it got in
	aCore->mTaskArray[anInsertPos] = theTask;
	gsiLeaveCriticalSection(&aCore->mQueueCrit);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// cancelling a task is an *async request*
// A task that doesn't support cancelling, such as a blocking socket operation,
// may complete normally even though it was cancelled.
void gsiCoreCancelTask(GSTask* theTask)
{
	GSCoreMgr* aCore = gsiGetStaticCore();

	gsiEnterCriticalSection(&aCore->mQueueCrit);
	if (theTask->mIsRunning && !theTask->mIsCanceled)
	{
		theTask->mIsCanceled = 1;
		if (theTask->mCancelFunc)
			(theTask->mCancelFunc)(theTask->mTaskData);
	}
	gsiLeaveCriticalSection(&aCore->mQueueCrit);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GSTask* gsiCoreCreateTask()
{
	GSTask* aTask = (GSTask*)gsimalloc(sizeof(GSTask));
	memset(aTask, 0, sizeof(GSTask));
	return aTask;
}