#! ruby -Ks

  require File.dirname(__FILE__) + '/../label_make/label_make'
  require File.dirname(__FILE__) + '/../gmm_make/gmm_make'
  require File.dirname(__FILE__) + '/../constant'
  require File.dirname(__FILE__) + '/../hash/monsno_hash.rb'
  require File.dirname(__FILE__) + '/../hash/wazano_hash.rb'
  require File.dirname(__FILE__) + '/../hash/item_hash.rb'
  require File.dirname(__FILE__) + '/../hash/tokusei_hash.rb'
  require 'date'

=begin
	u8			data_type;        //データタイプ
	u8			tr_type;					//トレーナー分類
  u8      fight_type;       //1vs1or2vs2 
	u8			poke_count;       //所持ポケモン数

	u16			use_item[4];      //使用道具

	u32			aibit;						//AIパターン

//トレーナー持ちポケモンパラメータ（データタイプノーマル）
	u16		pow;			//セットするパワー乱数(u8でOKだけど4バイト境界対策）
	u16		level;			//セットするポケモンのレベル
	u16		monsno;			//セットするポケモン

//トレーナー持ちポケモンパラメータ（データタイプ技）
	u16		pow;			//セットするパワー乱数(u8でOKだけど4バイト境界対策）
	u16		level;			//セットするポケモンのレベル
	u16		monsno;			//セットするポケモン
	u16		waza[4];		//持ってる技

//トレーナー持ちポケモンパラメータ（データタイプアイテム）
	u16		pow;			//セットするパワー乱数(u8でOKだけど4バイト境界対策）
	u16		level;			//セットするポケモンのレベル
	u16		monsno;			//セットするポケモン
	u16		itemno;			//セットするアイテム

//トレーナー持ちポケモンパラメータ（データタイプマルチ）
	u16		pow;			//セットするパワー乱数(u8でOKだけど4バイト境界対策）
	u16		level;			//セットするポケモンのレベル
	u16		monsno;			//セットするポケモン
	u16		itemno;			//セットするアイテム
	u16		waza[ PTL_WAZA_MAX ];		//持ってる技
=end

class PARA
  enum_const_set %w[
    FIGHT_TYPE
    DATA_TYPE
    GOLD
    GENDER
    TR_ID
    TR_TYPE
    TR_NAME
    POKE1_POW
    POKE1_LV
    POKE1_GENDER
    POKE1_FORM
    POKE2_POW
    POKE2_LV
    POKE2_GENDER
    POKE2_FORM
    POKE3_POW
    POKE3_LV
    POKE3_GENDER
    POKE3_FORM
    POKE4_POW
    POKE4_LV
    POKE4_GENDER
    POKE4_FORM
    POKE5_POW
    POKE5_LV
    POKE5_GENDER
    POKE5_FORM
    POKE6_POW
    POKE6_LV
    POKE6_GENDER
    POKE6_FORM
    AI
    POKE1_ITEM
    POKE2_ITEM
    POKE3_ITEM
    POKE4_ITEM
    POKE5_ITEM
    POKE6_ITEM
    USE_ITEM1
    USE_ITEM2
    USE_ITEM3
    USE_ITEM4
    POKE1_WAZA1
    POKE1_WAZA2
    POKE1_WAZA3
    POKE1_WAZA4
    POKE2_WAZA1
    POKE2_WAZA2
    POKE2_WAZA3
    POKE2_WAZA4
    POKE3_WAZA1
    POKE3_WAZA2
    POKE3_WAZA3
    POKE3_WAZA4
    POKE4_WAZA1
    POKE4_WAZA2
    POKE4_WAZA3
    POKE4_WAZA4
    POKE5_WAZA1
    POKE5_WAZA2
    POKE5_WAZA3
    POKE5_WAZA4
    POKE6_WAZA1
    POKE6_WAZA2
    POKE6_WAZA3
    POKE6_WAZA4
    POKE1_NAME
    POKE2_NAME
    POKE3_NAME
    POKE4_NAME
    POKE5_NAME
    POKE6_NAME
    MAX
  ]
