//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//- System includes
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

#include <nitro.h>
#include <stdlib.h>
#include <string.h>
#include <Mobiclip.h>

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//- User defines
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

// This is the total number of frames that we bufferize before doing any playback
#define NB_FRAME_IN_ADVANCE					12	
// If the number of bufferized frames is equal or lower than this value, we start skipping frames 
#define NB_FRAME_SKIP 						3
// If the number of bufferized frames is equal or higher than this value, we use medium 
// visual quality enhancement while blitting
#define NB_FRAME_MEDIUM_QUALITY_ENHANCEMENT 7
// If the number of bufferized frames is equal or higher than this value, we use high 
// visual quality enhancement while blitting
#define NB_FRAME_HIGH_QUALITY_ENHANCEMENT 	10

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//- Implementation of allocation/deallocation routines used by Mobiclip library
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

void* MO_Malloc ( u32 size )
{
	return malloc(size);
}

//-----------------------------------------------------------------------------------------------

void MO_Free ( void* mem_p )
{
	free(mem_p);
}

//-----------------------------------------------------------------------------------------------

// Alarm structure
static OSAlarm gMODemo_BlitAlarm;

// Tells if the movie playback has ended
static bool 	gMODemoPlaybackFinished;

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-	Sound management defines, data and functions
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

// This is our circular sound buffer
static s16*				gMODemo_AudioSampleBuffer;

// This is the index in circular sound buffer where you have to put the new audio samples
static u32				gMODemo_AudioSampleBufferIndex;

// This is the number of MO packets of our circular sound buffer
static s32				gMODemo_SoundBufferNbPackets;

//-------------------------------------------------------------------------------------------
// This function starts a looping PCM 16 bits sound of 'frequency' Hz.
// It also starts the alarm callback routine to refresh the number of sound packets streamed
//-------------------------------------------------------------------------------------------

static void MODemo_StartSoundStream (u32 frequency, u32 nbSamplePerAudioPacket )
{
	//----------------------------------------------------------------------------
	//- Variables initialisation
	//----------------------------------------------------------------------------
	
	u32 soundClockTick;
			
	// This variable contains the sound frequency in number of sound clocks 		
	soundClockTick = SND_TIMER_CLOCK/frequency;
	
	//----------------------------------------------------------------------------
	
	// Call DC_StoreRange to flush all remaining data of sound buffer from the cpu data cache 
	DC_StoreRange(gMODemo_AudioSampleBuffer,gMODemo_SoundBufferNbPackets*nbSamplePerAudioPacket*2);

	// We reserve the channel 0 for our sound playing, SND_LockChannels parameter is then (1<<0)		
	SND_LockChannel(1,0);

	// We setup our looping sound playback
	SND_SetupChannelPcm(
        0,
        SND_WAVE_FORMAT_PCM16,
        (void*)gMODemo_AudioSampleBuffer,
        SND_CHANNEL_LOOP_REPEAT,
        0,
        (int)(gMODemo_SoundBufferNbPackets*nbSamplePerAudioPacket/2),
        127,
        SND_CHANNEL_DATASHIFT_NONE,
        (int)soundClockTick,
        64);

    SND_StartTimer(1, 0, 0, 0);
    
	// We call this function in order to flush our sound requests to the ARM9/ARM7 FIFO
	SND_FlushCommand(SND_COMMAND_BLOCK);
}

//-------------------------------------------------------------------------------------------
// This function stops the playing sound
//-------------------------------------------------------------------------------------------

