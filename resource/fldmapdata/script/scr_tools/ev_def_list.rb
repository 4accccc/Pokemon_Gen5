#=======================================================================
# ev_def_list.rb
# ファイル一覧をruby実行形式で出力
#=======================================================================
$KCODE = "SJIS"

#script_offset_id.h パス
#EVDEF_FPATH_OFFS_ID = "../../../prog/src/field/script_offset_id.h"
#EVDEF_FPATH_OFFS_ID = "../scrid_offset/scr_offset_id.h"
EVDEF_FPATH_OFFS_ID = ARGV[1]

fname_list = ARGV[0]

if( fname_list == nil )
  printf( "ERROR ev_def_list file none\n" )
  exit 1
end

file_list = File.open( fname_list, "r" )

while line = file_list.gets
  line = line.strip
  str = line.split()
  
  if( str[0] =~ /\Asp_/ ) #特殊スクリプトは対象から外す
    next
  end
  
  printf( "ruby ./scr_tools/ev_def.rb %s %s\n", str[0], EVDEF_FPATH_OFFS_ID )
end

file_list.close
