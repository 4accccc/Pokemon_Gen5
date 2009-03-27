root = ENV["PROJECT_ROOT"]
result =`svnversion #{root}`.chomp
puts "SVN_REVISION=\'\"#{result}\"\'"
