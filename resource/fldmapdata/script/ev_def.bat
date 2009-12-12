rem ev_def.bat
rem スクリプトファイル *_def.h生成
ls -1 *.ev > ev_def_tmp
ruby scr_tools/ev_def_list.rb ev_def_tmp ./scrid_offset/scr_offset_id.h > ev_def_tmp.bat
call ev_def_tmp.bat
del ev_def_tmp ev_def_tmp.bat
