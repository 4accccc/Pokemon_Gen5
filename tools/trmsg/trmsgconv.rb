#! ruby -Ks

  require File.dirname(__FILE__) + '/../gmm_make/gmm_make'
  require File.dirname(__FILE__) + '/../constant'
  require 'csv'

class PARA
  enum_const_set %w[
    TR_ID
    MSG_KIND
    MSG_JPN
    MSG_KANJI
  ]
end

  #メッセージタイプテーブル
  msg_kind = {
    "フィールド戦闘開始前"                            =>"TRMSG_FIGHT_START",
    "フィールド戦闘前"                                =>"TRMSG_FIGHT_START",
    "フィールド戦闘負け"                              =>"TRMSG_FIGHT_LOSE",
    "フィールド戦闘後"                                =>"TRMSG_FIGHT_AFTER",
    "フィールド戦闘開始前：右"                        =>"TRMSG_FIGHT_START_1",
    "フィールド戦闘負け：右"                          =>"TRMSG_FIGHT_LOSE_1",
    "フィールド戦闘後：右"                            =>"TRMSG_FIGHT_AFTER_1",
    "ポケモン１匹：右"                                =>"TRMSG_POKE_ONE_1",
    "フィールド戦闘開始前：左"                        =>"TRMSG_FIGHT_START_2",
    "フィールド戦闘負け：左"                          =>"TRMSG_FIGHT_LOSE_2",
    "フィールド戦闘後：左"                            =>"TRMSG_FIGHT_AFTER_2",
    "ポケモン１匹：左"                                =>"TRMSG_POKE_ONE_2",
    "昼夜、戦闘ない時"                                =>"TRMSG_FIGHT_NONE_DN",
    "フィールド戦闘開始前：♂"                        =>"TRMSG_FIGHT_START_1",
    "フィールド戦闘負け：♂"                          =>"TRMSG_FIGHT_LOSE_1",
    "フィールド戦闘後：♂"                            =>"TRMSG_FIGHT_AFTER_1",
    "ポケモン１匹：♂"                                =>"TRMSG_POKE_ONE_1",
    "フィールド戦闘開始前：♀"                        =>"TRMSG_FIGHT_START_2",
    "フィールド戦闘負け：♀"                          =>"TRMSG_FIGHT_LOSE_2",
    "フィールド戦闘後：♀"                            =>"TRMSG_FIGHT_AFTER_2",
    "ポケモン１匹：♀"                                =>"TRMSG_POKE_ONE_2",
    "昼、戦闘ない時"                                  =>"TRMSG_FIGHT_NONE_D",
    "フィールド戦闘後：回復前"                        =>"TRMSG_FIGHT_AFTER_RECOVER_BEFORE",
    "フィールド戦闘後：回復後"                        =>"TRMSG_FIGHT_AFTER_RECOVER_AFTER",
    "フィールド戦闘後：アイテムをくれる前"            =>"TRMSG_FIGHT_AFTER_GIFTITEM_BEFORE",
    "フィールド戦闘後：アイテムをくれた後"            =>"TRMSG_FIGHT_AFTER_GIFTITEM_AFTER",
    "戦闘中：相手ポケモンに最初のダメージを与えたとき"=>"TRMSG_FIGHT_FIRST_DAMAGE",
    "戦闘中：相手ポケモンのＨＰ半分以下"              =>"TRMSG_FIGHT_POKE_HP_HALF",
    "戦闘中：相手ポケモンが残り１匹"                  =>"TRMSG_FIGHT_POKE_LAST",
    "戦闘中：相手ポケモンが残り１匹でＨＰ半分以下"    =>"TRMSG_FIGHT_POKE_LAST_HP_HALF",
  }

	if ARGV.size < 2
		print "error: ruby trmsgconv.rb read_file gmm_file\n"
		print "read_file:読み込むファイル\n"
		print "gmm_file:gmmファイルを書き出すための元になるファイル\n"
		exit( 1 )
	end

  ARGV_READ_FILE = 0
  ARGV_READ_GMM_FILE = 1

  read_data = []
  cnt = 0

  CSV.open( ARGV[ ARGV_READ_FILE ], 'r' ) {|row|
    next if row[ 0 ] == nil && row[ 1 ] == nil
    if row[ 0 ] != nil
      next if row[ 0 ].index("#") == 0
      next if row[ 0 ].index("＃") == 0
    end
    read_data[ cnt ] = row
    cnt += 1
  }

  gmm = GMM::new
  gmm.open_gmm( ARGV[ ARGV_READ_GMM_FILE ] , "trmsg.gmm" )

  fp_tbl = open( "trmsgtbl.s", "w" )
  fp_tbl.print( "\t.text\n\n" )
  fp_tbl.print( "\t.include trmsg.h\n\n" )

  tr_id = ""

  read_data.each {|row|
    if row[ PARA::TR_ID ] != nil
      tr_id = row[ PARA::TR_ID ]
    end
    if msg_kind[ row[ PARA::MSG_KIND ] ] == nil
      p "定義されていないメッセージタイプです"
      p row[ PARA::TR_ID ]
      p row[ PARA::MSG_KIND ]
      exit( 1 )
    end
    label = "TRID_" + tr_id.upcase + "_" + msg_kind[ row[ PARA::MSG_KIND ] ]
    if row[ PARA::MSG_JPN ] == nil && row[ PARA::MSG_KANJI ] == nil
      gmm.make_row_kanji( label, "かりの　メッセージ", "仮の　メッセージ" )
    elsif row[ PARA::MSG_JPN ] == nil
      gmm.make_row_kanji( label, "かりの　メッセージ", row[ PARA::MSG_KANJI ] )
    elsif row[ PARA::MSG_KANJI ] == nil
      gmm.make_row_kanji( label, row[ PARA::MSG_JPN ], "仮の　メッセージ" )
    else
      gmm.make_row_kanji( label, row[ PARA::MSG_JPN ], row[ PARA::MSG_KANJI ] )
    end
    fp_tbl.printf( "\t.short\tTRID_%s,\t%s\n", tr_id.upcase, msg_kind[ row[ PARA::MSG_KIND ] ] )
  }

  fp_tbl.close
  gmm.close_gmm

  #タイムスタンプ比較用のダミーファイルを生成
  fp_w = open( "out_end", "w" )
  fp_w.close

