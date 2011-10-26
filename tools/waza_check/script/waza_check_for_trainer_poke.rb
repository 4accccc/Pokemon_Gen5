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
out_file = File::open( "../result_trainer.txt", "w" )
trainer_list.each do |trainer|
  out_data = ""

  trainer.pokemon_list.each do |poke|
    is_out = false
    poke.waza_list.each do |waza|
      oshiewaza_list = $oshiewaza_hash[ poke.fullname ]
      unless ((personal_accessor.check_waza_learning( poke.fullname, poke.level, waza )) ||
        ( is_oshiewaza( waza ) == true && oshiewaza_list.include?( waza ) == true )) then
        if is_out == false then
          # トレーナー名首都力
          if out_data == "" then
            out_data << "\r\n#{trainer.type} #{trainer.name}\r\n"
          end
          # ポケモン名出力
          out_data << " #{poke.fullname} Lv.#{poke.level}\r\n"
        end
        # 技名出力
        out_data << "  ×#{waza}\r\n"
      end
    end
  end
  unless out_data == "" then out_file.puts( out_data ) end
end

out_file.close 
