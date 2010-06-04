require "personal_accessor.rb"
require "trdata_parser.rb"

personal_accessor = PersonalAccessor.new

# トレーナーデータ解析
trainer_list = GetTrainerList( "../data/trainer_wb.csv" )

# 技チェック
out_data = Array.new
trainer_list.each do |trainer|
  out_data << "#{trainer.type} #{trainer.name}\r\n"

  trainer.pokemon_list.each do |poke|
    out_data << " #{poke.fullname} Lv.#{poke.level}\r\n"

    poke.waza_list.each do |waza|
      if personal_accessor.check_waza_learning( poke.fullname, poke.level, waza ) then
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
