# -*- coding: utf-8 -*-

from __future__ import print_function
from __future__ import with_statement
import io
import os
import sys
import stat
import subprocess


KNOWN_TEXT_SUFFIX = [
    "c", "C", "cpp", "cxx", "cc",
    "h", "H", "hpp", "hxx", "hh",
    "sh", "bash", "py",
    "txt", "md",
]

KNOWN_TEXT_FILENAME = [
    "LICENSE",
]

KNOWN_BINARY_SUFFIX = [
]

KNOWN_BINARY_FILENAME = [
]

KNOWN_EXECUTABLE_SUFFIX = [
    "bash",
]


def run(cmd):
    ps = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    ps.wait()
    out = ps.stdout.read()
    err = ps.stderr.read()
    if ps.returncode != 0:
        raise RuntimeError("`%s` exited with %d. \n==== stdout ====\n%s\n==== stderr ====\n%s" % (cmd, ps.returncode, out, err))
    return (out, err)

def check_file(file, dofix):
    filename = os.path.basename(file)
    fileext = os.path.splitext(filename)[1]
    if fileext:
        assert fileext[0] == '.'
        fileext = fileext[1:]
    success = True

    # Check: file permission (executable or not)
    mode = os.stat(file).st_mode
    if fileext in KNOWN_EXECUTABLE_SUFFIX:
        expect_mode = mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH
    else:
        expect_mode = mode & (~stat.S_IXUSR) & (~stat.S_IXGRP) & (~stat.S_IXOTH)
    if mode != expect_mode:
        if dofix:
            print("[fix] %s: chmod %s -> %s" % (file, oct(mode)[1:], oct(expect_mode)[1:]))
            os.chmod(file, expect_mode)
        else:
            print("%s: permission %s, expect %s" % (file, oct(mode)[1:], oct(expect_mode)[1:]))
            success = False

    # Check: file ends with LF (if text)
    if fileext in KNOWN_TEXT_SUFFIX or filename in KNOWN_TEXT_FILENAME:
        is_text = True
    elif fileext in KNOWN_BINARY_SUFFIX or filename in KNOWN_BINARY_FILENAME:
        is_text = False
    else:
        print("%s: is it text or binary? modify %s" % (file, __file__))
        is_text = False  # Don't do text line-end test
        success = False

    if is_text:
        with io.open(file, "r", encoding="utf8", newline='') as fp:
            lines = fp.readlines()

        file_dofix = False
        lineno = 0
        for line in lines:
            lineno += 1
            line_txt = line
            line_end = ""
            while line_txt and line_txt[-1] in ('\r', '\n'):
                line_end = line_txt[-1] + line_end
                line_txt = line_txt[:-1]
            assert line_end

            if line_end != "\n":
                if dofix:
                    print("[fix] %s:%d: change newline to <LF>" % (file, lineno))
                    line_end = "\n"
                    file_dofix = True
                else:
                    print("%s:%d: doesn't end with <LF>" % (file, lineno))
                    success = False
            if line_txt and line_txt[-1] in (' ', '\t'):
                if dofix:
                    print("[fix] %s:%d: trim <SPACE> and <TAB> at line end" % (file, lineno))
                    while line_txt and line_txt[-1] in (' ', '\t'):
                        line_txt = line_txt[:-1]
                    file_dofix = True
                else:
                    print("%s:%d: found extra <SPACE> or <TAB> at line end" % (file, lineno))
                    success = False
            if line_txt and line_txt[0] == '\t':
                if dofix:
                    print("[fix] %s:%d: found <TAB> at line begin" % (file, lineno))
                else:
                    print("%s:%d: found <TAB> at line begin (manual fix!)" % (file, lineno))
                success = False

            line = line_txt + line_end
            lines[lineno - 1] = line

        # Check last lines
        if lines:
            if lines[-1][-1] !='\n':
                if dofix:
                    print("[fix] %s: add <LF> after last line" % (file,))
                    lines[-1] += '\n'
                    file_dofix = True
                else:
                    print("%s: expect <LF> after last line" % (file,))
                    success = False
            elif lines[-1] == "\n":
                if dofix:
                    print("[fix] %s: trim extra empty line(s) at file end" % (file,))
                    while lines and lines[-1] == "\n":
                        lines = lines[:-1]
                    file_dofix = True
                else:
                    print("%s: found extra empty line(s) at file end" % (file,))
                    success = False

        # Fix file if necessary
        if file_dofix:
            assert dofix
            with io.open(file, "w", encoding="utf8", newline='') as fp:
                fp.writelines(lines)

    return success

def main(args):
    assert len(args) <= 1
    assert not args or args[0] in ["--fix"]
    dofix = args and (args[0] == "--fix")

    success = True
    out, _ = run("git ls-files")
    files = [file for file in out.split('\n') if file]
    for file in files:
        if not os.path.islink(file):
            assert os.path.isfile(file)
            success = check_file(file, dofix) and success
    return 0 if success else 1


if __name__ == "__main__":
    # cd dirname(__file__)/../..
    os.chdir(os.path.dirname(os.path.dirname(os.path.dirname(__file__))))
    sys.exit(int(main(sys.argv[1:]) or 0))
