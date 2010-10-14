
:: =====================================================
:: prompt.bat‚©‚ç©‚ç‚ğ‹N“®‚·‚éˆ—
:: =====================================================
@echo off
if "%1" == "" goto call_cmd
goto EX

:call_cmd
../../prompt.bat cmd %0 ex
goto end

:: =====================================================
:: ÀÛ‚Ìˆ—
:: =====================================================
:EX
@echo on
call update_hash.bat
cd ../waza_check/script
ruby -Ks -I ./hash;./hash_conv;./personal;./trainer waza_check_for_trainer_poke.rb
@echo off

:end
Pause
