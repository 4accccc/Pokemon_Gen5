#!  ruby -Ks

require File.dirname(__FILE__) + '/../../tools/constant'


class COL
  enum_const_set %w[
    BLANK1
    BLANK2
    USERNAME
    FRIENDCODE
    NICKNAME
  ]
end

def convertNumber( numstr )
  num = 0
  count = 0
  numstr.split(//).each{|chr|
    add = -1
    if '0' <= chr && chr <='9' then
      add = chr.to_i
    end
    case chr
    when "‚O"
      add = 0
    when "‚P"
      add = 1
    when "‚Q"
      add = 2
    when "‚R"
      add = 3
    when "‚S"
      add = 4
    when "‚T"
      add = 5
    when "‚U"
      add = 6
    when "‚V"
      add = 7
    when "‚W"
      add = 8
    when "‚X"
      add = 9
    end
    if add >= 0 then
      num = num * 10 + add
      count += 1
      #puts "count:#{count} num=#{num} chr=\"#{chr}\""
    end
  }
  raise Exception, "\"#{numstr}\" count error!" if count != 12
  return num
end

class FRIENDCODE
  def initialize( id, name )
    @id = convertNumber(id)
    @name = name
  end
  def putCDAT()
    output = sprintf("\t{ %12d, \"%s\" },\n", @id, @name )
    return output
  end
end

def readList( filename )
  flist = []
  File.open( filename ){|file|
    file.each_with_index{|line, linecount|
      if linecount < 1 then
        next
      end
      column = line.split(",")
      if column[COL::FRIENDCODE] == "" then
        break
      end
      flist << FRIENDCODE.new( column[COL::FRIENDCODE], column[COL::NICKNAME] )
      #puts "number(#{convertNumber(column[COL::FRIENDCODE])}) nicname(#{column[COL::NICKNAME]})"
      #puts "number(#{column[COL::FRIENDCODE]}) nicname(#{column[COL::NICKNAME]})"
    }
  }
  return flist
end

def printCDAT( flist )
  printf("static const DEBUG_WIFINOTE_CONST_DATA DebugWifiNoteConstData[] = {\n")
  flist.each{|fdata| puts fdata.putCDAT() }
  printf("};\n\n")
end

flist = readList( "friendlist.csv" )

printCDAT( flist )



