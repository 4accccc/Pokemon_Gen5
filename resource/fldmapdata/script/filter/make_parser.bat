@echo off
if "%1" == "diff" goto diff
if "%1" == "install" goto install

echo %0 sp3.yを元にスクリプト用パーサを作成します
echo コマンド：
echo 　　%0 diff  現在のパーサと比較します
echo 　　%0 install  現在のパーサと置き換えます
goto end

:install
REM ruby c:\cygwin\bin\racc -o parser.rb sp3.y
"C:\Program Files\ruby-1.8\bin\racc.bat" -o parser.rb sp4.y
goto end

:diff
REM ruby c:\cygwin\bin\racc -o sample.rb sp3.y
"C:\Program Files\ruby-1.8\bin\racc.bat" -o sample.rb sp4.y
diff sample.rb parser.rb
:end
