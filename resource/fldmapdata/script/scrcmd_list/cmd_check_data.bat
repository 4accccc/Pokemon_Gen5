Path=c:\cygwin\bin;C:\Program Files\Java\jre6\bin;
cp ../../../../prog/src/field/scrcmd_table.cdat ./scrcmd_table.cdat
cp ../scrcmd_list.xls ./scrcmd_list.xls 
ruby ../../../../tools/exceltool/xls2xml/tab_out.rb ./scrcmd_list.xls > ./scrcmd_list.txt
ruby scrcmd_list.rb
pause
