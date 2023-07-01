FS_MOUNT_POINT="prueba"
RED='\033[0;31m'
NO_COLOR='\033[0m'
GREEN='\033[0;32m'

# Function to trim leading/trailing whitespace
trim() {
    local var=$1
    var="${var#"${var%%[![:space:]]*}"}"
    var="${var%"${var##*[![:space:]]}"}"
    echo -n "$var"
}

cd $FS_MOUNT_POINT


testName="test00WritesFileWithEchoAndReadsWithCat"

echo "hola mundo" > hola.txt

expected="hola mundo"
actual=$(cat hola.txt)

if [[ $expected = $actual ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}"
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}"
fi

rm hola.txt


testName="test01MkdirThenStatNameShouldBeCorrect"

mkdir mydir
expected="  File: mydir"
actual=$(stat mydir | grep 'File')

if [[ $expected = $actual ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}"
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}"
fi

rmdir mydir


testName="test02MkdirThenRmdirAndStatNameShouldNotBeCorrect"

mkdir mydir2
rmdir mydir2
notExpected="  File: mydir2"
actual=$(stat mydir2 | grep 'File')

if [[ $notExpected != $actual ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}"
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Not Expected: ${notExpected}
         Got: ${actual}"
fi


testName="test03WriteFiveBytesStatSizeShouldBeFive"

echo asdf > file.txt
expected="  Size: 5"
actual=$(stat file.txt | grep 'Size'| cut -f 1)

# Trim expected and actual values
expected=$(trim "$expected")
actual=$(trim "$actual")

if [[ $expected = $actual ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}"
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}"
fi

rm file.txt


testName="test04WriteFiveBytesThenWriteEightStatSizeShouldBeEight"

echo hola > file2.txt
echo palabra > file2.txt
expected="  Size: 8"
actual=$(stat file.txt | grep 'Size'| cut -f 1)

# Trim expected and actual values
expected=$(trim "$expected")
actual=$(trim "$actual")

if [[ $expected = $actual ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}"
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}"
fi

rm file2.txt