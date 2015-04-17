#!/usr/bin/env python
import os, sys, time, fnmatch

# Linked libraries
LIBRARIES = [
    "sqlite3"
]

# Included directories or files
INCLUDES = [
    "src/include/",
]

# Source files to ignore
FIGNORES = []

# Flags
FLAGS = []

def find_source_files(start_dir, ext=[".cpp", ".c"]):
    result = []
    for (path, folders, files) in os.walk(start_dir):
        for fname in files:
            if any(map(lambda i: fnmatch.fnmatch(fname, i), FIGNORES)):
                continue
            if any(map(lambda i: fname.endswith(i), ext)):
                result.append(os.path.join(path, fname))
    return result

def gen_build_command():
    # gcc -Wall -fPIC -c *.c
    # gcc -shared -Wl,-soname,libctest.so.1 -o libctest.so.1.0   *.o

    base = ["g++ -Wall -fPIC -c"]
    base.append(" ".join(find_source_files(".")))
    base.append(" ".join(["-I%s" % i for i in INCLUDES]))
    base.append(" ".join(["-l%s" % i for i in LIBRARIES]))
    base.append(" ".join(["-D%s" % i for i in FLAGS]))
    base.append("-L/usr/local/lib")
    # base.append("-o %s" % BINARY_NAME)
    return " ".join(base)

def build():
    ghash = os.popen("git rev-parse --verify HEAD").read().strip()
    FLAGS.append('GIT_HASH=\\"%s\\"' % ghash)

    start = time.time()
    i = os.system(gen_build_command())
    i = i and os.system("gcc -shared -Wl,-soname,libsqueel.so -o libsqueel.o *.o")

    return not bool(i), (time.time() - start)

if __name__ == "__main__":
    if len(sys.argv) <= 1:
        print "Usage: ./build.py <build|run|setup>"
        sys.exit(1)

    if sys.argv[1] == "build":
        print "Building..."
        success, dura = build()
        if success:
            print "FINISHED! Took %ss" % dura

    if sys.argv[1] == "run":
        print "Building and running..."
        success, _ = build()
        if not success:
            print "Errors during build, not running..."
            sys.exit(1)

        os.system("./%s %s" % (BINARY_NAME, ' '.join(sys.argv[2:])))
