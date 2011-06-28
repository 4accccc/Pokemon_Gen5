#------------------------------------------------------------------------------
#
#   trno_def.hからブリーダーのIDだけを配列に抽出するスクリプト
#
#   2011.06.28  tamada
#
#
#   swanでブリーダーは再戦可能となるのでそのチェックのために追加
#------------------------------------------------------------------------------

def make_breeder_list( filename )
  output = ""
  output += "static const u16 breeder_id_list[] = {\n"

  File.open( filename ){|file|
    file.each{|line|
      column = line.split
      if column[0] =~/\#define/ && column[1] =~/TRID_BREEDER[MW]/ then
        output += sprintf("\t%s,\n", column[1])
      end
    }
  }
  output += "};\n"
  return output
end

puts make_breeder_list( "trno_def.h" )
