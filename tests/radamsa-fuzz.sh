# Requires the radamsa fuzzer to be installed

#ARGC=("$#")
#
#if [ "$ARGC" -ne 2 ]; then
#     printf  "\nUsage INPUT_FILE PROGRAM_TO_TEST\n"
#     exit
#fi;

input_file=$1
program=$2

while true
do
  radamsa "$input_file" > fuzzed.las
   "$program" fuzzed.las > /dev/null
  test $? -gt 127 && break
done
