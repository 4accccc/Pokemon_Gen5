#!ruby -Ks
#---------------------------
# wbt_dldata_conv.rb
#---------------------------

# debug(0:off、1:on)
$debug_code = 0

# カレントの言語
# ($lang_tbl_hash参照のこと)
$current_lang_id = 0

# debug(0:別,1:同じ)
$debug_filename = 1;

# カレントファイル名
$current_xls_name = ""

# 文字列チェック(0:off、1:on)
# うるさいかもしれないので切り替えにしておく
$debug_chkstr = 1

# ruby or exe(0:ruby、1:exe)
$current_mode = 0
#
require "my_tab_out_direct_sheet.rb"

require "wbt_dldata_data.rb"	  # トーナメントリスト(WBTDL_DATA)のシート用
require "wbt_dldata_rule.rb"	  # レギュレーション(WBTDL_RULE)用
require "wbt_dldata_character.rb" # キャラクタ(WBTDL_CHARACTER)用
require "wbt_dldata_match.rb"     # 組み合わせ(WBTDL_MATCH)用
require "wbt_dldata_config.rb"    # config(WBTDL_CONFIG)用

# 出力ファイル名
$output_str = [ "_J",	# 日本語
                "_E",   # 英語
                "_F",   # フランス語
                "_I",   # イタリア語
                "_G",   # ドイツ語
                "_S",   # スペイン語
                "_K",   # 韓国語
]

# 言語最大数
LANG_MAX_NUM = 7

# 使用方法メッセージ
$usage_str = [
  "\n使い方\nruby wbt_dldata_conv.rb in.xls",    # ruby
  "\n使い方\nwbt_dldata_conv.exe in.xls",        # exe
]

#-------------------------------------------------------
#-------------------------------------------------------
#------------
# 引数チェック
#------------
if ARGV[0] == nil
  puts "引数1に入力(xls)ファイル名を指定してください"
  puts $usage_str[$current_mode]
  exit(1)
end
#
# 存在チェック
unless FileTest.exists?(ARGV[0])
  puts "入力(xls)ファイル #{ARGV[0]} が見つかりません"
  exit(1)
end

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
# ファイル名を取得
@data = [];
@data = wbt_config.get_param();
# 出力オプションを取得
@output = [];
@output = wbt_config.get_output();
#
conv.close_file();
#
#-----------------------
# クラスの初期化
#-----------------------
wbt_data      = WBT_DATA.new()
wbt_rule      = WBT_RULE.new()
wbt_character = WBT_CHARACTER.new()
wbt_match     = WBT_MATCH.new()

#-----------------------
# 出力ファイルのセット
#-----------------------
# 出力
LANG_MAX_NUM.times{|m|
  #
  # 出力指定があるときだけセット
  if(@output[m] == 1)
    $current_lang_id = m;
    # 出力ファイル名セット
    if($debug_filename == 0)
      str = $output_str[$current_lang_id] + ".bin"
    else
      str = ".bin"
    end
    outfilename = ARGV[0].sub(/\.xls/, str )
    File.open( outfilename,"w"){|file|
      file.binmode
      
      #---------------------------
      # 各ファイルのセット(max12)
      #---------------------------
      # ファイルを取得
      num = get_array_num( @data, 12*m, 12 )
      size = num
      size.times{|i|
        $current_xls_name = @data[12*m+i]
        filename = getAbsolutePath( @data[12*m+i] );
        # ファイルの有無チェック
        unless FileTest.exists?( filename )
          p sprintf("ファイル %s が見つかりません",@data[12*m+i]);
          exit(1)
        end
        conv.open_file( filename );
        
        # 情報の参照
        wbt_data.set_data( conv )
        wbt_rule.set_data( conv )
        wbt_character.set_data( conv )
        wbt_match.set_data( conv )
        #
        # 各情報出力
        file.write( wbt_data.pack )
        file.write( wbt_rule.pack )
        file.write( wbt_character.pack )
        file.write( wbt_match.pack )
        #
        conv.close_file();
        #
        p sprintf("list(%02d): %s",i,@data[12*m+i])
      }
      p sprintf("end :%s",outfilename)
    }
  end
}
ensure
  if($debug_code == 1)
    p sprintf("ensure !")
  end
  #
  conv.close_file()
  conv.exit()
end

#---------------
# 終了
#---------------
p sprintf("end")

#-------------------------------------------------------
#-------------------------------------------------------

