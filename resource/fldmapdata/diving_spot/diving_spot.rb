#============================================================================
#============================================================================

class DivingSpotConvertError < Exception; end

def check_head_line( line )
  column = line.chomp.split(",")
  raise DivingSpotConvertError if column[0] != "No."
  raise DivingSpotConvertError if column[1] != "ZONE"
  raise DivingSpotConvertError if column[2] != "グリッドX"
  raise DivingSpotConvertError if column[3] != "グリッドZ"
  raise DivingSpotConvertError if column[4] != "接続先"
end
def make_output( file )
  output = ""
  line = file.gets
  STDERR.puts line
  check_head_line( line )
  output += sprintf("static const DIVING_SPOT_TABLE DivingSpotTable[] = {\n")
  file.each{|line|
    column = line.chomp.split(",")
    output += sprintf("\t{// No.%2d\n", column[0])
    output += sprintf("\t\t%-20s, %3d, %3d,//チェック地点\n",
                      "ZONE_ID_" + column[1], column[2], column[3])
    output += sprintf("\t\t%-20s //接続先\n", "ZONE_ID_" + column[4] )
    output += sprintf("\t},\n")
  }
  output += sprintf("};\n\n")
  return output
end


output = ""
File.open(ARGV[0],"r"){|file|
  output = make_output( file )
}

File.open(ARGV[1],"w"){|outputfile|
  outputfile.puts output
}
system("unix2dos -D #{ARGV[1]}")

