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

cd $FS_MOUNT_POINT || exit


testName="test00WritesFileWithEchoAndReadsWithCat"

echo "hola mundo" > hola.txt

expected="hola mundo"
actual=$(cat hola.txt)

if [[ $expected == "$actual" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}

Expected: ${expected}
     Got: ${actual}
     "
fi

rm hola.txt


testName="test01MkdirThenStatNameShouldBeCorrect"

mkdir mydir
expected="  File: mydir"
actual=$(stat mydir | grep 'File')

if [[ $expected == "$actual" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi

rmdir mydir


testName="test02MkdirThenRmdirThenLsShouldBeEmpty"

mkdir mydir2
rmdir mydir2

expected=""
actual=$(ls)

if [[ $expected == "$actual" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi


testName="test03WriteFiveBytesWithEchoThenStatSizeShouldBeFive"

echo asdf > file.txt
expected="  Size: 5"
actual=$(stat file.txt | grep 'Size'| cut -f 1)

# Trim expected and actual values
expected=$(trim "$expected")
actual=$(trim "$actual")

if [[ $expected == "$actual" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi

rm file.txt


testName="test04WriteFiveBytesWithEchoThenWriteEightAndStatSizeShouldBeEight"

echo hola > file2.txt
echo palabra > file2.txt
expected="  Size: 8"
actual=$(stat file2.txt | grep 'Size'| cut -f 1)

# Trim expected and actual values
expected=$(trim "$expected")
actual=$(trim "$actual")

if [[ $expected = "$actual" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi

rm file2.txt


testName="test05WriteWithEchoAndCopyWithCat"

echo "texto a copiar" > file3.txt
touch file4.txt
cat file3.txt>>file4.txt
expected="texto a copiar"
actual=$(cat file4.txt)

if [[ $expected == "$actual" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi

rm file3.txt
rm file4.txt


testName="test06FileRemoval"

touch file5
rm file5
expected=""
actual=$(ls)

if [[ $expected == "$actual" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi


testName="test07OverwriteFileData"

echo "texto a sobreescribir" > file6.txt
echo "texto que sobreescribe" > file6.txt

expected="texto que sobreescribe"
actual=$(cat file6.txt)

if [[ $expected == "$actual" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi

rm file6.txt


testName="test08NestingThreeDirsThenLsInSecondDirShouldOnlyShowThirdDir"

mkdir mydir3
cd mydir3 || exit
mkdir mydir4
cd mydir4 || exit
mkdir mydir5
cd mydir5 || exit
cd .. || exit

expected="mydir5"
actual=$(ls)

if [[ $expected == "$actual" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi

rmdir mydir5
cd .. || exit
rmdir mydir4
cd .. || exit
rmdir mydir3

testName="test09AppendTextShouldNotOverwrite"

echo "linea que no deberia sobreescribirse" > file7
echo "linea nueva que no sobreescribe la anterior" >> file7

expected="linea que no deberia sobreescribirse\nlinea nueva que no sobreescribe la anterior"
actual=$(cat file7)

expected_trimmed=$(echo -e "$expected" | tr -d '[:space:]')
actual_trimmed=$(echo -e "$actual" | tr -d '[:space:]')

if [[ $expected_trimmed == "$actual_trimmed" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi

rm file7


testName="test10TouchFileThenLsShouldShowFile"

mkdir mydir5
cd mydir5 || exit
touch file8

expected="file8"
actual=$(ls)

if [[ $expected_trimmed == "$actual_trimmed" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi

rm file8
cd .. || exit
rmdir mydir5

testName="test11LsMultipleFiles"

mkdir mydir6
touch file9
echo "hola" > file10.txt
mkdir mydir7

expected="file9  file10  mydir6  mydir7"
actual=$(ls)

if [[ $expected_trimmed == "$actual_trimmed" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi

rm file9
rm file10.txt
rmdir mydir7
rmdir mydir6


testName="test12MultipleCreationWritingAndRemovalOfFile"

for i in $(seq 0 5000)
do
  touch file11
  echo "This is an intentional design decision and not a bug.
  If you have variables that will not have direct references,
  consider using an associative array in bash, or just Ignore the warning.

  Tracking indirect references is a common problem for compilers and
  static analysis tool, and it is known to be unsolvable in the most general case.
  There are two ways to handle unresolved indirections (which in a
  realistic program will essentially be all of them):" > file11
  rm file11
done

echo "texto esperado" >> file11

expected="texto esperado"
actual=$(cat file11)

if [[ $expected == "$actual" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi

rm file11
