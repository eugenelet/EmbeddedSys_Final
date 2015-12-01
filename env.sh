
#export PATH=/opt/arm-2012.03/bin:$PATH

#export LD_LIBRARY_PATH=/opt/qt-pandaboard-lib/lib:$LD_LIBRARY_PATH

#export LD_LIBRARY_PATH=/opt/opencvlib_arm/lib:$LD_LIBRARY_PATH

/opt/qt-pandaboard-lib/bin/qmake -project

/opt/qt-pandaboard-lib/bin/qmake -spec /opt/qt-pandaboard-lib/mkspecs/qws/linux-omap4430-g++

