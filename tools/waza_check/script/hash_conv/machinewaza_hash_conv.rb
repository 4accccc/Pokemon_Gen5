# @brief  ƒp[ƒ\ƒiƒ‹ƒf[ƒ^‚©‚ç, Šeƒ|ƒPƒ‚ƒ“‚ªK“¾‰Â”\‚Èƒ}ƒVƒ“‹Z‚ÌƒnƒbƒVƒ…ƒe[ƒuƒ‹‚ğì¬‚·‚é
# @file   machinewaza_hash_conv.rb
# @author obata
# @date   2010.05.26 

require "personal_parser.rb"
require "machine_wazaname_hash.rb"


# modules ################################################### 

# ‘SŠp”š‚Ö‚Ì•ÏŠ·
def GetZenkakuNumber( num )
  num_hash = {
    0 => "‚O",
    1 => "‚P",
    2 => "‚Q",
    3 => "‚R",
    4 => "‚S",
    5 => "‚T",
    6 => "‚U",
    7 => "‚V",
    8 => "‚W",
    9 => "‚X",
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

# ‹Zƒ}ƒVƒ“–¼‚Ìæ“¾
def GetWazaMachineName( machine_no ) 
  str_number = GetZenkakuNumber( machine_no ) 
  machine_name = nil
  if machine_no < 10 then
    machine_name = "‚í‚´ƒ}ƒVƒ“‚O" + str_number
  else
    machine_name = "‚í‚´ƒ}ƒVƒ“" + str_number
  end 
  return machine_name
end

# ”é“`ƒ}ƒVƒ“•œŠˆ‚É‚Æ‚à‚È‚¤•ÏŠ·
def CorrectWazaMachineName_byHidenMachineRecovery( machine_name )
  correct_hash = {
    "‚í‚´ƒ}ƒVƒ“‚X‚S"   => "‚Ğ‚Å‚ñƒ}ƒVƒ“‚O‚P", # ‚¢‚ ‚¢‚¬‚è
    "‚í‚´ƒ}ƒVƒ“‚X‚T"   => "‚Ğ‚Å‚ñƒ}ƒVƒ“‚O‚S", # ‚©‚¢‚è‚«
    "‚í‚´ƒ}ƒVƒ“‚X‚U"   => "‚Ğ‚Å‚ñƒ}ƒVƒ“‚O‚R", # ‚È‚İ‚Ì‚è
    "‚í‚´ƒ}ƒVƒ“‚X‚V"   => "‚Ğ‚Å‚ñƒ}ƒVƒ“‚O‚Q", # ‚»‚ç‚ğ‚Æ‚Ô
    "‚í‚´ƒ}ƒVƒ“‚X‚W"   => "‚Ğ‚Å‚ñƒ}ƒVƒ“‚O‚U", # ƒ_ƒCƒrƒ“ƒO
    "‚í‚´ƒ}ƒVƒ“‚X‚X"   => "‚Ğ‚Å‚ñƒ}ƒVƒ“‚O‚T", # ‚½‚«‚Ì‚Ú‚è
    "‚í‚´ƒ}ƒVƒ“‚P‚O‚O" => "‚í‚´ƒ}ƒVƒ“‚X‚S",   # ‚¢‚í‚­‚¾‚«
    "‚í‚´ƒ}ƒVƒ“‚P‚O‚P" => "‚í‚´ƒ}ƒVƒ“‚X‚T",   # ƒo[ƒNƒAƒEƒg
  } 
  
  if correct_hash.has_key?( machine_name ) then
    machine_name = correct_hash[ machine_name ]
  end
  return machine_name
end


# main ######################################################

def CreateMachineWazaHash( personal_filename, output_path )

  personal_parser = PersonalDataParser.new
  personal_parser.load_personal_file( personal_filename )

  out_data = Array.new
  out_data << "$machinewaza_hash = {"

  $monsname.each do |mons_name| 
    if mons_name == "|||||" then
      next
    end 

    usable_waza_machine_list = Array.new
    machine_list = personal_parser.get_usable_waza_machine_list( mons_name )

    # ”é“`ƒ}ƒVƒ“•œŠˆ‚É‚Æ‚à‚È‚¤•ÏŠ·
    machine_list.each do |machine_name|
      usable_waza_machine_list << CorrectWazaMachineName_byHidenMachineRecovery( machine_name )
    end

    machine_waza_list = Array.new
    usable_waza_machine_list.each do |machine_name|
      machine_waza_list << $machine_wazaname_hash[ machine_name ]
    end

    # o—Íƒf[ƒ^ì¬
    out_data << "\t\"#{mons_name}\"=>["
    machine_waza_list.each do |waza_name|
      out_data << "\t\t\"#{waza_name}\","
    end
    out_data << "\t],"
  end

  out_data << "}"

  # o—Í
  output_filename = output_path + "machinewaza_hash.rb"
  output_file = File::open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close

end
