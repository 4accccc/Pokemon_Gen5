#!/usr/bin/ruby


require 'net/http'

require 'rubygems'
require 'httpclient'
require 'base64'



##
POSTURI = "http://219.118.175.21:10610/cgi-bin/cgeartest/gsyncget.cgi"

boundary = "123456"
c = HTTPClient.new

postdata = { "name" => "dataget", "mac" => "1" }
r = c.get_content(POSTURI, postdata)
# c.post_content(POSTURI, postdata)

	 handle = File.open("./savetest11.bin",'wb')
	 len = r.length
	 p r
#	 handle.write(r)
	 handle.write( Base64::decode64(r[4,len]) )
	 handle.close()
	 
#c.close()

	 exit(0)


open("/ohno.txt") do |file|
  postdata = { "name" => "michael", "mac" => "1",  "savefile" => file }
##  postdata = { "name" => "michael", "mac" => "12",  "savefile" => "savetxt" }
  puts c.post_content(POSTURI, postdata, "content-type" => "multipart/form-data, boundary=#{boundary}")
		end



	 

exit(0)




require 'rubygems'
require 'httpclient'
POSTURI = "http://219.118.175.21:10610/cgi-bin/cgeartest/gsync.cgi"

boundary = "123456"
c = HTTPClient.new


#open("/ohno.txt") do |file|

postdata = { "name" => "michael", "mac" => "333" }
#puts c.post_content(POSTURI, postdata, "content-type" => "multipart/form-data, boundary=#{boundary}")

puts c.post_content(POSTURI, "name=ohno&save=ruby2&mac=1234567", "content-type" => "multipart/form-data, boundary=#{boundary}")




	 
#		end

exit(0)






#
#require 'rubygems'
#require 'httpclient'

#boundary = "123456"
#c = HTTPClient.new
#open("./Makefile") do |file|
#  postdata = { "first_name" => "michael", "last_name" => "jackson",
#    "file" => file }
#  puts c.post_content(POSTURI, postdata, "content-type" => "multipart/form-data, boundary=#{boundary}")
#end

exit(0)












#!/usr/bin/ruby
## 
##
##



#require 'net/http'
#
#Net::HTTP.version_1_2   # おまじない
#
#Net::HTTP.start('219.118.175.21', 10610){	|http|
#		response = http.post('/cgi-bin/cgeartest/gsync.cgi','name=ohno&save=ruby2&mac=1234567')
#
#			Net::FTP#storbinary(cmd, file, blocksize, rest_offset = nil, callback = nil)
#
#
#		}
#



require 'rubygems'
require 'httpclient'

boundary = "123456"
c = HTTPClient.new

postdata = { "name" => "michael", "mac" => "12" }
puts c.post_content(POSTURI, postdata)
exit(0)


open("/ohno.txt") do |file|
  postdata = { "name" => "michael", "mac" => "12",  "savefile" => file }
##  postdata = { "name" => "michael", "mac" => "12",  "savefile" => "savetxt" }
  puts c.post_content(POSTURI, postdata, "content-type" => "multipart/form-data, boundary=#{boundary}")
		end

