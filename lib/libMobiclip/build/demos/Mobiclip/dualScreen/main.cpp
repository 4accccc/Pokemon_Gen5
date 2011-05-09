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

// Define used for heap initialisation
#define ROUND(n, a)	(((u32) (n) + (a) - 1) & ~((a) - 1))

// Transform MicroSeconds to Ticks
#define ALARM_COUNT( x )	(u32)(OS_MicroSecondsToTicks( x ))

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//- Declaration of our function prototypes
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

void MODemo_SoundCallbackFunction	( u32 data );
void MODemo_StartSoundStream		( u32 frequency, u32 nbSamplePerAudioPacket );
void MODemo_StopSound				( void );
void MODemo_VBlankIntr				( void );
u16* MODemo_GetBackBuffer			( void );
u16* MODemo_GetSubBackBuffer		( void );
void MODemo_Flip					( void );
void MODemo_BlitAndFlip				(MOHandle handleUp, MOHandle handleDown);
void MODemo_PlayMovieFileDualScreen	(char* fileNameUp,char* fileNameDown);

// Alarm interrupt handler
// Declaration of alarm callback function prototype to be used for playing
// back by the animation framerate in case there is no audio data in movie file
void AlarmIntr( void* arg );

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

// Flag to be used to judge the operation timing of MO_ReadFrame()
static vu32 gMODemo_BlitImageFlag;

// Alarm structure
static OSAlarm alarm;

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-	Sound management defines, data and functions
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

// This contains the total number of MO sound packet streamed by the sound hardware
// since the beginning of the playback
static vu64 			gMODemo_NbSoundPacketStreamed;	

// This contains the total number of MO sound packet pushed by the user in the circular sound buffer
// since the beginning of the playback
static u64 				gMODemo_NbSoundPacketPushed;	

// This is our circular sound buffer
static s16*				gMODemo_AudioSampleBuffer;

// This is the index in circular sound buffer where you have to put the new audio samples
static u32				gMODemo_AudioSampleBufferIndex;

// This is the number of MO packets of our circular sound buffer
static u32				gMODemo_SoundBufferNbPackets;

//-------------------------------------------------------------------------------------------
// This is our callback function who's refreshing the 'gFVDemo_NbSoundPacketStreamed' variable
//-------------------------------------------------------------------------------------------

static void MODemo_SoundCallbackFunction (void * dummy)
{
	#pragma unused (dummy)
	gMODemo_NbSoundPacketStreamed++;
}
//-------------------------------------------------------------------------------------------
// This function starts a looping PCM 16 bits sound of 'frequency' Hz.
// It also starts the alarm callback routine to refresh the number of sound packets streamed
//-------------------------------------------------------------------------------------------

void MODemo_StartSoundStream (u32 frequency, u32 nbSamplePerAudioPacket)
{
	//----------------------------------------------------------------------------
	//- Variables initialisation
	//----------------------------------------------------------------------------
	
	u32 soundClockTick;
	u32 alarmClockTick;
	
	// This variable contains the sound frequency in number of sound clocks 		
	soundClockTick = SND_TIMER_CLOCK/frequency;
	
	// This variable contains the alarm callback period, which is every
	// 'nbSamplePerAudioPacket' sound samples
	// An alarm tick is 1/64 of the hardware system clock, ie (SND_CHANNEL_CLOCK * 2)/64
	alarmClockTick = (soundClockTick * 2) * (nbSamplePerAudioPacket / 64);
	
	// Call DC_StoreRange to flush all remaining data of sound buffer from the cpu data cache 
	DC_StoreRange(gMODemo_AudioSampleBuffer,gMODemo_SoundBufferNbPackets*nbSamplePerAudioPacket*2);
			
	//----------------------------------------------------------------------------

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

	// We setup our sound alarm callback
	SND_SetupAlarm(
		0,
		alarmClockTick,
		alarmClockTick,
		MODemo_SoundCallbackFunction,0);

	// We start our sound looping playback and the alarm timer
    SND_StartTimer(1, 0, 1, 0);

	// We call this function in order to flush our sound requests to the ARM9/ARM7 FIFO
	SND_FlushCommand(SND_COMMAND_BLOCK);
}

