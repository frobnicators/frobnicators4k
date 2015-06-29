def usage 
	$stderr.puts "Usage: ruby packer.rb [options] demo_folder"
	$stderr.puts "Options:"
	$stderr.puts " --keep-tmp: Keep temporaries"
	$stderr.puts " --verbose: Be verbose"
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
cfile_file = "generated/shaders.inc"

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
			# export pragma found, add to export_fields list
			export_fields = export_fields + $1.split(",").map{|s| s.strip }
		else
			# check if the current line matches any fields, if so remove them
			# from the exports list, and push them to the @exports list
			export_fields.reject! do |f|
				if l =~ /^\s*\w+\s+#{f}\s*\(.*\)\s*;/ then
					@exports[f] = l.gsub(/\/\*.*\*\//,"").gsub(/\/\/.*$/,"").strip
					l = "#pragma func(#{f})"
					true
				else
					false
				end
			end
			out.puts l
		end
	end
	out.close

	if !export_fields.empty?
		puts "Error, unknown function(s) " + export_fields.join(",") + " declared with #pragma export."
		exit
	end

	return tmp_name
end

def define_name(name) 
	"SHADER_" + name.upcase.gsub(/\W/,"_")
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
			@cfile.puts "	{ \"#{data}\" },"
			puts ""


			if !@keep_tmp && @minify
				File.delete tmp_name
				File.delete intermediate_file
			end

		elsif File.directory?(path) then
			hndl_dir(path, "#{named_path}#{f}/")
		end
	end
end

@cfile.puts "#pragma data_seg(push, \".shaders\")\n"

@cfile.puts "const char * _shaders[] = {";
hndl_dir("shaders/#{dir}", "")
hndl_dir("shaders/shared", "")

@cfile.puts "};\n"
@cfile.puts "#pragma data_seg(pop)\n"

# Time to build the header

processed_filenames = @files.map do |f|
	[ define_name(f), f ]
end

@header.puts "#ifndef SHADERS_H
#define SHADERS_H

#if _DEBUG

#define SHADER_TYPE const char *

"
processed_filenames.each do |f|
	@header.puts "#define #{f[0]} \"#{f[1]}\""
end

@header.puts "
#else

#define SHADER_TYPE char

"

processed_filenames.each_with_index do | f , index |
	@header.puts "#define #{f[0]} #{index}"
end

@header.puts "
#endif

#endif"


puts "Header written to #{header_file}"
puts ".C-file written to #{cfile_file}"
