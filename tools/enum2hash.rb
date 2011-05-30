#!  ruby -Ks
# enum定義を探し出して、hashにして返す
# あんまり変な書き方をしているenum定義だとちゃんと置き換わんないかも…

#ENUM_SIMPLE = /^\s*\w,\s*$/
#ENUM_DEFINE = /^\s*\w\s*=\s*\d+,\s*$/

def enum2hash( filename )

  aHash = Hash.new
  read_count = 0

  read_data = File.open( filename ).map{|line|
    read_count += 1
    line.sub(/^\s+/,"").sub(/\/\/.*$/,"").chomp
  }
  #STDERR.printf("read %d lines\n", read_data.size )

  mode = 0
  now_value = nil

  read_data.each_with_index{| line, index |
    if line == nil then next end
    column = line.split(/\s/)
    case mode
    when 0
      if line =~/^\s*enum/ || line =~/^\s*typedef\s+enum/ then
        #STDERR.printf( "found enum start(%d)\n", index )
        mode = 1
      end

    when 1
      #p column
      if line =~/}/ then
        #STDERR.printf( "found enum end(%d)\n", index )
        mode = 0
        next
      end
      #printf("column[0] is /word,/ == %s\n", column[0] =~ /\w+,/ ? "true" : "false" )
      #printf("column[0] is /word/ == %s\n", column[0] =~ /\w+/ ? "true" : "false" )
      #printf("column[1] is ',' == %s\n", column[1] == ',' ? "true" : "false" )
      #printf("column[1] is '=' == %s\n", column[1] == '=' ? "true" : "false" )
      #printf("column[2] is decimal == %s\n", column[2] =~/\d/ ? "true" : "false" )

      if column[0] =~ /\w+,/ || (column[0] =~/\w+/ && column[1] == ",") then
        key = column[0].sub(/,$/,"")
        #printf(" key is '%s'\n", key )
        if aHash.has_key?(key) then
          STDERR.printf("(%s)同じ定義があります！\n", key )
          abort( -1 )
        end
        if now_value == nil then
          now_value = -1
        end
        now_value += 1
        aHash[ key ] = now_value
        STDERR.printf( "Assign (%s) = (%d)\n", key, now_value )
      elsif column[0] = ~/\w+/ && column[1] == "=" && column[3] == ',' then
        unless column[2] =~/\d/ then
          STDERR.printf(" 数値以外は指定できません\n")
          abort( 1 )
        end
        now_value = column[2].to_i
        key = column[0].sub(/,$/,"")
        aHash[ column[0] ] = now_value
        #STDERR.printf( "Assign (%s) = (%d)\n", column[0], now_value )
      end
    end
  }
  return aHash
end
