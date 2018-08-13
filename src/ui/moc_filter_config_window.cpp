/****************************************************************************
** Meta object code from reading C++ file 'filter_config_window.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "filter_config_window.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'filter_config_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CFilter_Config_Window_t {
    QByteArrayData data[5];
    char stringdata0[48];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CFilter_Config_Window_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CFilter_Config_Window_t qt_meta_stringdata_CFilter_Config_Window = {
    {
QT_MOC_LITERAL(0, 0, 21), // "CFilter_Config_Window"
QT_MOC_LITERAL(1, 22, 5), // "On_OK"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 9), // "On_Cancel"
QT_MOC_LITERAL(4, 39, 8) // "On_Apply"

    },
    "CFilter_Config_Window\0On_OK\0\0On_Cancel\0"
    "On_Apply"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CFilter_Config_Window[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x09 /* Protected */,
       3,    0,   30,    2, 0x09 /* Protected */,
       4,    0,   31,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CFilter_Config_Window::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CFilter_Config_Window *_t = static_cast<CFilter_Config_Window *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->On_OK(); break;
        case 1: _t->On_Cancel(); break;
        case 2: _t->On_Apply(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CFilter_Config_Window::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_CFilter_Config_Window.data,
      qt_meta_data_CFilter_Config_Window,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CFilter_Config_Window::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CFilter_Config_Window::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CFilter_Config_Window.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int CFilter_Config_Window::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
struct qt_meta_stringdata_CInteger_Container_Edit_t {
    QByteArrayData data[1];
    char stringdata0[24];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CInteger_Container_Edit_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CInteger_Container_Edit_t qt_meta_stringdata_CInteger_Container_Edit = {
    {
QT_MOC_LITERAL(0, 0, 23) // "CInteger_Container_Edit"

    },
    "CInteger_Container_Edit"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CInteger_Container_Edit[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void CInteger_Container_Edit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CInteger_Container_Edit::staticMetaObject = {
    { &QLineEdit::staticMetaObject, qt_meta_stringdata_CInteger_Container_Edit.data,
      qt_meta_data_CInteger_Container_Edit,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CInteger_Container_Edit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CInteger_Container_Edit::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CInteger_Container_Edit.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "filter_config_window::CContainer_Edit"))
        return static_cast< filter_config_window::CContainer_Edit*>(this);
    return QLineEdit::qt_metacast(_clname);
}

int CInteger_Container_Edit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLineEdit::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
