@echo off

rem ------------------------
rem 単独実行時エラー
rem ------------------------
if "%1" == "" goto NOT_EXIST


rem ------------------------
rem 正しくドラッグされた場合
rem ------------------------
wbt_dldata_conv.exe %1


rem --------------
rem 日本語版(test)
rem --------------
if not exist "%~n1.bin" goto TEST_END
my_crc_ccitt.exe %~n1.bin %~n1_crc.bin
del %~n1.bin
:TEST_END

rem --------------
rem 日本語版
rem --------------
if not exist "%~n1_J.bin" goto J_END
my_crc_ccitt.exe %~n1_J.bin %~n1_J_crc.bin
copy %~n1_J_crc.bin WBT_J.bin
del %~n1_J.bin
del %~n1_J_crc.bin
:J_END

rem --------------
rem 英語版
rem --------------
if not exist "%~n1_E.bin" goto E_END
my_crc_ccitt.exe %~n1_E.bin %~n1_E_crc.bin
copy %~n1_E_crc.bin WBT_E.bin
del %~n1_E.bin
del %~n1_E_crc.bin
:E_END

rem --------------
rem フランス語版
rem --------------
if not exist "%~n1_F.bin" goto F_END
my_crc_ccitt.exe %~n1_F.bin %~n1_F_crc.bin
copy %~n1_F_crc.bin WBT_F.bin
del %~n1_F.bin
del %~n1_F_crc.bin
:F_END

rem --------------
rem イタリア語版
rem --------------
if not exist "%~n1_I.bin" goto I_END
my_crc_ccitt.exe %~n1_I.bin %~n1_I_crc.bin
copy %~n1_I_crc.bin WBT_I.bin
del %~n1_I.bin
del %~n1_I_crc.bin
:I_END

rem --------------
rem ドイツ語版
rem --------------
if not exist "%~n1_G.bin" goto G_END
my_crc_ccitt.exe %~n1_G.bin %~n1_G_crc.bin
copy %~n1_G_crc.bin WBT_G.bin
del %~n1_G.bin
del %~n1_G_crc.bin
:G_END

rem --------------
rem スペイン語版
rem --------------
if not exist "%~n1_S.bin" goto S_END
my_crc_ccitt.exe %~n1_S.bin %~n1_S_crc.bin
copy %~n1_S_crc.bin WBT_S.bin
del %~n1_S.bin
del %~n1_S_crc.bin
:S_END

rem --------------
rem 韓国語版
rem --------------
if not exist "%~n1_K.bin" goto K_END
my_crc_ccitt.exe %~n1_K.bin %~n1_K_crc.bin
copy %~n1_K_crc.bin WBT_K.bin
del %~n1_K.bin
del %~n1_K_crc.bin
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

