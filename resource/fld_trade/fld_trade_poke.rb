#################################################################
# @brief  ゲーム内ポケモン交換データ コンバータ
# @file   fld_trade_poke.h
# @author obata
# @date   2009.12.11
#################################################################
require "gmm_generator.rb"


#===================================================================
# ■行インデックス
#===================================================================
ROW_MONSNO        = 1   # モンスターナンバー 
ROW_NICKNAME      = 2   # ニックネーム
ROW_FORM          = 3   # フォームナンバー
ROW_LEVEL         = 4   # レベル
ROW_HP_RND        = 5   # HP乱数
ROW_AT_RND        = 6   # 攻撃乱数
ROW_DF_RND        = 7   # 防御乱数
ROW_AG_RND        = 8   # 速さ乱数
ROW_SA_RND        = 9   # 特攻乱数
ROW_SD_RND        = 10  # 特防乱数
ROW_TOKUSEI       = 11  # 特殊能力
ROW_SEIKAKU       = 12  # 性格
ROW_SEX           = 13  # 性別
ROW_ID            = 14  # ID
ROW_STYPE         = 15  # かっこよさ
ROW_BEAUTIFUL     = 16  # うつくしさ
ROW_CUTE          = 17  # かわいさ
ROW_CLEVER        = 18  # かしこさ
ROW_STRONG        = 19  # たくましさ
ROW_ITEM          = 20  # アイテム
ROW_OYA_NAME      = 21  # 親名
ROW_OYA_SEX       = 22  # 親性別
ROW_FUR           = 23  # 毛艶
ROW_COUNTRY       = 24  # 親の国コード
ROW_CHANGE_MONSNO = 25  # 交換するモンスターナンバー
ROW_CHANGE_SEX    = 26  # 交換するポケモンの性別
ROW_STRID_NICKNAME= 27  # ニックネームラベル
ROW_STRID_OYA_NAME= 28  # 親名ラベル

