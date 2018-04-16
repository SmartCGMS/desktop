/****************************************************************************
** Meta object code from reading C++ file 'filters_window.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "filters_window.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'filters_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CFilters_Window_t {
    QByteArrayData data[12];
    char stringdata0[203];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CFilters_Window_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CFilters_Window_t qt_meta_stringdata_CFilters_Window = {
    {
QT_MOC_LITERAL(0, 0, 15), // "CFilters_Window"
QT_MOC_LITERAL(1, 16, 13), // "On_Add_Filter"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 17), // "On_Move_Filter_Up"
QT_MOC_LITERAL(4, 49, 19), // "On_Move_Filter_Down"
QT_MOC_LITERAL(5, 69, 16), // "On_Remove_Filter"
QT_MOC_LITERAL(6, 86, 19), // "On_Configure_Filter"
QT_MOC_LITERAL(7, 106, 17), // "On_Commit_Filters"
QT_MOC_LITERAL(8, 124, 27), // "On_Applied_Filter_Dbl_Click"
QT_MOC_LITERAL(9, 152, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(10, 169, 4), // "item"
QT_MOC_LITERAL(11, 174, 28) // "On_Filter_Configure_Complete"

    },
    "CFilters_Window\0On_Add_Filter\0\0"
    "On_Move_Filter_Up\0On_Move_Filter_Down\0"
    "On_Remove_Filter\0On_Configure_Filter\0"
    "On_Commit_Filters\0On_Applied_Filter_Dbl_Click\0"
    "QListWidgetItem*\0item\0"
    "On_Filter_Configure_Complete"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CFilters_Window[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x09 /* Protected */,
       3,    0,   55,    2, 0x09 /* Protected */,
       4,    0,   56,    2, 0x09 /* Protected */,
       5,    0,   57,    2, 0x09 /* Protected */,
       6,    0,   58,    2, 0x09 /* Protected */,
       7,    0,   59,    2, 0x09 /* Protected */,
       8,    1,   60,    2, 0x09 /* Protected */,
      11,    0,   63,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void,

       0        // eod
};

void CFilters_Window::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CFilters_Window *_t = static_cast<CFilters_Window *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->On_Add_Filter(); break;
        case 1: _t->On_Move_Filter_Up(); break;
        case 2: _t->On_Move_Filter_Down(); break;
        case 3: _t->On_Remove_Filter(); break;
        case 4: _t->On_Configure_Filter(); break;
        case 5: _t->On_Commit_Filters(); break;
        case 6: _t->On_Applied_Filter_Dbl_Click((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 7: _t->On_Filter_Configure_Complete(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CFilters_Window::staticMetaObject = {
    { &QMdiSubWindow::staticMetaObject, qt_meta_stringdata_CFilters_Window.data,
      qt_meta_data_CFilters_Window,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CFilters_Window::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CFilters_Window::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CFilters_Window.stringdata0))
        return static_cast<void*>(this);
    return QMdiSubWindow::qt_metacast(_clname);
}

int CFilters_Window::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMdiSubWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
