/****************************************************************************
** Meta object code from reading C++ file 'drawing_tab_widget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "drawing_tab_widget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'drawing_tab_widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CDrawing_Tab_Widget_t {
    QByteArrayData data[4];
    char stringdata0[43];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CDrawing_Tab_Widget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CDrawing_Tab_Widget_t qt_meta_stringdata_CDrawing_Tab_Widget = {
    {
QT_MOC_LITERAL(0, 0, 19), // "CDrawing_Tab_Widget"
QT_MOC_LITERAL(1, 20, 17), // "Show_Context_Menu"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 3) // "pos"

    },
    "CDrawing_Tab_Widget\0Show_Context_Menu\0"
    "\0pos"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDrawing_Tab_Widget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void, QMetaType::QPoint,    3,

       0        // eod
};

void CDrawing_Tab_Widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CDrawing_Tab_Widget *_t = static_cast<CDrawing_Tab_Widget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->Show_Context_Menu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CDrawing_Tab_Widget::staticMetaObject = {
    { &CAbstract_Simulation_Tab_Widget::staticMetaObject, qt_meta_stringdata_CDrawing_Tab_Widget.data,
      qt_meta_data_CDrawing_Tab_Widget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CDrawing_Tab_Widget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDrawing_Tab_Widget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CDrawing_Tab_Widget.stringdata0))
        return static_cast<void*>(this);
    return CAbstract_Simulation_Tab_Widget::qt_metacast(_clname);
}

int CDrawing_Tab_Widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CAbstract_Simulation_Tab_Widget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
