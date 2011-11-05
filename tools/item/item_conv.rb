#! ruby -Ks
# アイテムコンバート
# 現状はハッシュテーブルを生成するだけ
#

  require File.dirname(__FILE__) + '/../label_make/label_make'
  require File.dirname(__FILE__) + '/../gmm_make/gmm_make'
  require File.dirname(__FILE__) + '/../constant'

  # sort_remover.rbを通るとソート２，３が削られる
class PARA
  enum_const_set %w[
    NO
    KIND
    NAME
    KANZI
    ICON_C
    ICON_P
    PRICE
    EQ_EFFECT
    POWER
    POWER_HELP
    THROW_POWER
    THROW_EFFECT
    SHIZENNOMEGUMI_POWER
    SHIZENNOMEGUMI_TYPE
    TSUIBAMU_EFFECT
    INFO_KANA_1
    INFO_KANA_2
    INFO_KANA_3
    INFO_KANZI_1
    INFO_KANZI_2
    INFO_KANZI_3
    MULTI
    WAZA_MACHINE
    IMPORTANT
    BENRI
    SORTNO_NORMAL
    POCKET_TYPE
  ]
end

	if ARGV.size < 3
		print "error: ruby item_conv.rb read_file gmm_file\n"
		print "read_file:読み込むファイル\n"
		print "gmm_file:特性名のgmmファイルを書き出すための元になるファイル\n"
		print "pocket_file:ポケット番号を出力するファイル名\n"
		exit( 1 )
	end

  ARGV_READ_FILE = 0
  ARGV_READ_GMM_FILE = 1
  ARGV_POCKET_NO_FILE = 2

  label = LabelMake.new
  read_data = []
  cnt = -1
  open( ARGV[ ARGV_READ_FILE ] ) {|fp_r|
    while str = fp_r.gets
      if cnt == -1
        cnt += 1
        next
      end
      str = str.tosjis
      str = str.tr( "\"\r\n", "" )
      split_data = str.split(/,/)
      next if split_data.size <= 1       #サーバからのエクスポートでゴミレコードが入ることがあるので、排除する
      read_data[ cnt ] = str
      cnt += 1
    end
  }

=begin  
  tokuno = Hash::new
  tokuname = []
  #アイテムラベル＆gmmファイル生成
  print "アイテムラベル＆gmmファイル　生成中\n"
  fp_tokuno = open( "tokusyu_def.h", "w" )

  fp_tokuno.print( "//============================================================================================\n" )
  fp_tokuno.print( "/**\n" )
  fp_tokuno.print( " * @file	tokusyu_def.h\n" )
  fp_tokuno.print( " * @bfief	特性NoのDefine定義ファイル\n" )
  fp_tokuno.print( " * @author	TokuseiConverter\n" )
  fp_tokuno.print( " * 特性コンバータから生成されました\n" )
  fp_tokuno.print( "*/\n")
  fp_tokuno.print( "//============================================================================================\n" )
  fp_tokuno.print( "\n#pragma once\n\n" )
  fp_tokuno.printf( "#define\t\tTOKUSYU_NULL\t\t\t\t\t\t( 0 )\n" )
=end  
  #ハッシュ
  fp_hash = open( "item_hash.rb", "w" )
  fp_hash.printf("#! ruby -Ks\n\n" )
  fp_hash.printf("\t$item_hash = {\n" )
  fp_hash.printf("\t\t\"－－－－－－－－\"=>0,\n" )

=begin
  #GMM
  tokuname_gmm = GMM::new
  tokuname_gmm.open_gmm( ARGV[ ARGV_READ_GMM_FILE ] , "tokusei.gmm" )
  tokuname_gmm.make_row_index( "TOKUSEI_", 0, "ー" )
  tokuinfo_gmm = GMM::new
  tokuinfo_gmm.open_gmm( ARGV[ ARGV_READ_GMM_FILE ] , "tokuseiinfo.gmm" )
  tokuinfo_gmm.make_row_index( "TOKUSEIINFO_", 0, "ー\rー\r" )
