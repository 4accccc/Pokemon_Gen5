Path=c:\cygwin\bin;
call update_hash.bat
cd ../waza_check/script
ruby -Ks -I ./hash;./hash_conv;./personal;./trainer waza_check_for_trainer_poke.rb
pause