#===================================================================
# ■出力データ
#===================================================================
class OutputData
  #--------
  # 初期化
  #--------
	def initialize
    # ラベル
    @label_nickname      = nil  # ニックネーム
    @label_monsno        = nil  # モンスターナンバー
    @label_formno        = nil  # フォームナンバー
    @label_tokusei       = nil  # 特殊能力
    @label_seikaku       = nil  # 性格
    @label_sex           = nil  # 性別
    @label_item          = nil  # アイテム
    @label_oya_name      = nil  # 親名
    @label_oya_sex       = nil  # 親性別
    @label_country       = nil  # 親の国コード
    @label_chnage_monsno = nil  # 交換するモンスターナンバー
    @label_change_sex    = nil  # 交換するポケモンの性別
    @label_str_id_nickname = nil # ニックネームの文字列ID
    @label_str_id_oya_name = nil # 親名の文字列ID
    # データ
    @trade_no      = 0  # 交換No.
		@monsno        = 0  # モンスターナンバー  
		@formno        = 0  # フォームナンバー
		@level         = 0  # レベル
		@hp_rnd        = 0  # HP乱数
		@at_rnd        = 0  # 攻撃乱数
		@df_rnd        = 0  # 防御乱数
		@ag_rnd        = 0  # 速さ乱数
		@sa_rnd        = 0  # 特攻乱数
		@sd_rnd        = 0  # 特防乱数
		@tokusei_idx   = 0  # 特性 ( 何番目の特性か )
		@seikaku       = 0  # 性格
		@sex           = 0  # 性別
		@id            = 0  # ID
		@style         = 0  # かっこよさ
		@beautiful     = 0  # うつくしさ
		@cute          = 0  # かわいさ
		@clever        = 0  # かしこさ
		@strong        = 0  # たくましさ
		@item          = 0  # アイテム
		@oya_sex       = 0  # 親性別
		@fur           = 0  # 毛艶
		@country       = 0  # 親の国コード
		@change_monsno = 0  # 交換するモンスターナンバー
		@change_sex    = 0  # 交換するポケモンの性別
    @str_id_nickname= 0  # ニックネームの文字列ID
    @str_id_oya_name= 0  # 親名の文字列ID
	end 
  #------------
	# アクセッサ
  #------------
	attr_accessor :label_nickname, :label_monsno, :label_formno, 
                :label_tokusei, :label_seikaku, :label_sex, :label_item, 
                :label_oya_name, :label_oya_sex, :label_country,
                :label_change_monsno, :label_change_sex,
                :label_str_id_nickname, :label_str_id_oya_name,
                :trade_no, :monsno, :formno, :level,
		            :hp_rnd, :at_rnd, :df_rnd, :ag_rnd, :sa_rnd, :sd_rnd,
                :tokusei_idx, :seikaku, :sex, :id,
                :style, :beautiful, :cute, :clever, :strong,
                :item, :oya_sex, :fur, :country, 
                :change_monsno, :change_sex,
                :str_id_nickname, :str_id_oya_name
  # デバッグ出力
  def debug_out(path)
    # データが設定されていない
    if @label_monsno == nil then return end
    # ファイル名を生成
    monsname = @label_monsno.sub("MONSNO_", "").downcase
    filename = path + "/trade_poke_#{monsname}.txt"
    # ファイルに出力
    file = File::open(filename, "w")
    file.puts("TRADE_NO      = #{trade_no}")
    file.puts("NICKNAME      = #{@label_nickname}")
    file.puts("MONSNO        = #{@monsno}(#{@label_monsno})")
    file.puts("FORMNO        = #{@formno}(#{@label_formno})")
    file.puts("LEVEL         = #{@level}")
    file.puts("HP_RND        = #{@hp_rnd}")
    file.puts("DF_RND        = #{@df_rnd}")
    file.puts("AG_RND        = #{@ag_rnd}")
    file.puts("SA_RND        = #{@sa_rnd}")
    file.puts("SD_RND        = #{@sd_rnd}")
    file.puts("TOKUSEI       = #{@tokusei_idx}(#{@label_tokusei})")
    file.puts("SEIKAKU       = #{@seikaku}(#{@label_seikaku})")
    file.puts("SEX           = #{@sex}(#{@label_sex})")
    file.puts("ID            = #{@id}")
    file.puts("STYLE         = #{@style}")
    file.puts("BEAUTIFUL     = #{@beautiful}")
    file.puts("CUTE          = #{@cute}")
    file.puts("CLEVER        = #{@clever}")
    file.puts("STRONG        = #{@strong}")
    file.puts("ITEM          = #{@item}(#{@label_item})")
    file.puts("OYA_NAME      = #{@label_oya_name}")
    file.puts("OYA_SEX       = #{@oya_sex}(#{@label_oya_sex})")
    file.puts("FUR           = #{@fur}")
    file.puts("COUNTRY       = #{@country}(#{@label_country})")
    file.puts("CHANGE_MONSNO = #{@change_monsno}(#{@label_change_monsno})")
    file.puts("CHANGE_SEX    = #{@change_sex}(#{@label_change_sex})")
    file.puts("STRID_NICKNAME = #{@str_id_nickname}(#{label_str_id_nickname})")
    file.puts("STRID_OYA_NAME = #{@str_id_oya_name}(#{label_str_id_oya_name})")
    file.close
  end
  #------------------------
  # バイナリファイル名取得
  #------------------------
  def binary_filename
    monsname = @label_monsno.sub("MONSNO_", "").downcase
    filename = "trade_poke_#{monsname}.bin"
    return filename
  end
  #--------------
  # バイナリ出力
  #--------------
  def binary_out(path)
    file = File::open( path + "/" + self.binary_filename, "wb")
    data = Array::new
    data << @trade_no << @monsno << @formno << @level <<
            @hp_rnd << @at_rnd << @df_rnd << @ag_rnd << @sa_rnd << @sd_rnd <<
            @tokusei_idx << @seikaku << @sex << @id <<
            @style << @beautiful << @cute << @clever << @strong <<
            @item << @oya_sex << @fur << @country <<
            @change_monsno << @change_sex <<
            @str_id_nickname << @str_id_oya_name
    file.write(data.pack("I*"))
    file.close
  end
  #-------------------------------
  # 交換データNo.定義ラベル名取得
  #-------------------------------
  def def_label
    monsname = @label_monsno.sub("MONSNO_", "")
    label    = "FLD_TRADE_POKE_#{monsname}"
    return label
  end
end

#===================================================================
# ■サブルーチン
#===================================================================

