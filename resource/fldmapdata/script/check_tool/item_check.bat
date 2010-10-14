
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
cp ../../../../prog/include/item/itemsym.h ./item/copy/itemsym.h
cp ../../../message/src/place_name.gmm ./item/copy/place_name.gmm
cp ../../zonetable/zonetable.xls ./item/copy/zonetable.xls
cd ./item/script
ruby ./excel_tab_out.rb ../copy/zonetable.xls 1 ../copy/zonetable.txt
ruby ./item_listup.rb
@echo off

:end
Pause
@echo on
