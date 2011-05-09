#ifndef NITRO_LIBMOBICLIP_H_
#define NITRO_LIBMOBICLIP_H_

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#include <nitro/types.h>

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*--------------------------------------------------------------------------*/
/* If MO_OpenMovie function fails, MO_INVALID_HANDLE is returned.           */
/*--------------------------------------------------------------------------*/

#define MO_INVALID_HANDLE   0

/*--------------------------------------------------------------------------*/
/* This is the minimum value allowed for the parameter 'nbBufferedFrameMax' */
/* of 'MO_OpenMovie' function                                               */
/*--------------------------------------------------------------------------*/

#define MO_NB_BUFFERED_FRAME_MIN    6

/*--------------------------------------------------------------------------*/
/* These are the various quality enhancement options you can pass to the    */
/* 'MO_BlitFrameImage' function                                             */
/* The more the quality enhancement used, the higher the CPU requirements   */
/* will be.                                                                 */
/*--------------------------------------------------------------------------*/

#define MO_QUALITY_ENHANCEMENT_NONE		0
#define MO_QUALITY_ENHANCEMENT_MEDIUM	1
#define MO_QUALITY_ENHANCEMENT_HIGH		2

/*--------------------------------------------------------------------------*/
/* This is a Mobliclip movie handle.                                        */
/* Create one whith either MO_OpenMovieFromMemory or MO_OpenMovieFromFile   */
/* functions.                                                               */
/*--------------------------------------------------------------------------*/

typedef void*   MOHandle;

/*--------------------------------------------------------------------------*/
/* Allocator/Deallocator functions :                                        */
/*                                                                          */
/* MO_Malloc and MO_Free are used to perform any dynamic                    */
/* allocation/deallocation done in the Mobiclip library.                    */
/*                                                                          */
/* MO_Malloc and MO_Free are not implemented in the Mobliclip library,   	*/
/* but instead declared as extern.                                          */
/* You must provide an implementation to these functions in your program.   */
/* This gives you freedom to use classic malloc/free functions or any       */
/* custom allocation routines you may have.                                 */
/* Infos :                                                                  */
/*      MO_Malloc is used only by 'MO_OpenMovie' and                        */
/*      'MO_BlitFrameImage' functions.                                      */
/*      MO_Free is used only by 'MO_CloseMovie' function.                   */
/*--------------------------------------------------------------------------*/

/* Implement this function in your program */
extern void*    MO_Malloc   ( u32 size );

/* Implement this function in your program */
extern void     MO_Free     ( void* mem_p );

/*--------------------------------------------------------------------------*/
/* Retrieves the quantity in bytes of ITCM needed by Mobiclip to achieve    */
/* optimal playback performance. This function is used for the code         */
/* relocation management.                                                   */
/*                                                                          */
/* Parameters :                                                             */
/*      None.                                                               */
/*                                                                          */
/* Returns :                                                                */
/*      (out) Size in bytes of the desired ITCM quantity.                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/

u32		MO_GetOptimalITCMSize	( void );

/*--------------------------------------------------------------------------*/
/* Retrieves the quantity in bytes of DTCM needed by Mobiclip to achieve 		*/
/* optimal playback performance. This function is used for the data         */
/* relocation management.                                                   */
/*                                                                          */
/* Parameters :                                                             */
/*      None.                                                               */
/*                                                                          */
/* Returns :                                                                */
/*      (out) Size in bytes of the desired DTCM quantity.               	*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

u32		MO_GetOptimalDTCMSize	( void );

/*--------------------------------------------------------------------------*/
/* Give to Mobiclip the address and size in bytes of the usable ITCM        */
/* area. This function is used for the code relocation management.          */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) void* itcmPtr : Address of the usable ITCM area.           	*/
/*      (in) u32 size      : Size in bytes of the usable ITCM area.     	*/
/*                                                                          */
/* Returns :                                                                */
/*      Nothing.                                                            */
/*                                                                          */
/* Infos :                                                                  */
/*      If this function is not called or the size given in parameter is 0, */
/* Mobiclip core routines will run from SDRAM memory.                		*/
/*      If the size given in parameter is greater or equal to the value		*/
/* returned by the 'MO_GetOptimalITCMSize' function, all the core routines  */
/* of Mobiclip will be relocated to this area.                           	*/
/*      If the size given in parameter is less than the value returned by   */
/* 'MO_GetOptimalITCMSize' function, Mobiclip will try to relocate   		*/
/* parts of its core routines to this area.                                 */
/*      If only parts or none of the core routines resides in ITCM, 	  	*/
/* playback performance will not be optimal.                       			*/
/*      You can call this function several times with different paramaters; */
/* for each call the code relocation state will be reseted.                 */ 
/* This function must not be called when a Mobiclip movie handle is still   */
/* opened.																	*/
/*--------------------------------------------------------------------------*/