#-----------------------------------------------------------
# @brief 交換データが定義されている全ての列番号を検索する
# @return 交換データの列番号配列
#-----------------------------------------------------------
def SearchPokeDataColumn()
	columns   = Array::new
	# ファイル読み込み
	file      = File::open(ARGV[0], "r")
	line      = file.readlines[ROW_MONSNO]
	file.close
	# 検索
	line_data = line.split(/\s/)
	0.upto(line_data.size-1) do |idx|
		if line_data[idx].index("MONSNO_")!=nil then
			columns << idx
		end
	end
	return columns
end

#-----------------------------------------------------------
# @brief モンスターNo.の定義番号を取得する
# @param label ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetMonsnoVal( label )
  #----------------------------------------
	# 参照ファイル読み込み
	file = File::open("monsno_def.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{label}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end 
  #----------------------------------------
	# 参照ファイル読み込み
	file = File::open("monsnum_def.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{label}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: モンスターNo.(#{label})は定義されていません。")
end

#-----------------------------------------------------------
# @brief フォームNo.の定義番号を取得する
# @param label ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetFormnoVal( label )
	# 参照ファイル読み込み
	file = File::open("monsno_def.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{label}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
  return 0
end

#-----------------------------------------------------------
# @brief アイテムの定義番号を取得する
# @param label ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetItemVal( label )
	# 参照ファイル読み込み
	file = File::open("itemsym.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{label}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: アイテム(#{label})は定義されていません。")
end

#-----------------------------------------------------------
# @brief 性別の定義番号を取得する
# @param label ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetSexVal( label )
	# 参照ファイル読み込み
	file = File::open("pm_version.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{label}\s*(\d*)\s*/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: 性別(#{label})は定義されていません。")
end

#-----------------------------------------------------------
# @brief 言語コードの定義番号を取得する
# @param label ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetLangCodeVal( label )
	# 参照ファイル読み込み
	file = File::open("pm_version.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{label}.*\s*(\d*)\s/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: 言語コード(#{label})は定義されていません。")
end

#-----------------------------------------------------------
# @brief 特性の定義番号を取得する
# @param label ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetTokuseiVal( label )
	# 参照ファイル読み込み
	file = File::open("tokusyu_def.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{label}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: 特性(#{label})は定義されていません。")
end

#-----------------------------------------------------------
# @brief 特性のインデックス番号を取得する
# @param label ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetTokuseiIndex( label )
  hash = Hash.new
  hash[ "TOKUSEI_1" ] = 0
  hash[ "TOKUSEI_2" ] = 1
  hash[ "TOKUSEI_3" ] = 2

  if hash.has_key?(label) == false then
    abort( "特性(#{label})の指定が間違っています\n" )
  end

  return hash[ label ]
end

#-----------------------------------------------------------
# @brief 性格の定義番号を取得する
# @param label ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetSeikakuVal( label )
	# 参照ファイル読み込み
	file = File::open("poke_tool.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{label}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: 性格(#{label})は定義されていません。")
end


#===================================================================
# ■メイン
#===================================================================

#-------------------------------------------------------------------
# @brief ゲーム内交換データをコンバートする
# @param ARGV[0] コンバート対象ファイル
# @param ARGV[1] バイナリ出力先
#-------------------------------------------------------------------
trade_data = Array::new
# ファイルデータ読み込み
file = File::open(ARGV[0], "r")
file_data = file.readlines
file.close
row_data = Array.new
file_data.each do |row|
  row_data << row.split(/\s/)
end
# 有効データの列番号を取得
column_idx = SearchPokeDataColumn()
trade_no = 0
# 全データを取得
column_idx.each do |idx|
  data = OutputData::new
  # 文字列として取得
  data.trade_no              = trade_no
  data.label_monsno          = row_data[ROW_MONSNO       ][idx]
  data.label_nickname        = row_data[ROW_NICKNAME     ][idx]
  data.label_formno          = row_data[ROW_FORM         ][idx]
  data.level                 = row_data[ROW_LEVEL        ][idx]
  data.hp_rnd                = row_data[ROW_HP_RND       ][idx]
  data.at_rnd                = row_data[ROW_AT_RND       ][idx]
  data.df_rnd                = row_data[ROW_DF_RND       ][idx]
  data.ag_rnd                = row_data[ROW_AG_RND       ][idx]
  data.sa_rnd                = row_data[ROW_SA_RND       ][idx]
  data.sd_rnd                = row_data[ROW_SD_RND       ][idx]
  data.label_tokusei         = row_data[ROW_TOKUSEI      ][idx]
  data.label_seikaku         = row_data[ROW_SEIKAKU      ][idx]
  data.label_sex             = row_data[ROW_SEX          ][idx]
  data.id                    = row_data[ROW_ID           ][idx]
  data.style                 = row_data[ROW_STYPE        ][idx]
  data.beautiful             = row_data[ROW_BEAUTIFUL    ][idx]
  data.cute                  = row_data[ROW_CUTE         ][idx]
  data.clever                = row_data[ROW_CLEVER       ][idx]
  data.strong                = row_data[ROW_STRONG       ][idx]
  data.label_item            = row_data[ROW_ITEM         ][idx]
  data.label_oya_name        = row_data[ROW_OYA_NAME     ][idx]
  data.label_oya_sex         = row_data[ROW_OYA_SEX      ][idx]
  data.fur                   = row_data[ROW_FUR          ][idx]
  data.label_country         = row_data[ROW_COUNTRY      ][idx]
  data.label_change_monsno   = row_data[ROW_CHANGE_MONSNO][idx]
  data.label_change_sex      = row_data[ROW_CHANGE_SEX   ][idx]
  data.label_str_id_nickname = row_data[ROW_STRID_NICKNAME][idx]
  data.label_str_id_oya_name = row_data[ROW_STRID_OYA_NAME][idx]
  # 値に変換
  data.monsno        = GetMonsnoVal(data.label_monsno)
  data.formno        = GetFormnoVal(data.label_formno)
  data.level         = data.level.to_i
  data.hp_rnd        = data.hp_rnd.to_i
  data.at_rnd        = data.at_rnd.to_i
  data.df_rnd        = data.df_rnd.to_i
  data.ag_rnd        = data.ag_rnd.to_i
  data.sa_rnd        = data.sa_rnd.to_i
  data.sd_rnd        = data.sd_rnd.to_i
  data.tokusei_idx   = GetTokuseiIndex(data.label_tokusei)
  data.seikaku       = GetSeikakuVal(data.label_seikaku)
  data.sex           = GetSexVal(data.label_sex)
  data.id            = data.id.to_i
  data.style         = data.style.to_i
  data.beautiful     = data.beautiful.to_i
  data.cute          = data.cute.to_i
  data.clever        = data.clever.to_i
  data.strong        = data.strong.to_i
  data.item          = GetItemVal(data.label_item)
  data.oya_sex       = GetSexVal(data.label_oya_sex)
  data.fur           = data.fur.to_i
  data.country       = GetLangCodeVal(data.label_country)
  data.change_monsno = GetMonsnoVal(data.label_change_monsno)
  data.change_sex    = GetSexVal(data.label_change_sex)
  data.str_id_nickname = trade_no * 2
  data.str_id_oya_name = trade_no * 2 + 1
  # 配列に登録
  trade_data << data
  # 交換No.をインクリメント
  trade_no = trade_no + 1
end 
# デバッグ出力
trade_data.each do |data|
  data.debug_out(ARGV[1])
end
# バイナリ出力
trade_data.each do |data|
  data.binary_out(ARGV[1])
end
# バイナリ リスト出力
file = File::open("fld_trade_list.lst", "w")
trade_data.each do |data|
  file.puts("\"#{ARGV[1]}/#{data.binary_filename}\"")
end
file.close
# ヘッダー出力
file = File::open("fld_trade_list.h", "w")
comment = "
//-------------------------------------------------
/**
 * @brief 交換データ指定ID
 * @file fld_trade_list.h
 * @author obata
 *
 * fld_trade_poke.rb により自動生成
 */ 
//-------------------------------------------------"
file.puts(comment)
val = 0
trade_data.each do |data|
  label = data.def_label
  file.puts("#define #{label} (#{val})")
  val = val + 1
end
file.puts("#define FLD_TRADE_POKE_NUM (#{val})")
file.close


# gmm を出力
directory = "./"
filename = "fld_trade.gmm"
stringLabel = Array.new
string = Array.new
trade_data.each do |data|
  stringLabel << data.label_str_id_nickname
  stringLabel << data.label_str_id_oya_name
  string << data.label_nickname
  string << data.label_oya_name
end
GenerateGMM( directory, filename, stringLabel, string, string )
