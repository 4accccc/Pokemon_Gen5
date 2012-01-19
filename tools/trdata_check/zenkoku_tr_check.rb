
require "mev_analyzer.rb"
require "mev_parser.rb"
require "trdata_parser.rb"
require "trdata_parser.rb"
require "monsno_hash.rb"
require "chihou_zukanno_hash.rb"

# トレーナーデータ解析
trainer_list = GetTrainerList( "../../resource/trdata/trainer_wb.csv" )

# 全国ポケモンを所持しているポケモントレーナーIDリスト
zenkoku_tr_id_hash = Hash.new

# 全国ポケモンを所持しているトレーナーIDリストを作成
trainer_list.each{|data|
  tr_id = data.id.strip
  data.pokemon_list.each{|poke|
    if $monsno_hash.key?(poke.mons_name) then
      mons_no   = $monsno_hash[poke.mons_name]
      chihou_no = $chihou_zukanno_hash[mons_no]

      # 地方図鑑に載っているかチェック
      if chihou_no == 999 then

        # 未登録の場合は配列を生成
        unless zenkoku_tr_id_hash.key?(tr_id) then
          zenkoku_tr_id_hash[tr_id]  = Array.new
        end

        # 全国ポケモンを登録
        zenkoku_tr_id_hash[tr_id] << poke.mons_name
      end
    end
  }
}

# チェックするmevのディレクトリ
PATH_TO_DIR     = "../../resource/fldmapdata/eventdata/data/"
TARGET_FILENAME = "*.mev"

out_data = Array.new

filenames = Dir[ PATH_TO_DIR + TARGET_FILENAME ]
filenames.each{|filename|

  mev_data = MEVFile.new
  mev_data.load( filename )
  
  mev_data.obj_section.event.each{|event|
    if event.Event_Script_Name =~ /^SCRID_TRID_(.*)/ then
      
      # トレーナーID取得
      tr_id = $1.downcase.strip

      # 全国ポケモン所持トレーナかどうかをしらべる
      if zenkoku_tr_id_hash.key?(tr_id) then
          out_data << "%s:%s=>%s"%[File::basename(filename).ljust(15),tr_id.ljust(15),zenkoku_tr_id_hash[tr_id].join(",")]
      end
    end
  }
}

# 全国トレーナーIDリスト
file = File.open( "zenkoku_trid_list.txt", "w" )
file.puts "#=============================================="
file.puts "#全国ポケモンを所持しているトレーナーリストです"
file.puts "#=============================================="
file.puts out_data
file.close

