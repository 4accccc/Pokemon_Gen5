#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#   @file		  tr_mev_param_check.rb
#   @brief    mevを解析して、トレーナー設置情報が正しいかチェック
#   @author		Masayuki Onoue
#   @date		  2011.01.17
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$KCODE = "SJIS"
require "mev_analyzer.rb"
require "mev_parser.rb"
require 'csv'

# チェックするmevのディレクトリ
PATH_TO_DIR     = "../data/"
TARGET_FILENAME = "*.mev"

RND_MOVE_CODE = [
  "MV_RND",
  "MV_RND_V",
  "MV_RND_H",
  "MV_RND_UL",
  "MV_RND_UR",
  "MV_RND_DL",
  "MV_RND_DR",
  "MV_RND_UDL",
  "MV_RND_UDR",
  "MV_RND_ULR",
  "MV_RND_DLR",
]

# 結果出力先
result_file = ARGV[0]

# Errorデータ型定義
$assert_data = Struct.new("AssertData", :filename, :id ,:type ,:text )

# エラーデータを設定する
def set_error_data( dst ,filename ,event ,text )
  if event.Event_Script_Name =~ /^SCRID_TRID_(.*)_/ then
    dst << $assert_data.new( filename ,event.ID_name ,$1 ,text )
  else
    raise
  end
end


# Errorデータ配列
error_array = Array.new


filenames = Dir[ PATH_TO_DIR + TARGET_FILENAME ]
filenames.each{|filename|
  puts filename
  mev_data = MEVFile.new
  mev_data.load( filename )
 
  # =====================================
  # エラーデータ収集
  mev_data.obj_section.event.each{|event|
    if event.Event_Script_Name =~ /^SCRID_TRID_(.*)_/ then
    
      # =====================================
      # トレーナータイプ限定チェック
      tr_type = $1
      case tr_type
        when "DOCTOR"
          if event.Parameter_0_Number.to_i != 0 then
            puts "ドクターなのに視線が0でない"
            set_error_data( error_array ,filename ,event ,"回復トレーナーの視線が0でない")
          end
        when "NURSE"
          if event.Parameter_0_Number.to_i != 0 then
            set_error_data( error_array ,filename ,event ,"回復トレーナーの視線が0でない")
          end
        when "FISHING"
          #釣り人は視線0でも良い
        else
          # 通常トレーナーは視線０ではいけない
          if event.Parameter_0_Number.to_i <= 0 then
            set_error_data( error_array ,filename ,event ,"通常トレーナー（釣り、回復以外）の視線が0以下になっている")
          end
      end

      # =================================
      # 全トレーナーチェック
     
      # ランダム移動チェック
      # ランダム移動は原則禁止
      if RND_MOVE_CODE.include?(event.MOVE_CODE_Number) then

        # 移動可能方数チェック
        # どちらかが0以上の必要がある
        if (( event.Move_Limit_X_Number.to_i > 0 ) ||
            ( event.Move_Limit_Z_Number.to_i > 0) ) then
          set_error_data( error_array ,filename ,event ,"ランダム移動(%s)が設定されていているが、limit_xとlimit_yが0になっていない。"%[event.MOVE_CODE_Number])
        end
      end

      # 視線が6以上
      if event.Parameter_0_Number.to_i >= 6 then
        set_error_data( error_array ,filename ,event ,"視線が6以上")
      end
    
    end
  }
 
  # =====================================
  # エラーデータ出力
  
  # データ出力
  CSV.open(result_file, 'w') do |writer|
    writer << [ "ファイル名", "ID" ,"トレーナータイプ" ,"エラー文章" ]
    # 検索結果出力
    error_array.each{|data|
      writer << [ data.filename, data.id ,data.type ,data.text ]
    }
  end

}

