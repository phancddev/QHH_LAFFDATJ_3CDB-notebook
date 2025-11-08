from subprocess import call
import os

tex_file = open("part1.tex").readlines()

# Collect all code files
code_files = []
for (dirpath, dirnames, filenames) in os.walk('.'):
    for x in filenames:
        if x.split('.')[-1] in ["cpp", "java", "sh"]:
            code_files.append((x.split('.')[0], os.path.join(dirpath, x).replace('\\', '/')))

# Sort: judge files first, then others
def sort_key(item):
    name, path = item
    # Judge files go first (priority 0), others go after (priority 1)
    if 'judge' in path.lower():
        return (0, name)
    else:
        return (1, name)

code_files.sort(key=sort_key)

# Add sorted code files to tex
for name, path in code_files:
    tex_file.append("\\code{%s}{%s}\n" % (name, path))

tex_file.extend(open("part2.tex").readlines())
open("build.tex", "w").write("".join(tex_file))
call(["lualatex", "build.tex"])
call(["lualatex", "build.tex"])
