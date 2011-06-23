
$KCODE = "SJIS" #文字コードをSJISに設定。
require File.dirname(__FILE__) + '/data_hash.rb'
require "csv"

#======================
# テクスチャデータクラス
#======================
class TEX_DATA
  # サクセサ
  attr_accessor :id, :model_id, :model_index ,:trans_tex_name ,:trans_pal_name , :ncg_name ,:ncl_name ,:cut_x, :cut_y, :cut_w, :cut_h ,:pal_id ,:is_dummy ,:is_nocut ,:ncg_dir ,:ncl_dir ,:line_num ,:cut_data_str ,:ncgr_name ,:nclr_name ,:bin_name ,:player_trans_type ,:ncg_w

  #======================
  # プライベートデータ
  #======================
  private
  # プレイヤー転送情報(PLAYER_TRANS_TYPEとあわせること)
  DUMMY_NCG_NAME_ARRAY = 
  {
    "PLAYER_NCG" =>1,
    "PLAYER_UP_NCG" => 2,
    "PLAYER_DOWN_NCG" => 2,
  }
  DUMMY_NCL_NAME_ARRAY = 
  {
    "PLAYER_NCL" =>1,
    "PLAYER_UP_NCL" => 2,
    "PLAYER_DOWN_NCL" => 2,
  }
  MODEL_ID = 
  {
    "ModelA" =>0,
    "ModelB" =>1,
  }

  def DataCheck
    unless MODEL_ID.key?(@model_id) then 
      raise "テクスチャデータ(%d行目)->未定義のモデルIDが使用されています[%s]"%[@line_num,@model_id]
    end
    unless $cutin_ncg_hash.key?(ncg_name) then
      unless DUMMY_NCG_NAME_ARRAY.key?(ncg_name) then 
        raise "テクスチャデータ(%d行目)->NCGファイルが見つかりません[%s]"%[@line_num,@ncg_name]
      end
    end
    unless $cutin_ncl_hash.key?(ncl_name) then
      unless DUMMY_NCL_NAME_ARRAY.key?(ncl_name) then 
        raise "テクスチャデータ(%d行目)->NCLファイルが見つかりません[%s]"%[@line_num,@ncl_name]
      end
    end
  end

  #======================
  # パブリックデータ
  #======================
  public
  def initialize( id ,data ,line_num )
    @id = id
    @line_num = line_num+1
    @model_id        = data[1]
    @trans_tex_name  = data[2]
    @trans_pal_name  = data[3]
    @ncg_name        = data[4]
    @ncg_w           = data[5].to_i
    @ncl_name        = data[6]
    @cut_x           = data[7].to_i
    @cut_y           = data[8].to_i
    @cut_w           = data[9].to_i
    @cut_h           = data[10].to_i
    @pal_id          = data[11].to_i
    @ncgr_name       = @ncg_name.gsub(/\.ncg/,".NCGR")
    @nclr_name       = @ncl_name.gsub(/\.ncl/,".NCLR")
    @cut_data_str       = ""
    @player_trans_type  = 0
    # ダミーファイルを使用するか調べる
    @is_dummy = FALSE
    if DUMMY_NCG_NAME_ARRAY.key?(@ncg_name) then 
      @is_dummy = TRUE
    elsif DUMMY_NCL_NAME_ARRAY.key?(@ncl_name) then 
      @is_dummy = TRUE
    end
    
    # プレイヤーの転送情報を取得
    if @is_dummy == TRUE then
      @player_trans_type     = DUMMY_NCG_NAME_ARRAY[@ncg_name]
    end

    # 切り出しを行わない
    @is_nocut = FALSE
    if @cut_w == 0 then
      @is_nocut = TRUE
    elsif @cut_h == 0 then
      @is_nocut = TRUE
    end

    # サフィックスの設定
    unless @is_nocut then
      @cut_data_str = @cut_x.to_s+"_"+@cut_y.to_s+"_"+@cut_w.to_s+"_"+@cut_h.to_s
      @ncgr_name = @ncgr_name.gsub(/\./,"_"+cut_data_str+".")
    end

    # binファイル名を設定
    @bin_name = @ncgr_name.gsub(/\.NCGR/,".bin")
    @bin_name.gsub!(/\./,"_LZ.")  # 圧縮プレフィックス追加
    DataCheck()

    # モデルのインデックスを格納
    @model_index = MODEL_ID[@model_id]

    # 取得ディレクトリを調べる
    @ncg_dir = $cutin_ncg_hash[@ncg_name]
    @ncl_dir = $cutin_ncl_hash[@ncl_name]
  end


end
# ========================================================
# メイン
# ========================================================
TARGET_FILE = ARGV[0]
DEPEND_FILE = ARGV[1]
DATA_FILE = ARGV[2]
ARC_LIST_FILE = ARGV[3]
DATA_ARC_LIST_FILE = ARGV[4]
target_file = open(TARGET_FILE,"w")
depend_file = open(DEPEND_FILE,"w")
arc_list_file = open(ARC_LIST_FILE,"w")
data_arc_list_file = open(DATA_ARC_LIST_FILE,"w")
data_file = open(DATA_FILE,"r")

