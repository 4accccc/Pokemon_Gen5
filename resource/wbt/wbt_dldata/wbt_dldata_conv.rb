#!ruby -Ks
#---------------------------
# wbt_dldata_conv.rb
#---------------------------

# debug(0:off、1:on)
$debug_code = 0

require "my_tab_out_direct_sheet.rb"

require "wbt_dldata_data.rb"	  # トーナメントリスト(WBTDL_DATA)のシート用
require "wbt_dldata_rule.rb"	  # レギュレーション(WBTDL_RULE)用
require "wbt_dldata_character.rb" # キャラクタ(WBTDL_CHARACTER)用
require "wbt_dldata_match.rb"     # 組み合わせ(WBTDL_MATCH)用
require "wbt_dldata_config.rb"    # config(WBTDL_CONFIG)用

#-------------------------------------------------------
#-------------------------------------------------------
#-------------------
# エクセルファイルアクセス
#-------------------
conv = MY_ExcelConv.new();
begin   # begin

#-------------------
# 設定ファイル用
#-------------------
filename = getAbsolutePath( ARGV[0] );
conv.open_file( filename );
#
wbt_config = WBT_CONFIG.new(conv)
@data = [];
@data = wbt_config.get_param( 0 );
#
conv.close_file();
#
# 設定ファイル出力(debug)
if($debug_code == 1)
  len = @data.size
  len.times{|i|
    p sprintf("config : %d,%s",i,@data[i])
  }
end

#-----------------------
# 出力ファイルのセット
#-----------------------
outfilename = ARGV[0].sub(/\.xls/,".bin")
File.open( outfilename,"w"){|file|	# debug
  file.binmode
  
  #---------------------------
  # 各ファイルのセット(max12)
  #---------------------------
  size = @data.size;
  size.times{|i|
    filename = getAbsolutePath( @data[i] );
    conv.open_file( filename );
    
    # 情報の参照
    wbt_data      = WBT_DATA.new( conv )
    wbt_rule      = WBT_RULE.new( conv )
    wbt_character = WBT_CHARACTER.new( conv )
    wbt_match     = WBT_MATCH.new( conv )
    #
    # 各情報出力
    file.write( wbt_data.pack )
    file.write( wbt_rule.pack )
    file.write( wbt_character.pack )
    file.write( wbt_match.pack )
    #
    conv.close_file();
    #
    p sprintf("list(%d): %s",i,@data[i])
  }
}

ensure
#  p sprintf("ensure !")
  #
  conv.close_file()
  conv.exit()
end

#---------------
# 終了
#---------------
p sprintf("%s -> %s",ARGV[0],outfilename)

#-------------------------------------------------------
#-------------------------------------------------------

