#====================================================
#
# 2009.05.26  tamada
#
#====================================================

def read_uniq_scripts( filename )
  File.open(filename){|file|
    file.each{|line|
      if line =~/\w+\.ev/ then
        column = line.split
        puts "\#include \"../../script/#{column[0].sub(/\.ev/,"")}_def.h\""
      end
    }
  }
end


begin
#  names = Array.new
#  contents = ""
#  header = ""
#  footer = "static const EVENTDATA_TOTAL_TABLES TotalTables[] = {\n"

  read_uniq_scripts("../../script/uniq_script.list")
#  ARGV.each{|filename|
#       File.open(filename){|file|
#      headername = "../tmp/#{File.basename(filename,".*")}.h"
#      header += "//\#include \"#{headername}\"\n"
#      header += File.open(headername).read
#      id = File.basename(filename,".*").sub(/^event_/,"")
#      footer += "\t{ ZONE_ID_#{id.upcase}, &event_#{id} },\n"
#      contents += file.read
#    }
#  }
#  footer += "\t{ 0, NULL }\n"
#  footer += "};\n"
#  puts header
#  puts ""
#  puts contents
#  puts ""
#  puts footer
#  puts ""

end