end

  fight_type = {  
	  "１対１"=>0,
	  "２対２"=>1,
  }

  data_type = { 
		"ノーマル"  =>0,  #DATATYPE_NORMAL
		"技"        =>1,  #DATATYPE_WAZA
		"道具"      =>2,  #DATATYPE_ITEM
		"マルチ"    =>3,  #DATATYPE_MULTI
  }

  gender = {  
    "♂"=>"PTL_SEX_MALE",
    "♀"=>"PTL_SEX_FEMALE",
    "－"=>"PTL_SEX_UNKNOWN",
  }

	if ARGV.size < 2
		print "error: ruby trdata_conv.rb read_file gmm_file\n"
		print "read_file:読み込むファイル\n"
		print "gmm_file:トレーナー名のgmmファイルを書き出すための元になるファイル\n"
		exit( 1 )
	end

  ARGV_READ_FILE = 0
  ARGV_READ_GMM_FILE = 1

  label = LabelMake.new
  read_data = []
  cnt = 0
  open( ARGV[ ARGV_READ_FILE ] ) {|fp_r|
    while str = fp_r.gets
      read_data[ cnt ] = str
      read_data[ cnt ] = read_data[ cnt ].tr( "\"\r\n", "" )
      cnt += 1
    end
  }

	#トレーナーグラフィックlstファイル生成
	fp_trgra = open( "trfgra_wb.lst", "w" )

	#トレーナーグラフィックscrファイル生成
	fp_trfgra = open( "trfgra_wb.scr", "w" )
  fp_trfgra.print "\"trwb_hero.NCGR\"\n"
  fp_trfgra.print "\"trwb_hero.NCBR\"\n"
  fp_trfgra.print "\"trwb_hero.NCER\"\n"
  fp_trfgra.print "\"trwb_hero.NANR\"\n"
  fp_trfgra.print "\"trwb_hero.NMCR\"\n"
  fp_trfgra.print "\"trwb_hero.NMAR\"\n"
  fp_trfgra.print "\"trwb_hero.NCEC\"\n"
  fp_trfgra.print "\"trwb_hero.NCLR\"\n"
  fp_trfgra.print "\"trwb_heroine.NCGR\"\n"
  fp_trfgra.print "\"trwb_heroine.NCBR\"\n"
  fp_trfgra.print "\"trwb_heroine.NCER\"\n"
  fp_trfgra.print "\"trwb_heroine.NANR\"\n"
  fp_trfgra.print "\"trwb_heroine.NMCR\"\n"
  fp_trfgra.print "\"trwb_heroine.NMAR\"\n"
  fp_trfgra.print "\"trwb_heroine.NCEC\"\n"
  fp_trfgra.print "\"trwb_heroine.NCLR\"\n"

	#トレーナーデータファイル生成
	fp_trdata = open( "trdata_000.bin", "wb" )

=begin
	fp_trdata.print "	.text\n"
	fp_trdata.print "\n"
	fp_trdata.print "	.include trdata.h\n"
	fp_trdata.print "\n"
	fp_trdata.print "	.byte   0			//データタイプ\n"
	fp_trdata.print "	.byte   0			//トレーナータイプ\n"
	fp_trdata.print "	.byte   0			//戦闘タイプ\n"
	fp_trdata.print "	.byte   0			//所持ポケモン数\n"
	fp_trdata.print "	.short	0			//所持アイテム１\n"
	fp_trdata.print "	.short	0			//所持アイテム２\n"
	fp_trdata.print "	.short	0			//所持アイテム３\n"
	fp_trdata.print "	.short	0			//所持アイテム４\n"
	fp_trdata.print "	.long   0			//AI\n"
=end
  
  data = [ 0,0,0,0,0,0,0,0,0 ].pack( "C4 S4 L" )
	data.size.times{ |c|
		fp_trdata.printf("%c",data[ c ])
	}

	fp_trdata.close

	#トレーナー持ちポケモンデータファイル生成
	fp_trpoke = open( "trpoke_000.bin", "wb" )

=begin
	fp_trpoke.print "	.text\n"
	fp_trpoke.print "\n"
	fp_trpoke.print "	.include trdata.h\n"
	fp_trpoke.print "\n"
	fp_trpoke.print "	.short	0				//ポケモン１パワー乱数\n"
	fp_trpoke.print "	.short	0				//ポケモン１レベル\n"
	fp_trpoke.print "	.short	0				//ポケモン１\n"
