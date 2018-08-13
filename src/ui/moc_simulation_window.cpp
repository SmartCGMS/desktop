/****************************************************************************
** Meta object code from reading C++ file 'simulation_window.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "simulation_window.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'simulation_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CSimulation_Window_t {
    QByteArrayData data[12];
    char stringdata0[176];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CSimulation_Window_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CSimulation_Window_t qt_meta_stringdata_CSimulation_Window = {
    {
QT_MOC_LITERAL(0, 0, 18), // "CSimulation_Window"
QT_MOC_LITERAL(1, 19, 8), // "On_Start"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 7), // "On_Stop"
QT_MOC_LITERAL(4, 37, 13), // "On_Tab_Change"
QT_MOC_LITERAL(5, 51, 5), // "index"
QT_MOC_LITERAL(6, 57, 15), // "On_Solve_Params"
QT_MOC_LITERAL(7, 73, 25), // "On_Reset_And_Solve_Params"
QT_MOC_LITERAL(8, 99, 16), // "On_Suspend_Solve"
QT_MOC_LITERAL(9, 116, 15), // "On_Resume_Solve"
QT_MOC_LITERAL(10, 132, 18), // "On_Simulation_Step"
QT_MOC_LITERAL(11, 151, 24) // "On_Segments_Draw_Request"

    },
    "CSimulation_Window\0On_Start\0\0On_Stop\0"
    "On_Tab_Change\0index\0On_Solve_Params\0"
    "On_Reset_And_Solve_Params\0On_Suspend_Solve\0"
    "On_Resume_Solve\0On_Simulation_Step\0"
    "On_Segments_Draw_Request"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CSimulation_Window[] = {

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
       1,    0,   59,    2, 0x09 /* Protected */,
       3,    0,   60,    2, 0x09 /* Protected */,
       4,    1,   61,    2, 0x09 /* Protected */,
       6,    0,   64,    2, 0x09 /* Protected */,
       7,    0,   65,    2, 0x09 /* Protected */,
       8,    0,   66,    2, 0x09 /* Protected */,
       9,    0,   67,    2, 0x09 /* Protected */,
      10,    0,   68,    2, 0x09 /* Protected */,
      11,    0,   69,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CSimulation_Window::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CSimulation_Window *_t = static_cast<CSimulation_Window *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->On_Start(); break;
        case 1: _t->On_Stop(); break;
        case 2: _t->On_Tab_Change((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->On_Solve_Params(); break;
        case 4: _t->On_Reset_And_Solve_Params(); break;
        case 5: _t->On_Suspend_Solve(); break;
        case 6: _t->On_Resume_Solve(); break;
        case 7: _t->On_Simulation_Step(); break;
        case 8: _t->On_Segments_Draw_Request(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CSimulation_Window::staticMetaObject = {
    { &QMdiSubWindow::staticMetaObject, qt_meta_stringdata_CSimulation_Window.data,
      qt_meta_data_CSimulation_Window,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CSimulation_Window::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CSimulation_Window::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CSimulation_Window.stringdata0))
        return static_cast<void*>(this);
    return QMdiSubWindow::qt_metacast(_clname);
}

int CSimulation_Window::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMdiSubWindow::qt_metacall(_c, _id, _a);
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
