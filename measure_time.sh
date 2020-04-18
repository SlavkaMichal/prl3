#!/bin/bash

EXECUTABLE='test.sh'
TEST_FILE='test_data'
RED='\033[0;31m'
GREEN='\033[0;32m'
NOTCOLORED='\033[0m'
PASSED=0
TOTAL=0

if [ ! -f "$EXECUTABLE" ]; then
  >&2 echo "$EXECUTABLE not found!" && exit 1
fi

chmod +x "$EXECUTABLE" || exit 1

#if [ ! -f "$TEST_FILE" ]; then
#  >&2 echo "$TEST_FILE not found!" && exit 1
#fi

#while IFS= read -r line
#do
#  TOTAL=$((TOTAL+1))
#
#  INPUT=$(echo $line | cut -d' ' -f1)
#  TARGET=$(echo $line | cut -d' ' -f2)
#
#  OUTPUT=$(./"$EXECUTABLE" "$INPUT")
#  echo $OUTPUT
#  echo $TARGET
#  if [[ "$OUTPUT" == "$TARGET" ]]
#  then
#    echo -e "${GREEN}$TEST_INPUT passed.${NOTCOLORED}" && PASSED=$((PASSED+1))
#  else
#    echo -e "${RED}$TEST_INPUT failed!${NOTCOLORED}"
#  fi
#
#done < "$TEST_FILE"

for len in {2..30}
do
    # generate random input
    INPUT="$((1+$RANDOM%100))"

    for n in $(eval echo "{1..$len}")
    do
        INPUT="$INPUT,$((1+$RANDOM%(10*n+1)))"
    done

    for x in {1..20}
    do
        #INPLEN=$(($RANDOM%30))

        #echo "INPLEN: $INPLEN"
        #echo "INPUT:  $INPUT"
        #TARGET=$(python3 vid.py -i "$INPUT")
        OUTPUT=$(./"$EXECUTABLE" "$INPUT")
        echo "OUTPUT $OUTPUT"
        #echo "TARGET $TARGET"

        #if [[ "$OUTPUT" == "$TARGET" ]]
        #then
        #  echo -e "${GREEN}$TEST_INPUT passed.${NOTCOLORED}" && PASSED=$((PASSED+1))
        #else
        #  echo -e "${RED}$TEST_INPUT failed!${NOTCOLORED}"
        #fi
    done
done

#echo -e "$PASSED/$TOTAL tests passed!"
