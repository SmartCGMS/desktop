/****************************************************************************
** Meta object code from reading C++ file 'Model_Bounds_Panel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "Model_Bounds_Panel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Model_Bounds_Panel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CModel_Bounds_Panel_t {
    QByteArrayData data[5];
    char stringdata0[69];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CModel_Bounds_Panel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CModel_Bounds_Panel_t qt_meta_stringdata_CModel_Bounds_Panel = {
    {
QT_MOC_LITERAL(0, 0, 19), // "CModel_Bounds_Panel"
QT_MOC_LITERAL(1, 20, 14), // "On_Reset_Lower"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 17), // "On_Reset_Defaults"
QT_MOC_LITERAL(4, 54, 14) // "On_Reset_Upper"

    },
    "CModel_Bounds_Panel\0On_Reset_Lower\0\0"
    "On_Reset_Defaults\0On_Reset_Upper"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CModel_Bounds_Panel[] = {

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

void CModel_Bounds_Panel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CModel_Bounds_Panel *_t = static_cast<CModel_Bounds_Panel *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->On_Reset_Lower(); break;
        case 1: _t->On_Reset_Defaults(); break;
        case 2: _t->On_Reset_Upper(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CModel_Bounds_Panel::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CModel_Bounds_Panel.data,
      qt_meta_data_CModel_Bounds_Panel,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CModel_Bounds_Panel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CModel_Bounds_Panel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CModel_Bounds_Panel.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "filter_config_window::CContainer_Edit"))
        return static_cast< filter_config_window::CContainer_Edit*>(this);
    return QWidget::qt_metacast(_clname);
}

int CModel_Bounds_Panel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
