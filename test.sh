./minishell <<EOF
ls
pwd
date
ls -l
echo Hello, World!
ls | grep minishell
cd /tmp
pwd
cd ..
pwd
cd non_existing_directory
sleep 5 &
nonexistentcommand
exit
EOF