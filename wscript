#! /usr/bin/env python
# encoding: utf-8

APPNAME = 'recycle'
VERSION = '1.1.1'

import waflib.extras.wurf_options


def options(opt):

    opt.load('wurf_common_tools')


def configure(conf):

    import waflib.extras.wurf_dependency_resolve as resolve

    conf.load("wurf_common_tools")

    conf.add_dependency(resolve.ResolveVersion(
        name='waf-tools',
        git_repository='github.com/steinwurf/waf-tools.git',
        major=2))

    # Internal dependencies
    if conf.is_toplevel():

        conf.add_dependency(resolve.ResolveVersion(
            name='gtest',
            git_repository='github.com/steinwurf/gtest.git',
            major=2))


def build(bld):

    bld.load("wurf_common_tools")

    bld.env.append_unique(
        'DEFINES_STEINWURF_VERSION',
        'STEINWURF_RECYCLE_VERSION="{}"'.format(VERSION))

    bld(name='recycle_includes',
        includes='./src',
        export_includes='./src')

    if bld.is_toplevel():

        # Only build test when executed from the top-level wscript,
        # i.e. not when included as a dependency
        bld.recurse('test')