target_file.puts "TARGET_LIST = \\"

id_tmp = ""
tex_data_arry = Array.new
data_size = 0
# データの取得
data_file.each_with_index{|line,index|
  # 1行目はヘッダ
  if index == 0 then next end

  # データの取得
  data = line.split(/,/)
  if data[0].include?("#END") then break end
  if data[0] != "" then
    #IDのテスト
    unless $cutin_id_array.include?(data[0]) then
      puts data[0]
      raise "テクスチャデータ->カットインデータにIDが定義されていません"
    end
    id_tmp = data[0]
  end

  tex_data = TEX_DATA.new( id_tmp ,data ,index )
  tex_data_arry << tex_data
  data_size = index
}
# =============================
# 主人公用男女顔グラフィック
# =============================
# ヒーロー一枚転送
data = ["DUMMY","ModelA","dummy","dummy","enc_hero.ncg",128,"enc_hero.ncl",0,0,0,0,0]
tex_data = TEX_DATA.new( "DUMMY" ,data ,data_size )
tex_data_arry << tex_data
data_size+=1

# ヒーロー上
data = ["DUMMY","ModelA","dummy","dummy","enc_hero_up_down.ncg",128,"enc_hero_up_down.ncl",0,0,16,16,0]
tex_data = TEX_DATA.new( "DUMMY" ,data ,data_size )
tex_data_arry << tex_data
data_size+=1

# ヒーロー下
data = ["DUMMY","ModelA","dummy","dummy","enc_hero_up_down.ncg",128,"enc_hero_up_down.ncl",0,16,16,16,0]
tex_data = TEX_DATA.new( "DUMMY" ,data ,data_size )
tex_data_arry << tex_data
data_size+=1

# ヒロイン一枚転送
data = ["DUMMY","ModelA","dummy","dummy","enc_heroine.ncg",128,"enc_heroine.ncl",0,0,0,0,0]
tex_data = TEX_DATA.new( "DUMMY" ,data ,data_size )
tex_data_arry << tex_data
data_size+=1

# ヒロイン上
data = ["DUMMY","ModelA","dummy","dummy","enc_heroine_up_down.ncg",128,"enc_heroine_up_down.ncl",0,0,16,16,0]
tex_data = TEX_DATA.new( "DUMMY" ,data ,data_size )
tex_data_arry << tex_data
data_size+=1

# ヒロイン下
data = ["DUMMY","ModelA","dummy","dummy","enc_heroine_up_down.ncg",128,"enc_heroine_up_down.ncl",0,16,16,16,0]
tex_data = TEX_DATA.new( "DUMMY" ,data ,data_size )
tex_data_arry << tex_data
data_size+=1

# 依存関係の記述
bin_depend = <<-EOS
cnv_tmp\\%s : cnv_tmp\\%s
\tntrcomp -l 2 cnv_tmp\\%s

EOS

ncg_depend = <<-EOS
cnv_tmp\\%s : %s
\t@g2dcvtr $< -bg -o/cnv_tmp

EOS

# リネーム・切り取りをする場合
ncg_rename_depend = <<-EOS
cnv_tmp\\%s : %s
\t@g2dcvtr $< -bg -o/cnv_tmp -cr/%s
\tmv  cnv_tmp\\%s cnv_tmp\\%s

EOS

#ncgファイル列挙
binname_array = Array.new
ncgrname_array = Array.new
nclrname_array = Array.new
tex_data_arry.each{|target|

  # ダミーは登録しない
  unless target.is_dummy then
    ncg    = target.ncg_name
    ncg_dir= target.ncg_dir
    ncgr   = target.ncgr_name
    ncl    = target.ncl_name
    ncl_dir= target.ncl_dir
    nclr   = target.nclr_name
    bin    = target.bin_name

    # すでに同じデータが登録されている場合は実行しない
    # NCGR
    if ncgrname_array.index( ncgr ) == nil then
      ncgrname_array << ncgr

      # 依存関係を書き込み
      if target.cut_data_str == "" then
        depend_file.puts ncg_depend%[ncgr,ncg_dir+ncg]
      else
        cut_str = target.cut_data_str
        make_file_name = ncg.gsub(/ncg/,"NCGR")# 生成後のファイル名
        depend_file.puts ncg_rename_depend%[ncgr,ncg_dir+ncg,cut_str,make_file_name,ncgr]
      end

      # binファイル
      binname_array << "cnv_tmp\\"+bin
      target_file.puts "\tcnv_tmp\\"+bin+" \\"
      depend_file.puts bin_depend%[bin,ncgr,ncgr]
    end

    # NCLR
    if nclrname_array.index( "cnv_tmp\\"+nclr ) == nil then
      
      # ターゲットファイルへの書き込み
      target_file.puts "\tcnv_tmp\\"+nclr+" \\"
      
      # 依存関係を書き込み
      nclrname_array << "cnv_tmp\\"+nclr
      depend_file.puts ncg_depend%[nclr,ncl_dir+ncl]
    end

  else # ダミーファイルの登録
  end
}