//-------------------------------------------------------------------------------------------
// This function stops the playing sound
//-------------------------------------------------------------------------------------------

void MODemo_StopSound ( void )
{
	// We stop our looping sound and our alarm callback	
	SND_StopTimer(1, 0, 1, 0);

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

// This is the Oam attributes for image display on OBJ bitmap
static GXOamAttr		gOamAttr[128];

// This variable contains the current displayed VRAM bank index
static u32	gMODemo_DisplayedBuffer;	

// This variable tells if a flip has been done by the VBlank interrupt routine
// if 0, the VBlank interrupt routine must perform the flip and the client must wait that flip has 
// been done before blitting to the screen
// if 1, the VBlank interrupt routine must not perform the flip because it has already been done
static vu32	gMODemo_FlipStatus;	

//-----------------------------------------------------------------------------------------------
//- This is our VBlank interrupt routine
//-----------------------------------------------------------------------------------------------

void MODemo_VBlankIntr()
{
	if (gMODemo_FlipStatus == 0)
	{
		gMODemo_FlipStatus = 1;
		MODemo_Flip();
	}
	
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//-----------------------------------------------------------------------------------------------
//- Get the current back buffer
//-----------------------------------------------------------------------------------------------

u16* MODemo_GetBackBuffer()
{
	if (gMODemo_DisplayedBuffer == 0)
		return (u16*)HW_LCDC_VRAM_B;
	else
		return (u16*)HW_LCDC_VRAM_A;
}

//-----------------------------------------------------------------------------------------------
//- Get the current back buffer
//-----------------------------------------------------------------------------------------------

u16* MODemo_GetSubBackBuffer()
{
	if (gMODemo_DisplayedBuffer == 0)
		return (u16*)G2S_GetOBJCharPtr();
	else
		return (u16*)G2S_GetBG2ScrPtr();
}

//-----------------------------------------------------------------------------------------------
//- Flip the video buffers
//-----------------------------------------------------------------------------------------------

void MODemo_Flip()
{
	if (gMODemo_DisplayedBuffer == 0)
	{
		// Flip for Main LCD
		GX_SetGraphicsMode(GX_DISPMODE_VRAM_B, (GXBGMode)0, (GXBG0As)0 );
		
		// Flip for Sub LCD, OBJ mode, VRAM D displayed
		GXS_SetVisiblePlane(GX_PLANEMASK_OBJ);
	}
	else
	{
		// Flip for Main LCD
		GX_SetGraphicsMode(GX_DISPMODE_VRAM_A, (GXBGMode)0, (GXBG0As)0 );
		
		// Flip for Sub LCD, BG mode, VRAM C displayed
		GXS_SetVisiblePlane(GX_PLANEMASK_BG2);
	}
	
	gMODemo_DisplayedBuffer ^= 1;
}

//-----------------------------------------------------------------------------------------------
//- Blit current video frame and flip the video buffers
//-----------------------------------------------------------------------------------------------

void MODemo_BlitAndFlip(MOHandle handleUp, MOHandle handleDown)
{
	// We do an active loop until flip has been done in the VBlank interrupt routine
	while (gMODemo_FlipStatus == 0){};

	// Wait until flag to be 1 by alarm 
	while( gMODemo_BlitImageFlag != 1 ) ;
	gMODemo_BlitImageFlag = 0;
	
	// We blit the current frame image to the video back buffer
	MO_BlitFrameImage(handleUp,MODemo_GetBackBuffer(),256,MO_QUALITY_ENHANCEMENT_NONE);
	
	MO_BlitFrameImage(handleDown,MODemo_GetSubBackBuffer(),256,MO_QUALITY_ENHANCEMENT_NONE);
	
	// Set flip status to 'flip not done', flip will occur at the next VBlank
	gMODemo_FlipStatus = 0;
}

//-----------------------------------------------------------------------------------------------
//- Alarm interrupt handler 
//-----------------------------------------------------------------------------------------------

void AlarmIntr( void* arg )
{
	#pragma unused (arg)

	// Blit transfer flag of Image is ON.
	gMODemo_BlitImageFlag = 1;
}

//-----------------------------------------------------------------------------------------------
//- Boolean used for cover open/close management
//-----------------------------------------------------------------------------------------------

static BOOL gCoverClosed = FALSE;

//-----------------------------------------------------------------------------------------------
//- Plays a MO movie for file
//-----------------------------------------------------------------------------------------------

void MODemo_PlayMovieFileDualScreen(char* fileNameUp,char* fileNameDown)
{
	//-----------------------------------------------------------------------

	MOHandle 	handleUp;
	MOHandle 	handleDown;
	bool 		soundStarted = false;
	u16			lastPadValue = 0;	
	bool 		playbackCancelled = false; // If at some point the playback is cancelled 
										   // (ie the B button is pressed), this value will be true.
	
	//-----------------------------------------------------------------------
	
	// ITCM code relocation is done here, to have an optimal playback speed
	u32 SizeITCM = (u32)OS_GetITCMArenaHi() - (u32)OS_GetITCMArenaLo();
	if (SizeITCM < MO_GetOptimalITCMSize())
		OS_Printf("WARNING: Not enough available ITCM in order to have optimum playback speed ...\n");	
	else
		SizeITCM = MO_GetOptimalITCMSize();
			
	// DTCM code relocation is done here, to have an optimal playback speed
	u32 SizeDTCM = (u32)OS_GetDTCMArenaHi() - (u32)OS_GetDTCMArenaLo();
	if (SizeDTCM < MO_GetOptimalDTCMSize())
		OS_Printf("WARNING: Not enough available DTCM in order to have optimum playback speed ...\n");	
	else
		SizeDTCM = MO_GetOptimalDTCMSize();
	
	MO_InitITCMArea (OS_GetITCMArenaLo(), SizeITCM);
	MO_InitDTCMArea (OS_GetDTCMArenaLo(), SizeDTCM);
			
	//-----------------------------------------------------------------------
	
	OS_Printf("Movie playback has started\n");
	OS_Printf("Looping is enabled\n");
	OS_Printf("----------------------------------------------------\n");
	OS_Printf("Press B button to stop the video playback\n");
	OS_Printf("Press A button to start the video playback\n");
	OS_Printf("Press SELECT button to deactivate/activate looping\n");
	OS_Printf("Press START button to pause/resume video playback\n");
	OS_Printf("----------------------------------------------------\n");
			
	//-----------------------------------------------------------------------

	FSFile fileUp, fileDown;
	
	FS_InitFile(&fileUp);
	FS_InitFile(&fileDown);
			
	if (FS_OpenFile(&fileUp,fileNameUp) == FALSE)
	{
		OS_Printf("ERROR: Cannot open file %s\n",fileNameUp);
		return;
	}
	if (FS_OpenFile(&fileDown,fileNameDown) == FALSE)
	{
		OS_Printf("ERROR: Cannot open file %s\n",fileNameDown);
		return;
	}
	
	// We just check that stack has not been owerwritten by Mobiclip datas by the MO_OpenMovie function
	OS_CheckStack(OS_GetCurrentThread());
	
	// We set the maximum number of frames who can be unpacked without been blitted to the minimum
	// ie 6
	u32 nbFrameBufferedMax = MO_NB_BUFFERED_FRAME_MIN;

	// Open the MO movies from files
	handleUp = MO_OpenMovie(&fileUp, nbFrameBufferedMax);
	if (handleUp == MO_INVALID_HANDLE)
		return;
	
	handleDown = MO_OpenMovie(&fileDown, nbFrameBufferedMax);
	if (handleDown == MO_INVALID_HANDLE)
		return;
	
	// No Frames buffered for the moment
	int nbFrameBuffered = 0;
	
	// Perform looping by default
    bool loopEnabled = true;
    
    // The current frame is the first one
    int currentFrame = 0;
    
    // Retrieve the number of frames of the movie
	u32 totalNbFrames = MO_GetNbFrame(handleUp);
	
	// Retrieve the number of samples per audio packet (it can be 128 or 256 depending on  the audio codec used)
	u32 nbSamplePerAudioPacket = MO_GetNbSamplePerAudioPacket(handleUp);
	
	//-----------------------------------------------------------------------
	
	// Initialization of the sound buffer size in MO sound packet units

	// This is roughly the number of MO sound packet that we can have per frame		
	int nbSoundPacketPerFrame = (int)(((u64)MO_GetAudioFrequency(handleUp) * (u64)16777216) / ((u64)MO_GetVideoFps(handleUp) * (u64)nbSamplePerAudioPacket));
	// We add a packet more to compensate the rounding of the previous computation
	nbSoundPacketPerFrame += 1;
	
	// We want to be able to fill up top 'nbFrameBufferedMax'+1 frames of sound data, 
	// ie 'nbFrameBufferedMax' buffered frames + 1 frame more for the unpack of the next frame to avoid waiting
	// for free space in the circular sound buffer
	gMODemo_SoundBufferNbPackets = nbSoundPacketPerFrame * (nbFrameBufferedMax+1);
	
	// Allocation of the sound buffer
	gMODemo_AudioSampleBuffer = (s16*)malloc(gMODemo_SoundBufferNbPackets * nbSamplePerAudioPacket * 2);
	
	// Clean the sound buffer with 0 values
	(void)memset (gMODemo_AudioSampleBuffer,0,gMODemo_SoundBufferNbPackets*nbSamplePerAudioPacket*2);
	DC_StoreAll();
	
	//-----------------------------------------------------------------------
	
	// No sound packets are streamed by the sound hardware for the moment
	gMODemo_NbSoundPacketStreamed = 0;
	
	// Initialization the index in the sound buffer where we have to put the new audio samples
	gMODemo_AudioSampleBufferIndex = 0;
	
	// No sound packets are pushed by the user for the moment
	gMODemo_NbSoundPacketPushed = 0;

	// Initialization of our display buffer index
    gMODemo_DisplayedBuffer = 0;
    
    // Initialize flip status to 'flip done'
    gMODemo_FlipStatus = 1;
    
    // No frame skip has been performed
    bool	skipDone = false;
	
	// initialize the alarm structure
	OS_CreateAlarm( &alarm );

	// set periodic alarm to call HandlerAlarmIntr() every (1 / framerate) sec after 10 usec.
	OS_SetPeriodicAlarm( &alarm, OS_GetTick()+ALARM_COUNT( 10 ), ALARM_COUNT( (u64)16777216000000/(u64)MO_GetVideoFps( handleUp ) ), &AlarmIntr, (void *)0 );
	
	//-----------------------------------------------------------------------
	
	// Execute this loop while there is an available frame
	while (MO_ReadFrame(handleUp) != 0)
	{
		//-----------------------------------------------------------------------
		// Cover open/close management, turns on/off LCD POWER if detected
		//-----------------------------------------------------------------------
		
		if ( PAD_DetectFold() == TRUE )
		{
			if(!gCoverClosed)
			{
				if(PM_GetLCDPower() == PM_LCD_POWER_ON)
				{
					GX_DispOff();
					GXS_DispOff();
					PM_SetLCDPower( PM_LCD_POWER_OFF );

					gCoverClosed = TRUE;
				}
			}
		}
		else
		{
			if(gCoverClosed)
			{
				if(PM_GetLCDPower() == PM_LCD_POWER_OFF)
				{
					// Big note, when the LCD is off, it totally ignores setBrightness calls
					// so remember to restore the correct brightness when turning the power 
					// back on
					//
					if (PM_SetLCDPower( PM_LCD_POWER_ON ))
					{
						GX_DispOn();
						GXS_DispOn();
						gCoverClosed = FALSE;
					}
				}
			}
			
		}
		
		//-----------------------------------------------------------------------
		
		u64 nbUnpackedSoundPackets;
		
		(void)MO_ReadFrame(handleDown);
			
		// Unpack the current frame image
		MO_UnpackFrameImage(handleUp);
		MO_UnpackFrameImage(handleDown);
		
		// One more frame buffered
		nbFrameBuffered++;
		
		// Sound streaming doesn't work on ensata software emulator
		if (OS_IsRunOnEmulator() == FALSE && MO_GetAudioFrequency(handleUp) > 0) 
		{
			// Unpack the current frame sound
			// There is 'nbUnpackedSoundPackets' sound packets for this frame
			nbUnpackedSoundPackets = MO_GetFrameNbAudioPacket(handleUp);
			
			//-----------------------------------------------------------------------
			
			// This loop unpacks all sound packets of the current frame
			for (int i=0;i<nbUnpackedSoundPackets;i++)
			{
				// If the sound is playing, we do a little bit of synchronization
				if (soundStarted)
				{
					if (gMODemo_NbSoundPacketStreamed >= gMODemo_NbSoundPacketPushed)
						OS_Printf("Sound underflow !!!!\n");
					else
					{
						// 'nbSoundPacketsAvailableInBuffer' contains the size (in number of 
						// sound packets) of available space in the circular buffer to put our new 
						// sound data
						u64 nbSoundPacketsAvailableInBuffer;
					
						// While there is no space available for new sound data we do an active loop
						do
						{
							nbSoundPacketsAvailableInBuffer = gMODemo_NbSoundPacketPushed - gMODemo_NbSoundPacketStreamed;
							nbSoundPacketsAvailableInBuffer = gMODemo_SoundBufferNbPackets - nbSoundPacketsAvailableInBuffer;
						}
						while (nbSoundPacketsAvailableInBuffer == 0);
					}
				}
			
				// We have at least one sound packet free to put our sample data, so we blit one 
				// packet in the circular sound buffer at the position 
				// 'gMODemo_AudioSampleBufferIndex' (this is the user write buffer index)
				MO_BlitFrameSoundOnePacket(handleUp,gMODemo_AudioSampleBuffer+gMODemo_AudioSampleBufferIndex);
				
				// We flush cpu data cache for the memory range written previously to be sure that 
				// the sound playback will be glitch free
				DC_StoreRange(gMODemo_AudioSampleBuffer+gMODemo_AudioSampleBufferIndex,nbSamplePerAudioPacket * 2);
				
				// Increment the user write buffer index by one sound packet sample 
				// If the index goes to the end of the sound buffer, we must put it to the beginning
				gMODemo_AudioSampleBufferIndex+=nbSamplePerAudioPacket;
				if (gMODemo_AudioSampleBufferIndex == gMODemo_SoundBufferNbPackets * nbSamplePerAudioPacket)
					gMODemo_AudioSampleBufferIndex = 0;
			
				// Increment the number of user sound packets pushed
				gMODemo_NbSoundPacketPushed++;
			}
			
			//-----------------------------------------------------------------------
							
			// If the sound is not already playing, we start it
			// Sound is started when: 
			//   - 'nbFrameBufferedMax' frames are decoded.
			//   - all movie frames are decoded, this occurs when movie have less than 
			//     'nbFrameBufferedMax' frames.
			if (soundStarted == false && 
			    (nbFrameBuffered >= nbFrameBufferedMax || nbFrameBuffered == totalNbFrames))
			{
				MODemo_StartSoundStream(MO_GetAudioFrequency(handleUp),nbSamplePerAudioPacket);
				soundStarted = true;
			}
		}

		//-----------------------------------------------------------------------
		
		if (nbFrameBuffered >= nbFrameBufferedMax || MO_GetAudioFrequency(handleUp) == 0)
		{
			// If there is 3 or less audio frames buffered, it is possible not to have enough time
			// to unpack the next frame without sound glitches, so we are skipping the blit of 
			// this frame.
			// The 'skipDone' variable check is used to maintain the framerate to a minimum of half 
			// of the original movie framerate, thus guaranteeing a minimum of playback fluidity.
			// 3 or less audio frames to begin skipping is a safe value and given only as an exemple.
			// It really depends of the video content (most importantly the framerate and the 
			// compression quality criterium).
			// On some movies you could diminish this value (2 audio frames for exemple) or even 
			// not make any frame skipping without sound glitches.
			s32 nbRemainingSoundPacketsToStream = (int)(gMODemo_NbSoundPacketPushed - gMODemo_NbSoundPacketStreamed);
			
			// Note for LCD POWER management:
			// We must not access LCDC VRAM when LCD power is off, in this case we always skip the 
			// blit of the frame.
			// We also don't do skipping if the movie has less than 'nbFrameBufferedMax' frames: as
			// all frames are already decoded, there will not be any sound underflow problems.
			if (gCoverClosed || 
			    (nbRemainingSoundPacketsToStream <= nbSoundPacketPerFrame * 3 && 
			     skipDone == false && 
			     MO_GetAudioFrequency(handleUp) > 0 &&
			     nbFrameBuffered >= nbFrameBufferedMax))
			{
				MO_SkipFrameImage(handleUp);
				MO_SkipFrameImage(handleDown);
				skipDone = true;
			}
			else
			{
				// We blit the current frame images to the video back buffers
				// and we flip video buffers
				MODemo_BlitAndFlip(handleUp, handleDown);
				
				skipDone = false;
			}
		}
		
		//-----------------------------------------------------------------------
		//- Management of user input actions
		//-----------------------------------------------------------------------
		
		u16 padValue = PAD_Read();
		
		if (padValue != lastPadValue)
		{
			lastPadValue = padValue;
			
			
			if (padValue & PAD_BUTTON_SELECT) //Management of loop enable/disable
			{
				loopEnabled = !loopEnabled;
				
				if (loopEnabled == true)
					OS_Printf("Looping is enabled\n");
				else
					OS_Printf("Looping is disabled\n");
			}
			else if (padValue & PAD_BUTTON_START) //Management of pause/resume
			{
				OS_Printf("Playback is paused\n");
				
				if (soundStarted == true)
					MODemo_StopSound();
				
				while (PAD_Read())
					OS_WaitInterrupt(FALSE, OS_IE_V_BLANK);
				while (!(PAD_Read() & PAD_BUTTON_START))
					OS_WaitInterrupt(FALSE, OS_IE_V_BLANK);
					
				OS_Printf("Playback is resumed\n");	
				
				if (soundStarted == true)
				{
					//----------------------------------------------------------------------------
				    //- We dump the existing audio data.
				    //- If playback has been previously paused, we will play 
				    //- (gMODemo_NbSoundPacketStreamed * nbSamplePerAudioPacket) samples 
				    //- with silence, which is the number of audio packets previously pushed 
				    //- without being played.
				    //----------------------------------------------------------------------------
					
					(void)memset(gMODemo_AudioSampleBuffer,0,gMODemo_SoundBufferNbPackets*nbSamplePerAudioPacket*2);
					DC_StoreAll();
					
				    if (gMODemo_NbSoundPacketPushed > gMODemo_NbSoundPacketStreamed)
					    gMODemo_NbSoundPacketPushed = gMODemo_NbSoundPacketPushed - gMODemo_NbSoundPacketStreamed;
					else
						gMODemo_NbSoundPacketPushed = 0;
					
				    gMODemo_NbSoundPacketStreamed = 0;
				    gMODemo_AudioSampleBufferIndex = (u32)gMODemo_NbSoundPacketPushed * nbSamplePerAudioPacket; 	
				    if (gMODemo_AudioSampleBufferIndex == gMODemo_SoundBufferNbPackets * nbSamplePerAudioPacket)
					gMODemo_AudioSampleBufferIndex = 0;
				
					MODemo_StartSoundStream(MO_GetAudioFrequency(handleUp),nbSamplePerAudioPacket);
				}
			}
			else if (padValue & PAD_BUTTON_B) //Quit the movie playback if pressed
			{
				playbackCancelled = true;
				break;
			}
		}
		
		//-----------------------------------------------------------------------
		
		currentFrame++;
		if (currentFrame == totalNbFrames) 
		{
			if (loopEnabled == true)
			{
				currentFrame = 0;
				MO_JumpBeginning(handleUp);
				MO_JumpBeginning(handleDown);
			}
			else
				break;
		}
		
		//-----------------------------------------------------------------------
	}
	
	//-----------------------------------------------------------------------

	if (playbackCancelled == false) 
	{
		// We didn't pressed the B button to cancel the movie, loop mode has 
		// only been disabled.
		// We display the last 'nbFrameBufferedMax-1' buffered video frames 
		// before ending playback.

		for (int i=0;i<nbFrameBufferedMax-1;i++)	
		{	
			// Note for LCD POWER management:
			// We must not access LCDC VRAM when LCD power is off, in this case we always skip the 
			// blit of the frame.
			if (gCoverClosed)
			{
				MO_SkipFrameImage(handleUp);
				MO_SkipFrameImage(handleDown);
			}
			else
				MODemo_BlitAndFlip(handleUp, handleDown);
		}
	}
	
	// We do an active loop until the last frame has been effectively flipped.
	while (gMODemo_FlipStatus == 0){};
	
	//-----------------------------------------------------------------------
	
	// We stop the sound playback if necessary
	if (soundStarted == true)
		MODemo_StopSound();
	
	//-----------------------------------------------------------------------
	
	// Cancel the alarm 
	OS_CancelAlarm( &alarm );
	
	// Release of the sound buffer
	free(gMODemo_AudioSampleBuffer);
	
	// In the case where we opened the MO movie with MO_LOOP_DISABLE option,
	// we go there and so we stop the close the MO movie
	MO_CloseMovie (handleUp);	
	MO_CloseMovie (handleDown);	
	
	FS_CloseFile(&fileUp);
	FS_CloseFile(&fileDown);
	
	OS_Printf("Movie playback has ended\n");
		
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
    
    // Enables banks A and B on LCDC
	GX_SetBankForLCDC(GX_VRAM_LCDC_A | GX_VRAM_LCDC_B);

	// Enables graphics display of Main LCD on VRAM A
	GX_SetGraphicsMode(GX_DISPMODE_VRAM_A, (GXBGMode)0, (GXBG0As)0 );

	// Main display is on Top Screen , Sub Display is on Bottom Screen
	GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
	
	//---------------------------------------------------------------------------------
	//- GXS initialisation, Oam init for OBJ mode display of 16 bits bitmaps on Sub LCD
	//---------------------------------------------------------------------------------

	GXS_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_2D_W256);
    
    for (int i = 0; i < 128; ++i)
    {
        gOamAttr[i].attr01 = 0;
        gOamAttr[i].attr23 = 0;
    }
    
	int idx = 0;
    for (int y = 0; y < 192; y += 64)
    {
        for (int x = 0; x < 256; x += 64, idx++)
        {
            G2_SetOBJAttr(&gOamAttr[idx],
                          x,
                          y,
                          0,
                          GX_OAM_MODE_BITMAPOBJ,
                          FALSE,
                          GX_OAM_EFFECT_NONE,
                          GX_OAM_SHAPE_64x64,
                          GX_OAM_COLOR_16,
                          (y / 8) * 32 + (x / 8),
                          15,
                          0);
        }
    }
    	
    DC_FlushRange(&gOamAttr[0], sizeof(gOamAttr));
    GXS_LoadOAM(&gOamAttr[0], 0, sizeof(gOamAttr));

	//--------------------------------------
	//- Init of Sub LCD BG and OBJ 
	//--------------------------------------
	
	(void)GX_ResetBankForSubBG();
	(void)GX_ResetBankForSubOBJ();
	
	GX_SetBankForSubOBJ(GX_VRAM_SUB_OBJ_128_D);			// VRAM-D for OBJs
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);       	// VRAM-C for BGs
    
	GXS_SetGraphicsMode(GX_BGMODE_5);
    G2S_SetBG2ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256,
                           GX_BG_AREAOVER_XLU,
                           GX_BG_BMPSCRBASE_0x00000);
    G2S_SetBG2Priority(0);
    G2S_BG2Mosaic(FALSE);
        
    // Init of Sub LCD on BG mode on VRAM C
    GXS_SetVisiblePlane(GX_PLANEMASK_BG2);

	//--------------------------------------
	//- Enable Display on Main and Sub LCD
	//--------------------------------------

	// Waiting the end of VBlank interrupt
    SVC_WaitVBlankIntr();
    // Enable display of Main LCD sreen
    GX_DispOn();
	// Enable display of Sub LCD sreen
	GXS_DispOn();
    
    //----------------------------
	//- FS initialisation
	//----------------------------
    
    FS_Init(1);
	
	//----------------------------
	//- Sound initialisation
	//----------------------------
	
    SND_Init();
    
    //--------------------------------------------------
	//- Play the 2 different movie files on both screens
	//--------------------------------------------------
	
	do
	{
		MODemo_PlayMovieFileDualScreen("videos/DS_up.mods","videos/DS_down.mods");
		
		while (!(PAD_Read() & PAD_BUTTON_A))
			OS_WaitInterrupt(FALSE, OS_IE_V_BLANK);
		
	}
	while(true);
		
	//--------------------------------------
	//- End Nitro main
	//--------------------------------------
	
	OS_Terminate();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------