#!/usr/bin/python
#
# Copyright (c) 2011, The Native Client Authors.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import optparse

# This script generates a JSON .nmf file, which provides the mapping to indicate
# which .nexe file to load and execute for a particular architecture.
# The script must have -nmf <filename> as an option, which designates the name
# of the .nmf file to be generated.
# One or more nexes must be specified on the command line.  Each
# nexe file is preceded by an argument that specifies the architecture
# that the nexe is associated with: --x86-64, --x86-32, --arm.
#
# For example:
#   generate_nmf.py --nmf test.nmf --x86-64 hello_world_x86-64.nexe \
#     --x86-32 hello32.nexe
# will create test.nmf that contains 2 entries, while
#
#   generate_nmf.py --nmf hello.nmf --arm arm.nexe
#
# will create hello.nmf with a single entry.

# Note: argv has been passed in without the program name in argv[0]
def main(argv):
  parser = optparse.OptionParser()
  parser.add_option('--nmf', dest='nmf_file', help='nmf file to generate')
  parser.add_option('--x86-64', dest='x86_64', help='x86_64 nexe')
  parser.add_option('--x86-32', dest='x86_32', help='x86_32 nexe')
  parser.add_option('--arm', dest='arm', help='arm nexe')

  (options, args) = parser.parse_args(argv)

  if options.nmf_file == None:
    parser.error("nmf file not specified.  Use --nmf")
  # Make sure that not all nexes are None -- i.e. at least one was specified.
  if options.x86_64 == None and options.x86_32 == None and options.arm == None:
    parser.error("No nexe files were specified")

  nmf_file = open(options.nmf_file, 'w')
  nmf_file.write('{\n')
  nmf_file.write('  "nexes": {\n')

  # Output an entry in the manifest file for each specified architecture
  if options.x86_64:
    nmf_file.write('    "x86-64": "%s",\n' % options.x86_64)
  if options.x86_32:
    nmf_file.write('    "x86-32": "%s",\n' % options.x86_32)
  if options.arm:
    nmf_file.write('    "arm": "%s",\n' % options.arm)

  nmf_file.write('  }\n')
  nmf_file.write('}\n')
  nmf_file.close()

if __name__ == '__main__':
  main(sys.argv[1:])
