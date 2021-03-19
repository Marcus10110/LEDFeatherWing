from os.path import isfile, join
from pathlib import Path
import os 

def encode_string(s, encoding='ascii'):
   if isinstance(s, str):
      s = s.encode(encoding)
   result = ''
   for c in s:
    print(c)
    if not (32 <= c < 127) or c in ('\\', '"'):
        result += '\\%03o' % c
    elif chr(c) == '"':
        result += '\\"'
    else:
        result += chr(c)
   return '"' + result + '"'

# Let's open the index.html file, and create a *.cpp and *.h file with the content.

output_file_cpp = 'generated.cpp'
output_file_h ='generated.h'
h_content = '#pragma once\n\nnamespace Content {'
cpp_content = '#include "{}";\n\nnamespace Content {{'.format(output_file_h)


def processFile(path):
    source_content = ''
    with open(path, 'r') as file:
        source_content = file.read()
        file.close()

    content = encode_string(source_content)
    var_name = Path(path).stem;
    global h_content
    global cpp_content
    h_content += '\n    extern const char* {};\n'.format(var_name)
    cpp_content += '\n    const char* {} = {};\n'.format(var_name, content)
    

allowed_extensions = ['.html']
www_directory = os.path.dirname(os.path.realpath(__file__))
src_directory = join(www_directory, '../src')
print(src_directory)
all_files = [f for f in os.listdir(www_directory) if isfile(join(www_directory, f)) and Path(f).suffix in allowed_extensions]
for file in all_files:
    processFile(join(www_directory, file))
    print(file)

h_content += '}\n'
cpp_content += '}\n'
with open(join(src_directory,output_file_cpp), 'w') as file:
    file.write(cpp_content)
    file.close()
with open(join(src_directory,output_file_h), 'w') as file:
    file.write(h_content)
    file.close()