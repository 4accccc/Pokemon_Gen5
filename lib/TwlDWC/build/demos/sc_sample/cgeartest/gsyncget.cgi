#!/usr/bin/ruby
## 
##  データをDSに返すスクリプト
##
require 'cgi'
require 'mysql'


cgi = CGI.new



dataname = cgi["name"]   # => database uniqe name
macadd = cgi["mac"]   # => database uniqe name

puts "Content-type: text/html\n\n"

##cgi.close()

if dataname == "dataget"
  dbase = Mysql::new("localhost", "ohno", "katsumi", "pokemonsavedatas")
  res = dbase.query("select data from wbsavedata where mac in (#{macadd});")
  res.each_hash do |row|
    puts row
  end
 dbase.close
end

sendx = 0
if dataname == "data1"
  sendx = 0
end
if dataname == "data2"
  sendx = 1
end
if dataname == "data3"
  sendx = 2
end
if dataname == "data4"
  sendx = 3
end
if dataname == "data5"
  sendx = 4
end
if dataname == "data6"
  sendx = 5
end
if dataname == "data7"
  sendx = 6
end



  dbase = Mysql::new("localhost", "ohno", "katsumi", "pokemonsavedatas")
  res = dbase.query("select data from wbsavedata where mac in (#{macadd});")
  res.each do |row|
       #128Kづつ転送 ##       len = row[0].length   ##710703
      stx = sendx*131072
      enx = (sendx+1)*131072

      puts row[0][stx..enx]
  end
 dbase.close


cgi.close()
	 
exit(0)



