def usage 
	$stderr.puts "Usage: ruby packer.rb [options] demo_folder"
	$stderr.puts "Options:"
	$stderr.puts " --keep-tmp: Keep temporaries"
	$stderr.puts " --verbose: Keep temporaries"
	$stderr.puts " --no-minify: Don't minify"
	$stderr.puts " --multiline: Split into multiple lines"
	exit
end

i = 0
@keep_tmp = false
@verbose = ""
@minify = true
@multiline = false
dir = nil
ARGV.each do |a|
	if a =~ /--/ then
		@keep_tmp = true if a == "--keep-tmp"
		@minify = false if a == "--no-minify"
		@verbose = "-v" if a == "--verbose"
		@multiline = true if a == "--multiline"
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

@exports

def preprocess(path) 
	export_fields = []
	@exports = {}
	tmp_name = "." + File.basename(path) + ".tmp"
	out = File.open(tmp_name, "w")
	File.open(path).readlines.each do |l|
		l = l.gsub(/[\r\n]+/,"")
		if l =~ /#pragma export\s*\((.+)\)/ then
			export_fields = export_fields + $1.split(",").map{|s| s.strip }
		else
			export_fields.each do |f|
				if l =~ /^\s*\w+\s+#{f}\s*\(.*\)\s*;/ then
					@exports[f] = l.gsub(/\/\*.*\*\//,"").gsub(/\/\/.*$/,"").strip
					l = "#pragma func(#{f})"
				end
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


			
			# Minifiy
			if @minify then
				puts "	Preprocessing..."
				intermediate_file = preprocess(path)
				puts "	Minifying..."
				tmp_name = "." + File.basename(path) + ".min"
				unless system("./shader_minifier.exe --format none --preserve-all-globals #@verbose --preserve-externals #{intermediate_file} -o #{tmp_name}")
					puts "Minifying failed"
					exit
				end

			else
				tmp_name = path
			end

			puts "	Outputing..."
			
			data = IO.read(tmp_name)
				.gsub("\r","")
				.gsub(/\n+/,"\n")
				.gsub(/;\n(?!#)/,";")
			if @minify then
				@exports.each do |f, l|
					data = data.gsub("#pragma func(#{f})\n", l);
				end
				data = data.gsub(";",";\n").gsub("{","{\n").gsub("}","\n}\n") if @multiline
				IO.write(tmp_name, data)
			end
			data = data
				.gsub(/\n/, "\\n")
			@files.push(name)
			@cfile.puts "	{ \"#{name}\" , \"#{data}\" },"
			puts ""


			if !@keep_tmp && @minify
				File.delete tmp_name
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
