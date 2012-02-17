
:: =====================================================
:: prompt.bat‚©‚ç©‚ç‚ğ‹N“®‚·‚éˆ—
:: =====================================================
@echo off
if "%1" == "" goto call_cmd
goto EX

:call_cmd
../../../../prompt.bat cmd %0 ex
goto end

:: =====================================================
:: ÀÛ‚Ìˆ—
:: =====================================================
:EX
@echo on
ruby attr_bit_check.rb attr_use_list.csv attr_use_list_num.csv null_attribute_list.csv
@echo off

:end
Pause

