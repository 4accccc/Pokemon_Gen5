#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
# *
# *	@file		ci_effect.rb
# *	@brief    カットインエフェクトデータファイル生成
# *	@author	Masayuki Onoue
# *	@date		2011.06.12
# *
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$KCODE = "SJIS"
load "rbdefine"

#---------------------------------------------------------------
# SEデータクラス
#---------------------------------------------------------------
class SeData
  def initialize(label,frame)
    @frame    = frame
    @label     = label
  end
  attr_accessor :frame, :label
end

#---------------------------------------------------------------
# フェードデータクラス
#---------------------------------------------------------------
class FadeData
  def initialize( frame ,mode ,start_evy ,end_evy ,wait)
    @frame    = frame
    @mode     = mode
    @start_evy= start_evy
    @end_evy  = end_evy
    @wait     = wait
  end
  attr_accessor :frame, :mode, :start_evy, :end_evy, :wait 
end

#---------------------------------------------------------------
# SEコマンド収集クラス
#---------------------------------------------------------------
class SeCmd
  #------------------------------
  # 初期化
  def initialize( id , cnt)
    @id = id
    @list = Array.new
    @cnt = cnt
  end

  #------------------------------
  # データの追加
  def in( data )
    @list << data
  end

  #------------------------------
  # 溜め込まれているデータを全て出力する
  def out(file)
    # ソート
    @list.sort! {|a,b| a.frame.to_i <=> b.frame.to_i}

    # テーブル開始
    file.printf("const CI_SE_TBL Tbl%d[] = {\n",@cnt )
    # 溜め込んでいるデータを出力
    @list.each do |data|
      file.printf("\t{ %s, %d },\n",data.label ,data.frame);
    end

    # 終端文字を出力(データか存在していても書く必要がある)
    file.puts "\t{ CI_SE_NONE, 0 },"
    file.printf("};\n\n")
  end
end

#---------------------------------------------------------------
# FADEコマンド収集クラス
#---------------------------------------------------------------
class FadeCmd
  #------------------------------
  # 初期化
  def initialize( id , cnt)
    @id = id
    @cnt = cnt
    @list = Array.new
  end

  #------------------------------
  # データの追加
  def in( data )
    @list << data
  end

  #------------------------------
  # チェック関数
  def check()
    str = Array.new
    @list.each_with_index do |data,index|
      if index.to_i == (@list.size.to_i-1)
        break
      end

      # フェードがかぶっていないか判断
      if @list[ index ].frame.to_i + @list[ index ].wait.to_i > (@list[ index+1 ].frame.to_i+1)
        str << "frame " + @list[ index ].frame + " のフェードが frame"+@list[ index+1 ].frame+" とフェードと重なっています。\n"
      end
    end

    # 最低値チェック
    @list.each_with_index do |data,index|
      if @list[ index ].frame.to_i <= 0
        str << "frame " + @list[ index ].frame + " は定義できません。\n"
      end
      if (@list[ index ].start_evy.to_i < -16) or (@list[ index ].start_evy.to_i > 16)
        str << "frame " + @list[ index ].frame + " の輝度値が-16～16に収まっていません。\n"
      end
      if (@list[ index ].end_evy.to_i < -16) or (@list[ index ].end_evy.to_i > 16)
        str << "frame " + @list[ index ].frame + " の輝度値が-16～16に収まっていません。\n"
      end
    end

    unless str.size == 0
      puts @id 
      puts str
      $erro_flg = true
    end
  end

  #------------------------------
  # 溜め込まれているデータを全て出力する
  def out(file)

    # ソート
    @list.sort! {|a,b| a.frame.to_i <=> b.frame.to_i}

    # フェードかぶりチェック
    check()

    # テーブル開始
    file.printf("const CI_FADE_TBL FTbl%d[] = {\n",@cnt )
    # 溜め込んでいるデータを出力
    @list.each do |data|
      file.printf("\t{ %d, CI_FADE_MASTER_BRIGHT_%s ,%d ,%d ,%d },\n",data.frame ,data.mode ,data.start_evy ,data.end_evy ,data.wait );
    end

    # 終端文字を出力(データか存在していても書く必要がある)
    file.puts "\t{ CI_FADE_NONE ,0 ,0 ,0 ,0 },"
    file.printf("};\n\n")
  end
end

#---------------------------------------------------------------
# ファイルオープン
#---------------------------------------------------------------
data_csv = File.open( ARGV[0] , "r" );
se_cdat = File.open( ARGV[1] , "w" );
fade_cdat = File.open( ARGV[2] , "w" );
eff_id_hash = File.open( ARGV[3] , "w" );

#---------------------------------------------------------------
# SECDATのヘッダー部分を書き込む
#---------------------------------------------------------------
header =<<EOS
//======================================================================
/**
 * @file	ci_se_tbl.cdat
 * @brief	SEタイミング定義ファイル
 * @note	このファイルはコンバーターから出力されます
 *
 */
