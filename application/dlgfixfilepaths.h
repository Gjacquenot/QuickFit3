/*
Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License (LGPL) as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License (LGPL) for more details.

    You should have received a copy of the GNU Lesser General Public License (LGPL)
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DLGFIXFILEPATHS_H
#define DLGFIXFILEPATHS_H

#include "qfdialog.h"

namespace Ui {
    class DlgFixFilepaths;
}

class DlgFixFilepaths : public QFDialog
{
    Q_OBJECT

public:
    explicit DlgFixFilepaths(const QString& oldName, const QString& lastDir, QWidget *parent = 0);
    ~DlgFixFilepaths();
    QString getNewFilename() const;
    QString getLastDir() const;
    void init(const QString& oldName, const QString& lastDir);

private:
    Ui::DlgFixFilepaths *ui;
    QString lastDir;
protected slots:
    void showHelp();
protected slots:
    void selectFile();
    void on_btnApply_clicked();
    void on_btnIgnore_clicked();
    void on_btnCancel_clicked();
};

#endif // DLGFIXFILEPATHS_H
