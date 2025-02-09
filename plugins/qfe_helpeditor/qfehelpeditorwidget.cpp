﻿/*
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


#include "qfehelpeditorwidget.h"
#include "ui_qfehelpeditorwidget.h"

#include "qfpluginservices.h"
#include "qfextension.h"
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtGlobal>
#include <QtWidgets>
#else
#include <QtGui>
#endif


#include <QtCore>
#include "qfcompleterfromfile.h"
#include <QTemporaryFile>
#include "qftools.h"
#include "pasteimagedlg.h"
#include "selectresourceimage.h"
#include "pluginlinkdialog.h"
#include "subpluginlinkdialog.h"
#include "faqentrydialog.h"
#include "functionreferencedialog.h"
#include "qfhtmlhelptools.h"
#include "newtabledialog.h"
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>

#define AUTOSAVE_INTERVAL_MSEC 20000


QFEHelpEditorWidget::QFEHelpEditorWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::QFEHelpEditorWidget)
{
    modified=false;
    newScript=true;
    ui->setupUi(this);

    ui->edtScript->getEditor()->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    ui->edtScript->getEditor()->setLineWrapMode(QTextEdit::WidgetWidth);
    connect(ui->edtScript->getEditor()->document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));



    highlighter=new QFHTMLHighlighter("", ui->edtScript->getEditor()->document());
    highlighter->setUseSpecial2("$(", ")$");
    highlighter->setUseSpecial1("$$", "$$");

    completer = new QCompleter(ui->edtScript->getEditor());
    completermodel=modelFromFile(ProgramOptions::getInstance()->getAssetsDirectory()+"/qtscript/completer.txt");
    completer->setModel(completermodel);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    ui->edtScript->getEditor()->setCompleter(completer);



    recentHelpFiles=new QRecentFilesMenu(this);
    recentHelpFiles->setUseSystemFileIcons(false);
    recentHelpFiles->setAlwaysEnabled(true);
    connect(recentHelpFiles, SIGNAL(openRecentFile(QString)), this, SLOT(openScriptNoAsk(QString)));
    ui->btnOpen->setMenu(recentHelpFiles);
    connect(ui->edtScript->getEditor(), SIGNAL(cursorPositionChanged()), this, SLOT(edtScript_cursorPositionChanged()));


    actLoadAutosave=new QFActionWithNoMenuRole(tr("load last autosaved file ..."), this);
    connect(actLoadAutosave, SIGNAL(triggered()), this, SLOT(reloadLastAutosave()));

    cutAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/script_cut.png"), tr("Cu&t"), this);
    cutAct->setShortcut(tr("Ctrl+X"));
    cutAct->setToolTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), ui->edtScript->getEditor(), SLOT(cut()));

    copyAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/script_copy.png"), tr("&Copy"), this);
    copyAct->setShortcut(tr("Ctrl+C"));
    copyAct->setToolTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), ui->edtScript->getEditor(), SLOT(copy()));

    pasteAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/script_paste.png"), tr("&Paste"), this);
    pasteAct->setShortcut(tr("Ctrl+V"));
    pasteAct->setToolTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), ui->edtScript->getEditor(), SLOT(paste()));

    undoAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/script_undo.png"), tr("&Undo"), this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setToolTip(tr("Undo the last change "));
    connect(undoAct, SIGNAL(triggered()), ui->edtScript->getEditor(), SLOT(undo()));

    redoAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/script_redo.png"), tr("&Redo"), this);
    redoAct->setShortcut(tr("Ctrl+Shift+Z"));
    redoAct->setToolTip(tr("Redo the last undone change "));
    connect(redoAct, SIGNAL(triggered()), ui->edtScript->getEditor(), SLOT(redo()));

    findAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/script_find.png"), tr("&Find ..."), this);
    findAct->setShortcut(tr("Ctrl+F"));
    findAct->setToolTip(tr("Find a string in sequence "));
    connect(findAct, SIGNAL(triggered()), this, SLOT(findFirst()));

    findNextAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/script_find_next.png"), tr("Find &next"), this);
    findNextAct->setShortcut(tr("F3"));
    findNextAct->setToolTip(tr("Find the next occurence "));
    connect(findNextAct, SIGNAL(triggered()), this, SLOT(findNext()));
    findNextAct->setEnabled(false);

    replaceAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/script_find_replace.png"), tr("Find && &replace ..."), this);
    replaceAct->setShortcut(tr("Ctrl+R"));
    replaceAct->setToolTip(tr("Find a string in sequence and replace it with another string "));
    connect(replaceAct, SIGNAL(triggered()), this, SLOT(replaceFirst()));

    commentAct = new QFActionWithNoMenuRole(tr("&Comment text"), this);
    commentAct->setShortcut(tr("Ctrl+B"));
    commentAct->setToolTip(tr("add (single line) comment at the beginning of each line "));
    connect(commentAct, SIGNAL(triggered()), ui->edtScript->getEditor(), SLOT(comment()));

    unCommentAct = new QFActionWithNoMenuRole(tr("&Uncomment text"), this);
    unCommentAct->setShortcut(tr("Ctrl+Shift+B"));
    unCommentAct->setToolTip(tr("remove (single line) comment at the beginning of each line "));
    connect(unCommentAct, SIGNAL(triggered()), ui->edtScript->getEditor(), SLOT(uncomment()));

    indentAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/script_indent.png"), tr("&Increase indention"), this);
    commentAct->setShortcut(tr("Ctrl+I"));
    indentAct->setToolTip(tr("increase indention "));
    connect(indentAct, SIGNAL(triggered()), ui->edtScript->getEditor(), SLOT(indentInc()));

    unindentAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/script_unindent.png"), tr("&Decrease indention"), this);
    unindentAct->setShortcut(tr("Ctrl+Shift+I"));
    unindentAct->setToolTip(tr("decrease indention "));
    connect(unindentAct, SIGNAL(triggered()), ui->edtScript->getEditor(), SLOT(indentDec()));

    gotoLineAct = new QFActionWithNoMenuRole(tr("&Goto line ..."), this);
    gotoLineAct->setShortcut(tr("Alt+G"));
    gotoLineAct->setToolTip(tr("goto a line in the opened file "));
    connect(gotoLineAct, SIGNAL(triggered()), this, SLOT(gotoLine()));

    printAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/script_print.png"), tr("&Print ..."), this);
    printAct->setToolTip(tr("print the current SDFF file "));
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    actInsertIcon = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/insert_image.png"), tr("&Insert icon ..."), this);
    actInsertIcon->setToolTip(tr("print the current SDFF file "));
    connect(actInsertIcon, SIGNAL(triggered()), this, SLOT(insertIcon()));


    toEntityAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/toentity.png"), tr("&Convert characters to entities ..."), this);
    toEntityAct->setToolTip(tr("convert characters in the selected text to HTML entities"));
    connect(toEntityAct, SIGNAL(triggered()), this, SLOT(toEntity()));

    toCharAct = new QFActionWithNoMenuRole(QIcon(":/qfe_helpeditor/tochars.png"), tr("&Convert entities to characters ..."), this);
    toCharAct->setToolTip(tr("convert HTML entities in the selected text to characters"));
    connect(toCharAct, SIGNAL(triggered()), this, SLOT(toChars()));

    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    undoAct->setEnabled(false);
    redoAct->setEnabled(false);
    connect(ui->edtScript->getEditor(), SIGNAL(copyAvailable(bool)), cutAct, SLOT(setEnabled(bool)));
    connect(ui->edtScript->getEditor(), SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));
    connect(ui->edtScript->getEditor(), SIGNAL(undoAvailable(bool)), undoAct, SLOT(setEnabled(bool)));
    connect(ui->edtScript->getEditor(), SIGNAL(redoAvailable(bool)), redoAct, SLOT(setEnabled(bool)));
    connect(ui->edtScript->getEditor(), SIGNAL(findNextAvailable(bool)), findNextAct, SLOT(setEnabled(bool)));


    QMenu* menuMore=new QMenu(ui->tbMoreOptions);
    menuMore->addAction(indentAct);
    menuMore->addAction(unindentAct);
    menuMore->addAction(commentAct);
    menuMore->addAction(unCommentAct);
    menuMore->addSeparator();
    menuMore->addAction(toEntityAct);
    menuMore->addAction(toCharAct);
    menuMore->addSeparator();
    menuMore->addAction(actInsertIcon);
    menuMore->addSeparator();
    menuMore->addAction(gotoLineAct);
    menuMore->addAction(findAct);
    menuMore->addAction(replaceAct);
    menuMore->addAction(findNextAct);
    menuMore->addSeparator();
    menuMore->addAction(actLoadAutosave);
    ui->tbMoreOptions->setMenu(menuMore);
    ui->tbFind->setDefaultAction(findAct);
    ui->tbFindNext->setDefaultAction(findNextAct);
    ui->tbReplace->setDefaultAction(replaceAct);
    ui->tbPrint->setDefaultAction(printAct);
    ui->tbCopy->setDefaultAction(copyAct);
    ui->tbCut->setDefaultAction(cutAct);
    ui->tbPaste->setDefaultAction(pasteAct);
    ui->tbRedo->setDefaultAction(redoAct);
    ui->tbUndo->setDefaultAction(undoAct);
    ui->tbEntity->setDefaultAction(toEntityAct);
    ui->tbChars->setDefaultAction(toCharAct);



    setScriptFilename("");
    QMenu* menu;
    menu=new QMenu(tr("directories"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$mainhelpdir$$");
    addInsertAction(menu, "$$mainhelppicdir$$");
    addInsertAction(menu, "$$assetsdir$$");
    addInsertAction(menu, "$$examplesdir$$");
    addInsertAction(menu, "$$configdir$$");
    addInsertAction(menu, "$$maindir$$");
    addInsertAction(menu, "$$sourcedir$$");

    menu=new QMenu(tr("version/copyright info"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$version.svnrevision$$");
    addInsertAction(menu, "$$version.status$$");
    addInsertAction(menu, "$$version.date$$");
    addInsertAction(menu, "$$version$$");
    addInsertAction(menu, "$$version_full$$");
    menu->addSeparator();
    addInsertAction(menu, "$$thanksto$$");
    addInsertAction(menu, "$$copyright$$");
    addInsertAction(menu, "$$author$$");
    addInsertAction(menu, "$$weblink$$");
    addInsertAction(menu, "$$license$$");
    addInsertAction(menu, "$$maillist$$");
    addInsertAction(menu, "$$maillistrequest$$");
    menu->addSeparator();
    addInsertAction(menu, "$$qfcitation$$");
    addInsertAction(menu, "$$qfcitation_bibtex$$");
    addInsertAction(menu, "$$jankrieger_phdthesis$$");
    addInsertAction(menu, "$$jankrieger_phdthesis_ref$$");

    menu=new QMenu(tr("general help links"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, tr("Link: RDR general help"), "<a href=\"$qf_ui_rdr_helpfile$$\">$$qf_ui_rdr_helpfiletitle$$</a>" );
    addInsertAction(menu, tr("Link: Evaluation general help"), "<a href=\"$$qf_ui_eval_helpfile$$\">$$qf_ui_eval_helpfiletitle$$</a>" );
    addInsertAction(menu, tr("Link: Plotter Widget general help"), "<a href=\"$$qf_ui_jkqtplotter_helpfile$$\">$$qf_ui_jkqtplotter_helpfiletitle$$</a>" );
    addInsertAction(menu, tr("Link: LaTeX parser general help"), "<a href=\"$$qf_ui_latex_helpfile$$\">$$qf_ui_latex_helpfiletitle$$</a>" );
    addInsertAction(menu, tr("Link: Expression parser help"), "<a href=\"$$qf_mathparser_helpfile$$\">$$qf_mathparser_helpfiletitle$$</a>" );
    //addInsertAction(menu, tr("Link:  help"), "<a href=\"$$aa$$\">$$aa$$</a>" );
    //addInsertAction(menu, tr("Link:  help"), QString("<a href=\"$$mainhelpdir$$aa\">aa</a>") );
    addInsertAction(menu, tr("Link: Custom color palettes help"), QString("<a href=\"$$mainhelpdir$$colorpalettes.html\">custom color paletes</a>") );
    addInsertAction(menu, tr("Link: Available color palettes"), QString("<a href=\"$$mainhelpdir$$installedcolorpalettes.html\">available color paletes</a>") );
    addInsertAction(menu, tr("Link: Regular Expressions"), QString("<a href=\"$$mainhelpdir$$qf3_qtregexp.html\">regular expressions</a>") );
    addInsertAction(menu, tr("Link: Fit Functions"), QString("<a href=\"$$mainhelpdir$$qf3_fitfunc.html\">fit functions</a>") );
    addInsertAction(menu, tr("Link: Fit Algorithms"), QString("<a href=\"$$mainhelpdir$$qf3_fitalg.html\">fit algorithms</a>") );
    addInsertAction(menu, tr("Link: File Formats"), QString("<a href=\"$$mainhelpdir$$qf3_fileformats.html\">file formats</a>") );
    addInsertAction(menu, tr("Link: GPL 3.0"), QString("<a href=\"$$mainhelpdir$$gpl3_0.html\">GPL 3.0</a>") );
    addInsertAction(menu, tr("Link: FAQs"), QString("<a href=\"$$qf_faqfile$$\">Frequently asked questions</a>") );

    menu->addSeparator();
    addInsertAction(menu, "$$qf_ui_rdr_helpfile$$");
    addInsertAction(menu, "$$qf_ui_rdr_helpfiletitle$$");
    addInsertAction(menu, "$$qf_ui_eval_helpfile$$");
    addInsertAction(menu, "$$qf_ui_eval_helpfiletitle$$");
    addInsertAction(menu, "$$qf_ui_jkqtplotter_helpfile$$");
    addInsertAction(menu, "$$qf_ui_jkqtplotter_helpfiletitle$$");
    addInsertAction(menu, "$$qf_ui_latex_helpfile$$");
    addInsertAction(menu, "$$qf_ui_latex_helpfiletitle$$");
    addInsertAction(menu, "$$qf_mathparser_helpfile$$");
    addInsertAction(menu, "$$qf_mathparser_helpfiletitle$$");
    addInsertAction(menu, "$$qf_faqfile$$");


    menu=new QMenu(tr("page header/footer"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$qf_commondoc_backtop$$");
    menu->addSeparator();
    addInsertAction(menu, "$$qf_commondoc_footer.start$$");
    addInsertAction(menu, "$$qf_commondoc_footer.end$$");
    menu->addSeparator();
    addInsertAction(menu, "$$qf_commondoc_header.start$$");
    addInsertAction(menu, "$$qf_commondoc_header.end$$");
    addInsertAction(menu, "$$qf_commondoc_header.end_notitle$$");
    addInsertAction(menu, "$$qf_commondoc_header.rdr$$");
    addInsertAction(menu, "$$qf_commondoc_header.eval$$");
    addInsertAction(menu, "$$qf_commondoc_header.extension$$");
    addInsertAction(menu, "$$qf_commondoc_header.fitfunc$$");
    addInsertAction(menu, "$$qf_commondoc_header.fitalg$$");
    addInsertAction(menu, "$$qf_commondoc_header.separator$$");


    menu=new QMenu(tr("plugin info"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$local_plugin_icon$$");
    addInsertAction(menu, "$$local_plugin_iconfilename$$");
    addInsertAction(menu, "$$local_plugin_name$$");
    addInsertAction(menu, "$$local_plugin_author$$");
    addInsertAction(menu, "$$local_plugin_copyright$$");
    addInsertAction(menu, "$$local_plugin_weblink_url$$");
    addInsertAction(menu, "$$local_plugin_weblink$$");
    addInsertAction(menu, "$$local_plugin_id$$");
    addInsertAction(menu, "$$local_plugin_subid$$");
    addInsertAction(menu, "$$local_plugin_subname$$");
    addInsertAction(menu, "$$local_plugin_subshortname$$");
    addInsertAction(menu, "$$local_plugin_tutorial_file$$");
    addInsertAction(menu, "$$local_plugin_tutorial_link$$");
    addInsertAction(menu, "$$local_plugin_mainhelp_file$$");
    addInsertAction(menu, "$$local_plugin_mainhelp_link$$");
    addInsertAction(menu, "$$local_plugin_version$$");
    addInsertAction(menu, "$$local_plugin_typehelp_link$$");
    addInsertAction(menu, "$$local_plugin_typehelp_file$$");
    addInsertAction(menu, "$$local_plugin_assets$$");
    addInsertAction(menu, "$$local_plugin_examples$$");
    addInsertAction(menu, "$$local_plugin_help$$");
    addInsertAction(menu, "$$local_plugin_config$$");
    addInsertAction(menu, "$$local_plugin_tutorials$$");
    menu->addSeparator();
    addInsertAction(menu, "$$plugin_info:help:PLUGINID$$");
    addInsertAction(menu, "$$plugin_info:tutorial:PLUGINID$$");
    addInsertAction(menu, "$$plugin_info:helpdir:PLUGINID$$");
    addInsertAction(menu, "$$plugin_info:assetsdir:PLUGINID$$");
    addInsertAction(menu, "$$plugin_info:examplesdir:PLUGINID$$");
    addInsertAction(menu, "$$plugin_info:configdir:PLUGINID$$");
    menu->addSeparator();
    addInsertAction(menu, "$$fitfunction:help:PLUGINID$$");
    addInsertAction(menu, "$$fitfunction:name:PLUGINID$$");
    addInsertAction(menu, "$$fitfunction:short_name:PLUGINID$$");
    menu->addSeparator();
    addInsertAction(menu, "$$fitalgorithm:help:PLUGINID$$");
    addInsertAction(menu, "$$fitalgorithm:name:PLUGINID$$");
    addInsertAction(menu, "$$fitalgorithm:short_name:PLUGINID$$");
    menu->addSeparator();
    addInsertAction(menu, "$$importer:help:PLUGINID$$");
    addInsertAction(menu, "$$importer:name:PLUGINID$$");
    addInsertAction(menu, "$$exporter:help:PLUGINID$$");
    addInsertAction(menu, "$$exporter:name:PLUGINID$$");



    menu=new QMenu(tr("plugin lists"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$list:extension:INTERFACENAME$$");
    addInsertAction(menu, "$$list:fitfunc:$$");
    addInsertAction(menu, "$$list:fitfunc:STARTSWITH$$");
    addInsertAction(menu, "$$list:fitfunc_inplugin:PLUGIN_ID|STARTSWITH$$");
    addInsertAction(menu, "$$list:fitfunc_inplugin:PLUGIN_ID$$");
    addInsertAction(menu, "$$list:fitalg:$$");
    addInsertAction(menu, "$$list:fitalg:STARTSWITH$$");
    addInsertAction(menu, "$$list:importers:STARTSWITH$$");
    addInsertAction(menu, "$$list:exporters:STARTSWITH$$");
    addInsertAction(menu, "$$list:plugin_tutorials:PLUGIN_ID$$");


    menu=new QMenu(tr("help lists"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$tutorials_contents$$");
    addInsertAction(menu, "$$help_contents$$");
    addInsertAction(menu, "$$settings_contents$$");


    menu=new QMenu(tr("insert files"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$insert:FILENAME$$");
    addInsertAction(menu, "$$insertglobal:FILENAME$$");


    menu=new QMenu(tr("insert Equations (LaTeX)"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$math:LATEX$$");
    addInsertAction(menu, "$(LATEX)$");
    addInsertAction(menu, "$$bmath:LATEX$$");
    addInsertAction(menu, "$[LATEX]$");


    menu=new QMenu(tr("insert Literature References"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$ref:<ID>:Text$$");
    addInsertAction(menu, "$$ref:<ID>:$$");
    addInsertAction(menu, "$$invisibleref:<ID>:Text$$");
    addInsertAction(menu, "$$references$$");


    menu=new QMenu(tr("insert Figures"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$fig:FILENAME:CAPTION$$");


    menu=new QMenu(tr("insert Info boxes"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$see:Text$$");
    addInsertAction(menu, "$$note:Text$$");
    addInsertAction(menu, "$$info:Text$$");
    addInsertAction(menu, "$$warning:Text$$");
    addInsertAction(menu, "$$example:Text$$");
    addInsertAction(menu, "$$codeexample:Text$$");
    addInsertAction(menu, "$$bqtt:Text$$");
    addInsertAction(menu, "$$bqcode:Text$$");
    menu->addSeparator();
    addInsertAction(menu, "$$startbox$$");
    addInsertAction(menu, "$$startbox_info$$");
    addInsertAction(menu, "$$startbox_note$$");
    addInsertAction(menu, "$$startbox_see$$");
    addInsertAction(menu, "$$startbox_warning$$");
    addInsertAction(menu, "$$startbox_example$$");
    addInsertAction(menu, "$$startbox:backgroundcolor:bordercolor$$");
    addInsertAction(menu, "$$startbox:lightgrey:midnightblue$$");
    addInsertAction(menu, "$$endbox$$");


    menu=new QMenu(tr("insert Tooltips"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$tooltip:PHRASE$$");


    menu=new QMenu(tr("FAQs"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, tr("FAQ entry"), "$$faq_start$$\n  <a name=\"FAQ1\"><b>Question?</b>\n$$faq_answer$$\n  Answer ...\n$$faq_end$$");
    menu->addSeparator();
    addInsertAction(menu, "$$faq_start");
    addInsertAction(menu, "$$faq_answer");
    addInsertAction(menu, "$$faq_end");

    menu=new QMenu(tr("Function References"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, tr("Function reference entry"), "$$funcref_start$$<a name=\"NAME\"/><!-- func:NAME -->\n  <b><tt><!-- template -->NAME<!-- /template --></tt> - <i> DESCRIPTION </i>:</b>\n$$funcref_description$$\n    DESCRIPTION\n  <!-- /func:NAME -->\n$$funcref_end$$");
    menu->addSeparator();
    addInsertAction(menu, "$$funcref_start");
    addInsertAction(menu, "$$funcref_description");
    addInsertAction(menu, "$$funcref_end");

    menu=new QMenu(tr("insert other markups"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$$tt:Text$$");


    menu=new QMenu(tr("LaTeX"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "\\left(%1#\\right)");
    addInsertAction(menu, "\\left[%1#\\right]");
    addInsertAction(menu, "\\left\\{%1#\\right\\}");
    addInsertAction(menu, "\\left\\langle%1#\\right\\rangle");
    menu->addSeparator();
    addInsertAction(menu, "\\sqrt{%1#}");
    menu->addSeparator();
    addInsertAction(menu, "\\sum_{#}^{}%1");
    addInsertAction(menu, "\\prod_{#}^{}%1");
    addInsertAction(menu, "\\int_{#}^{}%1\\;\\mathrm{d}x");
    addInsertAction(menu, "\\iint_{#}^{}%1\\;\\mathrm{d}^2x");
    addInsertAction(menu, "\\iiint_{#}^{}%1\\;\\mathrm{d}^3x");
    addInsertAction(menu, "\\oint_{#}^{}%1\\;\\mathrm{d}x");
    addInsertAction(menu, "\\lim_{%1#}");
    addInsertAction(menu, "\\argmin_{%1#}");
    addInsertAction(menu, "\\argmax_{%1#}");
    menu->addSeparator();
    addInsertAction(menu, "\\vec{%1#}");
    addInsertAction(menu, "\\hat{%1#}");
    addInsertAction(menu, "\\tilde{%1#}");
    addInsertAction(menu, "\\mathbf{%1#}");
    addInsertAction(menu, "\\mathrm{%1#}");
    addInsertAction(menu, "\\mathit{%1#}");
    addInsertAction(menu, "\\mathsf{%1#}");
    addInsertAction(menu, "\\mathbb{%1#}");
    addInsertAction(menu, "\\mathscript{%1#}");
    addInsertAction(menu, "\\underline{%1#}");
    //addInsertAction(menu, "$$$$");

    menu=new QMenu(tr("Special Characters"), this);
    ui->edtScript->getEditor()->addAction(menu->menuAction());
    addInsertAction(menu, "$" "&#36;");
    addInsertAction(menu, "$$" "&#36;&#36;");


    QTimer::singleShot(AUTOSAVE_INTERVAL_MSEC, this, SLOT(autosave()));

}

QFEHelpEditorWidget::~QFEHelpEditorWidget()
{
    delete ui;
}

QString QFEHelpEditorWidget::getScript() const
{
    return ui->edtScript->getEditor()->toPlainText();
}

void QFEHelpEditorWidget::loadSettings(QSettings &settings, QString prefix)
{
    recentHelpFiles->readSettings(settings, prefix+"recentScripts/");
}

void QFEHelpEditorWidget::storeSettings(QSettings &settings, QString prefix) const
{
    recentHelpFiles->storeSettings(settings, prefix+"recentScripts/");
}

void QFEHelpEditorWidget::setWinID(int winID)
{
    this->m_winID=winID;
}

void QFEHelpEditorWidget::autosave()
{
    if (isVisible()) {
        QDir d(QFPluginServices::getInstance()->getConfigFileDirectory());
        QString ffn= d.absoluteFilePath(QString("helpeditor%1_autosave.html").arg(m_winID));
        if (!getScript().isEmpty() && !qfFileEqualsString(ffn, ui->edtScript->getEditor()->toPlainText().toUtf8())) {
            d.mkpath(d.absolutePath());
            // copy _autosave_old.html --> _autosave_older.html
            QString fn= d.absoluteFilePath(QString("helpeditor%1_autosave_old.html").arg(m_winID));
            QString newfn= d.absoluteFilePath(QString("helpeditor%1_autosave_older.html").arg(m_winID));
            if (QFile::exists(fn)) {
                if (QFile::exists(newfn)) QFile::remove(newfn);
                QFile::copy(fn, newfn);
            }

            // copy _autosave.html --> _autosave_old.html
            fn= ffn;
            newfn= d.absoluteFilePath(QString("helpeditor%1_autosave_old.html").arg(m_winID));
            if (QFile::exists(fn)) {
                if (QFile::exists(newfn)) QFile::remove(newfn);
                QFile::copy(fn, newfn);
            }
            saveFile(fn, false);
        } else {
            qDebug()<<"QFEHelpEditorWidget::autosave(): file contents didn't change!";
        }
    }
    QTimer::singleShot(AUTOSAVE_INTERVAL_MSEC, this, SLOT(autosave()));
}

void QFEHelpEditorWidget::reloadLastAutosave()
{
    QDir d(QFPluginServices::getInstance()->getConfigFileDirectory());
    //if (QFile::exists(d.absoluteFilePath("helpeditor_autosave.html"))) {
        openScript(d.absolutePath(), false);
        setScriptFilename("");
    //}
}

void QFEHelpEditorWidget::documentWasModified()
{
    modified=ui->edtScript->getEditor()->document()->isModified();
}


void QFEHelpEditorWidget::on_btnExecute_clicked()
{
    QFTemporaryFile f;
    QFileInfo fi(currentScript);
    if (QFile::exists(currentScript)) {
        f.setFileTemplate(fi.absolutePath()+"/qfe_helpeditor_temp_XXXXXX.html.tmp");
    }
    if (f.open()) {
        QTextStream txt(&f);
        txt<<ui->edtScript->getEditor()->toPlainText();
        f.close();

        QFPluginServices::getInstance()->displayHelpWindow(f.fileName());
    }
}



void QFEHelpEditorWidget::on_btnNew_clicked()
{
    if (maybeSave()) {
        ui->edtScript->getEditor()->setPlainText("");
        setScriptFilename("");
        lastScript=ui->edtScript->getEditor()->toPlainText();
    }
}

void QFEHelpEditorWidget::on_btnOpen_clicked()
{
    //openScript("last", true);
    if (maybeSave()) {
        openScript("last", true);
    }
}

bool QFEHelpEditorWidget::save() {

    if (currentScript.isEmpty() || newScript) {
        return saveAs();
    } else {
        return saveFile(currentScript);
    }

}

bool QFEHelpEditorWidget::saveFile(const QString &filename, bool setFilename)
{
    QFile f(filename);
    //qDebug()<<"saving to "<<filename;
    if (f.open(QIODevice::WriteOnly|QIODevice::Text)) {
        /*QTextStream s(&f);
        s.setCodec(QTextCodec::codecForName("UTF-8"));
        s<<ui->edtScript->getEditor()->toPlainText().toUtf8();*/
        f.write(ui->edtScript->getEditor()->toPlainText().toUtf8());
        if (setFilename) lastScript=ui->edtScript->getEditor()->toPlainText();
        f.close();
        if (setFilename) newScript=false;
        if (setFilename) setScriptFilename(filename);
    }
    return true;
}

