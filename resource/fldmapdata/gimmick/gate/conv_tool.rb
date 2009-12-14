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
