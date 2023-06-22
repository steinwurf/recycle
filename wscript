#! /usr/bin/env python
# encoding: utf-8

APPNAME = "recycle"
VERSION = "7.0.0"


def configure(conf):
    conf.set_cxx_std(14)


def build(bld):
    bld(name="recycle_includes", includes="./src", export_includes="./src")

    if bld.is_toplevel():
        # Only build test when executed from the top-level wscript,
        # i.e. not when included as a dependency
        bld.recurse("test")
