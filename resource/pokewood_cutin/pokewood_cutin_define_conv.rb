#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		pokewood_cutin_define_conv.rb
#	@brief  ポケウッドカットイン定義コンバータ
#	@author	Toru=Nagihashi
#	@data		2011.05.24
#
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "../resonance_resort/data/conv_tool.rb" #モジュール使い回しのためリゾートのを使っています。
#-------------------------------------
# 引数チェック
# @retval true  問題なし
# @retval false エラー
#=====================================
def checkArgument
  if ARGV[0] == nil
    puts "引数1にcsvのファイル名を指定してください"
    return false
  end
  if ARGV[1] == nil
    puts "引数２に出力ファイル名を指定してください"
    return false
  end
  unless FileTest.exists?(ARGV[0])
    puts "#{ARGV[0]}が見つかりません"
    return false
  end
  return true
end

#=============================================================================
#
#   モジュール
#
#=============================================================================
#-------------------------------------
#コマンドデータコンバート
#=====================================
CMD_PARAM_MAX       = 8

CMD_ROW_START       = 1         #行開始位置
CMD_COL_START       = 1         #列開始位置
CMD_COL_PG_NAME     = CMD_COL_START     #列:programネーム
CMD_COL_JP_NAME     = 2         #列:日本語ネーム
CMD_COL_PARAM_START = 3         #列：引数開始
CMD_COL_PARAM_END   =  CMD_COL_PARAM_START + CMD_PARAM_MAX #列：引数終了

def convertCmd( cmdCsv, header, rb )


  #cmdCsvのコンバート
  header.write( "//スクリプトコマンド情報\n" )

  rb.write( "$cmd_data = [\n" )

  cnt = 0
  for row in CMD_ROW_START...cmdCsv.row_length-1

    pg_name  = cmdCsv.get( row, CMD_COL_PG_NAME )
    jp_name  = cmdCsv.get( row, CMD_COL_JP_NAME )

    header.write( "//#{jp_name}\n" )
    header.write( "#define PW_CUTIN_CMD_#{pg_name}  (#{cnt.to_i})\n" )

    rb.write( "\t{\n" )
    rb.write( "\t\t:JP_NAME=>'#{jp_name}',\n" )
    rb.write( "\t\t:PG_VALUE=>#{cnt.to_i},\n" )

    param_cnt = 0
    for idx in CMD_COL_PARAM_START...CMD_COL_PARAM_END
      param  = cmdCsv.get( row, idx )
      if param != nil
        rb.write( "\t\t#{idx-CMD_COL_PARAM_START}=>'#{param}',\n" )
        param_cnt+=1
      else
        rb.write( "\t\t#{idx-CMD_COL_PARAM_START}=>'',\n" )
      end
    end
    rb.write( "\t\t:PARAM_CNT=>#{param_cnt},\n" )
    header.write( "#define PW_CUTIN_#{pg_name}_PARAM_NUM (#{param_cnt})\n" )

    rb.write( "\t},\n" )
    cnt+=1
  end

  rb.write( "]\n" )

end


#-------------------------------------
#定義データコンバート
#=====================================
DEF_ROW_START       = 1         #行開始位置
DEF_COL_START       = 1         #列開始位置
DEF_COL_PG_NAME     = DEF_COL_START     #列：programネーム
DEF_COL_JP_NAME     = 2         #列：日本語ネーム
DEF_COL_VALUE       = 3         #列：値
DEF_COL_KIND        = 4         #列：種類

def convertDef( defCsv, header, rb )

  #defCsvのコンバート
  header.write( "//定義情報\n" )

  rb.write( "$def_data = [\n" )

  for row in DEF_ROW_START...defCsv.row_length-1

    pg_name  = defCsv.get( row, DEF_COL_PG_NAME )
    jp_name  = defCsv.get( row, DEF_COL_JP_NAME )
    value    = defCsv.get( row, DEF_COL_VALUE )
    kind     = defCsv.get( row, DEF_COL_KIND )

    header.write( "//#{jp_name}\n" )
    header.write( "#define PW_CUTIN_DEF_#{pg_name}  (#{value.to_i})\n" )

    rb.write( "\t{\n" )
    rb.write( "\t\t:JP_NAME=>'#{jp_name}',\n" )
    rb.write( "\t\t:PG_VALUE=>#{value.to_i},\n" )
    rb.write( "\t\t:KIND=>'#{kind}',\n" )
    rb.write( "\t},\n" )
  end

  rb.write( "]\n" )
end

#=============================================================================
#
#   メイン処理
#
#=============================================================================
begin

  #引数チェック
  unless checkArgument
    exit(1)
  end

  #csv読込み
  cmdCsv = CsvLoader.new
  cmdCsv.setPrefix( '#cmd_data_start' )
  cmdCsv.setSuffix( '#cmd_data_end' )
  cmdCsv.load( ARGV[0] )

  defCsv = CsvLoader.new
  defCsv.setPrefix( '#define_data_start' )
  defCsv.setSuffix( '#define_data_end' )
  defCsv.load( ARGV[0] )

  #ヘッダ作成
  header    = TextBuilder.new
  header.write_header( "#{ARGV[1]}.h", 
                    "ポケウッドカットインの定義情報です",
                    "Toru=Nagihashi" )

  #ルビーファイル作成 
  rb        = TextBuilder.new

  #コンバート
  convertCmd( cmdCsv, header, rb )
  convertDef( defCsv, header, rb )

  #書き込み
  header.genelate( "#{ARGV[1]}.h" )
  rb.genelate( "#{ARGV[1]}.rb" )

end
