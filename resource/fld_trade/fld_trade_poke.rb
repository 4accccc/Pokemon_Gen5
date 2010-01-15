#################################################################
# @brief  ゲーム内ポケモン交換データ コンバータ
# @file   fld_trade_poke.h
# @author obata
# @date   2009.12.11
#################################################################

#===================================================================
# ■行インデックス
#===================================================================
ROW_MONSNO        = 1;   # モンスターナンバー 
ROW_FORM          = 3;   # フォームナンバー
ROW_LEVEL         = 4;   # レベル
ROW_HP_RND        = 5;   # HP乱数
ROW_AT_RND        = 6;   # 攻撃乱数
ROW_DF_RND        = 7;   # 防御乱数
ROW_AG_RND        = 8;   # 速さ乱数
ROW_SA_RND        = 9;   # 特攻乱数
ROW_SD_RND        = 10;  # 特防乱数
ROW_TOKUSEI       = 11;  # 特殊能力
ROW_SEIKAKU       = 12;  # 性格
ROW_SEX           = 13;  # 性別
ROW_ID            = 14;  # ID
ROW_STYPE         = 15;  # かっこよさ
ROW_BEAUTIFUL     = 16;  # うつくしさ
ROW_CUTE          = 17;  # かわいさ
ROW_CLEVER        = 18;  # かしこさ
ROW_STRONG        = 19;  # たくましさ
ROW_ITEM          = 20;  # アイテム
ROW_OYA_SEX       = 22;  # 親性別
ROW_FUR           = 23;  # 毛艶
ROW_COUNTRY       = 24;  # 親の国コード
ROW_CHANGE_MONSNO = 25;  # 交換するモンスターナンバー
ROW_CHANGE_SEX    = 26;  # 交換するポケモンの性別

