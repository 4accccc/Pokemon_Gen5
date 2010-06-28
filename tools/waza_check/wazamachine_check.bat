Path=c:\cygwin\bin;
call update_hash.bat
cd ../waza_check/script
ruby -Ks -I ./hash wazamachine_check.rb
pause
