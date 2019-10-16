
# Qt QML Models

QT += core qml

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/QQmlObjectListModel.h \
    $$PWD/QQmlObjectListModelBase.h \
    $$PWD/QQmlObjectListModelPtr.h \
    $$PWD/QQmlVariantListModel.h \
    $$PWD/QtQmlTricksPlugin_SmartDataModels.h

SOURCES += \
    $$PWD/QQmlObjectListModel.cpp \
    $$PWD/QQmlObjectListModelPtr.cpp \
    $$PWD/QQmlVariantListModel.cpp

