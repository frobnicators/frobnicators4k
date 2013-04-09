def usage 
	$stderr.puts "Usage: ruby packer.rb [options] demo_folder"
	$stderr.puts "Options:"
	$stderr.puts " --keep-tmp: Keep temporaries"
	$stderr.puts " --no-minify: Don't minify"
	$stderr.puts " --no-preprocess: Don't replace defines"
	exit
end

i = 0
@keep_tmp = false
@minify = true
@preprocess = true
dir = nil
ARGV.each do |a|
	if a =~ /--/ then
		@keep_tmp = true if a == "--keep-tmp"
		@minify = false if a == "--no-minify"
		@preprocess = false if a == "--no-preprocess"
		usage if a == "--help"
	else
		dir = a
	end
end

usage if dir.nil?

header_file = "generated/shaders.h"
cfile_file = "generated/shaders.c"

@header = File.open(header_file, "w")
@cfile = File.open(cfile_file, "w")

@files = Array.new

def preprocess(path) 
	tmp_name = "." + File.basename(path) + ".tmp"
	defines = {}
	out = File.open(tmp_name, "w")
	File.open(path).readlines.each do |l|
		if l =~ /#define (.+) (.+)$/ then
			defines[$1] = $2
		else
			defines.each do |define, val|
				l = l.gsub(/(?<=\W)#{define}(?=\W)/,val)
			end
			out.puts l
		end
	end
	out.close
	return tmp_name
end

def hndl_dir(dir, named_path)
	Dir.foreach("#{dir}") do |f|
		next if f[0] == '.' || f[-1] == "~"

		path = "#{dir}/#{f}"
		if File.file?(path) then
			name = "#{named_path}#{f}"
			next if @files.include? name

			puts "#{path}:"

			if @preprocess then
				puts "	Preprocessing..."
				intermediate_file = preprocess(path)
			else
				intermediate_file = path
			end

			
			# Minifiy
			if @minify then
				puts "	Minifying..."
				tmp_name = "." + File.basename(path) + ".min"
				system("./shader_minifier.exe --format none --preserve-all-globals --preserve-externals #{intermediate_file} -o #{tmp_name}")
			else
				tmp_name = intermediate_file
			end

			puts "	Outputing..."
			
			data = IO.read(tmp_name)
			data = data
				.gsub("\r","")
				.gsub(/\n+/,"\n")
				.gsub(/\n/, "\\n")
			@files.push(name)
			@cfile.puts "	{ \"#{name}\" , \"#{data}\" },"
			puts ""

			unless @keep_tmp
				File.delete intermediate_file if @preprocess
				File.delete tmp_name if @minify
			end

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