void QFEHelpEditorWidget::on_btnSave_clicked()
{
    save();
}

bool QFEHelpEditorWidget::saveAs()
{
    QString dir=ProgramOptions::getInstance()->getQSettings()->value("QFEHelpEditorWidget/lastScriptDir", ProgramOptions::getInstance()->getMainHelpDirectory()).toString();
    QDir d(dir);
    QString filename=qfGetSaveFileName(this, tr("save online-help page ..."), d.absoluteFilePath(currentScript), tr("QuickFit 3 help (*.html *.htm *.inc *.ini);;All Files (*.*)"));
    if (!filename.isEmpty()) {
        bool ok=true;
        if (ok) {
            bool res=saveFile(filename);
            dir=QFileInfo(filename).absolutePath();
            ProgramOptions::getInstance()->getQSettings()->setValue("QFEHelpEditorWidget/lastScriptDir", dir);
            return res;
        }
    }
    return false;
}

void QFEHelpEditorWidget::on_btnSaveAs_clicked()
{
    saveAs();
}



void QFEHelpEditorWidget::on_btnOpenExample_clicked()
{
    openScript(ProgramOptions::getInstance()->getAssetsDirectory()+"/plugins/qfe_helpeditor/examples/", false);
}

void QFEHelpEditorWidget::on_btnOpenTemplate_clicked()
{
    QDir d=QFileInfo(currentScript).absoluteDir();
    openScript(ProgramOptions::getInstance()->getAssetsDirectory()+"/plugins/qfe_helpeditor/templates/", false);
    setScriptFilename(d.absoluteFilePath(QFileInfo(currentScript).fileName()), true, false);
}

