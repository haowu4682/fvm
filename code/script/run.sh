#!/bin/sh
fvm $1 < $2
#echo "server localhost 1096\nlink /home/haowu/test/test_repo.git / /home/haowu/test/test_repo_mount\nexit\n" | bin/fvm haowu haowu@cs.utexas.edu
#echo "server localhost 1096\ncommit /home/haowu/test/test_repo.git / /home/haowu/test/test_repo_mount\nexit\n" | bin/fvm haowu haowu@cs.utexas.edu
