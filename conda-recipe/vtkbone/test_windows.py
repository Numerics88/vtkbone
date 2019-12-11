# This is a custom implementation of nosetests for testing
# the conda build on Python 3.8. The %PATH% variable is no
# longer used for resolving DLLs so they must be added
# using `os.add_dll_directory("%PATH%)"`
#
# Assuming %PREFIX$ is a defined environment variable
# Pass the test to run as normal

# -*- coding: utf-8 -*-
import re
import sys
import os

from nose import run_exit

if __name__ == '__main__':
    PREFIX = os.getenv('PREFIX')
    if hasattr(os, 'add_dll_directory'):
        os.add_dll_directory(os.path.join(
            PREFIX, 'Library', 'lib'
        ))
        os.add_dll_directory(os.path.join(
            PREFIX, 'Library', 'bin'
        ))

    sys.argv[0] = re.sub(r'(-script\.pyw?|\.exe)?$', '', sys.argv[0])
    sys.exit(run_exit())
