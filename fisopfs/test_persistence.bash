RED='\033[0;31m'
NO_COLOR='\033[0m'
GREEN='\033[0;32m'

TP_DIR=$(pwd)

testName="test00MountThenTouchFileThenUmountThenMountAgaintLsShouldShowFile"

mkdir ptest

echo prueba.fisopfs > ./fisopfs ptest/
cd ptest || exit

touch pfile

cd .. || exit
umount ptest

echo prueba.fisopfs > ./fisopfs ptest/
cd ptest || exit

expected="pfile"
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

rm pfile
cd .. || exit
umount ptest
rmdir ptest


testName="test01DirPersistence"

mkdir ptest

echo prueba.fisopfs > ./fisopfs ptest/
cd ptest || exit

mkdir pdir1

cd "$TP_DIR" || exit
umount ptest

echo prueba.fisopfs > ./fisopfs ptest/
cd ptest || exit

expected="pdir1"
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

rmdir pdir1
cd "$TP_DIR" || exit
umount ptest
rmdir ptest


testName="test02RecursionLevelsPersistence"

mkdir ptest

echo prueba.fisopfs > ./fisopfs ptest/
cd ptest || exit

mkdir pdir3
cd pdir3 || exit
mkdir pdir4
cd pdir4 || exit
touch pfile1

cd "$TP_DIR" || exit
umount ptest

echo prueba.fisopfs > ./fisopfs ptest/
cd ptest || exit
cd pdir3 || exit
cd pdir4 || exit

expected="pfile1"
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

rm pfile1
cd .. || exit
rmdir pdir4
cd .. || exit
rmdir pdir3

cd "$TP_DIR" || exit
umount ptest
rmdir ptest


testName="test03WritingAFileThenTheContentIsPersisted"

mkdir ptest

echo prueba.fisopfs > ./fisopfs ptest/
cd ptest || exit

touch pfile2
echo "texto a persistir" > pfile2

cd .. || exit
umount ptest/

echo prueba.fisopfs > ./fisopfs ptest/
cd ptest || exit

expected="texto a persistir"
actual=$(cat pfile2)

if [[ $expected == "$actual" ]]; then
  echo -e "${GREEN}[OK]${NO_COLOR} ${testName}
  "

else
  echo -e "${RED}[ERROR]${NO_COLOR} ${testName}
Expected: ${expected}
     Got: ${actual}
     "
fi

rm pfile2

cd .. || exit
umount ptest/
rmdir ptest