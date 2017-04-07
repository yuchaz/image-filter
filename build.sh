ps -erf | grep ImgFilter.app | grep -v grep | awk '{print "kill -9 " $2}' | sh
qmake -spec macx-g++ ImgFilter.pro
make
open output/ImgFilter.app/
