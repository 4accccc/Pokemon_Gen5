Path=c:\cygwin\bin;
call update_hash.bat
cd ../waza_check/script
ruby -Ks -I ./hash;./personal;./eggwaza_check eggwaza_check_main.rb
pause
