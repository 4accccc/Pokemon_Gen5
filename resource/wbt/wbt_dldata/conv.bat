@echo off
wbt_dldata_conv.exe test_all.xls
my_crc_ccitt.exe test_all.bin test_all_crc.bin
del test_all.bin
pause
