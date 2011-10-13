@echo off
rem -----------------
rem コンバート
rem (PWT.xlsを使用)
rem -----------------
wbt_dldata_conv.exe PWT.xls

rem --------------
rem 日本語版
rem --------------
if not exist PWT_JJ.bin goto J_END
my_crc_ccitt.exe PWT_JJ.bin PWT_J.bin
del PWT_JJ.bin
:J_END

rem --------------
rem 英語版
rem --------------
if not exist PWT_EE.bin goto E_END
my_crc_ccitt.exe PWT_EE.bin PWT_E.bin
del PWT_EE.bin
:E_END

rem --------------
rem フランス語版
rem --------------
if not exist PWT_FF.bin goto F_END
my_crc_ccitt.exe PWT_FF.bin PWT_F.bin
del PWT_FF.bin
:F_END

rem --------------
rem イタリア語版
rem --------------
if not exist PWT_II.bin goto I_END
my_crc_ccitt.exe PWT_II.bin PWT_I.bin
del PWT_II.bin
:I_END

rem --------------
rem ドイツ語版
rem --------------
if not exist PWT_GG.bin goto G_END
my_crc_ccitt.exe PWT_GG.bin PWT_G.bin
del PWT_GG.bin
:G_END

rem --------------
rem スペイン語版
rem --------------
if not exist PWT_SS.bin goto S_END
my_crc_ccitt.exe PWT_SS.bin PWT_S.bin
del PWT_SS.bin
:S_END

rem --------------
rem 韓国語版
rem --------------
if not exist PWT_KK.bin goto K_END
my_crc_ccitt.exe PWT_KK.bin PWT_K.bin
del PWT_KK.bin
:K_END

rem ----------
rem 終了
rem ----------
pause
