/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>),
    German Cancer Research Center/University Heidelberg

    

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef QFESPIMB040LIGHTPATHSAVEDIALOG_H
#define QFESPIMB040LIGHTPATHSAVEDIALOG_H

#include "qfdialog.h"
#include <QAbstractItemModel>
#include "qftablemodel.h"
#include <QList>

namespace Ui {
    class QFESPIMB040LightPathSaveDialog;
}

class QFESPIMB040LightPathSaveDialog : public QFDialog
{
        Q_OBJECT
        
    public:
        explicit QFESPIMB040LightPathSaveDialog(QString name, QStringList names, QFTableModel *model, QWidget *parent = 0);
        ~QFESPIMB040LightPathSaveDialog();

        QString getName() const;
        QList<bool> getCheckedItems() const;
        
    private:
        Ui::QFESPIMB040LightPathSaveDialog *ui;
        QFTableModel* m_model;
};

#endif // QFESPIMB040LIGHTPATHSAVEDIALOG_H
