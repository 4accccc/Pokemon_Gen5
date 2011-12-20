@echo off

rem --------------------------------------
rem ダブルクリックをする場合はここをセット
rem --------------------------------------
rem SET BATROOT=%~dp0
rem SET TOOLDIR=%BATROOT%/../../lib/TwlSystem/tools/bin/
rem PATH=%TOOLDIR%;%PATH%;

rem -----------------------------------------------
rem (tmpにコンバート)
rem (ファイル名は決めうち、必要なものをここに追加)
rem -----------------------------------------------

rem デオキシスアタック
g2dcvtr ../../../swan_doc/pokegra/houen/386/pbwb_386_attack.nmc -bg -cr/0_0_32_16 -o/tmp
g2dcvtr ../../../swan_doc/pokegra/houen/386/pfwb_386_attack.nmc -bg -cr/0_0_32_16 -o/tmp

rem ドラピオン
g2dcvtr ../../../swan_doc/pokegra/shinou/452/pbwb_452.nmc -bg -cr/0_0_32_16 -o/tmp
g2dcvtr ../../../swan_doc/pokegra/shinou/452/pfwb_452.nmc -bg -cr/0_0_32_16 -o/tmp

rem ペンドラー
g2dcvtr ../../../swan_doc/pokegra/issyu/550/pbwb_550.nmc -bg -cr/0_0_32_16 -o/tmp
g2dcvtr ../../../swan_doc/pokegra/issyu/550/pfwb_550.nmc -bg -cr/0_0_32_16 -o/tmp

rem エモンガ
g2dcvtr ../../../swan_doc/pokegra/issyu/560/pbwb_560.nmc -bg -cr/0_0_32_16 -o/tmp
g2dcvtr ../../../swan_doc/pokegra/issyu/560/pfwb_560.nmc -bg -cr/0_0_32_16 -o/tmp

rem レシラム
g2dcvtr ../../../swan_doc/pokegra/issyu/650/pbwb_650.nmc -bg -cr/0_0_32_16 -o/tmp
g2dcvtr ../../../swan_doc/pokegra/issyu/650/pfwb_650.nmc -bg -cr/0_0_32_16 -o/tmp

rem ブラックキュレム
g2dcvtr ../../../swan_doc/pokegra/issyu/653/pbwb_653_black.nmc -bg -cr/0_0_32_16 -o/tmp
g2dcvtr ../../../swan_doc/pokegra/issyu/653/pfwb_653_black.nmc -bg -cr/0_0_32_16 -o/tmp

rem ホワイトキュレム
g2dcvtr ../../../swan_doc/pokegra/issyu/653/pbwb_653_white.nmc -bg -cr/0_0_32_16 -o/tmp
g2dcvtr ../../../swan_doc/pokegra/issyu/653/pfwb_653_white.nmc -bg -cr/0_0_32_16 -o/tmp

dir tmp\*.nanr

rem --------------------
rem とりあえず消しておく
rem --------------------
del tmp\*.NANR
del tmp\*.NCER
del tmp\*.NMAR
del tmp\*.NMCR
del tmp\*.NCGR
del tmp\*.NCLR

rem ---------------------------------------
rem 終了
rem ダブルクリックをする場合はここをセット
rem ---------------------------------------
rem pause

