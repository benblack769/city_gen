#pragma once
#include <functional>
#include <QCheckBox>
using namespace std;

struct check_obj{
    function<void()> func;
    QCheckBox * box;
    check_obj(){
        box = nullptr;
    }
    check_obj(string str,bool def_check,function<void()> infunc,QWidget * parent=nullptr){
        func = infunc;
        box = new QCheckBox(QString(str.c_str()),parent);
        box->setChecked(def_check);
    }
    void call_if_checked(){
        if(box->isChecked()){
            func();
        }
    }
};
