
echo off

REM ============================================
REM     ROM版を作るために現状の定義を保存
REM ============================================
SET TMP_PM_DEBUG=%PM_DEBUG%
SET TMP_NITRO_FINALROM=%NITRO_FINALROM%
SET TMP_TWL_FINALROM=%TWL_FINALROM%
SET TMP_PM_DEBUG=%PM_DEBUG%
SET TMP_TWLSDK_PLATFORM=%TWLSDK_PLATFORM%
SET TMP_TARGET_PLATFORM=%TARGET_PLATFORM%

REM cp -r /cygdrive/e/home/gflib/gflib/src/network/*.* /cygdrive/c/home/gflib/gflib/src/network/
REM cp -r /cygdrive/e/home/gflib/gflib/src/network/tool/*.* /cygdrive/c/home/gflib/gflib/src/network/tool/
REM cp -r /cygdrive/e/home/gflib/gflib/src/ui/*.* /cygdrive/c/home/gflib/gflib/src/ui/
REM cp -r /cygdrive/e/home/gflib/gflib/include/*.* /cygdrive/c/home/gflib/gflib/include/


REM ============================================
REM     Release用定義指定
REM ============================================
SET PM_DEBUG=yes
SET NITRO_FINALROM=
SET TWL_FINALROM=


REM ============================================
REM     通常版を作成
REM ============================================
call e:\home\pokemon_wb\nitro_prompt.bat cmd mkgl.bat %1
call e:\home\pokemon_wb\prompt.bat cmd mkgl.bat %1


REM ============================================
REM     ROM用定義指定
REM ============================================
SET PM_DEBUG=no
SET NITRO_FINALROM=yes
SET TWL_FINALROM=yes


REM ============================================
REM     ROM版作成
REM ============================================
call e:\home\pokemon_wb\nitro_prompt.bat cmd mkgl.bat %1
call e:\home\pokemon_wb\prompt.bat cmd mkgl.bat %1


REM ============================================
REM     定義を元に戻す
REM ============================================
SET PM_DEBUG=%TMP_PM_DEBUG%
SET NITRO_FINALROM=%TMP_NITRO_FINALROM%
SET TWL_FINALROM=%TMP_TWL_FINALROM%
SET TWLSDK_PLATFORM=%TMP_TWLSDK_PLATFORM%
SET TARGET_PLATFORM=%TMP_TARGET_PLATFORM%
