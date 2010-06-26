#!ruby -Ks

require 'fileutils'

require '../personal/monsno_hash.rb'

require '../playable_keepmons.rb'

keepno = [ "000" ]

$keep_monsname.each{|name|
  if $monsno_hash.has_key?(name) then
    #keepno << sprintf("%03d", $monsno_hash[name])
    keepno << $gra2zukan_hash.invert[ $monsno_hash[name] ]
  end
}

def make_replace_list( keepno, filename )
  namelist = []
  File.open(filename){|file|
    file.each{|org_line|
      line = org_line.gsub(/"/,"").chomp
      is_not_keep = keepno.reject{|numstr| line.index(numstr) == nil }
      if is_not_keep.length != 0 || line =~/p[fbm]wb_egg/ then
        namelist << nil
        #puts "keep #{line}"
      else
        if m = /(p[fbm]wb_)(\d\d\dc?)(_[a-z]+)?(_[mfnr])?(\.N[A-Z][A-Z][A-Z])/.match(line)
          if m[3] == nil then
            newname = m[1] + m[2].sub(/\d\d\d/,"000") + m[5]
          elsif m[4] == nil then
            if m[3] =~/_[mfnr]$/ then
              newname = m[1] + m[2].sub(/\d\d\d/,"000") + m[3] + m[5]
            else
              newname = m[1] + m[2].sub(/\d\d\d/,"000") + m[5]
            end
          else
            newname = m[1] + m[2].sub(/\d\d\d/,"000") + m[4] + m[5]
          end
        else
          raise Exception, line
        end
        #newname = line.sub(/\d\d\d/,"000")
        #puts "replace #{line.chomp} --> #{newname}"
        namelist << [ line, newname ]
      end
    }
  }
  namelist
end

lists = make_replace_list( keepno, "pokegra_wb.scr" )
lists.each{|item|
  if item != nil then
    puts "cp #{item[1]} #{item[0]}"
    #FileUtils::rm item[0]
    FileUtils::cp( item[1], item[0] )
  end
}
p keepno
