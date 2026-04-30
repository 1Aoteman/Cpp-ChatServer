#include "global.h"

std::function<void(QWidget*)> repolish=[](QWidget *w){
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
};
QString gate_url_prefix = "";
