######################################################################################
# @brief コンバート共通モジュール
# @file  conv_tool.rb
# @autor obata
# @date  2009.12.14
######################################################################################

#-------------------------------------------------------------------------------------
# @brief ゾーンIDを取得する
# @param zone ゾーンIDを表す文字列
# @return 指定したゾーンIDの値
#-------------------------------------------------------------------------------------
def GetZoneID( zone )

  # 定義ファイルを開く
  filename = ENV["PROJECT_RSCDIR"] + "fldmapdata/zonetable/zone_id.h"
  file = File.open( filename, "r" )

  # 指定されたゾーンIDを検索
  file.each do |line|
    if line.index(zone) != nil then
      i0 = line.index("(") + 1
      i1 = line.index(")") - 1
      val = line[i0..i1].to_i
      return val
    end
  end
  file.close

  # 指定されたゾーンIDが定義されていない場合
  abort("ゾーンID:#{zone}は定義されていません")

end

#-------------------------------------------------------------------------------------
# @brief バージョンを取得する
# @param version "WB" or "W" or "B"
# @return "W"=>VERSION_WHITE, "B"=>VERSION_BLACK, "WB"=>0
#-------------------------------------------------------------------------------------
def GetVersion( version )

  # "WB"が指定された
  if version == "WB" then return 0 end

  # ハッシュテーブル作成
  hash_table = Hash::new
  hash_table["W"] = "VERSION_WHITE"
  hash_table["B"] = "VERSION_BLACK" 
  # 存在しないキーが指定された
  if hash_table.has_key?(version) == false then
    abort("バージョン指定[#{version}]は対応していません")
  end 
  # 指定されたバージョンについて, 検索ワードを決定
  target_word = hash_table[ version ]

  # 定義ファイルを開く
  filename = ENV["PROJECT_PROGDIR"] + "include/pm_version.h"
  file = File.open( filename, "r" )
  file_lines = file.readlines
  file.close

  # 検索ワードの定義値を取得
  file_lines.each do |line|
    if (line =~ /#define\s*#{target_word}\s*(\d+)/) != nil then return $1.to_i end
  end

  # 指定されたバージョンが定義されていない場合
  abort("バージョン:#{target_word}は定義されていません")
end

#-------------------------------------------------------------------------------------
# @brief メッセージIDを取得する
# @param gmm_filename gmmファイル名
# @param msg_id       メッセージIDを表す文字列
# @return 指定したメッセージIDの値
#-------------------------------------------------------------------------------------
def GetMsgID( gmm_filename, msg_id )

  # 定義ファイルを開く
  def_filename = 
    ENV["PROJECT_PROGDIR"] + "include/msg/msg_" + File.basename(gmm_filename, ".gmm") + ".h"
  file = File.open( def_filename, "r" )

  # 指定されたメッセージIDを検索
  file.each do |line|
    if line.index(msg_id) != nil then
      i0 = line.index("(") + 1
      i1 = line.index(")") - 1
      val = line[i0..i1].to_i
      return val
    end
  end
  file.close

  # 指定されたメッセージIDが定義されていない場合
  abort("メッセージID:#{msg_id}は定義されていません")

end

#-------------------------------------------------------------------------------------
# @brief 指定した向きに対応する値を取得する
# @param dir 向きを表す文字列(DIR_xxxx)
# @return 指定した向きを表す値
#-------------------------------------------------------------------------------------
def GetDir( dir )

  # 定義ファイルを開く
  def_filename = ENV["PROJECT_PROGDIR"] + "include/field/field_dir.h"
  file = File.open( def_filename, "r" )

  # 指定された向きを検索
  file.each do |line|
    if line.index(dir) != nil then
      i0 = line.index("(") + 1
      i1 = line.index(")") - 1
      val = line[i0..i1].to_i
      return val
    end
  end
  file.close

  # 指定された向きが定義されていない場合
  abort("向き:#{dir}は定義されていません")
end

#-------------------------------------------------------------------------------------
# @brief フラグIDを取得する
# @param flag フラグIDを表す文字列
# @return 指定したフラグIDの値
#-------------------------------------------------------------------------------------
def GetFlagID( flag )

  # 定義ファイルを開く
  filename = ENV["PROJECT_RSCDIR"] + "fldmapdata/flagwork/flag_define.h"
  file = File.open( filename, "r" )

  # 指定されたフラグIDを検索
  file.each do |line|
		if line.index(/#define.*#{flag}\s*(\d*)\s*/) != nil then 
			return $1.to_i
		end
  end
  file.close

  # 指定されたフラグIDが定義されていない場合
  abort("フラグID:#{flag}は定義されていません")

end
