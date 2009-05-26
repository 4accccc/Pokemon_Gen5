#====================================================
#
# 2009.05.26  tamada
#
#====================================================



begin
  names = Array.new
  contents = ""
  header = ""
  footer = "static const EVENTDATA_TOTAL_TABLES TotalTables[] = {\n"

  ARGV.each{|filename|
    File.open(filename){|file|
      headername = "#{File.basename(filename,".*")}.h"
      header += "//\#include \"#{File.basename(filename, ".*")}.h\"\n"
      header += File.open(headername).read
      id = File.basename(filename,".*").sub(/^event_/,"")
      footer += "\t{ ZONE_ID_#{id.upcase}, &event_#{id} },\n"
      contents += file.read
    }
  }
  footer += "\t{ 0, NULL }\n"
  footer += "};\n"
  puts header
  puts ""
  puts contents
  puts ""
  puts footer
  puts ""

end
