
DATA_TYPE = {
  "u8" => "C",
  "u16" => "S",
  "u32" => "I",
}

File.open(ARGV[0]){|file|
  strout = ""
  first_line = file.gets.chomp.split(",")
  if first_line.shift != "START" then
    raise Exception, "フォーマットが正しくありません"
  end
  first_line.each{|item|
    unless DATA_TYPE.has_key?(item) then
      p first_line
      raise Exception, "#{item}はデータタイプとして定義されていません"
    end
  }
  file.each{|line|
    column = line.chomp.split(",")
    if column.shift == "END" then
      break
    end
    column.each_with_index{|item, idx|
      type_str = DATA_TYPE[ first_line[idx] ]
      #p item
      #p type_str
      strout += [item.to_i].pack( type_str )
    }
    File.open(ARGV[1],"wb"){|file|
      file.write( strout)
    }
  }
}
