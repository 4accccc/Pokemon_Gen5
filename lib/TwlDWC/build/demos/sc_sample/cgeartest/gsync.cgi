#!/usr/bin/ruby
## 
##
##

require 'cgi'
require 'mysql'
require 'base64'

begin
## connect 

cgi = CGI.new


files = cgi["file1"]
names = cgi["name1"]


print "Content-type: text/html\n\n"

puts "filename: " + files.original_filename

#databin = files.read

#handle = File.open(files.original_filename,"wb")
#	 handle.write(files.read)
#	 handle.close


puts "----------------"
# puts files.read
puts "----------------"

#exit(0)


dataname = names.read

## name=ohno&mac=123456789012&save=popopohatopop
##dataname = cgi["name"]   # => database uniqe name
##databin = cgi.params["savefile"][0]   # => poke savedata

#databin = "dummy"    ###cgi["savefile"]   # => poke savedata

#macadd = cgi["mac"]   # => database uniqe name


#print "Content-type: text/html\n\n"
#p cgi["mac"]

#print "<html><body>Dummy Result</body></html>"


##cgi.out("type"=>"text/plain") {
  ##databin = cgi["file"].read
##}

#test
#p dataname
#p databin
#p macadd

##database name mac data id=autoinc
## insert into savedata (name,mac,data) values (ohno, 123456781212, 1234567788);

#dataname = "writetest"
macadd = 1

#handle = File::open("./filetest.cgi",'r')
#	 databin = b64encode(handle.read())
	 databin = files.read ##b64encode(files.read)
#	 handle.close()

	 
	# databin = "datadummy3"

dbase = Mysql::new("localhost", "ohno", "katsumi", "pokemonsavedatas")
	 dbase.query("replace into wbsavedata (name,mac,data) values ('#{dataname}', #{macadd}, '#{databin}')")
	 dbase.close
	 exit(0)
	 end

