###################################################################################
# @file   item_listup.rb
# @brief  .ev ファイルに定義されたアイテムを列挙する
# @author obata
# @date   2010.05.10
###################################################################################
require "item_info.rb"
require "place_name_hash.rb"

# チェック対象のファイル名
CHECK_TARGET_FILE_NAME = "*.ev"

# チェック対象のディレクトリ
PATH_TO_CHECK_DIR = "../../"

# チェック対象ファイルへのパス
PATH_TO_CHECK_TARGET_FILE = PATH_TO_CHECK_DIR + CHECK_TARGET_FILE_NAME


# チェック対象外のファイル一覧
item_get_skip_file_list = Array.new
item_get_skip_file_list << PATH_TO_CHECK_DIR + "item_get_scr.ev"
item_get_skip_file_list << PATH_TO_CHECK_DIR + "debug_scr.ev"
item_get_skip_file_list << PATH_TO_CHECK_DIR + "hide_item.ev"
item_get_skip_file_list << PATH_TO_CHECK_DIR + "hyouka_scr.ev"
item_get_skip_file_list << PATH_TO_CHECK_DIR + "poke_waza.ev"
item_get_skip_file_list << PATH_TO_CHECK_DIR + "fld_item.ev"
item_get_skip_file_list << PATH_TO_CHECK_DIR + "c03r0101.ev" # 調査隊スクリプト

# チェックに失敗したファイル一覧
item_get_failed_file_list = Array.new

# チェックするコマンド一覧
item_get_command_list = Array.new
item_get_command_list << "_ADD_ITEM"
item_get_command_list << "_EASY_TALK_ITEM_EVENT"
item_get_command_list << "_ITEM_EVENT_KEYWAIT"
item_get_command_list << "_ITEM_EVENT_NOWAIT"
item_get_command_list << "_ITEM_EVENT_FIELD"
item_get_command_list << "_ITEM_EVENT_HIDEITEM"
item_get_command_list << "_ITEM_EVENT_SUCCESS_KEYWAIT"
item_get_command_list << "_ITEM_EVENT_SUCCESS_NOWAIT"

# チェック対象コマンド ==> アイテム定義ラベルを取得するための正規表現
item_label_pattern = Hash.new
item_label_pattern[ "_ADD_ITEM" ] = /_ADD_ITEM.*\(\s*(ITEM_[^,]*),/
item_label_pattern[ "_EASY_TALK_ITEM_EVENT" ] = /_EASY_TALK_ITEM_EVENT*\(\s*(ITEM_[^,]*),/
item_label_pattern[ "_ITEM_EVENT_KEYWAIT" ] = /_ITEM_EVENT_KEYWAIT.*\(\s*(ITEM_[^,]*),/
item_label_pattern[ "_ITEM_EVENT_NOWAIT" ] = /_ITEM_EVENT_NOWAIT.*\(\s*(ITEM_[^,]*),/
item_label_pattern[ "_ITEM_EVENT_FIELD" ] = /_ITEM_EVENT_FIELD.*\(\s*(ITEM_[^,]*),/
item_label_pattern[ "_ITEM_EVENT_SUCCESS_KEYWAIT" ] = /_ITEM_EVENT_SUCCESS_KEYWAIT.*\(\s*(ITEM_[^,]*),/
item_label_pattern[ "_ITEM_EVENT_SUCCESS_NOWAIT" ] = /_ITEM_EVENT_SUCCESS_NOWAIT.*\(\s*(ITEM_[^,]*),/


# 取得アイテム情報
item_info = Array.new


#----------------------------------------------------------------------------------
# @brief main
#----------------------------------------------------------------------------------
create_place_name_hash


check_file_list = Dir[ PATH_TO_CHECK_TARGET_FILE ]
check_file_list -= item_get_skip_file_list


check_file_list.each do |file_path|
  # ファイルの内容を取得
  filename = File.basename( file_path )
  file = File::open( file_path, "r" )
  file_lines = file.readlines
  file.close
  
  file_lines.each do |line|
    # 各コマンドをチェック
    item_get_command_list.each do |command|
      # コマンドを発見
      if line.include?( command ) then
        # アイテムラベル名を発見
        if item_label_pattern[ command ] =~ line then
          item_label = $1.strip
          info = ItemInfo.new
          info.file_name  = filename
          info.item_label = item_label
          info.zone_id    = GetZoneID_byEvFileName( filename )
          info.item_name  = GetItemName( item_label )
          info.zone_name  = @@place_name_hash[ info.zone_id ]
          item_info << info
        # アイテムラベル名がない
        else
          item_get_failed_file_list << file_path
        end
      end
    end
  end

  puts filename
end


# 解析結果を出力
file = File::open( "../result/result.txt", "w" )
item_info.each do |info|
  data = info.zone_id + "\t" + info.zone_name + "\t" + info.item_name
  file.puts( data )
end
file.close

# 解析失敗リストを出力
file = File::open( "../result/failed.txt", "w" )
file.puts( item_get_skip_file_list )
file.puts( item_get_failed_file_list )
file.close
