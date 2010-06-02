# @brief  子技ハッシュテーブルを作成する
# @file   kowaza_hash_conv.rb
# @author obata
# @date   2010.05.27 

# 列インデックス
COLUMN_MONS_NAME    = 2
COLUMN_KOWAZA_FIRST = 7
COLUMN_KOWAZA_LAST  = 22


def OutputEggWazaHash( kowaza_filename, output_path )

  kowaza_file = File::open( kowaza_filename, "r" )
  kowaza_lines = kowaza_file.readlines
  kowaza_lines.slice!(0) # 先頭行は除外
  kowaza_file.close

  out_data = Array.new
  out_data << "$eggwaza_hash = {" 

  kowaza_lines.each do |line|
    items = line.split( /\s/ )
    mons_name = items[ COLUMN_MONS_NAME ] 
    kowaza_list = Array.new

    COLUMN_KOWAZA_FIRST.upto( COLUMN_KOWAZA_LAST ) do |column_idx|
      waza_name = items[ column_idx ]
      if waza_name != nil then
        kowaza_list << waza_name
      end
    end

    out_data << "\t\"#{mons_name}\"=>["
    kowaza_list.each do |waza_name|
      out_data << "\t\t\"#{waza_name}\","
    end
    out_data << "\t],"
  end

  out_data << "}"

  # 出力
  output_filename = output_path + "eggwaza_hash.rb"
  output_file = File::open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close

end
