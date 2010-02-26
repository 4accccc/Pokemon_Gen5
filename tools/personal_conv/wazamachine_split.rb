#! ruby -Ks
# ファイルメーカーからエクスポートされたcsv中の連続フィールド部分を分割してエクセルで閲覧できる形にコンバート

  require File.dirname(__FILE__) + '/../label_make/label_make'
  require File.dirname(__FILE__) + '/../gmm_make/gmm_make'
  require File.dirname(__FILE__) + '/../constant'
  require File.dirname(__FILE__) + '/../hash/wazano_hash.rb'
  require File.dirname(__FILE__) + '/../hash/item_hash.rb'
  require File.dirname(__FILE__) + '/../hash/tokusei_hash.rb'
  require 'date'

class PARA
  enum_const_set %w[
    POKENAME
    SHINKA_POKE1
    SHINKA_POKE2
    SHINKA_POKE3
    SHINKA_POKE4
    SHINKA_POKE5
    SHINKA_POKE6
    SHINKA_POKE7
    WAZA1
    WAZA2
    WAZA3
    WAZA4
    WAZA5
    WAZA6
    WAZA7
    WAZA8
    WAZA9
    WAZA10
    WAZA11
    WAZA12
    WAZA13
    WAZA14
    WAZA15
    WAZA16
    WAZA17
    WAZA18
    WAZA19
    WAZA20
    WAZA21
    WAZA22
    WAZA23
    WAZA24
    WAZA25
    ITEM1
    ITEM2
    ITEM3
    SPEABI1
    SPEABI2
    SPEABI3
    ZENKOKU_NO
    CHIHOU_NO
    SHINKA_LEVEL
    TYPE1
    TYPE2
    GROW
    EXP
    GET
    GENDER
    EGG_GROUP1
    EGG_GROUP2
    BIRTH_STEP
    FRIEND
    ESCAPE
    HP
    POW
    DEF
    AGI
    SPEPOW
    SPEDEF
    HP_EXP
    POW_EXP
    DEF_EXP
    AGI_EXP
    SPEPOW_EXP
    SPEDEF_EXP
    WAZA_LV1
    WAZA_LV2
    WAZA_LV3
    WAZA_LV4
    WAZA_LV5
    WAZA_LV6
    WAZA_LV7
    WAZA_LV8
    WAZA_LV9
    WAZA_LV10
    WAZA_LV11
    WAZA_LV12
    WAZA_LV13
    WAZA_LV14
    WAZA_LV15
    WAZA_LV16
    WAZA_LV17
    WAZA_LV18
    WAZA_LV19
    WAZA_LV20
    WAZA_LV21
    WAZA_LV22
    WAZA_LV23
    WAZA_LV24
    WAZA_LV25
    SHINKA_COND1
    SHINKA_COND2
    SHINKA_COND3
    SHINKA_COND4
    SHINKA_COND5
    SHINKA_COND6
    SHINKA_COND7
    WAZA_MACHINE
    GRA_NO
    FORM_NAME
    COLOR
    REVERSE
    OTHERFORM
    RANK
    PLTT_ONLY
    HEIGHT
    WEIGHT
    WAZA_OSHIE
    NO_JUMP
    MAX
  ]
end

	if ARGV.size < 2
		print "error: ruby wazamachine_split.rb read_file write_file\n"
		print "read_file:読み込むファイル\n"
		print "write_file:書き込むファイル\n"
		exit( 1 )
	end

  ARGV_READ_FILE = 0
  ARGV_WRITE_FILE = 1

  read_data = []
  cnt = 0
  open( ARGV[ ARGV_READ_FILE ] ) {|fp_r|
    while str = fp_r.gets
      str = str.tr( "\"\r\n", "" )
      split_data = str.split(/,/)
      next if split_data.size <= 1       #サーバからのエクスポートでゴミレコードが入ることがあるので、排除する
      read_data[ cnt ] = str
      cnt += 1
    end
  }

  fp_w = open( ARGV[ ARGV_WRITE_FILE ], "w" )

  read_data.size.times {|i|
    split_data = read_data[ i ].split(/,/)
    
    for i in PARA::POKENAME..PARA::SHINKA_COND7
      fp_w.printf( "%s,", split_data[ i ] )
    end

    waza_machine = split_data[ PARA::WAZA_MACHINE ].split(//)

    waza_machine.size.times {|j|
      if waza_machine[ j ] == "●"
        fp_w.print("●,")
      else
        fp_w.print(",")
      end
    }

    padding = 101 - waza_machine.size

    padding.times {|j|
      fp_w.print(",")
    }

    for i in PARA::GRA_NO..PARA::WEIGHT
      fp_w.printf( "%s,", split_data[ i ] )
    end

    waza_machine = split_data[ PARA::WAZA_OSHIE ].split(//)

    waza_machine.size.times {|j|
      if waza_machine[ j ] == "●"
        fp_w.print("●,")
      else
        fp_w.print(",")
      end
    }

    padding = 7 - waza_machine.size

    padding.times {|j|
      fp_w.print(",")
    }

    fp_w.printf( "%s,", split_data[ PARA::NO_JUMP ] )

    fp_w.print("\n")
  }

  fp_w.close
