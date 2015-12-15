/opt/qt-pandaboard-lib/bin/qmake -project

/opt/qt-pandaboard-lib/bin/qmake -spec /opt/qt-pandaboard-lib/mkspecs/qws/linux-omap4430-g++

sed -i.bak 's:INCPATH       = :INCPATH       = -I /opt/opencvlib_arm/include :g' Makefile
sed -i.bak 's:LIBS          = $(SUBLIBS) :LIBS          = $(SUBLIBS) -L/opt/opencvlib_arm/lib -lopencv_core -lopencv_objdetect -lopencv_highgui -lopencv_imgproc :g' Makefile

