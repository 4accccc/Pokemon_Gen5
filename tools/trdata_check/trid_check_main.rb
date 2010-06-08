require "trid_check_result.rb"
require "trid_checker.rb"
require "exceptional_hash.rb"


checker = TrIDChecker.new
checker.add_check_files( "../../resource/fldmapdata/eventdata/data/*.mev" )
checker.add_check_files( "../../resource/fldmapdata/script/*.ev" )
checker.setup_trid_list( "../../resource/trdata/trno_def.h" )
checker.analyze

trid_list = checker.trid_list 
result = checker.result

except_list = $exceptional_hash.keys
except_list.each do |trid|
  result.add( trid, "\t#{$exceptional_hash[ trid ]}" )
end

out_data = String.new
file = File.open( "result.txt", "w" )
trid_list.each do |trid|
  file.puts( "#{trid}\r\n" )
  if result.has_key( trid ) then 
    file.puts( result.get( trid ) )
  end
end 
file.close

file = File.open( "unused_trid.txt", "w" )
trid_list.each do |trid|
  if result.has_key( trid ) == false then
    file.puts( "#{trid}\r\n" )
  end
end
file.close
