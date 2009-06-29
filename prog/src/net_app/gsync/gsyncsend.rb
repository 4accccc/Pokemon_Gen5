#!/usr/bin/ruby


require 'net/http'

require 'rubygems'
require 'httpclient'
require 'base64'


p Time::now

boundary='-----------------------3948A8'



contents = File::open("./dp_1219.bin",'rb').read

contentstxt = Base64::encode64(contents)

#handle = File.open("./savetest01.bin",'wb')
##binarray =  ##contentstxt.unpack('m')[0]
#handle.write( Base64::decode64(contentstxt) )
#handle.write(1)
#handle.close()



data =  %[--#{boundary}] + "\r\n"
data << %[Content-Disposition: form-data; name="file1"; filename="txt.txt"] + "\r\n"
data << %[Content-Type: application/octet-stream] + "\r\n\r\n"

data << contentstxt
data << %[\r\n--#{boundary}--\r\n]

header ={
  'Content-Length' => data.length.to_s,
  'Content-Type' => "multipart/form-data; boundary=#{boundary}"
}

Net::HTTP.start('219.118.175.21', 10610) do |http|
  r= http.post('/cgi-bin/cgeartest/gsync.cgi', data, header)
  #print r.body
end

p Time::now

	 exit(0)

open("/ohno.txt") do |file|
  postdata = { "name" => "michael", "mac" => "12",  "savefile" => file }
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