static void MODemo_StopSound ( void )
{
	// We stop our looping sound
	SND_StopTimer(1, 0, 0, 0);

	// We release the channel 0
	SND_UnlockChannel(1,0);	
	
	// We call this function in order to flush our sound requests to the ARM9/ARM7 FIFO
	SND_FlushCommand(SND_COMMAND_BLOCK);
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-	VRAM video mode management data and functions
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

// This variable contains the current displayed VRAM bank index
static vu32	gMODemo_DisplayedBuffer;	

// This variable tells if a flip has been done by the VBlank interrupt routine
// if 0, the VBlank interrupt routine must perform the flip and the client must wait that flip has 
// been done before blitting to the screen
// if 1, the VBlank interrupt routine must not perform the flip because it has already been done
static vu32	gMODemo_FlipStatus;	

//-----------------------------------------------------------------------------------------------
//- Flip the video buffers
//-----------------------------------------------------------------------------------------------

static void MODemo_Flip()
{
	if (gMODemo_DisplayedBuffer == 0)
		GX_SetGraphicsMode(GX_DISPMODE_VRAM_B, (GXBGMode)0, (GXBG0As)0 );
	else
		GX_SetGraphicsMode(GX_DISPMODE_VRAM_A, (GXBGMode)0, (GXBG0As)0 );
	
	gMODemo_DisplayedBuffer ^= 1;
}


//-----------------------------------------------------------------------------------------------
//- This is our VBlank interrupt routine
//-----------------------------------------------------------------------------------------------

static void MODemo_VBlankIntr()
{
	// We read the value of the VCOUNT register
	// if vblankCounter < 192 we missed the VBL because we passed too much time in the  
	// BlitCallback routine; the reason is that IRQ are not reentrant.
	// If VBL is missed we will not flip screens to avoid tearing, we will do that at the next VBL.
	u16 vblankCounter = *((vu16*)REG_VCOUNT_ADDR);

	// If a flip has not been requested, or we missed the VBL we don't enter 
	if (gMODemo_FlipStatus == 0 && vblankCounter >= 192)
	{		
		MODemo_Flip();
		gMODemo_FlipStatus = 1;
	}
	
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//-----------------------------------------------------------------------------------------------
//- Get the current back buffer
//-----------------------------------------------------------------------------------------------

static u16* MODemo_GetBackBuffer()
{
	if (gMODemo_DisplayedBuffer == 0)
		return (u16*)HW_LCDC_VRAM_B;
	else
		return (u16*)HW_LCDC_VRAM_A;
}

//-----------------------------------------------------------------------------------------------
//- Variables used by the Alarm interrupt handler
//-----------------------------------------------------------------------------------------------

volatile int gMODemoPushedFrames;
//Total number of frames displayed
volatile int gMODemoBlittedFrames;
// Total number of frames skipped
int 		 gMODemoSkippedFrames; 
// Total number of frames blitted without visual enhancements
int 		 gMODemoBlittedFramesWithoutVisualEnhancement; 
// Total number of frames blitted with medium visual enhancements
int 		 gMODemoBlittedFramesWithMediumVisualEnhancement; 
// Total number of frames blitted with hight visual enhancements
int 		 gMODemoBlittedFramesWithHighVisualEnhancement;	

// Time at which playback is started
OSTick		gMODemoStartPlaybackTime;

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

static OSTick MODemo_NextCallbackTime(MOHandle handle)
{
	u64 fps;
	u32 soundFrequency = MO_GetAudioFrequency(handle);
	if (soundFrequency)
	{
		// As the played sound frequency is not exactly the one given in the movie,
		// We are altering the framerate of the movie to compensate the error.
		u64 soundClockTick = (u64)SND_TIMER_CLOCK / (u64)soundFrequency;
		fps = ((u64)MO_GetVideoFps(handle)*(u64)SND_TIMER_CLOCK)/(soundClockTick*(u64)soundFrequency);
	}
	else
		fps = (u64)MO_GetVideoFps(handle);
	
	//-----------------------------------------------------------------------

	OSTick currentTime = OS_GetTick()-gMODemoStartPlaybackTime;
	OSTick nextTime = ((u64)(gMODemoBlittedFrames+1)*(u64)OS_SYSTEM_CLOCK*(u64)(1<<18))/fps;
	
	if (nextTime < currentTime) // This case happens when decode is too slow, we give 
								// we give a bit of CPU time to the main(decode) loop before
								// calling the callback again.
		return 1000;
	else
		return nextTime - currentTime; // This is the number of ticks we need to wait to 
									   // blit of skip the next video frame. 
}

//-----------------------------------------------------------------------------------------------
//- Alarm interrupt handler 
//-----------------------------------------------------------------------------------------------

static void BlitCallback( void* arg )
{
	//-----------------------------------------------------------------------

	// The movie handle
	MOHandle handle = arg;
	
	//-----------------------------------------------------------------------
		
	int nbFrameRemaining = gMODemoPushedFrames - gMODemoBlittedFrames;
	
	if (nbFrameRemaining > 0)
	{
		if (nbFrameRemaining <= NB_FRAME_SKIP && gMODemoPlaybackFinished == false) // We skip a frame
		{
			MO_SkipFrameImage(handle);
			gMODemoSkippedFrames++;
		}
		else
		{
			//-----------------------------------------------------------------------

			// A flip request has not yet been executed, we exit the callback to give the 
			// hand to the VBlank interrupt routine. 
			if (gMODemo_FlipStatus == 0) 
			{
				OS_SetAlarm(&gMODemo_BlitAlarm,1,BlitCallback,(void *)handle);
				return;
			}

			u32 qualityEnhancement;
			if (nbFrameRemaining>=NB_FRAME_HIGH_QUALITY_ENHANCEMENT)
			{
				// We do a blit with high visual quality enhancement
				qualityEnhancement = MO_QUALITY_ENHANCEMENT_HIGH;
				gMODemoBlittedFramesWithHighVisualEnhancement++;
			}
			else if (nbFrameRemaining>=NB_FRAME_MEDIUM_QUALITY_ENHANCEMENT)
			{
				// We do a blit with medium visual quality enhancement
				qualityEnhancement = MO_QUALITY_ENHANCEMENT_MEDIUM;
				gMODemoBlittedFramesWithMediumVisualEnhancement++;
			}
			else
			{
				// We do a normal blit
				qualityEnhancement = MO_QUALITY_ENHANCEMENT_NONE;
				gMODemoBlittedFramesWithoutVisualEnhancement++;
			}

			// Execute the blit
			MO_BlitFrameImage(handle,MODemo_GetBackBuffer(),256,qualityEnhancement);	
			
			// Send a flip request to the VBlank interrupt routine. 
			gMODemo_FlipStatus = 0;

			//-----------------------------------------------------------------------
		}	
		
		//-----------------------------------------------------------------------

		gMODemoBlittedFrames++;
		
		//-----------------------------------------------------------------------
	}
		
	//-----------------------------------------------------------------------
	
	OS_SetAlarm(&gMODemo_BlitAlarm,MODemo_NextCallbackTime(handle),BlitCallback,(void *)handle);

	//-----------------------------------------------------------------------
}

//-----------------------------------------------------------------------------------------------
//- Unpacks Audio and Video frames
//-----------------------------------------------------------------------------------------------

static void MODemo_UnpackFrame(MOHandle handle)
{
	//-----------------------------------------------------------------------
	
	// Unpack video frame
	MO_UnpackFrameImage(handle);
	
	if (MO_GetAudioFrequency(handle) > 0) 
	{
		u32 nbSamplePerAudioPacket = MO_GetNbSamplePerAudioPacket(handle);
		u64 nbUnpackedSoundPackets = MO_GetFrameNbAudioPacket(handle);

		// Unpack all audio packets.
		// Synchronisation is not done with an audio timer, but with the alarm callback.
		for (int i=0;i<nbUnpackedSoundPackets;i++)
		{
			MO_BlitFrameSoundOnePacket(handle,gMODemo_AudioSampleBuffer+gMODemo_AudioSampleBufferIndex);
			
			DC_StoreRange(gMODemo_AudioSampleBuffer+gMODemo_AudioSampleBufferIndex,nbSamplePerAudioPacket * 2);
			
			gMODemo_AudioSampleBufferIndex+=nbSamplePerAudioPacket;
			if (gMODemo_AudioSampleBufferIndex == gMODemo_SoundBufferNbPackets * nbSamplePerAudioPacket)
				gMODemo_AudioSampleBufferIndex = 0;
		}
	}
	
	//-----------------------------------------------------------------------
}

//-----------------------------------------------------------------------------------------------
//- Plays a MO movie for file
//-----------------------------------------------------------------------------------------------

static void MODemo_PlayMovieFile(char* fileName)
{
	//-----------------------------------------------------------------------

	// The Mobiclip movie handle
	MOHandle	handle;
	
	//-----------------------------------------------------------------------
	
	// ITCM code relocation is done here, to have an optimal playback speed
	u32 SizeITCM = (u32)OS_GetITCMArenaHi() - (u32)OS_GetITCMArenaLo();
	if (SizeITCM < MO_GetOptimalITCMSize())
		OS_Printf("WARNING: Not enough available ITCM in order to have optimum playback speed ...\n");	
	else
		SizeITCM = MO_GetOptimalITCMSize();
			
	// DTCM data relocation is done here, to have an optimal playback speed
	u32 SizeDTCM = (u32)OS_GetDTCMArenaHi() - (u32)OS_GetDTCMArenaLo();
	if (SizeDTCM < MO_GetOptimalDTCMSize())
		OS_Printf("WARNING: Not enough available DTCM in order to have optimum playback speed ...\n");	
	else
		SizeDTCM = MO_GetOptimalDTCMSize();
	
	MO_InitITCMArea (OS_GetITCMArenaLo(), SizeITCM);
	MO_InitDTCMArea (OS_GetDTCMArenaLo(), SizeDTCM);
			
	//-----------------------------------------------------------------------
	
	FSFile file;
	FS_InitFile(&file);
	if (FS_OpenFile(&file,fileName) == FALSE)
	{
		OS_Printf("ERROR: Cannot open file %s\n",fileName);
		return;
	}
	
	// We set the maximum number of frames who can be unpacked without been blitted to 'NB_FRAME_IN_ADVANCE'
	u32 nbFrameBufferedMax = NB_FRAME_IN_ADVANCE;
	
	// Open a MO movie
	handle = MO_OpenMovie(&file,nbFrameBufferedMax);
	if (handle == MO_INVALID_HANDLE)
		return;
	
	// We just check that stack has not been owerwritten by Mobiclip datas by the MO_OpenMovie function
	OS_CheckStack(OS_GetCurrentThread());
	
	// Retrieve the number of samples per audio packet (it can be 128 or 256 depending on  the audio codec used)
	u32 nbSamplePerAudioPacket = MO_GetNbSamplePerAudioPacket(handle);
	    
    //-----------------------------------------------------------------------
	
	// Initialization of the sound buffer size in MO sound packet units
	
	// This is the real audio Frequency that will be used for playback.
	// There is some difference due to roundings in the PCM playback frequency computation.
	u64 realSoundFrequency = (u64)SND_TIMER_CLOCK / (u64)MO_GetAudioFrequency(handle);
	realSoundFrequency = (u64)SND_TIMER_CLOCK / realSoundFrequency;
	
	// This is roughly the number of MO sound packet that we can have per frame
	int nbSoundPacketPerFrame = (int)((realSoundFrequency * (u64)(1<<24)) / ((u64)MO_GetVideoFps(handle) * (u64)nbSamplePerAudioPacket));
	// We add a packet more to compensate the rounding of the previous computation
	nbSoundPacketPerFrame += 1;
	
	// We want to be able to fill up top 'nbFrameBufferedMax' frames of sound data, 
	gMODemo_SoundBufferNbPackets = nbSoundPacketPerFrame * (s32)nbFrameBufferedMax;

	// Allocation of the sound buffer
	gMODemo_AudioSampleBuffer = (s16*)malloc(gMODemo_SoundBufferNbPackets * nbSamplePerAudioPacket * 2);

	// Clean the sound buffer with 0 values
	(void)memset (gMODemo_AudioSampleBuffer,0,gMODemo_SoundBufferNbPackets * nbSamplePerAudioPacket * 2);
	DC_StoreAll();
	
	//-----------------------------------------------------------------------
	
	// Initialization the index in the sound buffer where we have to put the new audio samples
	gMODemo_AudioSampleBufferIndex = 0;
	
	// Initialization of our display buffer index
    gMODemo_DisplayedBuffer = 0;

	// Initialize flip status to 'flip done'
    gMODemo_FlipStatus = 1;

	// Initialize the alarm structure
	OS_CreateAlarm( &gMODemo_BlitAlarm );
	
	// No frames blitted for the moment
	gMODemoBlittedFrames = 0;
	// No frames unpacked for the moment
	gMODemoPushedFrames = 0;
	
	//-----------------------------------------------------------------------
	
	// We bufferize our video and audio data up to 'nbFrameBufferedMax' frames
	for (int i=0;i<nbFrameBufferedMax;i++)
	{
		MO_ReadFrame(handle);
		MODemo_UnpackFrame(handle);
		gMODemoPushedFrames++;
	}
	
	//Playback is in progress
	gMODemoPlaybackFinished = false;
	
	// We reset statistics
	gMODemoSkippedFrames = 0;
	gMODemoBlittedFramesWithoutVisualEnhancement = 0;
	gMODemoBlittedFramesWithMediumVisualEnhancement = 0;
	gMODemoBlittedFramesWithHighVisualEnhancement = 0;
	
	// We start sound playback
	MODemo_StartSoundStream(MO_GetAudioFrequency(handle), MO_GetNbSamplePerAudioPacket(handle));
	
	// By firing the alarm we start the video and audio playback
	gMODemoStartPlaybackTime = OS_GetTick();
	OS_SetAlarm(&gMODemo_BlitAlarm,MODemo_NextCallbackTime(handle),BlitCallback,(void *)handle);
	
	// Execute this loop while there is an available frame in the file
	while (MO_ReadFrame(handle) != 0)
	{
		// Wait until we have a 1 frame slot free
		while( (gMODemoPushedFrames - gMODemoBlittedFrames) >= nbFrameBufferedMax){}
				
		//-----------------------------------------------------------------------
		
		// Unpack video and audio
		MODemo_UnpackFrame(handle);

		gMODemoPushedFrames++;

		//-----------------------------------------------------------------------
	}
	
	//-----------------------------------------------------------------------
	
	//Playback is finished
	gMODemoPlaybackFinished = true;
	
	//Wait until all remaining pushed frames have been displayed
	while( gMODemoPushedFrames > gMODemoBlittedFrames ){}
	
	//Wait until the last frame is effectively shown (a flip has been done)
	while( gMODemo_FlipStatus == 0){}
	
	//-----------------------------------------------------------------------
	
	// Stop sound playback
	if (MO_GetAudioFrequency(handle) > 0) 
		MODemo_StopSound();
	
	// Cancel the alarm
	OS_CancelAlarm( &gMODemo_BlitAlarm );
			
	// Free audio buffer
	free(gMODemo_AudioSampleBuffer);
	
	// Close movie
	MO_CloseMovie (handle);	
	
	FS_CloseFile(&file);
	
		
	//-----------------------------------------------------------------------
	
	// Print statistics
	OS_Printf("On %d displayed frames:\n",gMODemoBlittedFrames);
	OS_Printf("    - %d have been skipped:\n",gMODemoSkippedFrames);
	OS_Printf("    - %d have been blitted without visual enhancements\n",gMODemoBlittedFramesWithoutVisualEnhancement);
	OS_Printf("    - %d have been blitted with medium visual enhancements\n",gMODemoBlittedFramesWithMediumVisualEnhancement);
	OS_Printf("    - %d have been blitted with high visual enhancements\n",gMODemoBlittedFramesWithHighVisualEnhancement);
	
	//-----------------------------------------------------------------------
}

//---------------------------------------------------------------------------
//- Nitro main
//---------------------------------------------------------------------------

void NitroMain ()
{	
	//----------------------------
	//- OS and heap initialisation
	//----------------------------
	
	void*    arenaLo;
	
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
            
    // Create a heap
    arenaLo = OS_InitAlloc( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1 );
    OSHeapHandle heapHandle = OS_CreateHeap( OS_ARENA_MAIN, arenaLo, OS_GetMainArenaHi() );
    // From here on out, OS_Alloc and OS_Free behave like malloc and free, respectively
    OS_SetCurrentHeap( OS_ARENA_MAIN, heapHandle );
    
    
    //------------------------------------
	//- GX initialisation, VRAM video mode
	//------------------------------------
	
	// Init Graphics
	GX_Init();
	
	// Disable display of main LCD sreen
	GX_DispOff();
	
	// Disable display of sub LCD sreen
    GXS_DispOff();
    
    // Out VBlank is 'VFDemo_VBlankIntr'
    OS_SetIrqFunction(OS_IE_V_BLANK, MODemo_VBlankIntr);
    
    // Enable VBlank interrupts
    (void)OS_EnableIrqMask( OS_IE_V_BLANK );
    
    // Enable all interrupts
    (void)OS_EnableIrq();
    
    // Activate VBlank interrupts
    (void)GX_VBlankIntr(TRUE);	
    
    // All VRAM banks to LCDC
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

    // Clear all LCDC space
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    // Disable the banks on LCDC
    (void)GX_DisableBankForLCDC();

	// Clear OAM
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    
    // Clear the standard palette
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    
	// Clear OAM
    MI_CpuFillFast((void*)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    
    // Clear the standard palette
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);  
    
    // Enables all the banks on LCDC
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    
    // Waiting the end of VBlank interrupt
    SVC_WaitVBlankIntr();

    // Enable display of main LCD sreen
    GX_DispOn();
    
    //----------------------------
	//- FS initialisation
	//----------------------------
    
    FS_Init(1);
	
	//----------------------------
	//- Sound initialisation
	//----------------------------
	
    SND_Init();
    
    //----------------------------
	//- Play movie file
	//----------------------------

	MODemo_PlayMovieFile("videos/DS_effectiveUsage.mods");

	//----------------------------
	//- End Nitro main
	//----------------------------
	
	OS_Terminate();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------