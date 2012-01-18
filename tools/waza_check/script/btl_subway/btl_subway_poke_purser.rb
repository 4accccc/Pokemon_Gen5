#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#	@file		btl_subway_poke_purser.rb
#	@brief  バトルサブウェイポケモンデータを解析する
#	@author	Masayuki Onoue
#	@date		2012.01.17
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
$KCODE = 'SJIS'
require File.dirname(__FILE__) + "/../hash/monsno_hash.rb"
require File.dirname(__FILE__) + "/../hash/form_name_hash.rb"

# =======================================================
# バトルサブウェイデータ型モンスターデータ
class BSUB_MONS_DATA

  # アクセサー
  attr_reader :index ,:mons_no ,:mons_name ,:item_name ,:waza_list ,:form_no ,:full_name

  # データ初期化
  def initialize( index ,mons_no ,mons_name ,item_name ,form_no)
    @index      = index
    @mons_no    = mons_no
    @mons_name  = mons_name
    @item_name  = item_name
    @waza_list = Array.new
    @form_no = form_no

    # フォルム込み名を取得
    form_list = $form_name_hash[ mons_name ]
    unless form_list == nil then
       @full_name = @mons_name + form_list[ @form_no ].to_s
    else
       @full_name = @mons_name
    end

  end

  # 技を追加する
  def add_waza(waza_name)
    @waza_list << waza_name
  end
end

# =======================================================
# バトルサブウェイデータ
class BSUB_POKE
 
  # アクセサー
  attr_reader :mons_list

  # 初期化
  def initialize(filename)

    # 技データ開始インデックス
    @@WAZA_NO_START_INDEX = 2

    # 技データ開始インデックス
    @@FORM_NO_INDEX = 13
        
    # dummy_item名
    @@DUMMY_ITEM_TEXT = "－－－－－－－－"
    
    # dummy_waza名
    @@DUMMY_WAZA_TEXT = "－－－－－－－"
     
    # モンスターデータリスト
    @mons_list = Array.new

    # モンスターナンバー->名前変換ハッシュ生成
    monsno_to_name = $monsno_hash.invert
  
    # 全ラインを調査して、ポケモンリストを作成する
    open(filename).each_with_index{|line,index|
      # ヘッダ行は飛ばす
      if index == 0 then next end

      # モンスターNOが取得できたときは有効データ
      split_line  =  line.split
      mons_no     = $gra2zukan_hash[split_line[0]]

      # 有効データをリスト化する
      unless mons_no == nil then

        # 有効データをチェック
        mons_name   = monsno_to_name[ mons_no ]
        item_name   = split_line[1]
        form_no     = split_line[@@FORM_NO_INDEX].to_i

        # ダミーの時は空に変換（アイテムなし）
        if @@DUMMY_ITEM_TEXT.include?(item_name) then
          item_name = ""
        end

        mons_data = BSUB_MONS_DATA.new( index-1 ,mons_no ,mons_name ,item_name ,form_no)


        # 技名登録
        4.times{|i|
          # 技データを登録する
          waza_name = split_line[@@WAZA_NO_START_INDEX+i]
          unless waza_name == @@DUMMY_WAZA_TEXT then
            mons_data.add_waza( waza_name )
          end
        }

        # データをリストに登録
        @mons_list << mons_data
      end
    }
  end
end


