#=======================================================================
# ev_def.rb
# スクリプトファイル*.evからスクリプトIDを生成し、*_def.hに書き込む
# ev_def.rb ev_file offs_id_file
# ev_file 変換する.evファイルパス
# offs_id_file スクリプトID開始オフセットが定義されたファイルパス
#=======================================================================
$KCODE = "SJIS"

#=======================================================================
# 定数
#=======================================================================
RET_ERROR = (0xffffffff)

#引数offs_id_fileで参照する開始スクリプトIDシンボル
SCROFFS_START = "ID_START_SCR_OFFSET"

#引数offs_id_fileで参照する開始トレーナー2v2スクリプトIDシンボル
SCROFFS_START_TRAINER_2VS2 = "ID_TRAINER_2VS2_OFFSET"

#トレーナースクリプトファイル名
FNAME_TRAINER = "trainer.ev"

#=======================================================================
# 関数
#=======================================================================
#-----------------------------------------------------------------------
# 異常終了
#-----------------------------------------------------------------------
def error_end( file0, file1, file2, filepath_del )
  file0.close
  file1.close
  file2.close
  File.delete( filepath_del )
  printf( "ERROR ev_def create\n" )
end

#-----------------------------------------------------------------------
# ヘッダーファイル 指定の文字列が含まれたdefine定義の数値検索
# hfile 検索するヘッダーファイル
# search 検索する文字列
# none_str 検索から外す文字列 nil=無し
# error_ret searchが無い場合に返す値
#-----------------------------------------------------------------------
def hfile_search( hfile, search, none_str, error_ret )
  pos = hfile.pos
  hfile.pos = 0
  
  num = error_ret
  search = search.strip
  
  while line = hfile.gets
    line = line.strip
    
    if( line =~ /\A#define/ )
      len = line.length
      str = line.split()
      def_start = str[1].sub(/^ID_/,"").sub(/_OFFSET$/,"")
      
      if( none_str != nil && str[1].include?(none_str) )
        next #無効となる文字を含んでいる
      end
      
      if ( str[1] == search )
      #if( str[1].include?(search) ) #シンボル search含み
        str_num = str[2] #シンボル数値
        
        if( str_num =~ /\A0x/ ) #16進
          /([\A0x0-9a-fA-F]+)/ =~ str_num
          str_num = $1
          num = str_num.hex
        else					#10進
          /([\A0-9]+)/ =~ str_num
          str_num = $1
          num = str_num.to_i
        end
        
        break
      end
    end
  end
  
  hfile.pos = pos
  return num
end

#=======================================================================
# *_def.h生成
#=======================================================================
fname_ev = ARGV[0]

if( fname_ev == nil )
  printf( "ERROR SCRIPT ev_def file name none\n" )
  exit 1
end

if( fname_ev =~ /^sp_.*.ev/ )
  #sp_～.evは特殊スクリプトなのでSCRIDを生成する必要がない
  exit 0
end

fname_id = ARGV[1]

if( fname_id == nil )
  printf( "ERROR SCRIPT ev_def id file name none\n" )
  exit 1
end

file_ev = File.open( fname_ev, "r" )
file_id = File.open( fname_id, "r" )

fname_def_h = fname_ev.gsub( "\.ev", "_def\.h" )
file_def_h = File.open( fname_def_h, "w" )

fname_ev_big = File::basename( fname_ev )
fname_ev_big = fname_ev_big.gsub( "\.ev", "" )
fname_ev_big = fname_ev_big.upcase

search_id_offset = "ID_" + fname_ev_big + "_OFFSET"
#開始ID検索
start_id = hfile_search( file_id, search_id_offset, "OFFSET_END", RET_ERROR )

if( start_id == RET_ERROR )
  start_id = hfile_search( file_id, SCROFFS_START, nil, RET_ERROR )
  
  if( start_id == RET_ERROR )
    printf( "can't find start_id %s \n", fname_ev_big )
    error_end( file_ev, file_id, file_def_h, fname_def_h )
    exit 1
  end
end

#ファイル見出し書き込み
file_def_h.printf( "\/\/%s\n", fname_def_h )
file_def_h.printf( "\/\/このファイルはコンバータによって作成されました\n\n" )
file_def_h.printf( "#ifndef _%s_DEF_H_\n", fname_ev_big )
file_def_h.printf( "#define _%s_DEF_H_\n\n", fname_ev_big )
file_def_h.printf( "\/\/スクリプトデータID定義\n" )

#ID定義
count = 0
start_id_trainer = RET_ERROR

if( fname_ev.include?(FNAME_TRAINER) )
  start_id_trainer = hfile_search(
    file_id, SCROFFS_START_TRAINER_2VS2, nil, RET_ERROR )
  
  if( start_id_trainer == RET_ERROR )
    printf( "can't find start_id %s \n", SCROFFS_START_TRAINER_2VS2 )
    error_end( file_ev, file_id, file_def_h, fname_def_h )
    exit 1
  end
end

while line = file_ev.gets
  line = line.strip
  
  if( line =~ /\A_EVENT_DATA_END/ )
    break
  end
  
  if( line =~ /\A_EVENT_DATA/ )
    str = line.split()
    id_name = str[1]
    
    if( id_name == nil )
      printf( "ERROR ev_def EVENT_DATA label name none\n" )
      error_end( file_ev, file_id, file_def_h, fname_def_h )
      exit 1
    end
    
    id_name = id_name.gsub( /\Aev_/, "" ) #先頭のev_を削除
    id_name = id_name.upcase
    
    file_def_h.printf( "#define SCRID_%s (%d)\n", id_name, start_id+count )
    
    if( start_id_trainer != RET_ERROR )
      file_def_h.printf( "#define SCRID_%s_2 (%d)\n",
            id_name, start_id_trainer+count )
    end
    
    count = count + 1
  end
end

#ファイル終端書き込み
file_def_h.printf( "\n#endif \/\/_%s_DEF_H_", fname_ev_big )

#終了
file_def_h.close
file_id.close
file_ev.close

#なぜかunix行末コード（LF)になっている環境があるため、強制変換
system("unix2dos -D " + fname_def_h )

