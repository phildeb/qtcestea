/****************************************************************************
** Meta object code from reading C++ file 'csta_gui.h'
**
** Created: Fri Dec 18 09:39:43 2015
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "csta_gui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'csta_gui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_LiveEvents[] = {

 // content:
       6,       // revision
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

static const char qt_meta_stringdata_LiveEvents[] = {
    "LiveEvents\0"
};

void LiveEvents::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData LiveEvents::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject LiveEvents::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_LiveEvents,
      qt_meta_data_LiveEvents, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LiveEvents::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LiveEvents::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LiveEvents::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LiveEvents))
        return static_cast<void*>(const_cast< LiveEvents*>(this));
    return QThread::qt_metacast(_clname);
}

int LiveEvents::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_Dialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x0a,
      20,    7,    7,    7, 0x0a,
      32,    7,    7,    7, 0x0a,
      44,    7,    7,    7, 0x0a,
      56,    7,    7,    7, 0x0a,
      68,    7,    7,    7, 0x0a,
      80,    7,    7,    7, 0x0a,
      92,    7,    7,    7, 0x0a,
     104,    7,    7,    7, 0x0a,
     116,    7,    7,    7, 0x0a,
     128,    7,    7,    7, 0x0a,
     139,    7,    7,    7, 0x0a,
     150,    7,    7,    7, 0x0a,
     180,  176,    7,    7, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Dialog[] = {
    "Dialog\0\0onButtonA()\0onButtonB()\0"
    "onButtonC()\0onButtonD()\0onButtonE()\0"
    "onButtonF()\0onButtonG()\0onButtonH()\0"
    "onButtonI()\0onButtonJ()\0onTimer1()\0"
    "onTimer2()\0slot_timerJournaldeBord()\0"
    "str\0onDisplay(QString)\0"
};

void Dialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Dialog *_t = static_cast<Dialog *>(_o);
        switch (_id) {
        case 0: _t->onButtonA(); break;
        case 1: _t->onButtonB(); break;
        case 2: _t->onButtonC(); break;
        case 3: _t->onButtonD(); break;
        case 4: _t->onButtonE(); break;
        case 5: _t->onButtonF(); break;
        case 6: _t->onButtonG(); break;
        case 7: _t->onButtonH(); break;
        case 8: _t->onButtonI(); break;
        case 9: _t->onButtonJ(); break;
        case 10: _t->onTimer1(); break;
        case 11: _t->onTimer2(); break;
        case 12: _t->slot_timerJournaldeBord(); break;
        case 13: _t->onDisplay((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Dialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Dialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_Dialog,
      qt_meta_data_Dialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dialog))
        return static_cast<void*>(const_cast< Dialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int Dialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
