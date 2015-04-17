#!/usr/bin/env python
import os, sys, time, fnmatch

# Linked libraries
LIBRARIES = [
    "sqlite3",
    "lua",
    "m"
]

# Included directories or files
INCLUDES = [
    "src/include/",
    "/usr/include/lua5.1"
]

# Source files to ignore
FIGNORES = []

# Flags
FLAGS = [
]

def find_source_files(start_dir, ext=[".cpp", ".c"]):
    result = []
    for (path, folders, files) in os.walk(start_dir):
        for fname in files:
            if any(map(lambda i: fnmatch.fnmatch(fname, i), FIGNORES)):
                continue
            if any(map(lambda i: fname.endswith(i), ext)):
                result.append(os.path.join(path, fname))
    return result

def generate_compile_command():
    base = ["g++ -std=c++11 -Wall -fPIC -u symbol -c %s" % ' '.join(find_source_files('.'))]

    base.append(" ".join(["-I%s" % i for i in INCLUDES]))
    base.append(" ".join(["-l%s" % i for i in LIBRARIES]))
    base.append(" ".join(["-D%s" % i for i in FLAGS]))
    base.append("-L/usr/local/lib")

    return " ".join(base)

def generate_combine_command():
    # TODO: lolwut
    return "g++ -shared -o squeel *.o /usr/lib/x86_64-linux-gnu/liblua5.1.so"

def build():
    ghash = os.popen("git rev-parse --verify HEAD").read().strip()
    FLAGS.append('GIT_HASH=\\"%s\\"' % ghash)

    if os.path.exists("squeel.so"):
        os.system("rm squeel.so")

    start = time.time()
    os.system(generate_compile_command())
    os.system(generate_combine_command())

    os.system("rm *.o")
    os.system("mv squeel squeel.so")
    return (time.time() - start)

if __name__ == "__main__":
    if len(sys.argv) <= 1:
        print "Usage: ./build.py <build|run|setup>"
        sys.exit(1)

    if sys.argv[1] == "build":
        print "Building..."
        dura = build()
        print "FINISHED! Took %ss" % dura

    if sys.argv[1] == "run":
        print "Building and running..."
        success, _ = build()
        if not success:
            print "Errors during build, not running..."
            sys.exit(1)

        os.system("./%s %s" % (BINARY_NAME, ' '.join(sys.argv[2:])))
