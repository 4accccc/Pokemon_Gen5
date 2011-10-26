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
rem my_crc_ccitt.exe PWT_JJ.bin PWT_J.bin
my_crc_ccitt.exe PWT_JJ.bin PWT_J2.bin
my_crc_ccitt.exe PWT_J2.bin PWT_J.bin -t
del PWT_JJ.bin
del PWT_J2.bin
:J_END

rem --------------
rem 英語版
rem --------------
if not exist PWT_EE.bin goto E_END
my_crc_ccitt.exe PWT_EE.bin PWT_E2.bin
my_crc_ccitt.exe PWT_E2.bin PWT_E.bin -t
del PWT_EE.bin
del PWT_E2.bin
:E_END

rem --------------
rem フランス語版
rem --------------
if not exist PWT_FF.bin goto F_END
my_crc_ccitt.exe PWT_FF.bin PWT_F2.bin
my_crc_ccitt.exe PWT_F2.bin PWT_F.bin -t
del PWT_FF.bin
del PWT_F2.bin
:F_END

rem --------------
rem イタリア語版
rem --------------
if not exist PWT_II.bin goto I_END
my_crc_ccitt.exe PWT_II.bin PWT_I2.bin
my_crc_ccitt.exe PWT_I2.bin PWT_I.bin -t
del PWT_II.bin
del PWT_I2.bin
:I_END

rem --------------
rem ドイツ語版
rem --------------
if not exist PWT_GG.bin goto G_END
my_crc_ccitt.exe PWT_GG.bin PWT_G2.bin
my_crc_ccitt.exe PWT_G2.bin PWT_G.bin -t
del PWT_GG.bin
del PWT_G2.bin
:G_END

rem --------------
rem スペイン語版
rem --------------
if not exist PWT_SS.bin goto S_END
my_crc_ccitt.exe PWT_SS.bin PWT_S2.bin
my_crc_ccitt.exe PWT_S2.bin PWT_S.bin -t
del PWT_SS.bin
del PWT_S2.bin
:S_END

rem --------------
rem 韓国語版
rem --------------
if not exist PWT_KK.bin goto K_END
my_crc_ccitt.exe PWT_KK.bin PWT_K2.bin
my_crc_ccitt.exe PWT_K2.bin PWT_K.bin -t
del PWT_KK.bin
del PWT_K2.bin
:K_END

rem ----------
rem 終了
rem ----------
pause