#===================================================================
# ■出力データ
#===================================================================
class OutputData
  #--------
  # 初期化
  #--------
	def initialize
    # ラベル
    @lavel_monsno        = nil  # モンスターナンバー
    @lavel_formno        = nil  # フォームナンバー
    @lavel_tokusei       = nil  # 特殊能力
    @lavel_seikaku       = nil  # 性格
    @lavel_sex           = nil  # 性別
    @lavel_item          = nil  # アイテム
    @lavel_oya_sex       = nil  # 親性別
    @lavel_country       = nil  # 親の国コード
    @lavel_chnage_monsno = nil  # 交換するモンスターナンバー
    @lavel_change_sex    = nil  # 交換するポケモンの性別
    # データ
		@monsno        = 0  # モンスターナンバー  
		@formno        = 0  # フォームナンバー
		@level         = 0  # レベル
		@hp_rnd        = 0  # HP乱数
		@at_rnd        = 0  # 攻撃乱数
		@df_rnd        = 0  # 防御乱数
		@ag_rnd        = 0  # 速さ乱数
		@sa_rnd        = 0  # 特攻乱数
		@sd_rnd        = 0  # 特防乱数
		@tokusei       = 0  # 特殊能力
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
	end 
  #------------
	# アクセッサ
  #------------
	attr_accessor :lavel_monsno, :lavel_formno, :lavel_tokusei, :lavel_seikaku,
                :lavel_sex, :lavel_item, :lavel_oya_sex, :lavel_country,
                :lavel_change_monsno, :lavel_change_sex,
                :monsno, :formno, :level,
		            :hp_rnd, :at_rnd, :df_rnd, :ag_rnd, :sa_rnd, :sd_rnd,
                :tokusei, :seikaku, :sex, :id,
                :style, :beautiful, :cute, :clever, :strong,
                :item, :oya_sex, :fur, :country, 
                :change_monsno, :change_sex
  # デバッグ出力
  def debug_out(path)
    # データが設定されていない
    if @lavel_monsno == nil then return end
    # ファイル名を生成
    monsname = @lavel_monsno.sub("MONSNO_", "").downcase
    filename = path + "/trade_poke_#{monsname}.txt"
    # ファイルに出力
    file = File::open(filename, "w")
    file.puts("MONSNO        = #{@monsno}(#{@lavel_monsno})")
    file.puts("FORMNO        = #{@formno}(#{@lavel_formno})")
    file.puts("LEVEL         = #{@level}")
    file.puts("HP_RND        = #{@hp_rnd}")
    file.puts("DF_RND        = #{@df_rnd}")
    file.puts("AG_RND        = #{@ag_rnd}")
    file.puts("SA_RND        = #{@sa_rnd}")
    file.puts("SD_RND        = #{@sd_rnd}")
    file.puts("TOKUSEI       = #{@tokusei}(#{@lavel_tokusei})")
    file.puts("SEIKAKU       = #{@seikaku}(#{@lavel_seikaku})")
    file.puts("SEX           = #{@sex}(#{@lavel_sex})")
    file.puts("ID            = #{@id}")
    file.puts("STYLE         = #{@style}")
    file.puts("BEAUTIFUL     = #{@beautiful}")
    file.puts("CUTE          = #{@cute}")
    file.puts("CLEVER        = #{@clever}")
    file.puts("STRONG        = #{@strong}")
    file.puts("ITEM          = #{@item}(#{@lavel_item})")
    file.puts("OYA_SEX       = #{@oya_sex}(#{@lavel_oya_sex})")
    file.puts("FUR           = #{@fur}")
    file.puts("COUNTRY       = #{@country}(#{@lavel_country})")
    file.puts("CHANGE_MONSNO = #{@change_monsno}(#{@lavel_change_monsno})")
    file.puts("CHANGE_SEX    = #{@change_sex}(#{@lavel_change_sex})")
    file.close
  end
  #------------------------
  # バイナリファイル名取得
  #------------------------
  def binary_filename
    monsname = @lavel_monsno.sub("MONSNO_", "").downcase
    filename = "trade_poke_#{monsname}.bin"
    return filename
  end
  #--------------
  # バイナリ出力
  #--------------
  def binary_out(path)
    file = File::open( path + "/" + self.binary_filename, "wb")
    data = Array::new
    data << @monsno << @formno << @level <<
            @hp_rnd << @at_rnd << @df_rnd << @ag_rnd << @sa_rnd << @sd_rnd <<
            @tokusei << @seikaku << @sex << @id <<
            @style << @beautiful << @cute << @clever << @strong <<
            @item << @oya_sex << @fur << @country <<
            @change_monsno << @change_sex
    file.write(data.pack("I*"))
    file.close
  end
  #-------------------------------
  # 交換データNo.定義ラベル名取得
  #-------------------------------
  def def_lavel
    monsname = @lavel_monsno.sub("MONSNO_", "")
    lavel    = "FLD_TRADE_POKE_#{monsname}"
    return lavel
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
# @param lavel ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetMonsnoVal( lavel )
  #----------------------------------------
	# 参照ファイル読み込み
	file = File::open("monsno_def.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{lavel}.*\(\s*(\d*)\s*\)/) != nil then 
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
		if line.index(/#define.*#{lavel}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: モンスターNo.(#{lavel})は定義されていません。")
end

#-----------------------------------------------------------
# @brief フォームNo.の定義番号を取得する
# @param lavel ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetFormnoVal( lavel )
	# 参照ファイル読み込み
	file = File::open("monsno_def.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{lavel}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
  return 0
end

#-----------------------------------------------------------
# @brief アイテムの定義番号を取得する
# @param lavel ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetItemVal( lavel )
	# 参照ファイル読み込み
	file = File::open("itemsym.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{lavel}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: アイテム(#{lavel})は定義されていません。")
end

#-----------------------------------------------------------
# @brief 性別の定義番号を取得する
# @param lavel ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetSexVal( lavel )
	# 参照ファイル読み込み
	file = File::open("pm_version.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{lavel}\s*(\d*)\s*/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: 性別(#{lavel})は定義されていません。")
end

#-----------------------------------------------------------
# @brief 言語コードの定義番号を取得する
# @param lavel ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetLangCodeVal( lavel )
	# 参照ファイル読み込み
	file = File::open("pm_version.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{lavel}.*\s*(\d*)\s/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: 言語コード(#{lavel})は定義されていません。")
end

#-----------------------------------------------------------
# @brief 特性の定義番号を取得する
# @param lavel ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetTokuseiVal( lavel )
	# 参照ファイル読み込み
	file = File::open("tokusyu_def.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{lavel}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: 特性(#{lavel})は定義されていません。")
end

#-----------------------------------------------------------
# @brief 性格の定義番号を取得する
# @param lavel ラベル名
# @return 指定ラベル名の定義値
#-----------------------------------------------------------
def GetSeikakuVal( lavel )
	# 参照ファイル読み込み
	file = File::open("poke_tool.h", "r");
	file_line = file.readlines
	file.close
	# 検索
	file_line.each do |line|
		if line.index(/#define.*#{lavel}.*\(\s*(\d*)\s*\)/) != nil then 
			return $1.to_i
		end
	end
	# 見つからず
	abort("error: 性格(#{lavel})は定義されていません。")
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
# 全データを取得
column_idx.each do |idx|
  data = OutputData::new
  # 文字列として取得
  data.lavel_monsno        = row_data[ROW_MONSNO       ][idx]
  data.lavel_formno        = row_data[ROW_FORM         ][idx]
  data.level               = row_data[ROW_LEVEL        ][idx]
  data.hp_rnd              = row_data[ROW_HP_RND       ][idx]
  data.at_rnd              = row_data[ROW_AT_RND       ][idx]
  data.df_rnd              = row_data[ROW_DF_RND       ][idx]
  data.ag_rnd              = row_data[ROW_AG_RND       ][idx]
  data.sa_rnd              = row_data[ROW_SA_RND       ][idx]
  data.sd_rnd              = row_data[ROW_SD_RND       ][idx]
  data.lavel_tokusei       = row_data[ROW_TOKUSEI      ][idx]
  data.lavel_seikaku       = row_data[ROW_SEIKAKU      ][idx]
  data.lavel_sex           = row_data[ROW_SEX          ][idx]
  data.id                  = row_data[ROW_ID           ][idx]
  data.style               = row_data[ROW_STYPE        ][idx]
  data.beautiful           = row_data[ROW_BEAUTIFUL    ][idx]
  data.cute                = row_data[ROW_CUTE         ][idx]
  data.clever              = row_data[ROW_CLEVER       ][idx]
  data.strong              = row_data[ROW_STRONG       ][idx]
  data.lavel_item          = row_data[ROW_ITEM         ][idx]
  data.lavel_oya_sex       = row_data[ROW_OYA_SEX      ][idx]
  data.fur                 = row_data[ROW_FUR          ][idx]
  data.lavel_country       = row_data[ROW_COUNTRY      ][idx]
  data.lavel_change_monsno = row_data[ROW_CHANGE_MONSNO][idx]
  data.lavel_change_sex    = row_data[ROW_CHANGE_SEX   ][idx]
  # 値に変換
  data.monsno        = GetMonsnoVal(data.lavel_monsno)
  data.formno        = GetFormnoVal(data.lavel_formno)
  data.level         = data.level.to_i
  data.hp_rnd        = data.hp_rnd.to_i
  data.at_rnd        = data.at_rnd.to_i
  data.df_rnd        = data.df_rnd.to_i
  data.ag_rnd        = data.ag_rnd.to_i
  data.sa_rnd        = data.sa_rnd.to_i
  data.sd_rnd        = data.sd_rnd.to_i
  data.tokusei       = GetTokuseiVal(data.lavel_tokusei)
  data.seikaku       = GetSeikakuVal(data.lavel_seikaku)
  data.sex           = GetSexVal(data.lavel_sex)
  data.id            = data.id.to_i
  data.style         = data.style.to_i
  data.beautiful     = data.beautiful.to_i
  data.cute          = data.cute.to_i
  data.clever        = data.clever.to_i
  data.strong        = data.strong.to_i
  data.item          = GetItemVal(data.lavel_item)
  data.oya_sex       = GetSexVal(data.lavel_oya_sex)
  data.fur           = data.fur.to_i
  data.country       = GetLangCodeVal(data.lavel_country)
  data.change_monsno = GetMonsnoVal(data.lavel_change_monsno)
  data.change_sex    = GetSexVal(data.lavel_change_sex)
  # 配列に登録
  trade_data << data
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
  lavel = data.def_lavel
  file.puts("#define #{lavel} (#{val})")
  val = val + 1
end
file.puts("#define FLD_TRADE_POKE_NUM (#{val})")
file.close
