TEMPLATE = app
DESTDIR  = bin/
HEADERS += src/*.h
SOURCES += src/*.cpp src/kit/log/*.cpp src/kit/args/*.cpp
LIBS += -L/usr/local/lib \
    -lpthread \
    -lboost_thread \
    -lboost_system \
    -lboost_filesystem \
    -ljsoncpp \

FORMS += src/ui/*.ui
UI_DIR += src/ui/
MOC_DIR = moc
DEFINES += QT_DLL MX_THREADS=1
CONFIG  += qt warn_on release c++11 no_keywords
QT += core gui widgets svg
OBJECTS_DIR = obj/
QMAKE_CXXFLAGS += -std=c++0x