void    MO_InitITCMArea   (void* itcmPtr, u32 size );

/*--------------------------------------------------------------------------*/
/* Give to Mobiclip the address and size in bytes of the usable DTCM   		*/
/* area. This function is used for the data relocation management.          */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) void* dtcmPtr : Address of the usable DTCM area.          		*/
/*      (in) u32 size      : Size in bytes of the usable DTCM area.    		*/
/*                                                                          */
/* Returns :                                                                */
/*      Nothing.                                                            */
/*                                                                          */
/* Infos :                                                                  */
/*      If this function is not called or the size given in parameter is 0, */
/* Mobiclip core data will reside in SDRAM memory.                  		*/
/*      If the size given in parameter is greater or equal to the value  	*/
/* returned by 'MO_GetOptimalDTCMSize' function, all the core data of       */
/* Mobiclip will be relocated to this area.                              	*/
/*      If the size given in parameter is less than the value returned by   */
/* 'MO_GetOptimalDTCMSize' function, Mobiclip will try to relocate   		*/
/* parts of its core data to this area.                                    	*/
/*      If only parts or none of the core data resides in DTCM, 			*/
/* playback performance will not be optimal.                       			*/
/*      You can call this function several times with different paramaters; */
/* for each call the data relocation state will be reseted.                 */ 
/* This function must not be called when a Mobiclip movie handle is still   */
/* opened.																	*/
/*--------------------------------------------------------------------------*/

void    MO_InitDTCMArea   (void* dtcmPtr, u32 size );


/*--------------------------------------------------------------------------*/
/* Opens a Mobiclip movie from a file.                                      */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) FSFile *file           : Already opened NITRO file handle.  	*/
/*      (in) u32 nbBufferedFrameMax : Maximum number of frames who can  	*/
/*                                    be successively unpacked without been */
/*                                    blitted or skipped.                   */
/*                                    MO_NB_BUFFERED_FRAME_MIN (6) is       */
/*                                    the minimum value allowed             */
/*                                                                          */
/* Returns :                                                                */
/*      (out) a MOHandle value if ok, MO_INVALID_HANDLE if error.           */
/*                                                                          */
/* Cause of error :                                                         */
/*      - impossible to Read Mobiclip header.                               */
/*      - incompatible file format (typically, this is not a valid Mobiclip */
/*        movie).                                                           */
/*      - wrong movie resolution (it must not exceed 256x192).              */
/*      - not enough memory (allocation error).                             */
/*      - 'nbBufferedFrameMax' value is below the minimum allowed           */
/*                                                                          */
/* Infos :                                                                  */
/*      'MO_Malloc' is called inside this function.                         */
/*--------------------------------------------------------------------------*/

MOHandle    MO_OpenMovie		        ( FSFile *file, 
                                          u32 nbBufferedFrameMax );

/*--------------------------------------------------------------------------*/
/* Closes a Mobiclip movie.                                                 */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle   : Valid handle movie, if this is                  	*/
/*                        MO_INVALID_HANDLE the function does nothing.      */
/*                                                                          */
/* Returns :                                                                */
/*      nothing.                                                            */
/*                                                                          */
/* Infos :                                                                  */
/*      'MO_Free' is called inside this function.                           */
/*--------------------------------------------------------------------------*/

