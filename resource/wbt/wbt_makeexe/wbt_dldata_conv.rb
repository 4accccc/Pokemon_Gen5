#!ruby -Ks
#---------------------------
# wbt_dldata_conv.rb
#---------------------------

# debug(0:off、1:on)
$debug_code = 0

# カレントの言語(通し番号、データ参照で使用)
# ($lang_tbl_hash参照のこと)
$current_lang_id = 0

# 互換用(国コード:あまり使わないはず)
$current_lang_num = 1

# debug(0:別,1:同じ)
$debug_filename = 0;

# カレントファイル名
$current_xls_name = ""

# 文字列チェック(0:off、1:on)
# うるさいかもしれないので切り替えにしておく
$debug_chkstr = 1

#
require "version.rb"
#
require "my_tab_out_direct_sheet.rb"

require "wbt_dldata_data.rb"	  # トーナメントリスト(WBTDL_DATA)のシート用
require "wbt_dldata_rule.rb"	  # レギュレーション(WBTDL_RULE)用
require "wbt_dldata_character.rb" # キャラクタ(WBTDL_CHARACTER)用
require "wbt_dldata_match.rb"     # 組み合わせ(WBTDL_MATCH)用
require "wbt_dldata_config.rb"    # config(WBTDL_CONFIG)用

require "wbt_dldata_error.rb"     # エラー
require "wbt_dldata_dictionary.rb"

# 出力ファイル名
$output_str = [ "_JJ",	# 日本語
                "_EE",   # 英語
                "_FF",   # フランス語
                "_II",   # イタリア語
                "_GG",   # ドイツ語
                "_SS",   # スペイン語
                "_KK",   # 韓国語
]

# 最終ファイル名(決めうち)
$output_last_str = [ "_J",	# 日本語
                "_E",   # 英語
                "_F",   # フランス語
                "_I",   # イタリア語
                "_G",   # ドイツ語
                "_S",   # スペイン語
                "_K",   # 韓国語
]

# 出力ファイル名(win)
$output_str_win = [ "Japanese", # 日本語
                "English", 	# 英語
                "French",	# フランス語
                "Italian",	# イタリア語
                "German",	# ドイツ語
                "Spanish",	# スペイン語
                "Korean",	# 韓国語
]

# 言語最大数
LANG_MAX_NUM = 7

# 使用方法メッセージ
$usage_str = [
  "\n使い方 (#{$version_str})\nruby wbt_dldata_conv.rb in.xls\n",    # ruby
  "\n使い方 (#{$version_str})\nwbt_dldata_conv.exe in.xls\n",        # exe
]

#-------------------------------------------------------
#-------------------------------------------------------
#------------
# 引数チェック
#------------
if ARGV[0] == nil
  puts "引数1に入力(xls)ファイル名を指定してください"
  puts $usage_str[$ruby_mode]
  exit(1)
end
#
# 存在チェック
unless FileTest.exists?(ARGV[0])
  puts "入力(xls)ファイル #{ARGV[0]} が見つかりません"
  exit(1)
end

#----------------
# エラーファイル
#----------------
$wbt_error = WBT_ERROR.new()
str = ARGV[0].sub(/\.xls/, "_err.txt" )
$wbt_error.open( str );

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
#
#-----------------------
# 辞書ファイルの読み込み
#-----------------------
DICT_FILENAME="Dictionary_EFIGSK.xls"
filename = getAbsolutePath(DICT_FILENAME)
if FileTest.exists?( filename ) then
  conv.open_file( filename )
  wbt_dict = WBT_DICTIONARY.new(conv)
  conv.close_file();
  WIN32OLE.codepage = WIN32OLE::CP_ACP  #get_unicodeのままなのでもどす
else
  printf("辞書ファイル\"%s\"が見つかりません。\n", DICT_FILENAME)
  printf("コンバートを中止します\n")
  exit (1)
end

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
    $current_lang_num = $lang_num_tbl[m]
    puts sprintf("---- %s ----",$output_str_win[$current_lang_id]);
    
    # エラー情報初期化
    $wbt_error.reset_sub_error();
    
    # 出力ファイル名セット
    if($debug_filename == 0)
      str = $output_str[$current_lang_id] + ".bin"
      mstr = $output_last_str[$current_lang_id] + ".bin"
    else
      str = ".bin"
      mstr = ".bin"
    end
    outfilename = ARGV[0].sub(/\.xls/, str )
    #
    # 最終ファイルがあれば削除する
    lastfilename = ARGV[0].sub(/\.xls/, mstr )
    if FileTest.exists?(lastfilename)
        File.delete( lastfilename );
    end
    #
    #
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
          puts sprintf("ファイル %s が見つかりません",@data[12*m+i]);
          exit(1)
        end
        conv.open_file( filename );
        
        # 情報の参照
        wbt_data.set_data( conv )
        wbt_rule.set_data( conv )
        wbt_character.set_data( conv, wbt_dict.get_local($current_lang_id) )
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
        puts sprintf("%02d : %s",(i+1),@data[12*m+i])
      }
#      p sprintf("end :%s",outfilename)
      puts "end"
      puts
      #
      # エラーがあった場合は出力しない
      if($wbt_error.chk_sub_error() == -1)
#         puts outfilename
        File.delete( outfilename );
      end
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

#----------------
# エラーファイル
#----------------
$wbt_error.close()

#---------------
# 終了
#---------------
#p sprintf("end")
puts "conv end"
puts

#-------------------------------------------------------
#-------------------------------------------------------

