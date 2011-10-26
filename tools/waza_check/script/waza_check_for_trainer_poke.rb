require "personal_accessor.rb"
require "trdata_parser.rb"
require "oshiewaza_hash.rb"
require "waza_oshie_hash.rb"

personal_accessor = PersonalAccessor.new

# トレーナーデータ解析
trainer_list = GetTrainerList( "../data/trainer_wb.csv" )

# 教え技か調べる
def is_oshiewaza( waza_name )
  if $oshie_waza_hash_red.values.include?( waza_name ) then return true end
  if $oshie_waza_hash_blue.values.include?( waza_name ) then return true end
  if $oshie_waza_hash_yellow.values.include?( waza_name ) then return true end
  if $oshie_waza_hash_green.values.include?( waza_name ) then return true end
  return false
end

# 技チェック
out_data = Array.new
trainer_list.each do |trainer|
  out_data << "#{trainer.type} #{trainer.name}\r\n"

  trainer.pokemon_list.each do |poke|
    out_data << " #{poke.fullname} Lv.#{poke.level}\r\n"

    poke.waza_list.each do |waza|
      oshiewaza_list = $oshiewaza_hash[ poke.fullname ]
      if (personal_accessor.check_waza_learning( poke.fullname, poke.level, waza )) ||
         ( is_oshiewaza( waza ) == true && oshiewaza_list.include?( waza ) == true ) then
        out_data << "  ○#{waza}\r\n"
      else
        out_data << "  ×#{waza}\r\n"
      end
    end
  end
end

# 出力
out_file = File::open( "../result_trainer.txt", "w" )
out_file.puts( out_data )
out_file.close 