void        MO_CloseMovie               ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Gets the number of frames of the movie.                                  */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : the number of frames of the movie or 0 if the handle is */
/*                  MO_INVALID_HANDLE.                                      */
/*--------------------------------------------------------------------------*/

u32         MO_GetNbFrame               ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Gets the number of IFrames in the movie.                                 */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : the number of IFrames of the movie or 0 if the handle   */
/*                  is MO_INVALID_HANDLE.                                   */
/*--------------------------------------------------------------------------*/

u32         MO_GetNbIFrame              ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Gets the framerate of the movie.                                         */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : the framerate of the movie or 0 if the handle is        */
/*                  MO_INVALID_HANDLE.                                      */
/*                                                                          */
/* Infos :                                                                  */
/*      The framerate is expressed in 8.24 fixed point format,              */
/*      ie : framerate = (u32)(number_of_frames_per_second * 16777216.f).   */
/*--------------------------------------------------------------------------*/

u32         MO_GetVideoFps              ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Gets the width of the movie in pixels.                                   */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : the width of the movie, 0 if the handle is              */
/*                  MO_INVALID_HANDLE.                                      */
/*--------------------------------------------------------------------------*/

u32         MO_GetVideoWidth            ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Gets the height of the movie in pixels.                                  */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : the height of the movie, 0 if the handle is             */
/*                  MO_INVALID_HANDLE.                                      */
/*--------------------------------------------------------------------------*/

u32         MO_GetVideoHeight           ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Gets the frequency of the movie audio, in Hz.                            */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : the frequency of the movie audio, 0 if the handle is    */
/*                  MO_INVALID_HANDLE or when there is no audio track in    */
/*                  the movie.                                              */
/*--------------------------------------------------------------------------*/

u32         MO_GetAudioFrequency        ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Gets the number of audio tracks in the movie.                            */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : the number of audio tracks in the movie, 0 if the       */
/*                  handle is MO_INVALID_HANDLE or when there is no audio   */
/*                  track in the movie.                                     */
/*--------------------------------------------------------------------------*/

u32         MO_GetNbAudioTrack          ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Gets the current read frame number of the movie.                         */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : the current read frame number.                          */
/*--------------------------------------------------------------------------*/

u32         MO_GetCurrentFrameNumber    ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Gets the frame number of the IFrame indexed by 'index'.                  */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*      (in) u32 index       : an index of the IFrame on which we want the  */
/*                             frame number.                                */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : the frame number of the IFrame indexed by 'index'.      */
/*--------------------------------------------------------------------------*/

u32         MO_GetIFrameInfo            ( MOHandle handle, u32 index );


/*--------------------------------------------------------------------------*/
/* Gets the number of samples of each audio packet stored in the movie.     */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : can be 128 or 256 depending of the audio codec used,    */
/*      0 if the handle is invalid.                                         */
/*--------------------------------------------------------------------------*/

u32         MO_GetNbSamplePerAudioPacket ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Reads a movie frame.                                                     */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : 1 if no error, 0 if error or if the handle is           */
/*                  MO_INVALID_HANDLE.                                      */
/*--------------------------------------------------------------------------*/

u32         MO_ReadFrame                ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Go to the beginning of the movie.                                        */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : 1 if no error, 0 if error or if the handle is           */
/*                  MO_INVALID_HANDLE.                                      */
/*--------------------------------------------------------------------------*/

u32         MO_JumpBeginning            ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Go to the next movie Iframe.                                             */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : 1 if no error, 0 if error or if the handle is           */
/*                  MO_INVALID_HANDLE.                                      */
/*--------------------------------------------------------------------------*/

u32         MO_JumpNextIframe           ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Go to the previous movie Iframe.                                         */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : 1 if no error, 0 if error or if the handle is           */
/*                  MO_INVALID_HANDLE.                                      */
/*--------------------------------------------------------------------------*/

