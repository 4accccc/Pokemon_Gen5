require 'fileutils'

keywords = [ "hero", "support", "rival" ]

DUMMYNAME = "hero"

File.open(ARGV[0]){|file|
  file.each{|line|
    name = line.gsub(/"/,"").chomp
    is_not_keep = keywords.reject{|key| name.index(key) == nil }
    if is_not_keep.length != 0 then
      puts "cp #{name} replace/#{name}"
      #FileUtils.cp( name, "replace/" + name )
    else
      if m = /(t[rb]wb_)(\w+)(\.N[A-Z][A-Z][A-Z])/.match(name) then
        replace = "#{m[1]}hero#{m[3]}"
        puts "cp #{replace} replace/#{name}"
        #FileUtils.cp( replace, "replace/" + name )
      else
        raise Exception, name
      end
    end
  }
}
