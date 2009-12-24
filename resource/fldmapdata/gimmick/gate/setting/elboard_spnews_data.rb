###########################################################################
#
# @brief  電光掲示板 臨時ニュース データ コンバータ
# @file   elboard_spnews_data.rb
# @author obata
# @date   2009.12.14
#
###########################################################################
require "conv_tool" 

#==========================================================================
# ■定数
#==========================================================================
# 行インデックス
ROW_DATAHEAD = 2  # 実データ開始行

# 列インデックス
COLUMN_FLAG         = 0  # フラグ 
COLUMN_FLAG_CONTROL = 1  # フラグ操作
COLUMN_MSG_ID       = 2  # 表示するニュースのメッセージID
COLUMN_NEWS_TYPE    = 3  # タイプ
COLUMN_ZONE_ID_1    = 4  # 表示するゾーン
COLUMN_ZONE_ID_2    = 5  # 表示するゾーン
COLUMN_ZONE_ID_3    = 6  # 表示するゾーン
COLUMN_ZONE_ID_4    = 7  # 表示するゾーン


#==========================================================================
# ■出力データ
#==========================================================================
class OutputData 
  #-----------------
  # コンストラクタ
  #-----------------
  def initialize 
    # ラベル
    @lavel_flag
    @lavel_flagControl
    @lavel_msgID
    @lavel_newsType
    @lavel_zoneID_1
    @lavel_zoneID_2
    @lavel_zoneID_3
    @lavel_zoneID_4
    # データ
    @priority    = 0  # プライオリティ
    @flag        = 0  # フラグ 
    @flagControl = 0  # フラグ操作
    @msgID       = 0  # 表示するニュースのメッセージID
    @newsType    = 0  # タイプ
    @zoneID_1    = 0  # 表示するゾーン
    @zoneID_2    = 0  # 表示するゾーン
    @zoneID_3    = 0  # 表示するゾーン
    @zoneID_4    = 0  # 表示するゾーン
  end 
  #------------
  # アクセッサ
  #------------ 
  attr_accessor :lavel_flag, :lavel_flagControl, :lavel_msgID, :lavel_newsType,
                :lavel_zoneID_1, :lavel_zoneID_2, :lavel_zoneID_3, :lavel_zoneID_4,
                :priority, :flag, :flagControl, :msgID, :newsType,
                :zoneID_1, :zoneID_2, :zoneID_3, :zoneID_4
  #--------------------
  # 出力ファイル名取得
  #--------------------
  def GetFilename()
    filename = "elboard_spnews_data_#{@priority}"
    return filename
  end
  #--------------
  # バイナリ出力
  #--------------
  def BinaryOut(path)
    filename = path + "/" + self.GetFilename() + ".bin"
    file = File::open(filename, "wb")
    data = Array.new
    data << @flag << @flagControl << @msgID << @newsType <<
            @zoneID_1 << @zoneID_2 << @zoneID_3 << @zoneID_4
    file.write(data.pack("I*"))
    file.close
  end
  #--------------
  # デバッグ出力
  #--------------
  def DebugOut(path)
    filename = path + "/" + self.GetFilename() + ".txt"
    file = File::open(filename, "w")
    file.puts("FLAG         = #{@lavel_flag}(#{@flag})")
    file.puts("FLAG_CONTROL = #{@lavel_flagControl}(#{@flagControl})")
    file.puts("MSG_ID       = #{@lavel_msgID}(#{@msgID})")
    file.puts("NEWS_TYPE    = #{@lavel_newsType}(#{@newsType})")
    file.puts("ZONE_ID_1    = #{@lavel_zoneID_1}(#{@zoneID_1})")
    file.puts("ZONE_ID_2    = #{@lavel_zoneID_2}(#{@zoneID_2})")
    file.puts("ZONE_ID_3    = #{@lavel_zoneID_3}(#{@zoneID_3})")
    file.puts("ZONE_ID_4    = #{@lavel_zoneID_4}(#{@zoneID_4})")
    file.close
  end
