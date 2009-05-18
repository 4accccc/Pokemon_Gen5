require 'filter/parser.rb'
require 'filter/node.rb'

#--- main

parser = PmScript::Parser.new
cenv = PmScript::Enviroment.new
begin
	tree = nil
	if ARGV[0] then
		File.open( ARGV[0] ) do |f|
			tree = parser.parse f
		end
	else
		tree = parser.parse $stdin
	end
	tree.compile cenv
end
