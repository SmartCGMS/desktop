/****************************************************************************
** Meta object code from reading C++ file 'main_window.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "main_window.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'main_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CMain_Window_t {
    QByteArrayData data[13];
    char stringdata0[185];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CMain_Window_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CMain_Window_t qt_meta_stringdata_CMain_Window = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CMain_Window"
QT_MOC_LITERAL(1, 13, 7), // "On_Quit"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 17), // "On_Update_Actions"
QT_MOC_LITERAL(4, 40, 12), // "On_Close_All"
QT_MOC_LITERAL(5, 53, 18), // "On_Tile_Vertically"
QT_MOC_LITERAL(6, 72, 20), // "On_Tile_Horizontally"
QT_MOC_LITERAL(7, 93, 21), // "On_Update_Window_Menu"
QT_MOC_LITERAL(8, 115, 13), // "On_Help_About"
QT_MOC_LITERAL(9, 129, 17), // "On_Filters_Window"
QT_MOC_LITERAL(10, 147, 21), // "Set_Active_Sub_Window"
QT_MOC_LITERAL(11, 169, 8), // "QWidget*"
QT_MOC_LITERAL(12, 178, 6) // "window"

    },
    "CMain_Window\0On_Quit\0\0On_Update_Actions\0"
    "On_Close_All\0On_Tile_Vertically\0"
    "On_Tile_Horizontally\0On_Update_Window_Menu\0"
    "On_Help_About\0On_Filters_Window\0"
    "Set_Active_Sub_Window\0QWidget*\0window"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CMain_Window[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x08 /* Private */,
       3,    0,   60,    2, 0x08 /* Private */,
       4,    0,   61,    2, 0x08 /* Private */,
       5,    0,   62,    2, 0x08 /* Private */,
       6,    0,   63,    2, 0x08 /* Private */,
       7,    0,   64,    2, 0x08 /* Private */,
       8,    0,   65,    2, 0x08 /* Private */,
       9,    0,   66,    2, 0x08 /* Private */,
      10,    1,   67,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 11,   12,

       0        // eod
};

void CMain_Window::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CMain_Window *_t = static_cast<CMain_Window *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->On_Quit(); break;
        case 1: _t->On_Update_Actions(); break;
        case 2: _t->On_Close_All(); break;
        case 3: _t->On_Tile_Vertically(); break;
        case 4: _t->On_Tile_Horizontally(); break;
        case 5: _t->On_Update_Window_Menu(); break;
        case 6: _t->On_Help_About(); break;
        case 7: _t->On_Filters_Window(); break;
        case 8: _t->Set_Active_Sub_Window((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject CMain_Window::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_CMain_Window.data,
      qt_meta_data_CMain_Window,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CMain_Window::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CMain_Window::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CMain_Window.stringdata0))
        return static_cast<void*>(const_cast< CMain_Window*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int CMain_Window::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
