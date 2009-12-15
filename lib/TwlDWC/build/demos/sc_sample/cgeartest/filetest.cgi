#!/usr/bin/ruby

require 'cgi'
require 'mysql'

begin
cgi = CGI.new


dataname="ok"
macadd = 1
databin="ok"

 dbase = Mysql::new("localhost", "ohno", "katsumi", "pokemonsavedatas")
 dbase.query("replace into wbsavedata (name,mac,data) values ('#{dataname}', #{macadd}, "#{databin}")")
 dbase.close


#
#cgi = CGI.new
#cgi.out("type"=>"text/plain") {
#  cgi["file"].read
#}
end
exit(0)


###!/usr/local/bin/ruby
##require 'cgi'
##cgi = CGI.new
##cgi.out("type"=>"text/plain") {
##  cgi["file"].read
##}
