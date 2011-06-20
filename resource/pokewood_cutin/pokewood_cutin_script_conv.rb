#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		pokewood_cutin_script_conv.rb
#	@brief  ポケウッドカットインスクリプトデータコンバータ
#	@author	Toru=Nagihashi
#	@data		2011.05.24
#
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "../resonance_resort/data/conv_tool.rb" #モジュール使い回しのためリゾートのを使っています。
require "../../tools/headerdata.rb"
require "pokewood_cutin_def.rb"


RESOUSE_PREFIX = "NARC_pokewood_cutin_gra_"
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
    puts "引数2にnaixファイル名を指定してください"
    return false
  end
  if ARGV[2] == nil
    puts "引数3にsadlファイル名を指定してください"
    return false
  end
  if ARGV[3] == nil
    puts "引数３に出力ファイル名を指定してください"
    return false
  end
  unless FileTest.exists?(ARGV[0])
    puts "#{ARGV[0]}が見つかりません"
    return false
  end
  unless FileTest.exists?(ARGV[1])
    puts "#{ARGV[1]}が見つかりません"
    return false
  end
  unless FileTest.exists?(ARGV[2])
    puts "#{ARGV[2]}が見つかりません"
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
# スクリプトコマンドデータ取得
#=====================================
def getCmdData( jp_name )
  $cmd_data.each{ |data|
    if data[:JP_NAME] == jp_name 
      return data
    end
  }

  p "そんなコマンドはありません#{jp_name}"
  exit(1)
  return nil
end

#-------------------------------------
# 定義データ取得
#=====================================
def getDefData( jp_name )
  $def_data.each{ |data|
    if data[:JP_NAME] == jp_name 
      return data
    end
  }

  ##存在しないことを許容している
  return nil
end

#-------------------------------------
# コマンドの引数チェック
#=====================================
def checkCmdParam( cmd, param, num )
    cmd_d = getCmdData( cmd )
    def_d = getDefData( param )

    if def_d == nil 
      return true
    else

      if num >= cmd_d[:PARAM_CNT] 
        puts "「#{cmd}」の引数がオーバーしています。最大「#{cmd_d[:PARAM_CNT]}」オーバー「#{num}」"
        return false
      end

      if cmd_d[num] == def_d[:KIND]
        return true
      else
        puts "「#{cmd}」の引数「#{param}」が間違っています。"
        return false
      end
    end
end

#-------------------------------------
# ヘッダーデータコンバート
#=====================================
H_ROW_START       = 0         #行開始位置
H_ROW_SENARIO     = 1 
H_ROW_EVENT       = 2 
H_ROW_PLAYTERM    = 3
H_ROW_WND_NUM     = 4
H_ROW_NCL         = 5
H_ROW_NCG         = 6
H_ROW_NCE         = 7
H_COL_START       = 0         #列開始位置
H_COL_VALUE       = 1         #列:番号1
H_COL_VALUE2      = 2        #列:番号2
def  convertHeader( csv, data, naix )

  #データ取得
  senario = csv.get( H_ROW_SENARIO, H_COL_VALUE )
  event   = csv.get( H_ROW_EVENT,   H_COL_VALUE )
  wnd_num = csv.get( H_ROW_WND_NUM, H_COL_VALUE )

  #データ書き込み    
  data.write_u16( senario )
  
  if event == 0 
    puts "イベント番号は１～です。"
    exit(1)
  end
  event = event.to_i - 1
  data.write_u16( event )
  data.write_u16( wnd_num )

  for col in H_COL_VALUE...H_COL_VALUE2+1
    ncl     = csv.get( H_ROW_NCL,     col )
    ncg     = csv.get( H_ROW_NCG,     col )
    nce     = csv.get( H_ROW_NCE,     col )
    nan     = csv.get( H_ROW_NCE,     col )

    begin
      ncl = RESOUSE_PREFIX+ncl.sub(/\.ncl/,"_NCLR")
      data.write_u16( naix.search( ncl, "#{H_ROW_NCL}行" ) )
    rescue HeaderSearchError
      exit(1)
    end
    begin
      ncg = RESOUSE_PREFIX+ncg.sub(/\.ncg/,"_NCGR")
      data.write_u16( naix.search( ncg, "#{H_ROW_NCG}行" ) )
    rescue HeaderSearchError
      exit(1)
    end
    begin
      nce = RESOUSE_PREFIX+nce.sub(/\.nce/,"_NCER")
      data.write_u16( naix.search( nce, "#{H_ROW_NCE}行" ) )
      nan = RESOUSE_PREFIX+nan.sub(/\.nce/,"_NANR")
      data.write_u16( naix.search( nan, "#{H_ROW_NCE}行" ) )
    rescue HeaderSearchError
      exit(1)
    end
  end

