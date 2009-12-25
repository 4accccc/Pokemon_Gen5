#! ruby -Ks

  require File.dirname(__FILE__) + '/../label_make/label_make'
  require File.dirname(__FILE__) + '/../gmm_make/gmm_make'
  require File.dirname(__FILE__) + '/../constant'

class PARA
  enum_const_set %w[
    WAZANAME
    ATKMSG
    INFO1
    INFO2
    INFO3
    INFO4
    INFO5
    Type
    Category
    DamageType
    Power
    HitPer
    BasePP
    Priority
    HitCntMax
    HitCntMin
    SickID
    SickPer
    SickCont
    SickTurnMin
    SickTurnMax
    CriticalRank
    ShrinkPer
    RankEffType1
    RankEffType2
    RankEffType3
    RankEffValue1
    RankEffValue2
    RankEffValue3
    RankEffPer1
    RankEffPer2
    RankEffPer3
    DmgRecoverRatio
    HPRecoverRatio
    Target
    FLG_Touch
    FLG_Tame
    FLG_Tire
    FLG_Mamoru
    FLG_MagicCoat
    FLG_Yokodori
    FLG_Oumu
    FLG_TetsuNoKobusi
    FLG_Bouon
    FLG_Juryoku
    FLG_KoriMelt
    FLG_TripleFar
  ]
