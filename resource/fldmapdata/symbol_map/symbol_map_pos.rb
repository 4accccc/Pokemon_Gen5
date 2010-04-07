$KCODE = "SJIS"

class SymbolMapError < Exception; end

little_list = []
large_list = []
keep_list = []

def read_map( lines )
  symmap = []
  line = lines.shift
  line = lines.shift
  0.upto(31){|i|
    column = lines.shift.split(",")
    raise SymbolMapError if column[3].to_i != i
    symmap << column[4 .. 4 + 31]
  }
  return symmap
end

def put_pos_list( symmap, symposname, subsymposname )

  pos_list = []
  pos_list_s = []
  symmap.each_with_index{|line, y|
    line.each_with_index{|p, x|
      if p != nil && p.to_i > 0 then
        pos = p.to_i
        if pos_list[pos] == nil then
          pos_list[pos] = [x,y]
        else
          pos_list_s[pos] = [x,y]
        end
      end
    }
  }
  printf( "static const u8 %s[] = {\n", symposname )
  pos_list[1 .. -1].each_with_index{| p, no |
    printf("\t%2d,%2d, //%2d\n", p[0],p[1],no+1 )
  }
    if subsymposname != nil then
      #printf( "static const u8 %s[] = {\n", subsymposname )
      pos_list[1 .. -1].each_with_index{| p, no |
        printf("\t%2d,%2d, //%2d\n", p[0],p[1],no+1 )
      }
      #printf("};\n\n")
    end
  printf("};\n\n")


end

File.open(ARGV[0]){|file|
  lines = file.read.to_a

  lines.shift
  lines.shift
  lines.shift

  if lines[0] =~ /小さいサイズエリア/ then
    little_map = read_map( lines )
    little_list = put_pos_list( little_map ,"littleMapPos", nil )
    #p little_list
  else
    p lines[0]
    raise SymbolMapError
  end
  lines.shift

  if lines[0] =~ /大きいサイズエリア/ then
    large_map = read_map( lines )
    large_list = put_pos_list( large_map, "largeMapPos", nil )
    #p large_list
  else
    raise SymbolMapError
  end
  lines.shift

  if lines[0] =~ /森の奥/ then
    keep_map = read_map( lines )
    keep_list = put_pos_list( keep_map, "keepMapPos", "dummy" )
    #p keep_list
  else
    raise SymbolMapError
  end
}