PLAYER_RES_DIR = "replace_tex/VSdemo_Hero_Heroine/"

### ヒーロー
#ncg = "enc_hero.ncg"
#ncgr = "enc_hero.NCGR"
#ncl = "enc_hero.ncl"
#nclr = "enc_hero.NCLR"
#bin = "enc_hero_LZ.bin"
#ncg_dir = "replace_tex/VSdemo_Hero_Heroine/"
#ncl_dir = "replace_tex/VSdemo_Hero_Heroine/"
#ncgrname_array << ncgr
#nclrname_array << "cnv_tmp\\"+nclr
#binname_array << "cnv_tmp\\"+bin
#depend_file.puts ncg_depend%[nclr,ncl_dir+ncl]
#depend_file.puts ncg_depend%[ncgr,ncg_dir+ncg]
#depend_file.puts bin_depend%[bin,ncgr,ncgr]
#target_file.puts "\tcnv_tmp\\"+bin+" \\"
#target_file.puts "\tcnv_tmp\\"+nclr+" \\"
#
#ncg = "enc_hero_up_down.ncg"
#ncgr = "enc_hero_up.NCGR"
#ncl = "enc_hero_up_down.ncl"
#nclr = "enc_hero_up_down.NCLR"
#bin = "enc_hero_up_LZ.bin"
#ncg_dir = "replace_tex/VSdemo_Hero_Heroine/"
#ncl_dir = "replace_tex/VSdemo_Hero_Heroine/"
#cut_str = "0_0_16_16"
#ncgrname_array << ncgr
#nclrname_array << "cnv_tmp\\"+nclr
#binname_array << "cnv_tmp\\"+bin
#depend_file.puts ncg_depend%[nclr,ncl_dir+ncl]
#make_file_name = ncg.gsub(/ncg/,"NCGR")# 生成後のファイル名
#depend_file.puts ncg_rename_depend%[ncgr,ncg_dir+ncg,cut_str,make_file_name,ncgr]
#depend_file.puts bin_depend%[bin,ncgr,ncgr]
#target_file.puts "\tcnv_tmp\\"+bin+" \\"
#target_file.puts "\tcnv_tmp\\"+nclr+" \\"

# アーカイブに格納するものリストを作成
arc_list_hash = Hash.new
binname_array.each{|bin_name|
  arc_list_file.puts '"'+bin_name+'"'
  arc_list_hash[ bin_name ] = arc_list_hash.size
}
nclrname_array.each{|nclr_name|
  arc_list_file.puts '"'+nclr_name+'"'
  arc_list_hash[ nclr_name ] = arc_list_hash.size
}

# 読み込み時に必要なデータアーカイブを作成する
$cutin_id_array.each{|id|
  # IDごとに必要なデータをあつめる
  trans_cnt = 0
  player_trans_type = 0
  tex_data_arry.each{|tex_data|
    unless tex_data.is_dummy then
      if tex_data.id == id then
        trans_cnt+=1
      end
    else
      # ダミーテクスチャ判定
      if tex_data.id == id then
        if tex_data.player_trans_type != 0 then
          player_trans_type = tex_data.player_trans_type
        end
      end
    end
  }

  # 全てのデータをバイナリデータにまとめる

  id_str = id.downcase
  file_name = "./cnv_tmp/"+id_str +"_tex_data.bin"
  bin_data = open(file_name,"wb")
  bin_data << [ trans_cnt ].pack( "C" ) # 転送数
  bin_data << [ player_trans_type ].pack( "C" ) # プレイヤー転送タイプ
  bin_data << [ 0,0 ].pack( "C2" )      # パディング
  tex_data_arry.each{|tex_data|
    if tex_data.id == id then
      unless tex_data.is_dummy then
        bin_data << [ tex_data.trans_tex_name+"\0" ].pack( "A20" ) # テクスチャ名
        bin_data << [ tex_data.trans_pal_name+"\0" ].pack( "A20" ) # パレット名
        bin_data << [ tex_data.model_index ].pack( "C" )      # 転送するモデル
        bin_index = arc_list_hash[ "cnv_tmp\\"+tex_data.bin_name ]
        bin_data << [ bin_index ].pack( "C" )                         # BINインデックス
        nclr_index = arc_list_hash[ "cnv_tmp\\"+tex_data.nclr_name ]
        bin_data << [ nclr_index ].pack( "C" )                         # NCLインデックス
        bin_data << [ tex_data.pal_id ].pack( "C" )           # パレットデータ
        bin_data << [ tex_data.cut_w.to_i ].pack( "C" )           # 切り取る幅
        bin_data << [ tex_data.cut_h.to_i ].pack( "C" )           # 切り取る高さ
        bin_data << [ tex_data.ncg_w.to_i ].pack( "C" )           # NCGの幅
        bin_data << [ 0 ].pack( "C" )      # パディング
      end
    end
  }
  bin_data.close
  data_arc_list_file.puts '"'+file_name+'"'
}

data_arc_list_file.close
arc_list_file.close
target_file.close
depend_file.close
data_file.close
