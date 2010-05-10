require 'kconv'  # for String.toutf8 


@@place_name_hash = Hash.new


def get_zone_name( place_name_id ) 
  file = File::open( "place_name.gmm", "r" )
  file_lines = file.readlines
  file.close

  id_found = false

  file_lines.each do |line|
    line = line.toutf8
    if id_found then
      if line =~ /<language.*>(.*)<\/language>/ then 
        return $1.tosjis
      end
    else
      if line.include?( place_name_id ) then id_found = true end
    end
  end

  return nil
end


def create_place_name_hash

  file = File::open( "zonetable.txt", "r" )
  zone_table = file.readlines
  file.close

  zone_table.each do |zone_line|
    items = zone_line.split( /\s/ )
    zone_id = items[0]
    place_name_id = items[25]
    if zone_id == "END" then break end
    zone_name = get_zone_name( place_name_id )
    if zone_name != nil 
      @@place_name_hash[ zone_id ] = zone_name
    end
  end

end
