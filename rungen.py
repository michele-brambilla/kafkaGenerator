import subprocess as sp
import sys
import fcntl
import os

proc = sp.Popen('./main',
#                stdin =sp.PIPE,
#                stdout=sp.PIPE,
#                stderr=sp.PIPE,
                bufsize = 1)

(output, err) = proc.communicate()
print output, err


