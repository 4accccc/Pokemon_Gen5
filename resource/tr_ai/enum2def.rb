#! ruby -Ks
# enum定義を探し出して、define定義に置き換える

	if ARGV.size < 1
		print "error: ruby enum2def.rb read_file\n"
		print "read_file:読み込むファイル\n"
		exit( 1 )
	end

  ARGV_READ_FILE = 0

  read_data = []
  cnt = 0

  open( ARGV[ ARGV_READ_FILE ] ) {|fp_r|
    while read_data[ cnt ] = fp_r.gets
      cnt += 1
    end
  }

  w_file = File::basename( ARGV[ ARGV_READ_FILE ], ".h" ) + "_def.h"
  fp_w = open( w_file, "w" )

  mode = 0
  num = 0
  num_label = []
  num_label_find = 0
  label = []
  label_find = 0

  read_data.size.times {|cnt|
    next if read_data[ cnt ] == nil
    split_data = read_data[ cnt ].split(/\s/)
    case mode
    when 0  #enum検索
      if split_data[ 0 ] == "enum" || split_data[ 1 ] == "enum"
        mode = 1
        num = 0
        label = ""
        num_label_find = 0
      end
    when 1  #ラベル
      label_find = 0
      next if split_data[ 0 ] == nil
      if split_data[ 0 ].index("}") == 0
        mode = 0
        next
      end
      if read_data[ cnt ].index("=") != nil
        pos = read_data[ cnt ].index("=")
        split_eq_data = []
        split_eq_data << read_data[ cnt ].slice(0..(pos-1))
        split_eq_data << read_data[ cnt ].slice((pos+1)..(read_data[ cnt ].size - 1))
        label = split_eq_data[ 0 ].sub(" ","")
        if label.index("/") != nil
          break
        end
        pos = (split_eq_data[ 1 ].sub(" ","")).index(",")
        num_label = (split_eq_data[ 1 ].sub(" ","")).slice(0..(pos-1))
        num = num_label.to_i
        num_keta = num.to_s
        if num_keta.size != num_label.size
          num_label_find = 1
          num = 0
        else
          num_label_find = 0
        end
        label_find = 1
      else
        split_data.size.times {|str|
          if split_data[ str ].index( "/" ) != nil
            #コメント行は無視
            break
          elsif ( split_data[ str ].size != 0 ) && ( label_find == 0 )
            label = split_data[ str ].sub( ",", "" )
            label_find = 1
          end
        }
      end
      if label_find == 1
        if num_label_find == 0
          fp_w.printf("#define\t%s\t( %d )\n",label,num)
        else
          fp_w.printf("#define\t%s\t( %s + %d )\n",label,num_label,num)
        end
        num += 1
      end
    end
  }

  fp_w.close