end

#-------------------------------------
# スクリプトコンバート
#=====================================
CMD_PARAM_MAX       = 8

SCR_ROW_START       = 1         #行開始位置
SCR_COL_START       = 0         #列開始位置
SCR_COL_CMD         = 1         #列:コマンド
SCR_COL_PARAM_START = 2         #列:引数開始
SCR_COL_PARAM_END   = SCR_COL_PARAM_START+CMD_PARAM_MAX #列:引数開始

def convertScript( csv, hCsv, data, naix, sadl )


  wnd_num = hCsv.get( H_ROW_WND_NUM, H_COL_VALUE )

  for row in SCR_ROW_START...csv.row_length-1

    #データ取得
    cmd = csv.get( row, SCR_COL_CMD )
    cmd_d = getCmdData( cmd )

    data.write_u16( cmd_d[:PG_VALUE] )

    next_param = 0
    for col in SCR_COL_PARAM_START...SCR_COL_PARAM_END
      param = csv.get( row, col )

      if param != nil
        def_d = getDefData( param )

        #引数の型チェック
        if checkCmdParam( cmd, param, col-SCR_COL_PARAM_START ) == false
          exit(1)
        end

        param_name = cmd_d[col-SCR_COL_PARAM_START]

        if def_d == nil

          case param_name
          when "読み替えるパレット"
            begin
              param = RESOUSE_PREFIX+param.sub(/\.ncl/,"_NCLR")
              data.write_u16( naix.search( param, cmd ) )
            rescue HeaderSearchError
              exit(1)
            end
          when "読み替えるキャラ"
            begin
              param = RESOUSE_PREFIX+param.sub(/\.ncg/,"_NCGR")
              data.write_u16( naix.search( param, cmd ) )
            rescue HeaderSearchError
              exit(1)
            end
          when "読み替えるセル"
            begin
              param = RESOUSE_PREFIX+param.sub(/\.nce/,"_NCER")
              data.write_u16( naix.search( param, cmd ) )
              param = param.sub(/_NCER/,"_NANR")
              next_param = naix.search( param, cmd )
            rescue HeaderSearchError
              exit(1)
            end
          when "SE名"
            begin
              data.write_u16( sadl.search( param.to_s, cmd ) )
            rescue HeaderSearchError
              exit(1)
            end
          when "コマ番号"
            if param >= wnd_num
              puts "コマ数が最大値を超えています\n"
              exit(1)
            end
            data.write_u16( param )
          else
            data.write_u16( param )
          end
        else
          data.write_u16( def_d[:PG_VALUE] )
        end
      else
        data.write_u16(next_param)
        next_param = 0
      end

    end

  end

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
  hCsv = CsvLoader.new
  hCsv.setPrefix( '#header_start' )
  hCsv.setSuffix( '#header_end' )
  hCsv.load( ARGV[0] )

  scrCsv = CsvLoader.new
  scrCsv.setPrefix( '#script_start' )
  scrCsv.setSuffix( '#script_end' )
  scrCsv.load( ARGV[0] )

  #data作成
  data    = BinaryBuilder.new

  #ヘッダデータの作成
  naixdata = HeaderDataArray.new
  naixdata.load( ARGV[1] )

  #sadlデータの作成
  sadldata = HeaderDataArray.new
  sadldata.load( ARGV[2] )

  #コンバート
  convertHeader( hCsv, data, naixdata )
  convertScript( scrCsv, hCsv, data, naixdata, sadldata )

  #date書き込み
  data.genelate( "#{ARGV[3]}.dat" )

end
