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

if [ ! -f "$TEST_FILE" ]; then
  >&2 echo "$TEST_FILE not found!" && exit 1
fi

while IFS= read -r line; do
  TOTAL=$((TOTAL+1))

  INPUT=$(echo $line | cut -d' ' -f1)
  TARGET=$(echo $line | cut -d' ' -f2)

  OUTPUT=$(./"$EXECUTABLE" "$INPUT")
  echo $OUTPUT
  diff "$OUTPUT" "$TARGET" &>/dev/null && [ $? -eq 0 ] && echo -e "${GREEN}$TEST_INPUT passed.${NOTCOLORED}" && PASSED=$((PASSED+1)) || echo -e "${RED}$TEST_INPUT failed!${NOTCOLORED}"

done < "$TEST_FILE"

echo -e "$PASSED/$TOTAL tests passed!"