end

#==========================================================================
# ■サブルーチン
#==========================================================================

#--------------------------------------------------------------------------
# @breif フラグ操作タイプの値を取得する
# @param lavel ラベル名
# @return 指定ラベルの値
#--------------------------------------------------------------------------
def GetFlagControlVal(lavel)
	# 参照ファイル読み込み
  def_filename = ENV["PROJECT_PROGDIR"] + "src/field/elboard_spnews_data.h"
	file = File::open(def_filename, "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{lavel}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: フラグ操作タイプ(#{lavel})は定義されていません。") 
end

#--------------------------------------------------------------------------
# @breif ニュースタイプの値を取得する
# @param lavel ラベル名
# @return 指定ラベルの値
#--------------------------------------------------------------------------
def GetNewsTypeVal(lavel)
	# 参照ファイル読み込み
  def_filename = ENV["PROJECT_PROGDIR"] + "src/field/elboard_spnews_data.h"
	file = File::open(def_filename, "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{lavel}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: ニュースタイプ(#{lavel})は定義されていません。") 
end


#==========================================================================
# ■メイン
#==========================================================================

#--------------------------------------------------------------------------
# @brief 臨時ニュース データをコンバートする
# @param ARGV[0] コンバート対象ファイル
# @param ARGV[1] バイナリ出力先
#--------------------------------------------------------------------------
news_array = Array::new
# ファイルデータ読み込み
file = File::open(ARGV[0], "r")
file_data = file.readlines
file.close 
# データ取得
ROW_DATAHEAD.upto(file_data.size - 1) do |row_idx|
  line_data = file_data[row_idx].split(/\s/)
  news = OutputData::new
  # 文字列として取得
  news.lavel_flag        = line_data[COLUMN_FLAG]
  news.lavel_flagControl = line_data[COLUMN_FLAG_CONTROL]
  news.lavel_msgID       = line_data[COLUMN_MSG_ID]
  news.lavel_newsType    = line_data[COLUMN_NEWS_TYPE]
  news.lavel_zoneID_1    = line_data[COLUMN_ZONE_ID_1]
  news.lavel_zoneID_2    = line_data[COLUMN_ZONE_ID_2]
  news.lavel_zoneID_3    = line_data[COLUMN_ZONE_ID_3]
  news.lavel_zoneID_4    = line_data[COLUMN_ZONE_ID_4]
  # ゾーンID固有変換
  if news.lavel_zoneID_1 == "-" then news.lavel_zoneID_1 = "ZONE_ID_MAX" end
  if news.lavel_zoneID_2 == "-" then news.lavel_zoneID_2 = "ZONE_ID_MAX" end
  if news.lavel_zoneID_3 == "-" then news.lavel_zoneID_3 = "ZONE_ID_MAX" end
  if news.lavel_zoneID_4 == "-" then news.lavel_zoneID_4 = "ZONE_ID_MAX" end
  # 数値に変換
  news.priority    = row_idx - ROW_DATAHEAD
  news.flag        = GetFlagID(news.lavel_flag)
  news.flagControl = GetFlagControlVal(news.lavel_flagControl)
  news.msgID       = GetMsgID("gate", news.lavel_msgID)
  news.newsType    = GetNewsTypeVal(news.lavel_newsType)
  news.zoneID_1    = GetZoneID(news.lavel_zoneID_1)
  news.zoneID_2    = GetZoneID(news.lavel_zoneID_2)
  news.zoneID_3    = GetZoneID(news.lavel_zoneID_3)
  news.zoneID_4    = GetZoneID(news.lavel_zoneID_4)
  # 配列に登録
  news_array << news
  # デバッグ出力
  news.DebugOut(ARGV[1])
  # バイナリ出力
  news.BinaryOut(ARGV[1])
end 