=end
  cnt = 1

  read_data.size.times {|i|
    split_data = read_data[ i ].split(/,/)
=begin
    fp_tokuno.print( "#define\t\t" )
    label_str = label.make_label( "TOKUSYU_", split_data[ PARA::NAME ] )
    fp_tokuno.print( label_str )
    tab_cnt = ( 25 - label_str.length ) / 2 + 2
    for j in 1..tab_cnt
      fp_tokuno.print( "\t" )
    end
    fp_tokuno.printf( "( %d )\t\t//%s\n", cnt, split_data[ PARA::NAME ] )
=end
    fp_hash.printf("\t\t\"%s\"=>%d,\n", split_data[ PARA::NAME ], split_data[ PARA::NO ].to_i )
=begin
    tokuname_gmm.make_row_index( "TOKUSEI_", cnt, split_data[ PARA::NAME ] )
    if split_data[ PARA::INFO1 ] == nil
      info1 = ""
    else
      info1 = split_data[ PARA::INFO1 ]
    end
    if split_data[ PARA::INFO2 ] == nil
      info2 = ""
    else
      info2 = split_data[ PARA::INFO2 ]
    end
    info = info1 + "\r\n" + info2
    #@todo 漢字説明文ありなんですが、現状データがないので、同じ文字列で生成
    tokuinfo_gmm.make_row_index_kanji( "TOKUSEIINFO_", cnt, info, info )
=end
    cnt += 1
  }

=begin
  fp_tokuno.printf( "#define\t\tTOKUSYU_MAX\t\t\t\t\t\t( %d )\n", cnt - 1 )
  fp_tokuno.close
=end
  fp_hash.printf("\t}\n" )

  fp_hash.close
=begin
  tokuname_gmm.close_gmm
  tokuinfo_gmm.close_gmm
  print "アイテムラベル＆gmmファイル　生成終了\n"
=end

# タイプ名からポケットタイプへ変換するテーブル
pocket_no_hash = {
  "NORMAL"=>"BAG_POKE_NORMAL", # 道具
  "DRUG"=>"BAG_POKE_DRUG", # 薬
  "WAZA"=>"BAG_POKE_WAZA", # 
  "NUTS"=>"BAG_POKE_NUTS", # 木の実
  "EVENT"=>"BAG_POKE_EVENT", # 大切なもの
  "0"=>"BAG_POKE_NORMAL", # 道具
}

#define	BAG_POKE_DRUG		    ( 1 )		// 薬
#define	BAG_POKE_WAZA		    ( 2 )		// 技マシン
#define	BAG_POKE_NUTS		    ( 3 )		// 木の実
#define	BAG_POKE_EVENT		  ( 4 )		// 大切な物
#define	BAG_POKE_REPOSITORY	( 5 )		// 保管

header = <<-EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  アイテム番号からポケット番号を引くための配列
 *	@author	Masayuki Onoue
 *	@date		2011.11.02
 *  @note   \\tools\\item\\item_conv.rbが作成したデータです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

static const u8 itemid_to_pocketno[] =
{
EOS

fname = ARGV[ ARGV_POCKET_NO_FILE ]
fp_pocket = open( fname , "w" )
fp_pocket.printf( header%[ fname ] )

# ダミーの登録
fp_pocket.printf("  %s,  // ダミー(0)\n",pocket_type = pocket_no_hash["0"] )

read_data.size.times {|i|
  split_data = read_data[ i ].split(/,/)

  p_type = split_data[ PARA::POCKET_TYPE ]
  if pocket_no_hash.key?(p_type) then
    # 未登録タイプ(0)の場合は道具を指定
    pocket_type = pocket_no_hash[p_type]
  else
    raise "エラー：未登録のfield保存先「%s」が使用されています!\n"%[p_type]
  end

  fp_pocket.printf("  %s,  // %s(%d)\n",pocket_type , split_data[ PARA::NAME ], split_data[ PARA::NO ].to_i )
}

fp_pocket.printf( "};\n\n" )
fp_pocket.close