=end

  data = [ 0,0,0 ].pack( "S3" )
	data.size.times{ |c|
		fp_trpoke.printf("%c",data[ c ])
	}

	fp_trpoke.close

	#トレーナーID定義ファイル生成
	fp_trno = open( "trno_def.h", "w" )

  fp_trno.print( "//============================================================================================\n" )
  fp_trno.print( "/**\n" )
  fp_trno.print( " * @file	trno_def.h\n" )
  fp_trno.print( " * @bfief	トレーナーID定義ファイル\n" )
  fp_trno.print( " * @author	TrainerDataConverter\n" )
  fp_trno.printf( " * @date\t%s\n", Date::today.to_s )
	fp_trno.print( " * @author	このファイルは、コンバータが吐き出したファイルです\n" )
  fp_trno.print( "*/\n")
  fp_trno.print( "//============================================================================================\n" )
  fp_trno.print( "\n#ifndef __TRNO_DEF_H_\n" )
  fp_trno.print( "#define __TRNO_DEF_H_\n\n" )
	fp_trno.print("#define	TRID_NULL   ( 0 )\n")

	#トレーナータイプ定義ファイル生成
	fp_trtype = open( "trtype_def.h", "w" )

  fp_trtype.print( "//============================================================================================\n" )
  fp_trtype.print( "/**\n" )
  fp_trtype.print( " * @file	trtype_def.h\n" )
  fp_trtype.print( " * @bfief	トレーナータイプ定義\n");
  fp_trtype.print( " * @author	TrainerDataConverter\n" )
  fp_trtype.printf( " * @date\t%s\n", Date::today.to_s )
	fp_trtype.print( " * @author	このファイルは、コンバータが吐き出したファイルです\n" )
  fp_trtype.print( "*/\n")
  fp_trtype.print( "//============================================================================================\n" )
  fp_trtype.print( "\n#pragma once\n\n" )
	fp_trtype.print("\n")
	fp_trtype.print("#define	TRTYPE_HERO	    ( 0 )\n")
	fp_trtype.print("#define	TRTYPE_HEROINE	( 1 )\n")

	#トレーナー性別定義ファイル生成
	fp_trtypesex = open( "trtype_sex.h", "w" )

  fp_trtypesex.print( "//============================================================================================\n" )
  fp_trtypesex.print( "/**\n" )
  fp_trtypesex.print( " * @file	trtype_sex.h\n" )
	fp_trtypesex.print( " * @bfief	トレーナー性別定義\n")
  fp_trtypesex.print( " * @author	TrainerDataConverter\n" )
  fp_trtypesex.printf( " * @date\t%s\n", Date::today.to_s )
	fp_trtypesex.print( " * @author	このファイルは、コンバータが吐き出したファイルです\n" )
  fp_trtypesex.print( "*/\n")
  fp_trtypesex.print( "//============================================================================================\n" )
  fp_trtypesex.print( "\n#pragma once\n\n" )
	fp_trtypesex.print( "\n" )
	fp_trtypesex.print( "static	const	u8	TrTypeSexTable[]={\n" )
	fp_trtypesex.print( "\tPTL_SEX_MALE,		//男主人公\n" )
	fp_trtypesex.print( "\tPTL_SEX_FEMALE,	//女主人公\n" )

	#gmmファイル生成
  gmm_name = GMM::new
  gmm_type = GMM::new
  gmm_name.open_gmm( ARGV[ ARGV_READ_GMM_FILE ] , "trname.gmm" )
  gmm_type.open_gmm( ARGV[ ARGV_READ_GMM_FILE ] , "trtype.gmm" )

  gmm_name.make_row( "TR_NONE","ー" )

  gmm_type.make_row( "MSG_TRTYPE_HERO", "ポケモントレーナー" )
  gmm_type.make_row( "MSG_TRTYPE_HEROINE", "ポケモントレーナー" )

	trno = 1
  trainer = []
  tr_type = Hash::new

  read_data.size.times {|i|
    split_data = read_data[ i ].split(/,/)

    #データタイプ抽出
    if data_type[ split_data[ PARA::DATA_TYPE ] ] == nil
			printf("TrainerName:%s\n定義されていないデータタイプです：%s\n", split_data[ PARA::TR_NAME ], split_data[ PARA::DATA_TYPE ] )
      exit( 1 )
    end

    #ポケモンデータテーブル
		name = sprintf( "trpoke_%03d.bin", trno )
		fp_trpoke = open( name, "wb" )

=begin
		fp_trpoke.print "	.text\n"
		fp_trpoke.print "\n"
		fp_trpoke.print "	.include trdata.h\n"
		fp_trpoke.print "\n"
=end

		for poke_count in 0..5
      if split_data[ PARA::POKE1_NAME + poke_count ] == nil
        break
      end

      if split_data[ PARA::POKE1_POW + poke_count * 4 ] == ""
				printf( "TrainerName:%s No:%d\nPOKEPOW NOTHING!!\n", split_data[ PARA::TR_NAME ], poke_count + 1 )
        exit( 1 )
      end
#			fp_trpoke.printf( "	.short	%s\n", split_data[ PARA::POKE1_POW + poke_count * 4 ] )
      pow = split_data[ PARA::POKE1_POW + poke_count * 4 ].to_i

      if split_data[ PARA::POKE1_LV + poke_count * 4 ] == ""
				printf( "TrainerName:%s No:%d\nPOKELEVEL NOTHING!!\n", split_data[ PARA::TR_NAME ], poke_count + 1 )
        exit( 1 )
      end
#			fp_trpoke.printf( "	.short	%s\n", split_data[ PARA::POKE1_LV + poke_count * 4 ] )
      lv = split_data[ PARA::POKE1_LV + poke_count * 4 ].to_i

#     str = label.make_label( "MONSNO_", split_data[ PARA::POKE1_NAME + poke_count ] )
#			fp_trpoke.printf( "	.short	%s\n", str )
      mons = $monsno_hash[ split_data[ PARA::POKE1_NAME + poke_count ] ]

      data = [ pow, lv, mons ].pack( "S3" )
	    data.size.times{ |c|
		    fp_trpoke.printf("%c",data[ c ])
	    }

      case data_type[ split_data[ PARA::DATA_TYPE ] ]
      when 0  #DATATYPE_NORMAL
				if split_data[ PARA::POKE1_ITEM + poke_count ] != ""
					printf( "TrainerName:%s No:%d\nデータタイプがノーマルなのにアイテムがついています\n",
                    split_data[ PARA::TR_NAME ], poke_count + 1 )
          exit( 1 )
        end
				4.times { |j|
					if split_data[ PARA::POKE1_WAZA1 + poke_count * 4 + j ] != ""
						printf("TrainerName:%s No:%d\nデータタイプがノーマルなのに技がついています\n",
                    split_data[ PARA::TR_NAME ], poke_count + 1 )
            exit( 1 )
          end
        }
      when 1  #DATATYPE_WAZA
				wazacnt = 0
				4.times{ |j|
					if split_data[ PARA::POKE1_WAZA1 + poke_count * 4 + j ] != ""
#						str = label.make_label( "WAZANO_", split_data[ PARA::POKE1_WAZA1 + poke_count * 4 + j ] )
#						fp_trpoke.printf( "	.short	%s\n", str )
            waza = $wazano_hash[ split_data[ PARA::POKE1_WAZA1 + poke_count * 4 + j ] ]
						wazacnt += 1
					else
#						fp_trpoke.printf( "	.short	0\n" )
            waza = 0
          end
          data = [ waza ].pack( "S" )
    	    data.size.times{ |c|
	    	    fp_trpoke.printf("%c",data[ c ])
	        }
        }
				if wazacnt==0
					printf("TrainerName:%s No:%d\nデータタイプが技なのにポケモンに技がついてません\n",
                  split_data[ PARA::TR_NAME ], poke_count + 1 )
          exit( 1 )
        end
				if split_data[ PARA::POKE1_ITEM + poke_count ] != ""
					printf( "TrainerName:%s No:%d\nデータタイプが技なのにアイテムがついています\n",
                  split_data[ PARA::TR_NAME ], poke_count + 1 )
          exit( 1 )
        end
      when 2  #DATATYPE_ITEM
				if split_data[ PARA::POKE1_ITEM + poke_count ] != ""
#					str = label.make_label( "ITEM_", split_data[ PARA::POKE1_ITEM + poke_count ] )
#					fp_trpoke.printf( "	.short	%s\n", str )
          item = $item_hash[ split_data[ PARA::POKE1_ITEM + poke_count ] ]
				else
#					fp_trpoke.printf( "	.short	0\n" )
          item = 0
        end
        data = [ item ].pack( "S" )
    	  data.size.times{ |c|
	    	  fp_trpoke.printf("%c",data[ c ])
	      }
      when 3  #DATATYPE_MULTI
				if split_data[ PARA::POKE1_ITEM + poke_count ] != ""
#					str = label.make_label( "ITEM_", split_data[ PARA::POKE1_ITEM + poke_count ] )
#					fp_trpoke.printf( "	.short	%s\n", str )
          item = $item_hash[ split_data[ PARA::POKE1_ITEM + poke_count ] ]
				else
#					fp_trpoke.printf( "	.short	0\n" )
          item = 0
        end
        data = [ item ].pack( "S" )
    	  data.size.times{ |c|
	    	  fp_trpoke.printf("%c",data[ c ])
	      }

				wazacnt = 0
				4.times { |j|
					if split_data[ PARA::POKE1_WAZA1 + poke_count * 4 + j ] != ""
#						str = label.make_label( "WAZANO_", split_data[ PARA::POKE1_WAZA1 + poke_count * 4 + j ] )
#						fp_trpoke.printf( "	.short	%s\n", str )
            waza = $wazano_hash[ split_data[ PARA::POKE1_WAZA1 + poke_count * 4 + j ] ]
						wazacnt += 1
					else
#						fp_trpoke.printf( "	.short	0\n" )
            waza = 0
          end
          data = [ waza ].pack( "S" )
    	    data.size.times{ |c|
	    	    fp_trpoke.printf("%c",data[ c ])
	        }
        }
				if wazacnt==0
					printf("TrainerName:%s No:%d\nデータタイプがマルチなのにポケモンに技がついてません\n",
                  split_data[ PARA::TR_NAME ], poke_count + 1 )
          exit( 1 )
        end
      end
    end

		if poke_count == 0
			printf( "TrainerName:%s\nポケモンを持っていません\n", split_data[ PARA::TR_NAME ] )
      exit( 1 )
    end

		fp_trpoke.close

    #トレーナーデータテーブル
		name = sprintf( "trdata_%03d.bin", trno )
		fp_trdata = open( name, "wb" )

=begin
		fp_trdata.print "	.text\n"
		fp_trdata.print "\n"
		fp_trdata.print "	.include trdata.h\n"
		fp_trdata.print "\n"
=end

#		fp_trdata.printf( "	.byte	%s\n", data_type[ split_data[ PARA::DATA_TYPE ] ] )
		type_data = data_type[ split_data[ PARA::DATA_TYPE ] ]

		#unless split_data[ PARA::TR_ID ].to_s =~/^[^ -~｡-ﾟ]*$/
    unless split_data[ PARA::TR_ID ].to_s =~/^[ -~｡-ﾟ]*$/
			printf( "ラベルに全角が入っています！->%s\n", split_data[ PARA::TR_ID ] )
			exit( 1 )
    end

		str = "TRTYPE_" + split_data[ PARA::TR_ID ][ 0..split_data[ PARA::TR_ID ].size - 4 ].upcase

    flag = 0
    cnt = 0
		trainer.size.times { |no|
      if trainer[ no ][ 0 ] == str
        flag = 1
				break
      end
      cnt += 1
    }
		if flag == 0
      trainer[ cnt ] = [ str, split_data[ PARA::TR_TYPE ], gender[ split_data[ PARA::GENDER ] ], split_data[ PARA::TR_ID ][ 0..split_data[ PARA::TR_ID ].size - 4 ] ]
      tr_type[ str ] = cnt + 2
    end

#		fp_trdata.printf( "	.byte	%s\n", str );
#		fp_trdata.printf( "	.byte	%s\n", fight_type[ split_data[ PARA::FIGHT_TYPE ] ] )
#		fp_trdata.printf( "	.byte	%d\n", poke_count )

    type_tr     = tr_type[ str ]
    type_fight  = fight_type[ split_data[ PARA::FIGHT_TYPE ] ]

    data = [ type_data, type_tr, type_fight, poke_count ].pack( "C4" )
	  data.size.times{ |c|
		  fp_trdata.printf("%c",data[ c ])
	  }

    4.times { |no|
			if split_data[ PARA::USE_ITEM1 + no ] != ""
#				str = label.make_label( "ITEM_", split_data[ PARA::USE_ITEM1 + no ] )
#				fp_trdata.printf( "	.short	%s\n", str )
        item = $item_hash[ split_data[ PARA::USE_ITEM1 + no ] ]
			else
#				fp_trdata.print "	.short	0\n"
        item = 0
      end
      data = [ item ].pack( "S" )
	    data.size.times{ |c|
		    fp_trdata.printf("%c",data[ c ])
	    }
    }
  
    ai = split_data[ PARA::AI ].split(//)
    aibit = 0

    flag = 1

    ai.size.times {|no|
      if ai[ no ] == "●"
          aibit |= flag
      end
      flag = flag << 1
    }
#		fp_trdata.printf( "	.long	0x%08x\n",aibit )

    data = [ aibit ].pack( "L" )
	  data.size.times{ |c|
		  fp_trdata.printf("%c",data[ c ])
	  }

		fp_trdata.close

    #トレーナーの絶対番号
		fp_trno.printf( "#define	TRID_%s   ( %d ) \n", split_data[ PARA::TR_ID ].upcase, trno )

    #トレーナー名
    gmm_name.make_row( split_data[ PARA::TR_ID ], split_data[ PARA::TR_NAME ] )

		trno += 1
	}
	fp_trno.print( "\n" )
	fp_trno.print( "#ifndef __ASM_NO_DEF_  //これ以降はアセンブラでは無視\n" )
	fp_trno.print( "typedef  int TrainerID;\n" )
	fp_trno.print( "#endif __ASM_NO_DEF_\n" )
	fp_trno.print( "\n" )
	fp_trno.print( "#endif __TRNO_DEF_H_\n" )
	fp_trno.close

  #トレーナータイプHash生成
  fp_hash = open( "trtype_hash.rb", "w" )
  fp_hash.printf("#! ruby -Ks\n\n" )

  fp_hash.printf("\t$trtype_hash = {\n" )

  #トレーナータイプ名
  no = 2
  trainer.size.times { |i|
    fp_trgra.printf( "trwb_%s,%s\n", trainer[ i ][ 3 ], trainer[ i ][ 2 ] )
    fp_trfgra.printf( "\"trwb_%s.NCGR\"\n", trainer[ i ][ 3 ] )
    fp_trfgra.printf( "\"trwb_%s.NCBR\"\n", trainer[ i ][ 3 ] )
    fp_trfgra.printf( "\"trwb_%s.NCER\"\n", trainer[ i ][ 3 ] )
    fp_trfgra.printf( "\"trwb_%s.NANR\"\n", trainer[ i ][ 3 ] )
    fp_trfgra.printf( "\"trwb_%s.NMCR\"\n", trainer[ i ][ 3 ] )
    fp_trfgra.printf( "\"trwb_%s.NMAR\"\n", trainer[ i ][ 3 ] )
    fp_trfgra.printf( "\"trwb_%s.NCEC\"\n", trainer[ i ][ 3 ] )
    fp_trfgra.printf( "\"trwb_%s.NCLR\"\n", trainer[ i ][ 3 ] )
		fp_trtype.printf( "#define	%s    ( %d )    //%s\n", trainer[ i ][ 0 ].upcase, no, trainer[ i ][ 1 ] )
    str = "MSG_" + trainer[ i ][ 0 ]
    gmm_type.make_row( str, trainer[ i ][ 1 ] )
		fp_trtypesex.printf( "\t%s,\t\t//%s\n", trainer[ i ][ 2 ], trainer[ i ][ 1 ] )
    fp_hash.printf("\t\t\"%s\"=>%d,\n", trainer[ i ][ 1 ], no )
    no += 1
	}
	fp_trtype.printf( "#define	TRTYPE_MAX	( %d )\n", no )

  #gmm後始末
  gmm_name.close_gmm
  gmm_type.close_gmm

	fp_trgra.close
	fp_trfgra.close
	fp_trtype.close

	fp_trtypesex.print( "};\n" )
	fp_trtypesex.print( "\n" )
	fp_trtypesex.close

  fp_hash.printf("\t}\n" )
  fp_hash.close

  #タイムスタンプ比較用のダミーファイルを生成
  fp_w = open( "out_end", "w" )
  fp_w.close
