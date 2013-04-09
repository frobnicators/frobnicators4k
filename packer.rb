dir = ARGV[0]

if dir.nil?
	$stderr.puts "Missing argument: demo folder (ex 'demo')"
	exit -1
end

@tmp_file = "shader_tmp~"

header_file = "generated/shaders.h"
cfile_file = "generated/shaders.c"

@header = File.open(header_file, "w")
@cfile = File.open(cfile_file, "w")

@files = Array.new

def hndl_dir(dir, named_path)
	Dir.foreach("#{dir}") do |f|
		next if f[0] == '.' || f[-1] == "~"

		path = "#{dir}/#{f}"
		if File.file?(path) then
			name = "#{named_path}#{f}"
			next if @files.include? name
			
			# Minifiy
			puts "Minifying #{path}"
			system("./shader_minifier.exe --format none --preserve-externals #{path} -o #{@tmp_file}")
			
			data = IO.read(@tmp_file)
			data = data
				.gsub("\r","")
				.gsub(/\n+/,"\n")
				.gsub(/;\n?!#/,";")
				.gsub(/\n/, "\\n")
			@files.push(name)
			@cfile.puts "	{ \"#{name}\" , \"#{data}\" },";
		elsif File.directory?(path) then
			hndl_dir(path, "#{named_path}#{f}/")
		end
	end
end

@header.puts "#ifndef SHADERS_H"
@header.puts "#define SHADERS_H\n\n"

@header.puts "struct shader_entry_t {
	const char * name;
	const char * data;
};\n\n"


@cfile.puts "#include \"shaders.h\"\n"

@cfile.puts "#pragma data_seg(\".shaders\")\n"

@cfile.puts "const struct shader_entry_t _shaders[NUM_SHADERS] = {";
hndl_dir("shaders/#{dir}", "")
hndl_dir("shaders/shared", "")

@cfile.puts "};\n"

@header.puts "#define NUM_SHADERS #{@files.size}"
@header.puts "extern const struct shader_entry_t _shaders[NUM_SHADERS];\n"

@header.puts "#endif"


puts "Header written to #{header_file}"
puts ".C-file written to #{cfile_file}"