/****************************************************************************
** Meta object code from reading C++ file 'main_window.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "main_window.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'main_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CMain_Window_t {
    QByteArrayData data[15];
    char stringdata0[228];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CMain_Window_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CMain_Window_t qt_meta_stringdata_CMain_Window = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CMain_Window"
QT_MOC_LITERAL(1, 13, 21), // "On_Save_Configuration"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 7), // "On_Quit"
QT_MOC_LITERAL(4, 44, 17), // "On_Update_Actions"
QT_MOC_LITERAL(5, 62, 12), // "On_Close_All"
QT_MOC_LITERAL(6, 75, 18), // "On_Tile_Vertically"
QT_MOC_LITERAL(7, 94, 20), // "On_Tile_Horizontally"
QT_MOC_LITERAL(8, 115, 21), // "On_Update_Window_Menu"
QT_MOC_LITERAL(9, 137, 13), // "On_Help_About"
QT_MOC_LITERAL(10, 151, 17), // "On_Filters_Window"
QT_MOC_LITERAL(11, 169, 20), // "On_Simulation_Window"
QT_MOC_LITERAL(12, 190, 21), // "Set_Active_Sub_Window"
QT_MOC_LITERAL(13, 212, 8), // "QWidget*"
QT_MOC_LITERAL(14, 221, 6) // "window"

    },
    "CMain_Window\0On_Save_Configuration\0\0"
    "On_Quit\0On_Update_Actions\0On_Close_All\0"
    "On_Tile_Vertically\0On_Tile_Horizontally\0"
    "On_Update_Window_Menu\0On_Help_About\0"
    "On_Filters_Window\0On_Simulation_Window\0"
    "Set_Active_Sub_Window\0QWidget*\0window"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CMain_Window[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x08 /* Private */,
       3,    0,   70,    2, 0x08 /* Private */,
       4,    0,   71,    2, 0x08 /* Private */,
       5,    0,   72,    2, 0x08 /* Private */,
       6,    0,   73,    2, 0x08 /* Private */,
       7,    0,   74,    2, 0x08 /* Private */,
       8,    0,   75,    2, 0x08 /* Private */,
       9,    0,   76,    2, 0x08 /* Private */,
      10,    0,   77,    2, 0x08 /* Private */,
      11,    0,   78,    2, 0x08 /* Private */,
      12,    1,   79,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 13,   14,

       0        // eod
};

void CMain_Window::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CMain_Window *_t = static_cast<CMain_Window *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->On_Save_Configuration(); break;
        case 1: _t->On_Quit(); break;
        case 2: _t->On_Update_Actions(); break;
        case 3: _t->On_Close_All(); break;
        case 4: _t->On_Tile_Vertically(); break;
        case 5: _t->On_Tile_Horizontally(); break;
        case 6: _t->On_Update_Window_Menu(); break;
        case 7: _t->On_Help_About(); break;
        case 8: _t->On_Filters_Window(); break;
        case 9: _t->On_Simulation_Window(); break;
        case 10: _t->Set_Active_Sub_Window((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CMain_Window::staticMetaObject = {
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
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int CMain_Window::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
