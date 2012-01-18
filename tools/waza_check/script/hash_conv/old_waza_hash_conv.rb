# @brief  旧作ポケモンの技データをハッシュ化
# @file   old_waza_hash_conv.rb
# @author Masayuki Onoue
# @date   2012.01.18 

# main ######################################################
$KCODE = 'SJIS'
require "personal_parser.rb"

# 定数定義
OLD_NO_ROW_INDEX                = 0 # ナンバー列
OLD_NAME_ROW_INDEX              = 1 # 名前列
OLD_FORM_NAME_ROW_INDEX         = 2 # フォルム名列
OLD_SKILL_NAME_STAET_ROW_INDEX  = 3 # 技開始列

def OutputOshieWazaHash_from_old_wazadata( personal_filename, old_waza_filename, output_path ,prefix )

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

  # タイトル行データ
  header_line = nil

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
      header_line = Marshal.load(Marshal.dump(split_data))
      split_data.each_with_index{|waza_name,index|

        # 技列のみ取得
        if index >= OLD_SKILL_NAME_STAET_ROW_INDEX then
          if waza_name.include?("習得レベル")  then
            break
          end

          # 技名登録(空白を削除しておく)
          waza_list << waza_name.strip
        end
      }
      header_skip = false
      next
    end

    mons_no   = split_data[OLD_NO_ROW_INDEX].to_i
    mons_name = split_data[OLD_NAME_ROW_INDEX]
    form_name = split_data[OLD_FORM_NAME_ROW_INDEX]
    full_mons_name = mons_name+form_name

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
    split_data.each_with_index{|cell_data,index|
      # 技列のみ取得
      if index >= OLD_SKILL_NAME_STAET_ROW_INDEX then
        if cell_data.include?("●") then
          waza_index = index - OLD_SKILL_NAME_STAET_ROW_INDEX
          mons_waza_data[mons_name] << waza_list[waza_index]
        else

          # ヘッダー行をみて覚え技かどうかを判断する
          if header_line[index].include?("習得技") then
            unless cell_data.include?("－") then
              # 一応一文字チェック
              if cell_data.size <= 2 then
                raise
              end

              # 取得技は直接書かれている
              mons_waza_data[mons_name] << cell_data.to_s

            end
          end

        end
      end
    }


  }

  # 出力
  output_filename = output_path + "%sold_owaza_hash.rb"%prefix
  output_file = File::open( output_filename, "w" )

  output_file.puts "$%sold_owaza_hash = {"%prefix

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
    out = File::open("./../%serror_name_list.txt"%prefix,"w")
    out.puts "%sに書かれている不正名リスト"%old_waza_filename
    error_name_data.each{|data|
      out.puts  data
    }
    out.close
  end

end

