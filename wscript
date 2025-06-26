#! /usr/bin/env python
# encoding: utf-8

APPNAME = "recycle"
VERSION = "7.0.0"

def options(ctx):
    ctx.load("cmake")

def configure(ctx):

    ctx.load("cmake")

    if ctx.is_toplevel():
        ctx.cmake_configure()


def build(ctx):

    ctx.load("cmake")

    if ctx.is_toplevel():
        ctx.cmake_build()

