QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
include(./Qt_xlsx/src/xlsx/qtxlsx.pri)

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DataModel/abstract_object.cpp \
    DataModel/element.cpp \
    FileIO/file_io.cpp \
    Processing/reaction_search_decluster.cpp \
    Widgets/checkbox_list.cpp \
    Widgets/file_path_editor.cpp \
    Widgets/label_value_editor.cpp \
    Widgets/rtp_tool_dialog.cpp \
    Widgets/table_viewer.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    DataModel/abstract_object.h \
    DataModel/element.h \
    FileIO/file_io.h \
    Processing/reaction_search_decluster.h \
    Widgets/checkbox_list.h \
    Widgets/file_path_editor.h \
    Widgets/label_value_editor.h \
    Widgets/rtp_tool_dialog.h \
    Widgets/table_viewer.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES = \
    Images/settings.qrc

ICON = Images/logo.icns

RC_ICONS = Images/logo.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
