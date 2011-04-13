=begin
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		map_array_conv.rb
 *	@brief  毒ジムのマップ情報のExcelコンバータ
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.04.13
 *
 *  @note   毒ジムの音量が変わる場所を配列にする
 *          Excelをコンバートして、配列にする
 *          0：音量 100%
 *          1：音量 90%
 *          2：音量 30%
 *          3：音量 0%
 *
 * 手順として、csv形式を読み込んで、データファイルとして書き出す
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
=end

def writeData( read_file, write_file, filecount )
  lines = fields = 0
  ret = 0
  array = Array.new

  while line = read_file.gets
    if line =~/^#NEXT/ then
      ret = 0
      break end
    if line =~/^#END/ then
      ret = 1
      break end

    lines += 1
    fields = line.split(',').size - 1  # 配列なので+1
    s = line.split(/,/);
    tmparray = Array.new
    0.upto(fields){|i|
      tmparray << s[i].to_i
    }
    array << tmparray
  end

  write_file.printf("static const int map_array_%d[%d][%d]", filecount, lines, fields )
  write_file.print " = {\n"

  # ファイルに配列書き込み
  0.upto(lines-1){|i|
    write_file.print "\t{ "
    0.upto(fields-1){|j|
      write_file.printf("%d,", array[i][j]);
    }
    write_file.print "},\n"
  }
  write_file.print "};\n"
  write_file.printf("#define MAP_ARRAY_%d_ROW\t%d\n", filecount, lines)
  write_file.printf("#define MAP_ARRAY_%d_COL\t%d\n", filecount, fields)
  write_file.print "\n"

  return ret
end


# ファイルオープン
f_old = open( ARGV.to_s, "r" );   # 読み込み
f_new = open( "map_array.cdat", "w" ); # 新規作成書き込み

f_new.print  <<EOS
//=============================================================================
//    map_array_conv.rb で出力されたファイルです
//=============================================================================
EOS

count = 1

# データ分配列作成
while writeData( f_old, f_new, count ) == 0
  count += 1
end

f_old.close
f_new.close

