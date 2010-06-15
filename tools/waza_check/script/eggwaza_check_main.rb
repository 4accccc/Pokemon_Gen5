require "mons_fullname_list.rb"
require "eggwaza_derive_route_tracer.rb"
require "eggwaza_derive_route.rb"
require "eggwaza_hash.rb"


def GetRouteOf( mons_name, waza_name )
  tracer = EggWazaDeriveRouteTracer.new
  route = tracer.full_trace( mons_name, waza_name )
  return route
end

def GetRouteHashOf( mons_name )
  route_hash = Hash.new
  waza_list = $eggwaza_hash[ mons_name ]
  if waza_list == nil then
    return nil
  end
  waza_list.each do |waza_name|
    puts( "--#{waza_name}" )
    route_hash[ waza_name ] = GetRouteOf( mons_name, waza_name )
  end
  return route_hash
end

def GetResultOf( mons_name )
  result = ""
  result += "#{mons_name}\r\n"
  route_hash = GetRouteHashOf( mons_name )
  waza_list = $eggwaza_hash[ mons_name ]
  if waza_list == nil then
    return result
  end 
  waza_list.each do |waza_name|
    route = route_hash[ waza_name ]
    if route == nil then
      result += "\t×#{waza_name}\r\n"
    else
      result += "\t○#{waza_name} route:#{route}\r\n"
    end
  end
  return result
end


# main ##################################################

all_result = Hash.new 
$mons_fullname_list.each do |mons_name| 
  puts "#{mons_name}"
  all_result[ mons_name ] = GetResultOf( mons_name )
end 

output_filename = "../result_kowaza.txt"
output_file = File.open( output_filename, "w" )
$mons_fullname_list.each do |mons_name| 
  result = all_result[ mons_name ]
  output_file.puts( result )
end
output_file.close


# テスト用
=begin
route = GetRouteOf( "ベトベター", "こわいかお" )
puts route
=end

=begin
checker = EggWazaChecker.new
result = checker.partial_analyze( "マッギョ", "こわいかお" )
puts result.get( "こわいかお" )
=end

=begin
personal = PersonalAccessor.new
if personal.check_waza_learning_except_eggwaza( "ベトベター", 100, "こわいかお" ) then
  puts "!!!"
end
=end
