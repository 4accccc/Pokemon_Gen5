REM hudson用のmakeバッチ
REM 用途    : prompt.bat cmd hudson_make.bat でhudsonから呼び出し
REM 制作者  : hosaka_genya
REM 日付    : 2009/11/24

SET USERNAME=hudson
@echo USERNAME = %USERNAME%
make
exit %errorlevel%