void QFEHelpEditorWidget::on_btnOpenTemplate2_clicked()
{
    on_btnOpenTemplate_clicked();
}

void QFEHelpEditorWidget::edtScript_cursorPositionChanged()
{
    QTextCursor tc = ui->edtScript->getEditor()->textCursor();
    /*
    tc.select(QTextCursor::WordUnderCursor);
    QString text=tc.selectedText();

    QString word=text.toLower();*/


    QString text=ui->edtScript->getEditor()->toPlainText();
    QString word;
    int newPos=tc.position();
    if (newPos>=0 && newPos<text.size()) {
        word+=text[newPos];
        int p=newPos-1;
        while (p>=0 && (text[p].isLetterOrNumber()||text[p]=='_'||text[p]=='.')) {
            word=text[p]+word;
            p--;
        }
        p=newPos+1;
        while (p<text.size() && (text[p].isLetterOrNumber()||text[p]=='_'||text[p]=='.')) {
            word=word+text[p];
            p++;
        }
        word=word.toLower();
    }




    ui->labCursorPos->setText(tr("Line %1, Column %2").arg(ui->edtScript->getEditor()->getLineNumber()).arg(ui->edtScript->getEditor()->getColumnNumber()));
}

void QFEHelpEditorWidget::on_btnHelp_clicked()
{
    QFPluginServices::getInstance()->displayHelpWindow(QFPluginServices::getInstance()->getMainHelpDirectory()+"/qf3_helpref.html");
}

