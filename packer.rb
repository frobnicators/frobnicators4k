dir = ARGV[0]

if dir.nil?
	$stderr.puts "Missing argument: demo folder (ex 'demo')"
	exit -1
end

@files = Array.new

@common = nil
if File.exists? "data/#{dir}/shaders/common.glsl" then
	@common = File.read("data/#{dir}/shaders/common.glsl")
else
	@common = File.read("data/shared/shaders/common.glsl")
end

# Put common.glsl in files list so it is not included
@files.push("shaders/common.glsl")



def hndl_dir(dir, named_path)
	Dir.foreach("#{dir}") do |f|
		if f[0] != '.' then
			path = "#{dir}/#{f}"
			if File.file?(path) then
				name = "#{named_path}#{f}"
				next if @files.include? name

				size = File.size(path)
				if(name[-4..-1] == "glsl") then
					data = @common + "\n" +IO.read(path)
					data = data.gsub("\r", "")
				elsif name[-1] != "~" then
					data = IO.binread(path)
				else
					next
				end

				data = data.gsub("\\", "\\\\").gsub("\n", "\\n").gsub("\r", "\\r").gsub("\"", "\\\"")
				@files.push(name)
				puts "	{ \"#{name}\" , \"#{data}\", #{size} },";
			elsif File.directory?(path) then
				hndl_dir(path, "#{named_path}#{f}/")
			end
		end
	end
end

puts "struct data_t files[] = {";
hndl_dir("data/#{dir}", "")
hndl_dir("data/shared", "")
puts "};"

puts "const int num_files = #{@files.size};"

$stderr.puts "Done"
