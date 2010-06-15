# @brief  アイテムデータから, わざマシン名 ==> 技名 の変換ハッシュテーブルを作成する
# @file   machine_wazaname_hash_conv.rb
# @author obata
# @date   2010.05.26 

# 列番号
COLUMN_ITEM_NAME = 2
COLUMN_WAZA_NAME = 22


def OutputMachineWazaNameHash( item_filename, output_path )

  file = File::open( item_filename, "r" )
  file_lines = file.readlines
  file_lines.slice( 0 ) # 先頭行を削除
  file.close 

  # リスト作成
  machine_name_list = Array.new
  waza_name_list = Array.new

  file_lines.each do |line|
    items = line.split( /\s/ )
    item_name = items[ COLUMN_ITEM_NAME ]
    waza_name = items[ COLUMN_WAZA_NAME ]
    if item_name == nil then
      next
    end
    if item_name.include?( "わざマシン" ) || item_name.include?( "ひでんマシン" ) then
      machine_name_list << item_name
      waza_name_list << waza_name
    end
  end

  # 出力
  output_filename = output_path + "machine_wazaname_hash.rb"
  output_file = File::open( output_filename, "w" )
  output_file.puts( "$machine_wazaname_hash = {" ) 
  0.upto( machine_name_list.size - 1 ) do |idx|
    machine_name = machine_name_list[ idx ]
    waza_name = waza_name_list[ idx ]
    output_file.puts( "  \"#{machine_name}\" => \"#{waza_name}\"," )
  end
  output_file.puts( "}" ) 

end
