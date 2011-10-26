@echo off

rem ----------------------------
rem batのあるディレクトリを基準
rem ----------------------------
SET BATCH_HOME=%~dp0
SET OUT_HOME=%~dp1

rem ------------------------
rem 単独実行時エラー
rem ------------------------
if "%1" == "" goto NOT_EXIST


rem ------------------------
rem 正しくドラッグされた場合
rem ------------------------
cd %OUT_HOME%
%BATCH_HOME%\wbt_dldata_conv.exe %1


rem --------------
rem 日本語版(未使用)
rem --------------
rem if not exist "%~n1.bin" goto TEST_END
rem my_crc_ccitt.exe %~n1.bin %~n1_crc.bin
rem del %~n1.bin
rem :TEST_END

rem --------------
rem 日本語版
rem --------------
if not exist "%~n1_JJ.bin" goto J_END
%BATCH_HOME%\my_crc_ccitt.exe %~n1_JJ.bin %~n1_J2.bin
%BATCH_HOME%\my_crc_ccitt.exe %~n1_J2.bin %~n1_J.bin -t
del %~n1_JJ.bin
del %~n1_J2.bin
:J_END

rem --------------
rem 英語版
rem --------------
if not exist "%~n1_EE.bin" goto E_END
%BATCH_HOME%\my_crc_ccitt.exe %~n1_EE.bin %~n1_E2.bin
%BATCH_HOME%\my_crc_ccitt.exe %~n1_E2.bin %~n1_E.bin -t
del %~n1_EE.bin
del %~n1_E2.bin
:E_END

rem --------------
rem フランス語版
rem --------------
if not exist "%~n1_FF.bin" goto F_END
%BATCH_HOME%\my_crc_ccitt.exe %~n1_FF.bin %~n1_F2.bin
%BATCH_HOME%\my_crc_ccitt.exe %~n1_F2.bin %~n1_F.bin -t
del %~n1_FF.bin
del %~n1_F2.bin
:F_END

rem --------------
rem イタリア語版
rem --------------
if not exist "%~n1_II.bin" goto I_END
%BATCH_HOME%\my_crc_ccitt.exe %~n1_II.bin %~n1_I2.bin
%BATCH_HOME%\my_crc_ccitt.exe %~n1_I2.bin %~n1_I.bin -t
del %~n1_II.bin
del %~n1_I2.bin
:I_END

rem --------------
rem ドイツ語版
rem --------------
if not exist "%~n1_GG.bin" goto G_END
%BATCH_HOME%\my_crc_ccitt.exe %~n1_GG.bin %~n1_G2.bin
%BATCH_HOME%\my_crc_ccitt.exe %~n1_G2.bin %~n1_G.bin -t
del %~n1_GG.bin
del %~n1_G2.bin
:G_END

rem --------------
rem スペイン語版
rem --------------
if not exist "%~n1_SS.bin" goto S_END
%BATCH_HOME%\my_crc_ccitt.exe %~n1_SS.bin %~n1_S2.bin
%BATCH_HOME%\my_crc_ccitt.exe %~n1_S2.bin %~n1_S.bin -t
del %~n1_SS.bin
del %~n1_S2.bin
:S_END

rem --------------
rem 韓国語版
rem --------------
if not exist "%~n1_KK.bin" goto K_END
%BATCH_HOME%\my_crc_ccitt.exe %~n1_KK.bin %~n1_K2.bin
%BATCH_HOME%\my_crc_ccitt.exe %~n1_K2.bin %~n1_K.bin -t
del %~n1_KK.bin
del %~n1_K2.bin
:K_END

rem -----------
rem 終了
rem -----------
goto END

rem -----------------------
rem 単独実行時のエラー対応
rem -----------------------
:NOT_EXIST
echo "ファイルをドラッグしてください"
goto END


:END
pause

