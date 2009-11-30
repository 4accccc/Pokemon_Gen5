#=======================================================================
# d06stone.rb
# 電気洞窟に配置する浮遊石一覧cdatファイル作成
#=======================================================================
$KCODE = "SJIS"

#=======================================================================
# 定数
#=======================================================================

#=======================================================================
# エラー終了
#=======================================================================
def error_end( f0, f1, f2, del )
  f0.close
  f1.close
  f2.close
  File.delete( del )
end

#=======================================================================
# 変換
#=======================================================================
fpath_cdat = ARGV[0]
fpath_csv0 = ARGV[1]
fpath_csv1 = ARGV[2]
file_cdat = File.open( fpath_cdat, "w" )
file_csv0 = File.open( fpath_csv0, "r" )
file_csv1 = File.open( fpath_csv1, "r" )

file_cdat.printf( "//%s\n", fpath_cdat )
file_cdat.printf( "//D06浮遊石配置\n" )
file_cdat.printf( "//このファイルはコンバータから出力されました\n\n" )

buf_index = Array.new()
buf = Array.new()

buf_index << "#define BIGSTONE_MAX (99)\n"
buf_index << "#define BIGSTONE_ZONECOUNT (99)\n"
buf_index << "//大きな浮遊石インデックス {{zone id,count,data pointer}}\n"
buf_index << "static const u32 data_d06BigStoneZoneIndex[BIGSTONE_ZONECOUNT][3] = {\n"

#------------------------------------------------------------
# 浮遊石大
#------------------------------------------------------------
zone_id = nil
zone_count = 0
stone_max = 0

while line = file_csv0.gets
  #ゾーンID検索
  line = line.strip
  str = line.split( "," )
  
  if( str[0] != nil && str[0].include?("●") )
    zone_id = str[0].gsub( "●", "" )
    zone_id = zone_id.strip
    
    if( zone_id == nil )
      printf( "ERROR:D06 BIG STONE ZONE ID ERROR\n" )
      error_end( file_cdat, file_csv0, file_csv1, fpath_cdat )
      exit 1
    end
    
    #配置データ作成
    stone_count = 0
    line = file_csv0.gets #データ開始位置まで進める
    data_name = sprintf( "data_d06BigStonePos%d", zone_count )
    
    buf_define_pos = buf.size
    buf << sprintf( "#define BIGSTONE_%d_MAX (99)\n", zone_count )
    buf << sprintf("static const int %s[BIGSTONE_%d_MAX][3] = {\n",
                   data_name, zone_count )
    
    while line = file_csv0.gets
      line = line.strip
      str = line.split( "," )
      
      if( str[0] == nil || str[0] == "" )
        break
      end
      
      if( str[1] != nil && str[1] != "" &&
          str[2] != nil && str[2] != "" &&
          str[3] != nil && str[3] != "" )
          buf << sprintf( "\t{%s,%s,%s},\n", str[1], str[2], str[3] )
          stone_count = stone_count + 1
      end
    end
    
    if( stone_count == 0 )
      printf( "ERROR:D06 BIG STONE NOTHING\n" )
      error_end( file_cdat, file_csv0, file_csv1, fpath_cdat )
      exit 1
    end
    
    buf << "};\n\n"
    buf[buf_define_pos] = sprintf(
      "#define BIGSTONE_%d_MAX (%d)\n", zone_count, stone_count )
    
    if( stone_count > stone_max )
      stone_max = stone_count
    end

    #ゾーンインデックス追加
    buf_index << sprintf(
      "\t{ZONE_ID_%s,%d,(u32)%s},\n", zone_id, stone_count, data_name )
    zone_count = zone_count + 1
  end
end

buf_index[0] = sprintf( "#define BIGSTONE_MAX (%d)\n", stone_max )
buf_index[1] = sprintf( "#define BIGSTONE_ZONECOUNT (%d)\n", zone_count )
buf_index << "};\n\n"

file_cdat.printf( "%s", buf )
file_cdat.printf( "%s", buf_index )

#------------------------------------------------------------
# 浮遊石小
#------------------------------------------------------------
max = 0
zone_count = 0
stone_max = 0
buf_index = Array.new()
buf = Array.new()

buf_index << "#define MINISTONE_MAX (99)\n"
buf_index << "#define MINISTONE_ZONECOUNT (99)\n"
buf_index << "//小さな浮遊石インデックス {{zone id,count,data pointer}}\n"
buf_index << "static const u32 data_d06MiniStoneZoneIndex[BIGSTONE_ZONECOUNT][3] = {\n"

zone_id = nil

while line = file_csv1.gets
  #ゾーンID検索
  line = line.strip
  str = line.split( "," )
  
  if( str[0] != nil && str[0].include?("●") )
    zone_id = str[0].gsub( "●", "" )
    zone_id = zone_id.strip
    
    if( zone_id == nil )
      printf( "ERROR:D06 MINI STONE ZONE ID ERROR\n" )
      error_end( file_cdat, file_csv0, file_csv1, fpath_cdat )
      exit 1
    end
    
    #配置データ作成
    stone_count = 0
    line = file_csv1.gets #データ開始位置まで進める
    data_name = sprintf( "data_d06MiniStonePos%d", zone_count )
    
    buf_define_pos = buf.size
    buf << sprintf( "#define MINISTONE_%d_MAX (99)\n", zone_count )
    buf << sprintf("static const int %s[MINISTONE_%d_MAX][3] = {\n",
                   data_name, zone_count )
    
    while line = file_csv1.gets
      line = line.strip
      str = line.split( "," )
      
      if( str[0] == nil || str[0] == "" )
        break
      end
      
      if( str[1] != nil && str[1] != "" &&
          str[2] != nil && str[2] != "" &&
          str[3] != nil && str[3] != "" )
          buf << sprintf( "\t{%s,%s,%s},\n", str[1], str[2], str[3] )
          stone_count = stone_count + 1
      end
    end
    
    if( stone_count == 0 )
      printf( "ERROR:D06 MINI STONE NOTHING\n" )
      error_end( file_cdat, file_csv0, file_csv1, fpath_cdat )
      exit 1
    end
    
    buf << "};\n\n"
    buf[buf_define_pos] = sprintf(
      "#define MINISTONE_%d_MAX (%d)\n", zone_count, stone_count )

    if( stone_count > stone_max )
      stone_max = stone_count
    end
    
    #ゾーンインデックス追加
    buf_index << sprintf(
      "\t{ZONE_ID_%s,%d,(u32)%s},\n", zone_id, stone_count, data_name )
    zone_count = zone_count + 1
  end
end

buf_index[0] = sprintf( "#define MINISTONE_MAX (%d)\n", stone_max )
buf_index[1] = sprintf( "#define MINISTONE_ZONECOUNT (%d)\n", zone_count )
buf_index << "};\n\n"

file_cdat.printf( "%s", buf )
file_cdat.printf( "%s", buf_index )

file_cdat.close
file_csv0.close
file_csv1.close
