# @brief  旧作の各ポケモンが習得可能なおしえ技のハッシュテーブルを作成する
# @file   old_oshiewaza_hash_conv.rb
# @author Masayuki Onoue
# @date   2012.01.18 

# main ######################################################

require "personal_parser.rb"

# 定数定義
NO_ROW_INDEX                = 0 # ナンバー列
NAME_ROW_INDEX              = 1 # 名前列
FORM_NAME_ROW_INDEX         = 2 # フォルム名列
SKILL_NAME_STAET_ROW_INDEX  = 3 # 技開始列
SKIP_NAME_LIST = ["？？？"]

def OutputOshieWazaHash_from_old_wazaoshiedata( personal_filename, old_waza_filename, output_path )

  # モンスター完全名リスト取得
  personal_parser = PersonalDataParser.new( personal_filename )
  mons_fullname_list = personal_parser.get_mons_fullname_list

  # エラー名リスト
  error_name_data = Array.new

  # ポケモンごとの技データ
  mons_waza_data = Hash.new

  # header行スキップ
  header_skip = true

  # シート番号
  sheet_no = 0
  
  # 技リスト
  waza_list = nil

  # 旧作データ解析
  puts old_waza_filename
  File::open(old_waza_filename,"r").each{|line|

    # 通常ライン
    split_data = line.split(",")

    # 終端文字検索
    if line.include?("#END") then
      header_skip = true
      next
    end

    if header_skip == true then
      sheet_no+=1
      # 技リスト取得
      waza_list = Array.new
      split_data.each_with_index{|waza_name,index|

        # 技列のみ取得
        if index >= SKILL_NAME_STAET_ROW_INDEX then
          if waza_name.include?("#SKILL_END")  then
            break
          end

          # 技名登録(空白を削除しておく)
          waza_list << waza_name.strip
        end
      }
      header_skip = false
      next
    end

    mons_no   = split_data[NO_ROW_INDEX].to_i
    mons_name = split_data[NAME_ROW_INDEX]
    form_name = split_data[FORM_NAME_ROW_INDEX]
    full_mons_name = mons_name+form_name

    # スキップするポケモン名
    if SKIP_NAME_LIST.include?(mons_name) then
      next
    end

    # エラーモンスター名取得
    if mons_fullname_list.index(full_mons_name) == nil then
      error_name_data << "シート番号%d:%s"%[sheet_no,full_mons_name]
      next
    end

    # モンスター名登録チェック 
    unless mons_waza_data.key?( mons_name ) then
      mons_waza_data[mons_name] = Array.new
    end

    # 技登録

    split_data.each_with_index{|mark,index|
      # 技列のみ取得
      if index >= SKILL_NAME_STAET_ROW_INDEX then
        if mark.include?("●") then
          waza_index = index - SKILL_NAME_STAET_ROW_INDEX
          mons_waza_data[mons_name] << waza_list[waza_index]
        end
      end
    }


  }

  # 出力
  output_filename = output_path + "old_oshiewaza_hash.rb"
  output_file = File::open( output_filename, "w" )

  output_file.puts "$old_oshiewaza_hash = {"

  mons_waza_data.each_key{|key|
    waza_array = mons_waza_data[key].uniq
    output_file.puts  '  "%s" => ['%key
    waza_array.each{|waza_name|
      output_file.puts  '    "%s",'%waza_name
    }
    output_file.puts  '  ],'
  }
  output_file.puts "}"
  output_file.close

  if error_name_data.size > 0 then
    # エラー名出力
    out = File::open("./../error_name_list.txt","w")
    out.puts ".\tools\waza_check\data\old_waza_oshie.xlsに書かれている不正名リスト"
    error_name_data.each{|data|
      out.puts  data
    }
    out.close
  end

end