bool QFEHelpEditorWidget::maybeSave() {
    //if (ui->edtScript->getEditor()->toPlainText().isEmpty()) return true;
    //if (ui->edtScript->getEditor()->toPlainText()==lastScript) return true;
    if (ui->edtScript->getEditor()->document()->isModified()) {
        int r=QMessageBox::question(this, tr("save help file ..."), tr("The document has been modified.\n"
                                                                       "Do you want to save your changes?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (r==QMessageBox::Save) {
            return save();
        } else if (r==QMessageBox::Cancel) {
            return false;
        }
    }

    return true;
}

void QFEHelpEditorWidget::setScriptFilename(QString filename, bool newscript, bool addToRecent)
{
    currentScript=filename;
    newScript=QFile::exists(filename)||newscript||filename.isEmpty();
    if (addToRecent && QFile::exists(filename)) recentHelpFiles->addRecentFile(filename);
    if (filename.isEmpty()) ui->labScriptFilename->setText(tr("current file: <tt><i>NEW FILE</i></tt>"));
    else if (QFile::exists(filename) && !newScript) ui->labScriptFilename->setText(tr("current file: <tt><i>%1</i></tt>").arg(QFileInfo(filename).fileName()));
    else ui->labScriptFilename->setText(tr("new file: <tt><i>%1</i></tt>").arg(QFileInfo(filename).fileName()));
    ui->edtScript->getEditor()->document()->setModified(false);
    modified=false;

}

void QFEHelpEditorWidget::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}