u32         MO_JumpPreviousIframe       ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Go to an indexed IFrame.                                                 */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*      (in) u32 index       : an index of the IFrame on which we want to   */
/*                             jump.                                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : 1 if no error, 0 if the handle is MO_INVALID_HANDLE or  */
/*                  when index is out of range (the maximum value allowed   */
/*                  is MO_GetNbIFrame()-1.                                  */
/*--------------------------------------------------------------------------*/

u32         MO_JumpToIframe             ( MOHandle handle, u32 index );

/*--------------------------------------------------------------------------*/
/* Unpack a frame and put it into internal image FIFO.						*/
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : 1 if no error, 0 if error (no free internal buffer      */
/*                  available) or if the handle is MO_INVALID_HANDLE.       */
/*                                                                          */
/* Infos :                                                                  */
/*      Mobiclip stores unpacked frames in an internal FIFO.       			*/
/*                                                                          */
/*      Each time MO_UnpackFrameImage is called, the new unpacked image is  */
/*      added to the FIFO.                                                  */
/*                                                                          */
/*      Each time MO_BlitFrameImage or MOSkipFrameImage function is 	    */
/*      called, the oldest unpacked image is removed from the FIFO (after   */
/*      being blitted or skipped).                                          */
/*                                                                          */
/*      You can call MO_UnpackFrameImage nbBufferedFrameMax times     		*/
/*      (see MO_OpenMovie) without the need to call 						*/
/*			MO_BlitFrameImage or MOSkipFrameImage.     						*/
/*                                                                          */
/*      This is a useful feature which allows to do frame buffering :       */
/*      At some point in the movie playback, calling MO_UnpackFrameImage    */
/*      can be a time consuming operation which can cause sound buffer      */
/*      underflow. Prebuffering images allow to smooth occasional    		*/
/*      CPU peaks, thus preventing sound buffer underflow.                  */   
/*--------------------------------------------------------------------------*/

u32     MO_UnpackFrameImage             ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Gets the number of audio packets per track for the current frame.        */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : the number of audio packets for the current frame for   */
/*                  each audio track, 0 if the handle is MO_INVALID_HANDLE  */
/*                  or when there is no audio track in the movie.           */
/*                                                                          */
/* Infos :                                                                  */
/*      An audio packet size is either 128 or 256 samples of PCM 16 bits.   */
/*                                                                          */
/*      Each audio track has the same number of packets for any given       */
/*      frame. Total number of audio packets for a given frame will be :    */
/*          MO_GetFrameNbAudioPacket(handle) * MO_GetNbAudioTrack(handle)   */
/*                                                                          */
/*      If you have more than one audio track, the audio packets of the     */
/*      different tracks are interleaved.                                   */
/*      For example if you have 2 audio tracks, the audio packets are       */
/*      interleaved in this manner : Packet0_Track0, Packet0_Track1,        */
/*      Packet1_Track0, Packet1_Track1, etc ...                             */
/*--------------------------------------------------------------------------*/

u32         MO_GetFrameNbAudioPacket    ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/* Blit the oldest unpacked image from the internal FIFO into a user       	*/
/* buffer and remove it from the FIFO.                                      */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle         : a valid handle movie.                */
/*      (in) u16* outFrameBufferPtr  : the pointer where the image frame    */
/*                                     must be blitted.                     */
/*      (in) u32 outFrameBufferPitch : the pitch (or scanline) in pixels of */
/*                                     the destination frame buffer.        */
/*      (in) u32 qualityEnhancement : the kind of visual quality            */
/*                                    enhancement wanted.                   */
/*                                                                          */
/* Infos :                                                                  */
/*      Output is done in NITRO Direct Color format (16 bits).              */
/*      To avoid unnecessary copies, give a direct address to video memory  */
/*      in 'outFrameBufferPtr'.                                             */
/*      'qualityEnhancement' parameter can have the following values:       */
/*       - MO_QUALITY_ENHANCEMENT_NONE: a simple color conversion is        */
/*         performed.                                                       */
/*       - MO_QUALITY_ENHANCEMENT_MEDIUM                                    */
/*		   - MO_QUALITY_ENHANCEMENT_HIGH: apply a post-filtering before     */
/*         color conversion, used to reduce video compression artefacts.    */
/*         The higher the quality enhancement is, the higher the CPU      	*/
/*         requirements will be. See effectiveUsage sample for an example   */
/*         on how to use this feature.								  	    */
/*                                                                          */
/*      'MO_Malloc' is called inside this function the first time a blit is */
/*      performed with visual quality enhancement.                          */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : 1 if no error, 0 if error (blit not performed) or if    */
/*                  the handle is MO_INVALID_HANDLE.                        */
/*--------------------------------------------------------------------------*/

u32     MO_BlitFrameImage               ( MOHandle handle,
                                          u16* outFrameBufferPtr,
                                          u32 outFrameBufferPitch,
                                          u32 qualityEnhancement );

/*--------------------------------------------------------------------------*/
/* Skip the oldest unpacked image, which means remove the oldest unpacked 	*/
/* image from the internal FIFO without blitting it.                        */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle         : a valid handle movie.                */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : 1 if no error, 0 if error (skip not performed) or if    */
/*                  the handle is MO_INVALID_HANDLE.                        */
/*                                                                          */
/* Infos :                                                                  */
/*      In some occationnal cases sound buffer underflow can occur, even    */
/*      when doing frame buffering. This can generally be seen with videos  */
/*      compresssed with very high quality settings.                        */
/*      This function can be used to gain additionnal time by not blitting  */
/*      the oldest unpacked image.                                          */
/*--------------------------------------------------------------------------*/
                                          
u32     MO_SkipFrameImage               ( MOHandle handle );                                          

/*--------------------------------------------------------------------------*/
/* Blit a packet of the current unpacked sound to a user buffer.           	*/
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle     : a valid handle movie.                    */
/*      (in) s16* outSoundBuffer : the pointer where the sound frame must   */
/*                                 be blitted.                              */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : 1 if no error, 0 if there is no more audio packet       */
/*                  available or if the handle is MO_INVALID_HANDLE.        */
/*                                                                          */
/* Infos :                                                                  */
/*      The output is in 16 bits PCM format.                                */
/*      To avoid unnecessary copies, give a direct address to your  		*/
/*      currently playing (circular) sound buffer using 'outSoundBuffer'.   */
/*--------------------------------------------------------------------------*/

u32     MO_BlitFrameSoundOnePacket      ( MOHandle handle,
                                          s16* outSoundBuffer );

/*--------------------------------------------------------------------------*/
/* Skips a packet of the current unpacked sound.                            */
/*                                                                          */
/* Parameters :                                                             */
/*      (in) MOHandle handle : a valid handle movie.                        */
/*                                                                          */
/* Returns :                                                                */
/*      (out) u32 : 1 if no error, 0 if there is no more audio packet       */
/*                  available or if the handle is MO_INVALID_HANDLE.        */
/*                                                                          */
/* Infos :                                                                  */
/*      This function is useful if you want to skip one or more audio       */
/*      tracks. Example: if you have 2 audio tracks, the first in English   */
/*      and the second in Japanese :                                        */
/*      If you want to play the English track you will do the following    	*/
/*      code sequence :                                                     */
/*          MO_BlitFrameSoundOnePacket, MO_SkipFrameSoundOnePacket times    */
/*      the value returned by MO_GetFrameNbAudioPacket.                     */
/*      If you want to play the Japanese track you will do the following   	*/
/*      code sequence :                                                     */
/*          MO_SkipFrameSoundOnePacket, MO_BlitFrameSoundOnePacket times    */
/*      the value returned by MO_GetFrameNbAudioPacket.                     */
/*--------------------------------------------------------------------------*/

u32     MO_SkipFrameSoundOnePacket      ( MOHandle handle );

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#endif /* NITRO_LIBMOBICLIP_H_ */
