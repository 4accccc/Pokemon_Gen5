##############################################################################
##                                                                          ##
##  Readme-TWLMiddleware-libMobiclip-1_2_2-20100903-en_US.txt               ##
##                                                                          ##
##  Mobiclip 1.2.2                                                          ##
##                                                                          ##
##  September 3rd, 2010                                                     ##
##                                                                          ##
##############################################################################



[Introduction]

Mobiclip Middleware for TWL is a library which uses the Mobiclip video
codec from Mobiclip, Inc.
This library makes playback of high quality video possible; therefore it can 
be used in variety of scenes in TWL and Nitro game software.
To play videos with the library, a video file must first be converted to the
Mobiclip for TWL video format. 
For detailed information about library usage and its package content, 
please see libMobiclip/docs/en_US/Mobiclip/QuickStart.pdf.



[ Installation of the SDK ] 

    To install the Mobiclip Middleware for TWL, unzip the downloaded package to
    the desired location.



[ Building the sample demos ]

    Please define the TWL_LIBMOBICLIP_ROOT environment variable to where 
    libMobiclip directory resides. Please note that this path must not contain
    any white spaces.  

    There is a Makefile in each sample code directory, please use the make
    command from a Cygwin shell to buid the demos:
     * type 'make TWLSDK_PLATFORM=NITRO' in you want to compile for the Nitro 
       platform.
     * type 'make TWLSDK_PLATFORM=TWL TWL_ARCHGEN=LIMITED' in you want to 
       compile for TWL platform in LIMITED mode.
     * type 'make TWLSDK_PLATFORM=TWL TWL_ARCHGEN=HYBRID' in you want to 
       compile for TWL platform in HYBRID mode.

    For detailed information about Mobiclip API usage, please see 
    libMobiclip/man/en_US/index.html for Mobiclip function reference manual.



[ Note : About the Copyright Notice ]
  
    In order to use the Mobiclip libraries in final products, it is
    mandatory to add a copyright notice in the product instruction manual.
    Please refer to soft print guideline, and e-manual guideline provided by 
    Nintendo to display sentences.

    
    
[ About Mobiclip Tools for NINTENDO DS License ]

    You need to obtain a license file in order to use the Mobiclip VFW Codec
    and its related tools. 
    See libMobiclip/docs/en_US/Mobiclip/QuickStart.pdf for more information.



[ Supported SDK version ]

    This library has been tested with...
    - TWL-SDK 5.5
    - TWL-SDK 5.4 + patch 2
    - CodeWarrior for NINTENDO DSi v1.2 + patch 2

    Also AviDoMobiclipDs program and Mobiclip Video For Windows codec have been
    tested on:
    - Windows XP SP3.
    - Windows 7 64 bits.



[ CPU requirements for Mobiclip VFW codec]

    The Mobiclip VFW codec makes extensive use of SSE2 x86 instruction set.
    Any CPU that don't support this instruction set will make the VFW codec crash.

    Here is a list of CPUs that are known to support the SSE2 instruction set:
    - Pentium 4 or later Intel CPUs and all Intel Centrino platforms.
    - Athlon 64 or later AMD CPUs.

    Here is a list of CPUs that are known NOT to support the SSE2 instruction set:
    - Pentium 3 or older Intel CPUs.
    - Athlon XP or older AMD CPUs.



[ Changes ]

Version 1.2.2 (September 3rd, 2010)
    + Added 'Mobiclip_Tools_for_DS.exe' self-installer. It installs the 
      Mobiclip VFW codec and the AviToMobiclipDS.exe program.
    + Modifications done in Mobiclip VFW codec:
        - StarForce protection system dropped, replaced by Mobiclip license 
          system.
        - installation using .inf file dropped, now installed by 
          'Mobiclip_Tools_for_DS.exe' self-installer.
        - GUI has changed.
        - New features added, some have been removed; please read 
          'Mobiclip Video for Windows (VFW) codec description' chapter of 
          QuickStart.pdf document for more information.
    + Updated QuickStart and MobiclipFAQ documents.
    + Following documents removed:
        - AboutLogo.pdf
        - Starforce protection - Administration Guide.pdf
        - Starforce protection - Automatic Activation.pdf
        - Starforce protection - Manual Activation.pdf
        - VX To Mobiclip Shifting Guide.pdf
    + Modifications done in html reference documentation in 
      MO_JumpNextIframe.html: described precise behaviour of the 
      MO_JumpNextIframe function.
        

Version 1.2.1 (October 22nd, 2009)
    + Removed bug in AviToMobiclipDS program: converted movies were showing
      audio artifacts at playback when Adobe Flash program was used to 
      generate uncompressed Avi file(used later as source for video 
      compression).
    + Updated QuickStart document: added batch job usage using 
      VirtualDub V1.9.6 in 'Multipass Encoding Howto' chapter.
    + Added Makefile in libMobiclip/build/demos/Mobiclip directory, used to
      compile all demos at once.
    + Modifications done in html reference documentation in singleScreen.html,
      singleScreenStereo.html, dualScreen.html and effectiveUsage.html files: 
      main.srl renamed to the real demo name in 'Compile Method' section
      (singleScreen.srl for example for the singleScreen demo).
    + Added 'CPU requirements for Mobiclip VFW codec' section in this file.