QStringListModel *QFEHelpEditorWidget::modelFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
     return new QStringListModel(completer);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty()) {
            //QMessageBox::information(this, "", line.trimmed());
            words << line.trimmed();
        }
    }
    words.sort();
    //QMessageBox::information(this, "", words.join(", "));
    QApplication::restoreOverrideCursor();
    return new QStringListModel(words, completer);
}

void QFEHelpEditorWidget::addInsertAction(const QString &insert)
{
    addInsertAction(tr("insert \"%1\"").arg(insert), insert);
}

void QFEHelpEditorWidget::addInsertAction(const QString &label, const QString &insert)
{
    QAction* act=new QFActionWithNoMenuRole(label, this);
    connect(act, SIGNAL(triggered()), this, SLOT(insertActionClicked()));
    insertmap[act]=insert;
    ui->edtScript->getEditor()->addAction(act);
    ui->edtScript->getEditor()->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void QFEHelpEditorWidget::addInsertAction(QMenu *menu, const QString &insert)
{
    addInsertAction(menu, tr("insert \"%1\"").arg(insert), insert);
}

void QFEHelpEditorWidget::addInsertAction(QMenu *menu, const QString &label, const QString &insert)
{
    QAction* act=new QFActionWithNoMenuRole(label, this);
    connect(act, SIGNAL(triggered()), this, SLOT(insertActionClicked()));
    insertmap[act]=insert;
    //ui->edtScript->getEditor()->addAction(act);
    menu->addAction(act);
    ui->edtScript->getEditor()->setContextMenuPolicy(Qt::ActionsContextMenu);
}


void QFEHelpEditorWidget::openScript(QString dir, bool saveDir, const QString &filename_in) {
    if (maybeSave()) {
        if (dir=="last") {
            dir=ProgramOptions::getInstance()->getQSettings()->value("QFEHelpEditorWidget/lastScriptDir", ProgramOptions::getInstance()->getMainHelpDirectory()).toString();
        }
        QString filename=filename_in;
        if (filename.isEmpty()) filename=qfGetOpenFileName(this, tr("Open help file ..."), dir, tr("QuickFit 3 Help (*.html *.htm *.inc *.ini);;All Files (*.*)"))    ;
        if (QFile::exists(filename)) {
            QFile f(filename);
            if (f.open(QIODevice::ReadOnly|QIODevice::Text)) {
                ui->edtScript->getEditor()->setPlainText(QString::fromUtf8(f.readAll().replace("\t", "    ")));
                setScriptFilename(filename);
                f.close();
                lastScript=ui->edtScript->getEditor()->toPlainText();
                dir=QFileInfo(filename).absolutePath();
            }
        }
        if (saveDir) ProgramOptions::getInstance()->getQSettings()->setValue("QFEHelpEditorWidget/lastScriptDir", dir);
    }

}

void QFEHelpEditorWidget::openScriptNoAsk(QString filename)
{
    if (maybeSave()) {
        if (QFile::exists(filename)) {
            QFile f(filename);
            if (f.open(QIODevice::ReadOnly|QIODevice::Text)) {
                ui->edtScript->getEditor()->setPlainText(QString::fromUtf8(f.readAll()));
                setScriptFilename(filename);
                f.close();
                lastScript=ui->edtScript->getEditor()->toPlainText();
            }
        }
    }
}

void QFEHelpEditorWidget::on_btnBold_clicked()
{
    insertAroundOld("<b>%1#</b>");
}

void QFEHelpEditorWidget::on_btnItalic_clicked()
{
    insertAroundOld("<i>%1#</i>");
}

void QFEHelpEditorWidget::on_btnUnderline_clicked()
{
    insertAroundOld("<u>%1#</u>");
}

void QFEHelpEditorWidget::on_btnImage_clicked()
{
    insertAroundOld("<img src=\"image.png\">%1#");

}

void QFEHelpEditorWidget::on_btnAnchor_clicked()
{
    insertAroundOld("<a name=\"\">%1");
}

void QFEHelpEditorWidget::on_btnLink_clicked()
{
    bool ok=false;
    QStringList links;
    links<<"http://";
    links<<"opendir://";
    links<<"openfile://";
    QString txt=ui->edtScript->getEditor()->toPlainText();
    QRegExp rxLink("<a\\s*[^>]*name\\s*=\\s*\"\\#([^\"]*)\"[^>]*>", Qt::CaseInsensitive);
    rxLink.setMinimal(false);
    int count = 0;
    int pos = 0;
    while ((pos = rxLink.indexIn(txt, pos)) != -1) {

        QString name=rxLink.cap(1).toLower();
        links<<QString("#"+name);
        ++count;
        pos += rxLink.matchedLength();
    }

    links<<"opendir://$$maindir$$";
    links<<"opendir://$$assetsdir$$";
    links<<"opendir://$$examplesdir$$";
    links<<"$$mainhelpdir$$/";
    links<<"$$assetsdir$$/";
    links<<"$$examplesdir$$/";
    links<<"$$configdir$$/";
    //links<<"$$$$/";
    links<<"mailto:$$email$$";
    links<<"mailto:$$maillist$$";
    links<<"$$weblink$$";
    //links<<"";

    QDir d=QFileInfo(currentScript).absoluteDir();
    QStringList filters;
    filters<<"*.html"<<"*.htm";
    links<<d.entryList(filters, QDir::Files);



    QString link=QInputDialog::getItem(this, tr("insert link"), tr("link target:"), links, 0, true,&ok);
    if (ok) {
        insertAroundOld(QString("<a href=\"%1\">").arg(link)+QString("%1#</a>"));
    }
}

void QFEHelpEditorWidget::on_btnBlockquote_clicked()
{
    insertAroundOld("\n<blockquote>\n    %1#\n</blockquote>\n");
}

void QFEHelpEditorWidget::on_btnNumberedList_clicked()
{
    insertAroundOld("\n<ol>\n    <li>%1#</li>\n</ol>");

}

void QFEHelpEditorWidget::on_btnBulletList_clicked()
{
    insertAroundOld("\n<ul>\n    <li>%1#</li>\n</ul>");
}

void QFEHelpEditorWidget::on_btnInsertH1_clicked()
{
    insertAroundOld("\n\n<h1>%1#</h1>\n<p></p>\n", true);
}

void QFEHelpEditorWidget::on_btnInsertH2_clicked()
{
    insertAroundOld("\n\n<h2>%1#</h2>\n<p></p>\n", true);
}

void QFEHelpEditorWidget::on_btnInsertH3_clicked()
{
    insertAroundOld("\n\n<h3>%1#</h3>\n<p></p>\n", true);
}

void QFEHelpEditorWidget::on_btnInsertH4_clicked()
{
    insertAroundOld("\n\n<h4>%1#</h4>\n<p></p>\n", true);
}

void QFEHelpEditorWidget::on_btnInsertParagraph_clicked()
{
    insertAroundOld("\n<p>\n    %1\n</p>\n");
}

void QFEHelpEditorWidget::on_btnInsertListItem_clicked()
{
    insertAroundOld("\n<li>%1#</li>");
}

void QFEHelpEditorWidget::on_btnPasteImage_clicked()
{
    PasteImageDlg* dlg=new PasteImageDlg(QFileInfo(currentScript).absolutePath(), this, QString(), QString("./pic/%1").arg(QFileInfo(currentScript).baseName())+QString("_pic%1.png") );
    if (dlg->exec()) {
        insertAroundOld(dlg->saveImage()+"%1");
    }
    delete dlg;
}

void QFEHelpEditorWidget::on_btnInsertAndCopyImage_clicked()
{
    QString dir=ProgramOptions::getInstance()->getQSettings()->value("QFEHelpEditorWidget/lastImageDir", ProgramOptions::getInstance()->getMainHelpDirectory()).toString();
    QString filename=qfGetOpenFileName(this, tr("select image file ..."), dir, tr("Image Files (*.png *.jpg *.tif *.svg *.bmp)") );
    if (!filename.isEmpty()) {
        dir=QFileInfo(filename).absolutePath();
        ProgramOptions::getInstance()->getQSettings()->setValue("QFEHelpEditorWidget/lastImageDir", dir);
        PasteImageDlg* dlg=new PasteImageDlg(QFileInfo(currentScript).absolutePath(), this, filename, QString("./pic/%1").arg(QFileInfo(currentScript).baseName())+QString("_pic%1.png") );
        if (dlg->exec()) {
            insertAroundOld("\n"+dlg->saveImage()+"%1");
        }
        delete dlg;
    }
}

void QFEHelpEditorWidget::on_btnInsertCode_clicked()
{
    insertAroundOld("<code>%1#</code>");
}

void QFEHelpEditorWidget::on_btnInsertMath_clicked()
{
    insertAroundOld("$$math:#$$%1");
}

void QFEHelpEditorWidget::on_btnInsertBMath_clicked()
{
    insertAroundOld("$$bmath:#$$%1");
}

void QFEHelpEditorWidget::on_btnInsertPluginLink_clicked()
{
    PluginLinkDialog* dlg=new PluginLinkDialog(this);
    if (dlg->exec()) {
        insertAroundOld(dlg->insertText()+"%1#");
    }
    delete dlg;

}

void QFEHelpEditorWidget::on_btnInsertSubPluginLink_clicked()
{
    SubPluginLinkDialog* dlg=new SubPluginLinkDialog(this);
    if (dlg->exec()) {
        insertAroundOld(dlg->insertText()+"%1#");
    }
    delete dlg;

}
void QFEHelpEditorWidget::on_btnCenter_clicked()
{
    insertAroundOld("\n<center>%1#</center>");
}

void QFEHelpEditorWidget::on_btnInsertFAQ_clicked()
{
    FAQEntryDialog* dlg=new FAQEntryDialog(this);
    if (dlg->exec()) {
        insertAroundOld(QString("%1#")+dlg->insertText());
    }
    delete dlg;
}

void QFEHelpEditorWidget::on_btnFunctionReference_clicked()
{
    FunctionReferenceDialog* dlg=new FunctionReferenceDialog(this);
    if (dlg->exec()) {
        insertAroundOld(QString("%1#")+dlg->insertText());
    }
    delete dlg;
}

void QFEHelpEditorWidget::on_btnTable_clicked()
{
    NewTableDialog* dlg=new NewTableDialog(this);
    if (dlg->exec()) {
        insertAroundOld(QString("%1#")+dlg->insertText());
    }
    delete dlg;
}

void QFEHelpEditorWidget::on_btnSelectImage_clicked()
{
    QString dir=QFileInfo(currentScript).absolutePath();//ProgramOptions::getInstance()->getQSettings()->value("QFEHelpEditorWidget/lastImageDir", ProgramOptions::getInstance()->getMainHelpDirectory()).toString();
    QStringList dirs, prefixes, baseNodeNames;
    dirs<<dir<<QFPluginServices::getInstance()->getMainHelpDirectory();
    prefixes<<""<<"$$mainhelpdir$$";
    baseNodeNames<<tr("local")<<tr("main help");

    SelectResourceImage* dlg=new SelectResourceImage(dirs, this, prefixes, baseNodeNames);
    if (dlg->exec()) {
        insertAroundOld(QString("<img src=\"%1\">#").arg(QDir(dir).relativeFilePath(dlg->getSelectFile())));
    }
    delete dlg;

/*

    QString filename=qfGetOpenFileName(this, tr("select image file ..."), dir, tr("Image Files (*.png *.jpg *.tif *.svg *.bmp)") );
    if (!filename.isEmpty()) {
        dir=QFileInfo(filename).absolutePath();
        insertAroundOld(QString("<img src=\"%1\">").arg(QFileInfo(dir).absoluteDir().relativeFilePath(filename)));
    }*/
}



void QFEHelpEditorWidget::findFirst()
{
    if (!findDlg) findDlg=new QFEHelpEditorFindDialog(this);

    if (ui->edtScript->getEditor()->hasSelection()) findDlg->setPhrase(ui->edtScript->getEditor()->getSelection());
    if (findDlg->exec()==QDialog::Accepted) {
        // enable "Find next" action
        findNextAct->setEnabled(true);
        findNextAct->setIcon(QIcon(":/qfe_helpeditor/script_find_next.png"));
        findNextAct->setText(tr("Find &next"));
        findNextAct->setStatusTip(tr("Find the next occurence "));
        disconnect(findNextAct, SIGNAL(triggered()), this, 0);
        connect(findNextAct, SIGNAL(triggered()), this, SLOT(findNext()));

        if (!ui->edtScript->getEditor()->findFirst(findDlg->getPhrase(), findDlg->getSearchFromStart(), findDlg->getMatchCase(), findDlg->getWholeWords())) {
            QMessageBox::information(this, tr("Find ..."),
                             tr("Did not find '%1' ...")
                             .arg(ui->edtScript->getEditor()->getPhrase()));
            findNextAct->setEnabled(false);
        }
    }
}

void QFEHelpEditorWidget::findNext()
{
    if (!ui->edtScript->getEditor()->findNext()) {
        QMessageBox::information(this, tr("Find ..."),
                         tr("Did not find '%1' ...")
                         .arg(ui->edtScript->getEditor()->getPhrase()));
    }
}

void QFEHelpEditorWidget::replaceFirst()
{
    if (!replaceDlg) replaceDlg=new QFEHelpEditorReplaceDialog(this);
    if (ui->edtScript->getEditor()->hasSelection()) replaceDlg->setPhrase(ui->edtScript->getEditor()->getSelection());
    if (replaceDlg->exec()==QDialog::Accepted) {
        // enable "Find next" action
        findNextAct->setEnabled(true);
        findNextAct->setIcon(QIcon(":/qfe_helpeditor/script_find_replace_next.png"));
        findNextAct->setText(tr("Replace &next"));
        findNextAct->setStatusTip(tr("Replace the next occurence "));
        disconnect(findNextAct, SIGNAL(triggered()), this, 0);
        connect(findNextAct, SIGNAL(triggered()), this, SLOT(replaceNext()));

        if (!ui->edtScript->getEditor()->replaceFirst(replaceDlg->getPhrase(), replaceDlg->getReplace(), replaceDlg->getSearchFromStart(), replaceDlg->getMatchCase(), replaceDlg->getWholeWords(), replaceDlg->getReplaceAll(), replaceDlg->getAskBeforeReplace())) {
            QMessageBox::information(this, tr("Find & Replace..."),
                             tr("Did not find '%1' ...")
                             .arg(ui->edtScript->getEditor()->getPhrase()));
        }
    }
}

void QFEHelpEditorWidget::replaceNext()
{
    if (! ui->edtScript->getEditor()->replaceNext()) {
        QMessageBox::information(this, tr("Find & Replace ..."),
                                 tr("Did not find '%1' ...")
                                 .arg(ui->edtScript->getEditor()->getPhrase()));
    }

}

void QFEHelpEditorWidget::gotoLine()
{
    int maxLine=ui->edtScript->getEditor()->document()->blockCount();
    bool ok;
    unsigned long line = QInputDialog::getInt(this, tr("Goto Line ..."),
                              tr("Enter a line number (1 - %2):").arg(maxLine), 1, 1, maxLine, 1, &ok);
    if (ok) {
        ui->edtScript->getEditor()->gotoLine(line);
        ui->edtScript->getEditor()->setFocus();
    }

}

void QFEHelpEditorWidget::print()
{
#ifndef QT_NO_PRINTER
   QPrinter printer;

   QPrintDialog *dialog = new QPrintDialog(&printer, this);
   dialog->setWindowTitle(tr("Print Document"));
   if (ui->edtScript->getEditor()->textCursor().hasSelection())
       dialog->addEnabledOption(QAbstractPrintDialog::PrintSelection);
   if (dialog->exec() != QDialog::Accepted)
       return;

   ui->edtScript->getEditor()->print(&printer);
#endif

}

void QFEHelpEditorWidget::printPreviewClick()
{
#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, SIGNAL(paintRequested(QPrinter *)), SLOT(printPreview(QPrinter *)));
    preview.exec();
#endif
}

void QFEHelpEditorWidget::printPreview(QPrinter *printer)
{
#ifndef QT_NO_PRINTER
    ui->edtScript->getEditor()->print(printer);
#endif

}

void QFEHelpEditorWidget::clearFindActions()
{
    findNextAct->setEnabled(false);
}

void QFEHelpEditorWidget::insertActionClicked()
{
    QAction* act=qobject_cast<QAction*>(sender());
    if (act) {
        QString txt=insertmap.value(act, "");
        if (!txt.isEmpty()) {
            //ui->edtScript->getEditor()->insertPlainText(txt);
            insertAroundOld(txt);
        }
    }
}

void QFEHelpEditorWidget::insertIcon()
{
    SelectResourceImage* dlg=new SelectResourceImage(":/", this);
    if (dlg->exec()) {
        insertAroundOld(QString("<img src=\"%1\">#").arg(dlg->getSelectFile()));
    }
}

void QFEHelpEditorWidget::toEntity()
{
    QString txt=getSelection();
    replaceSelection(escapeHTML(txt));
}

void QFEHelpEditorWidget::toChars()
{
    QString txt=getSelection();
    replaceSelection(deescapeHTML(txt));
}

void QFEHelpEditorWidget::insertAroundOld(const QString &newText, bool ensureNewLine)
{
    QString txt=ui->edtScript->getEditor()->getSelection()+newText;
    if (newText.contains("%1")) txt=newText.arg(ui->edtScript->getEditor()->getSelection());
    if (!txt.isEmpty()) {
        QTextCursor cur=ui->edtScript->getEditor()->textCursor();
        cur.select(QTextCursor::LineUnderCursor);
        QString line=cur.selectedText();
        cur.movePosition(QTextCursor::Up);
        cur.select(QTextCursor::LineUnderCursor);
        QString lastline=cur.selectedText();

        int column=ui->edtScript->getEditor()->textCursor().positionInBlock();

        int wscll=0, i=0; // starting whitespaces in previous line
        while (i<lastline.size() && (lastline[i]==' ' || lastline[i]=='\t')) {
            if (lastline[i]==' ') wscll++;
            if (lastline[i]=='\t') wscll+=4;
            i++;
        }

        bool lineEmpty=true;
        int wsc=0; // starting whitespaces in current line
        i=0;
        while (i<line.size() && (line[i]==' ' || line[i]=='\t')) {
            if (line[i]==' ') wsc++;
            if (line[i]=='\t') wsc+=4;
            i++;            
        }
        lineEmpty=(i>=line.size());
        if (lineEmpty && !ensureNewLine) {
            wsc=wscll;
            while (txt.size()>0 && txt[0].isSpace()) {
                txt=txt.remove(0,1);
            }
            if (column<wscll) {
                txt=QString(wscll-column, ' ')+txt;
            }
        }
        txt=txt.replace('\n', "\n"+QString(wsc, QChar(' ')));

        int pos=txt.indexOf('#');
        if (pos>=0) txt=txt.remove('#');
        int textpos=ui->edtScript->getEditor()->textCursor().position();

        ui->edtScript->getEditor()->insertPlainText(txt);
        if (pos>=0) {
            for (int i=0; i<txt.length()-pos; i++) {
                ui->edtScript->getEditor()->moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
            }
        }

    }

}

QString QFEHelpEditorWidget::getSelection() const
{
    return ui->edtScript->getEditor()->getSelection();
}

void QFEHelpEditorWidget::replaceSelection(const QString &newText)
{
    QTextCursor cur=ui->edtScript->getEditor()->textCursor();
    cur.insertText(newText);
}


