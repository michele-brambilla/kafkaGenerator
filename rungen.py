import subprocess as sp
import sys
import fcntl
import os


print 'Number of arguments:', len(sys.argv), 'arguments.'

cmd = sys.argv
#print 'Argument List:', cmd
cmd[0] = './main'
#print 'Argument List:', cmd
proc = sp.Popen(cmd,
#                stdin =sp.PIPE,
#                stdout=sp.PIPE,
#                stderr=sp.PIPE,
                bufsize = 1)

(output, err) = proc.communicate()
print output, err


