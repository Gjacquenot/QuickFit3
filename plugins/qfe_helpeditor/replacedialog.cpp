/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>),
    German Cancer Research Center/University Heidelberg



    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License (LGPL) as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include<QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif


#include <QCheckBox>

#include "replacedialog.h"


QFEHelpEditorReplaceDialog::QFEHelpEditorReplaceDialog(QWidget *parent)
     : QFDialog(parent)
 {
    matchCase=false;
    searchFromStart=true;
    wholeWords=false;
    replaceAll=false;
    askBeforeReplace=true;
    setupUi(this);
}

QFEHelpEditorReplaceDialog::~QFEHelpEditorReplaceDialog()
{

}

void QFEHelpEditorReplaceDialog::setPhrase(QString phr){
    phrase=phr;
    edt_what->setText(phr);
    edt_what->selectAll();
}


void QFEHelpEditorReplaceDialog::on_chk_matchCase_stateChanged(int state){
    matchCase = (state==Qt::Checked);
};

void QFEHelpEditorReplaceDialog::on_chk_searchFromStart_stateChanged(int state){
    searchFromStart = (state==Qt::Checked);
};

void QFEHelpEditorReplaceDialog::on_chk_wholeWords_stateChanged(int state){
    wholeWords = (state==Qt::Checked);
};

void QFEHelpEditorReplaceDialog::on_chk_replaceAll_stateChanged(int state){
    replaceAll = (state==Qt::Checked);
};

void QFEHelpEditorReplaceDialog::on_chk_ask_stateChanged(int state){
    askBeforeReplace = (state==Qt::Checked);
};

void QFEHelpEditorReplaceDialog::on_edt_what_textChanged ( const QString & text ){
    phrase=text;
};

void QFEHelpEditorReplaceDialog::on_edt_replace_textChanged ( const QString & text ){
    replace=text;
};

