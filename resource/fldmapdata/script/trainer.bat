echo create trainer.ev
@ruby scr_trainer/trainer.rb ../../../prog/include/tr_tool/trno_def.h scr_trainer/trainer_base.ev
@ruby scr_tools/ev_def.rb trainer.ev scrid_offset/scr_offset_id.h
@REM 行末コードをUNIXからWINDOWSに変更
@unix2dos -D trainer.ev
