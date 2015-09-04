#!/usr/bin/env python
# encoding: utf-8

import os
import sys
import json
import subprocess

project_name = 'recycle'


def run_cmd(args):
    print("Running: {}".format(args))
    subprocess.check_call(args)


def configure(properties):
    command = [sys.executable, 'waf', 'configure']

    if properties.get('build_distclean'):
        command += ['distclean']

    command += ['--git-protocol=git@']

    if 'waf_bundle_path' in properties:
        command += ['--bundle-path=' + properties['waf_bundle_path']]

    if 'dependency_project' in properties:
        command += ['--{0}-use-checkout={1}'.format(
            properties['dependency_project'],
            properties['dependency_checkout'])]

    options = "--options=cxx_mkspec={}".format(properties['cxx_mkspec'])

    if 'tool_options' in properties:
        # Make sure that the values are correctly comma separated
        for key, value in properties['tool_options'].iteritems():
            if value is None:
                options += ',{0}'.format(key)
            else:
                options += ',{0}={1}'.format(key, value)

    command += [options]

    run_cmd(command)


def build(options):
    pass


def run_tests(options):
    pass


def coverage_settings(options):
    options['required_line_coverage'] = 100.0


def main():
    argv = sys.argv

    if len(argv) != 3:
        print("Usage: {} <command> <options>".format(argv[0]))
        sys.exit(0)

    cmd = argv[1]
    if cmd == 'configure':
        configure(json.loads(argv[2]))
    else:
        print("Unknown command: {}".format(cmd))


if __name__ == '__main__':
    main()