//======================================================================

#pragma once

EOS
se_cdat.puts header 

#---------------------------------------------------------------
# FADECDATのヘッダー部分を書き込む
#---------------------------------------------------------------
header =<<EOS
//======================================================================
/**
 * @file	ci_fade_tbl.cdat
 * @brief	輝度変更タイミング定義ファイル
 * @note	このファイルはコンバーターから出力されます
 *
 */
//======================================================================

#pragma once

EOS
fade_cdat.puts header 

#---------------------------------------------------------------
# データの書き込み
#---------------------------------------------------------------
now_id = "" # ID名
id_cnt = 0  # IDの個数
$secmd = nil
$fadecmd = nil

# エラーフラグ
$erro_flg = false

# エフェクトIDテーブル
effect_id_array = Array.new

data_csv.each_with_index do |line,index|

  # 読み込み終了
  if( line =~ /#END/ )
    break
  end

  # 分離
  str = line.split( "," )

  # ==========================
  # 読み飛ばし処理
  if index < RBDEF_NOGET_ROW_NUM-1
    next
  end

  # ==========================
  # 現在のIDを取得
  unless str[RBDEF_NUM_ROW_ID] == "" then

    # 初回以外は記述する
    unless $secmd == nil
      $secmd.out(se_cdat)
      $fadecmd.out(fade_cdat)
    end

    # 新たなデータを生成
    now_id = str[RBDEF_NUM_ROW_ID]
    $secmd = SeCmd.new( now_id , id_cnt )
    $fadecmd = FadeCmd.new( now_id , id_cnt )
    id_cnt += 1

    # EFFECTIDを保存する
    effect_id_array << now_id
  end

  # コマンド列にSEコマンドが含まれているか
  if( str[RBDEF_NUM_ROW_CMD] =~ /#{RBDEF_CMD_SE}/ )
    sedata = SeData.new( str[RBDEF_NUM_ROW_SELABEL] , str[RBDEF_NUM_ROW_FRAME] )
    $secmd.in(sedata)
  end

  # コマンド列にFADEコマンドが含まれているか
  if( (str[RBDEF_NUM_ROW_CMD] =~ /#{RBDEF_CMD_BLACK_FADE}/) or (str[RBDEF_NUM_ROW_CMD] =~ /#{RBDEF_CMD_WHITE_FADE}/) )
    fadedata = FadeData.new( str[RBDEF_NUM_ROW_FRAME] , str[RBDEF_NUM_ROW_CMD], str[RBDEF_NUM_ROW_START_EVY], str[RBDEF_NUM_ROW_END_EVY], str[RBDEF_NUM_ROW_WAIT]  )
    $fadecmd.in(fadedata)
  end
end

# 最後のデータを書き込む
$secmd.out(se_cdat)
$fadecmd.out(fade_cdat)

#---------------------------------------------------------------
# SECDATのフッター部分を書き込む
#---------------------------------------------------------------
se_cdat.printf("#define CI_SE_TBL_MAX  (%d)\n",id_cnt)
se_cdat.printf("const CI_SE_TBL * const SeTbl[ CI_SE_TBL_MAX ] = {\n")

for i in 0..(id_cnt-1)
  se_cdat.printf("\tTbl%d,\n",i)
end
se_cdat.printf("};\n")

#---------------------------------------------------------------
# FADECDATのフッター部分を書き込む
#---------------------------------------------------------------
fade_cdat.printf("#define CI_FADE_TBL_MAX  (%d)\n",id_cnt)
fade_cdat.printf("const CI_FADE_TBL * const FadeTbl[ CI_FADE_TBL_MAX ] = {\n")

for i in 0..(id_cnt-1)
  fade_cdat.printf("\tFTbl%d,\n",i)
end
fade_cdat.printf("};\n")

#---------------------------------------------------------------
# EFFECTIDのハッシュテーブルを作成する
#---------------------------------------------------------------
eff_id_hash.puts "#! ruby -Ks"
eff_id_hash.puts "#フィールド3Dカットイン用エフェクトIDテーブル"
eff_id_hash.puts "$effect_id_hash = {"
	
effect_id_array.each_with_index{|data,i|
	eff_id_hash.puts '  "'+data+'" => '+i.to_s+','
}
eff_id_hash.puts "}"

#---------------------------------------------------------------
# ファイルクローズ
#---------------------------------------------------------------
data_csv.close
se_cdat.close
fade_cdat.close
eff_id_hash.close

if $erro_flg == true
  File.delete(ARGV[0])
  File.delete(ARGV[1])
  File.delete(ARGV[2])
  File.delete(ARGV[3])
  abort("定義値に誤りがあります。\n\\resource\\fld3d_ci_effect\\fld3d_ci_eff.xlsを確認してください\n")
end

