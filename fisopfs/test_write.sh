FS_MOUNT_POINT="prueba"
RED='\033[0;31m'
NO_COLOR='\033[0m'
GREEN='\033[0;32m'

cd $FS_MOUNT_POINT

testName="test00WritesFileWithEchoAndReadsWithCat"

echo "hola mundo" > hola.txt

expected="hola mundo"
actual=$(cat hola.txt)

if [[ $expected = $actual ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}"
else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName} expected: ${expected} but got: ${actual}."
fi

rm hola.txt