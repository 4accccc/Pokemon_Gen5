@echo off
if "%1" == "diff" goto diff
if "%1" == "install" goto install

echo %0 sp3.yを元にスクリプト用パーサを作成します
echo コマンド：
echo 　　%0 diff  現在のパーサと比較します
echo 　　%0 install  現在のパーサと置き換えます
goto end

:install
ruby c:\cygwin\bin\racc -o parser.rb sp3.y
goto end

:diff
ruby c:\cygwin\bin\racc -o sample.rb sp3.y
diff sample.rb parser.rb
:end