end

  atkmsg_table = {
    "をつかった！"=>"を　つかった！",
    "した！"=>"した！",
    "をした！"=>"を　した！",
    "こうげき！"=>"　こうげき！",
    "！"=>"！",
  }

  minemsg_table = {
    "をつかった！"  =>"[1:01:ニックネーム:0]は\r\n",
    "した！"        =>"[1:01:ニックネーム:0]は\r\n",
    "をした！"      =>"[1:01:ニックネーム:0]は\r\n",
    "こうげき！"    =>"[1:01:ニックネーム:0]の\r\n",
    "！"            =>"[1:01:ニックネーム:0]の\r\n",
  }


	if ARGV.size < 2
		print "error: ruby waza_tbl_conv.rb read_file gmm_file\n"
		print "read_file:読み込むファイル\n"
		print "gmm_file:ポケモン名のgmmファイルを書き出すための元になるファイル\n"
		exit( 1 )
	end

  ARGV_READ_FILE = 0
  ARGV_READ_GMM_FILE = 1

  label = LabelMake.new
  read_data = []
  cnt = 0
  open( ARGV[ ARGV_READ_FILE ] ) {|fp_r|
    while str = fp_r.gets
      str = str.tosjis
      str = str.tr( "\"\r\n", "" )
      split_data = str.split(/\t/)
      next if split_data.size <= 1       #サーバからのエクスポートでゴミレコードが入ることがあるので、排除する
      read_data[ cnt ] = str
      cnt += 1
    end
  }

  wazano = Hash::new
  wazaname = []
  
  #技ラベル＆gmmファイル生成
  print "技ラベル＆gmmファイル　生成中\n"
  fp_wazano = open( "wazano_def.h", "w" )

  fp_wazano.print( "//============================================================================================\n" )
  fp_wazano.print( "/**\n" )
  fp_wazano.print( " * @file	wazano_def.h\n" )
  fp_wazano.print( " * @bfief	技NoのDefine定義ファイル\n" )
  fp_wazano.print( " * @author	WazaTableConverter\n" )
  fp_wazano.print( " * 技テーブルコンバータから生成されました\n" )
  fp_wazano.print( "*/\n")
  fp_wazano.print( "//============================================================================================\n" )
  fp_wazano.print( "\n#pragma once\n\n" )
  fp_wazano.printf( "#define\t\tWAZANO_NULL\t\t\t\t\t\t( 0 )\n" )

  #ハッシュ
  fp_hash = open( "wazano_hash.rb", "w" )
  fp_hash.printf("#! ruby -Ks\n\n" )
  fp_hash.printf("\t$wazano_hash = {\n" )

  #GMM
  wazaname_gmm = GMM::new
  wazaname_gmm.open_gmm( ARGV[ ARGV_READ_GMM_FILE ] , "wazaname.gmm" )
  wazaname_gmm.make_row_index( "WAZANAME_", 0, "ーーーーー" )
  wazainfo_gmm = GMM::new
  wazainfo_gmm.open_gmm( ARGV[ ARGV_READ_GMM_FILE ] , "wazainfo.gmm" )
  wazainfo_gmm.make_row_index_kanji( "WAZAINFO_", 0, "ー\rー\rー\rー\rー", "ー\rー\rー\rー\rー" )
  atkmsg_gmm = GMM::new
  atkmsg_gmm.open_gmm( ARGV[ ARGV_READ_GMM_FILE ] , "btl_attack.gmm" )
  atkmsg_gmm.make_row_index( "ATKMSG_M_", 0, "！" )
  atkmsg_gmm.make_row_index( "ATKMSG_Y_", 0, "！" )
  atkmsg_gmm.make_row_index( "ATKMSG_E_", 0, "！" )

  #タメ技リスト
  tame_waza = []

  cnt = 1

  read_data.size.times {|i|
    split_data = read_data[ i ].split(/\t/)
    if split_data[ PARA::FLG_Tame ] == "1"
      tame_waza << cnt
    end
    fp_wazano.print( "#define\t\t" )
    label_str = label.make_label( "WAZANO_", split_data[ PARA::WAZANAME ] )
    fp_wazano.print( label_str )
    tab_cnt = ( 19 - label_str.length ) / 2 + 2
    for j in 1..tab_cnt
      fp_wazano.print( "\t" )
    end
    fp_wazano.printf( "( %d )\t\t//%s\n", cnt, split_data[ PARA::WAZANAME ] )
    fp_hash.printf("\t\t\"%s\"=>%d,\n", split_data[ PARA::WAZANAME ], cnt )
    wazaname_gmm.make_row_index( "WAZANAME_", cnt, split_data[ PARA::WAZANAME ] )
    info = split_data[ PARA::INFO1 ] + "\r\n" + split_data[ PARA::INFO2 ] + "\r\n" + split_data[ PARA::INFO3 ] + "\r\n" + split_data[ PARA::INFO4 ] + "\r\n" + split_data[ PARA::INFO5 ]
    #@todo 漢字説明文ありなんですが、現状データがないので、同じ文字列で生成
    wazainfo_gmm.make_row_index_kanji( "WAZAINFO_", cnt, info, info )
    if atkmsg_table[ split_data[ PARA::ATKMSG ] ] == nil
      p split_data[ PARA::ATKMSG ]
    end
    atkmsg = minemsg_table[ split_data[ PARA::ATKMSG ] ] + split_data[ PARA::WAZANAME ] + atkmsg_table[ split_data[ PARA::ATKMSG ] ]
    atkmsg_gmm.make_row_index( "ATKMSG_M_", cnt, atkmsg )
    atkmsg = "やせいの　" + minemsg_table[ split_data[ PARA::ATKMSG ] ] + split_data[ PARA::WAZANAME ] + atkmsg_table[ split_data[ PARA::ATKMSG ] ]
    atkmsg_gmm.make_row_index( "ATKMSG_Y_", cnt, atkmsg )
    atkmsg = "あいての　" + minemsg_table[ split_data[ PARA::ATKMSG ] ] + split_data[ PARA::WAZANAME ] + atkmsg_table[ split_data[ PARA::ATKMSG ] ]
    atkmsg_gmm.make_row_index( "ATKMSG_E_", cnt, atkmsg )
    cnt += 1
  }

  fp_wazano.printf( "#define\t\tWAZANO_MAX\t\t\t\t\t\t( %d )\n", cnt )
  fp_wazano.close

  fp_hash.printf("\t}\n" )

  fp_hash.print("\n\t$tame_waza = [\n")

  tame_waza.size.times{ |cnt|
    fp_hash.printf("\t\t%d,\n",tame_waza[ cnt ])
  }
  fp_hash.printf("\t]\n" )

  fp_hash.close

  wazaname_gmm.close_gmm
  wazainfo_gmm.close_gmm
  atkmsg_gmm.close_gmm

  print "技ラベル＆gmmファイル　生成終了\n"

