# @brief  ÉpÅ[É\ÉiÉãÉfÅ[É^Ç©ÇÁ, äeÉ|ÉPÉÇÉìÇ™èKìæâ¬î\Ç»É}ÉVÉìãZÇÃÉnÉbÉVÉÖÉeÅ[ÉuÉãÇçÏê¨Ç∑ÇÈ
# @file   machinewaza_hash_conv.rb
# @author obata
# @date   2010.05.26 

require "personal_index.rb"
require "machine_wazaname_hash.rb"


# modules ################################################### 

# ëSäpêîéöÇ÷ÇÃïœä∑
def GetZenkakuNumber( num )
  num_hash = {
    0 => "ÇO",
    1 => "ÇP",
    2 => "ÇQ",
    3 => "ÇR",
    4 => "ÇS",
    5 => "ÇT",
    6 => "ÇU",
    7 => "ÇV",
    8 => "ÇW",
    9 => "ÇX",
  }

  string = "" 
  while true
    key = num % 10
    num = num / 10
    string = num_hash[ key ] + string
    if num == 0 then break end
  end
  return string
end

# ãZÉ}ÉVÉìñºÇÃéÊìæ
def GetWazaMachineName( machine_no ) 
  str_number = GetZenkakuNumber( machine_no ) 
  machine_name = nil
  if machine_no < 10 then
    machine_name = "ÇÌÇ¥É}ÉVÉìÇO" + str_number
  else
    machine_name = "ÇÌÇ¥É}ÉVÉì" + str_number
  end 
  return machine_name
end

# îÈì`É}ÉVÉìïúäàÇ…Ç∆Ç‡Ç»Ç§ïœä∑
def CorrectWazaMachineName_byHidenMachineRecovery( machine_name )
  correct_hash = {
    "ÇÌÇ¥É}ÉVÉìÇXÇS"   => "Ç–Ç≈ÇÒÉ}ÉVÉìÇOÇP", # Ç¢Ç†Ç¢Ç¨ÇË
    "ÇÌÇ¥É}ÉVÉìÇXÇT"   => "Ç–Ç≈ÇÒÉ}ÉVÉìÇOÇS", # Ç©Ç¢ÇËÇ´
    "ÇÌÇ¥É}ÉVÉìÇXÇU"   => "Ç–Ç≈ÇÒÉ}ÉVÉìÇOÇR", # Ç»Ç›ÇÃÇË
    "ÇÌÇ¥É}ÉVÉìÇXÇV"   => "Ç–Ç≈ÇÒÉ}ÉVÉìÇOÇQ", # ÇªÇÁÇÇ∆Ç‘
    "ÇÌÇ¥É}ÉVÉìÇXÇW"   => "Ç–Ç≈ÇÒÉ}ÉVÉìÇOÇU", # É_ÉCÉrÉìÉO
    "ÇÌÇ¥É}ÉVÉìÇXÇX"   => "Ç–Ç≈ÇÒÉ}ÉVÉìÇOÇT", # ÇΩÇ´ÇÃÇ⁄ÇË
    "ÇÌÇ¥É}ÉVÉìÇPÇOÇO" => "ÇÌÇ¥É}ÉVÉìÇXÇS",   # Ç¢ÇÌÇ≠ÇæÇ´
    "ÇÌÇ¥É}ÉVÉìÇPÇOÇP" => "ÇÌÇ¥É}ÉVÉìÇXÇT",   # ÉoÅ[ÉNÉAÉEÉg
  } 
  
  if correct_hash.has_key?( machine_name ) then
    machine_name = correct_hash[ machine_name ]
  end
  return machine_name
end


# main ######################################################

usable_wazamachine_list = Array.new

personal_file = File::open( "personal_wb.csv", "r" )
personal_lines = personal_file.readlines
personal_file.close


output_file = File::open( "machinewaza_hash.rb", "w" )
output_file.puts( "$machinewaza_hash = {" )

personal_lines.each do |line|
  line.gsub!( "\"", "" )
  line.gsub!( /\s/, "" )
  items = line.split( "," )
  mons_name = items[ PARA::POKENAME ]
  output_file.puts( "\t\"#{mons_name}\"=>[" )
  machine_data = items[ PARA::WAZA_MACHINE ]
  machine_data_split = machine_data.split( // )
  0.upto( machine_data_split.size - 1 ) do |machine_idx|
    if machine_data_split[ machine_idx ] == "Åú" then 
      machine_name = GetWazaMachineName( machine_idx + 1 )
      machine_name = CorrectWazaMachineName_byHidenMachineRecovery( machine_name )
      waza_name = $machine_wazaname_hash[ machine_name ]
      output_file.puts( "\t\t\"#{waza_name}\"," )
    end
  end
  output_file.puts( "\t]," )
end

output_file.puts( "}" )