Version 1.2.0 (April 23rd, 2009)
    + Developpement environment changed from NitroSDK to TwlSDK.
    + Modifications done to singleScreen, singleScreenStereo, dualScreen and 
      multitrackAudio demos: sound was not playing for all TWL HYBRID and 
      LIMITED builds.
    + Modified build system of sample codes:
        - CodeWarrior projects have been deleted.
        - Build process is done now using makefiles in the same manner as the
          sample codes in the TwlSDK.
    + New libMobiclip libraries are added for TWL HYBRID and LIMITED targets:
        - libMobiclip.TWL.HYB.a
        - libMobiclip.TWL.LTD.a
    + Updated html reference documentation about the new ways to compile the
      sample codes for TWL target.
    + Updated QuickStart document about the support of the new TWL targets 
      and their related libraries.


Version 1.1.1 (March 9th, 2009)
    + Modifications done in effectiveUsage demo: removed a bug that could make 
      the playback loop forever if the decode is too slow.
    + Modifications done to singleScreen, singleScreenStereo, dualScreen and 
      multitrackAudio demos:
        - modifications to make the NINTENDO DS cover management match 
          the specification of NINTENDO DS hardware:
              * Added GX_DispOff/GXS_DispOff function calls before calling
                PM_SetLCDPower with 'PM_LCD_POWER_OFF' as parameter.
              * Added GX_DispOn/GXS_DispOn function calls after having 
                called PM_SetLCDPower with 'PM_LCD_POWER_ON' as parameter. 
        - modifications to allow playback of movies containing less than 
          'MO_NB_BUFFERED_FRAME_MIN' frames.
    + Added function to AviToMobiclipDS application:
        - Now always display an error when input audio is not PCM 16 bits.
        - Is able to read BWF WAV format, sound must be PCM 16 bits mono or 
          stereo.
        - Removed a crash with VirtualDub generated WAV files given as input.
        - Program version is now V1.2.0.2.


Version 1.1.0 (November 21st, 2008)

    + Modified sample demos:
        - singleScreen, singleScreenStereo and dualScreen demos modified because 
          no processing was done when DS closed during the end of movie playback.
    + Added function to AviToMobiclipDS application
        - Added IMA ADPCM and PCM support.
        - Use multiple [audio] options to specify different tracks.
          Doing multi-track audio video playback is now possible with more than
          2 audio tracks.
          There is no number of audio track limitation.
          Each audio track consumes about 5.2KB of main memory.
            Limitations:
              * All audio tracks must have the same frequency.
              * All audio tracks must have the same number of channels.
              * If more than one [audio] option is used, [debugsound] option 
                cannot be used.
    + Updated QuickStart
        - Added documentation about the new audio formats.
    + Updated Reference manual(Html)
        - Added restriction usage of MO_SkipFrameSoundOnePacket.
    + Added Sample code.
        - Added multitrackAudio sample code. It shows how to dynamically change
          the current played audio track.


Version 1.0.3 (September, 22nd 2008)

    + Upgraded Starforce protection so the Video For Windows codec can be
      installed and used on Windows Vista.
    + Updated Starforce documentation:
        - Added section about serial number associated limitations
        - Updated screenshots to match the new starforce version
        - Added section about renewing codec
    + Updated Reference manual(Html)
        - Added forgotten links in the a-z function listing.


Version 1.0.2 (May, 7th 2008)

    + Added function to AviToMobiclipDS application 
        - By selecting [audio] option, an external wav file can be used for the
          sound source. Application version is now V1.1.0.0.
    + Sample code has been modified to fix the below issue.
        - when the following conditions where met at the same time in the sample 
          code of singleScreen, singleScreenStereo, and dualScreen, 
          an arithmetic bug occurred:
                * There was a sound buffer underflow.
                * The sound playback was paused and restarted.
    + Added document to the package 
        - 'VX To Mobiclip Shifting Guide.pdf'. 
                * How to port a libVX application to Mobiclip has been 
                  explained.
        - 'MobiclipFAQ.pdf'. 
                * Support information regarding Frequently Asked 
                  Questions related to VX middleware has been created.
    + Added function to Mobiclip Video Codec For Windows 
        - By disabling keyframe detection you can avoid the insertion of 
          keyframes automatically. 


Version 1.0.1 (March, 14th 2008)

    + Sample code modifications:
        - define 4KB of stack to free up DTCM space
        - use OS_GetDTCMArenaHi and OS_GetDTCMArenaLo to compute available DTCM
          space.
        - added OS_CheckStack call after MO_OpenMovie call to check that stack 
          is not overwritten by Mo_OpenMovie function.
    + Html documentation updated:
        - added information about the need of specific stack allocation in lcf 
          files to free up DTCM space for Mobiclip usage.
            * all references to 'SDRAM memory' replaced by 'main memory'.


Version 1.0 (March, 3rd 2008)

    + Initial release
