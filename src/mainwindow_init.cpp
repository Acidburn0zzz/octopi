/*
* This file is part of Octopi, an open-source GUI for ArchLinux pacman.
* Copyright (C) 2013  Alexandre Albuquerque Arnt
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

/*
 * This is a MainWindow's initialization code
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "strconstants.h"
#include "uihelper.h"
#include "treeviewpackagesitemdelegate.h"
#include <QLabel>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QTextBrowser>
#include <QResource>
#include <QCryptographicHash>
#include <QTextStream>
#include <QDomDocument>
#include <QFile>
#include <iostream>

/*
 * If we have some outdated packages, let's put octopi in a red face/angry state ;-)
 */
void MainWindow::initAppIcon()
{
  m_outdatedPackageList = Package::getOutdatedPackageList();
  m_numberOfOutdatedPackages = m_outdatedPackageList->count();
  refreshAppIcon();
}

/*
 * Inits the toolbar, including taking that annoying default view action out of the game
 */
void MainWindow::initToolBar()
{
  ui->mainToolBar->addAction(ui->actionSyncPackages);
  ui->mainToolBar->addAction(ui->actionCommit);
  ui->mainToolBar->addAction(ui->actionRollback);
  ui->mainToolBar->addSeparator();
  ui->mainToolBar->addAction(ui->actionExit);

  ui->mainToolBar->toggleViewAction()->setEnabled(false);
  ui->mainToolBar->toggleViewAction()->setVisible(false);
}

/*
 * The only thing needed here is to create a dynamic label which will contain the package counters
 */
void MainWindow::initStatusBar()
{
  m_lblCounters = new QLabel(this);
}

/*
 * Sets the TabWidget Properties to the given index/tab and change app focus to its child widget
 */
void MainWindow::_changeTabWidgetPropertiesIndex(const int newIndex)
{
  ui->twProperties->setCurrentIndex(newIndex);
  ui->twProperties->currentWidget()->childAt(1,1)->setFocus();
}

/*
 * Sets the current tabWidget index, based on last user session
 */
void MainWindow::initTabWidgetPropertiesIndex()
{
  ui->twProperties->setCurrentIndex(ctn_TABINDEX_INFORMATION);
}

/*
 * This is the 4th tab (Transaction).
 * It pops up whenever the user selects a remove/install action on a selected package
 */
void MainWindow::initTabTransaction()
{
  m_modelTransaction = new QStandardItemModel(this);
  QWidget *tabTransaction = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabTransaction);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setMargin(0);

  QTreeView *tvTransaction = new QTreeView(tabTransaction);
  tvTransaction->setObjectName("tvTransaction");
  tvTransaction->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tvTransaction->setDropIndicatorShown(false);
  tvTransaction->setAcceptDrops(false);
  tvTransaction->setItemDelegate(new TreeViewPackagesItemDelegate(tvTransaction));
  tvTransaction->header()->setSortIndicatorShown(false);
  tvTransaction->header()->setClickable(false);
  tvTransaction->header()->setMovable(false);
  tvTransaction->setFrameShape(QFrame::NoFrame);
  tvTransaction->setFrameShadow(QFrame::Plain);
  tvTransaction->setStyleSheet(StrConstants::getTreeViewCSS(SettingsManager::getPkgListFontSize()));
  tvTransaction->expandAll();

  m_modelTransaction->setSortRole(0);
  m_modelTransaction->setColumnCount(0);

  QStringList sl;
  m_modelTransaction->setHorizontalHeaderLabels(sl << StrConstants::getPackages());

  QStandardItem *siToBeRemoved = new QStandardItem(IconHelper::getIconToRemove(),
                                                   StrConstants::getTodoRemoveText());
  QStandardItem *siToBeInstalled = new QStandardItem(IconHelper::getIconToInstall(),
                                                     StrConstants::getTodoInstallText());

  m_modelTransaction->appendRow(siToBeRemoved);
  m_modelTransaction->appendRow(siToBeInstalled);

  gridLayoutX->addWidget(tvTransaction, 0, 0, 1, 1);

  tvTransaction->setModel(m_modelTransaction);

  QString aux(StrConstants::getTabTransactionName());

  ui->twProperties->removeTab(ctn_TABINDEX_TRANSACTION);
  ui->twProperties->insertTab(ctn_TABINDEX_TRANSACTION, tabTransaction, QApplication::translate (
                                "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ));
}

/*
 * This is the LineEdit widget used to filter the package list
 */
void MainWindow::initLineEditFilterPackages(){
  connect(ui->leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
}

/*
 * This is the package treeview, it lists the installed [and not installed] packages in the system
 */
void MainWindow::initPackageTreeView()
{
  m_proxyModelPackages = new QSortFilterProxyModel(this);
  m_modelPackages = new QStandardItemModel(this);
  m_modelInstalledPackages = new QStandardItemModel(this);
  m_proxyModelPackages->setSourceModel(m_modelPackages);
  m_proxyModelPackages->setFilterKeyColumn(1);
  ui->tvPackages->setItemDelegate(new TreeViewPackagesItemDelegate(ui->tvPackages));
  ui->tvPackages->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->tvPackages->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->tvPackages->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tvPackages->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
  ui->tvPackages->setAllColumnsShowFocus( true );
  ui->tvPackages->setModel(m_proxyModelPackages);
  ui->tvPackages->setSortingEnabled( true );
  ui->tvPackages->sortByColumn( 1, Qt::AscendingOrder);
  ui->tvPackages->setIndentation( 0 );
  ui->tvPackages->header()->setSortIndicatorShown(true);
  ui->tvPackages->header()->setClickable(true);
  ui->tvPackages->header()->setMovable(false);
  ui->tvPackages->header()->setDefaultAlignment( Qt::AlignLeft );
  ui->tvPackages->header()->setResizeMode( QHeaderView::Fixed );
  ui->tvPackages->setStyleSheet(
        StrConstants::getTreeViewCSS(SettingsManager::getPackagesInDirFontSize()));

  //Prepare it for drag operations
  //tvPackage->setDragEnabled(true);
}

/*
 * This tab has a QTextBrowser which shows information about the selected package
 */
void MainWindow::initTabInfo(){
  QWidget *tabInfo = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout ( tabInfo );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );

  QTextBrowser *text = new QTextBrowser(tabInfo);
  text->setObjectName("textBrowser");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
  gridLayoutX->addWidget ( text, 0, 0, 1, 1 );

  QString aux(StrConstants::getTabInfoName());
  //QString translated_about = QApplication::translate ( "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 );

  ui->twProperties->removeTab(ctn_TABINDEX_INFORMATION);
  /*int tindex =*/ ui->twProperties->insertTab(ctn_TABINDEX_INFORMATION, tabInfo, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  //ui->twProperties->setTabText(ui->twProperties->indexOf(tabInfo), QApplication::translate(
  //    "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_INFORMATION);
  text->show();
  text->setFocus();
}

/*
 * This is the files treeview, which shows the directory structure of ONLY installed packages's files.
 */
void MainWindow::initTabFiles()
{
  QWidget *tabPkgFileList = new QWidget(this);
  QGridLayout *gridLayoutX = new QGridLayout ( tabPkgFileList );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );
  QStandardItemModel *modelPkgFileList = new QStandardItemModel(this);
  QTreeView *tvPkgFileList = new QTreeView(tabPkgFileList);
  tvPkgFileList->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tvPkgFileList->setDropIndicatorShown(false);
  tvPkgFileList->setAcceptDrops(false);
  tvPkgFileList->header()->setSortIndicatorShown(false);
  tvPkgFileList->header()->setClickable(false);
  tvPkgFileList->header()->setMovable(false);
  tvPkgFileList->setFrameShape(QFrame::NoFrame);
  tvPkgFileList->setFrameShadow(QFrame::Plain);
  tvPkgFileList->setObjectName("tvPkgFileList");
  tvPkgFileList->setStyleSheet(StrConstants::getTreeViewCSS(SettingsManager::getPkgListFontSize()));

  modelPkgFileList->setSortRole(0);
  modelPkgFileList->setColumnCount(0);
  gridLayoutX->addWidget(tvPkgFileList, 0, 0, 1, 1);
  tvPkgFileList->setModel(modelPkgFileList);

  QString aux(StrConstants::getTabFilesName());
  ui->twProperties->removeTab(ctn_TABINDEX_FILES);
  ui->twProperties->insertTab(ctn_TABINDEX_FILES, tabPkgFileList, QApplication::translate (
                                                  "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );

  /*twTODO->setTabText(twTODO->indexOf(tabPkgFileList), QApplication::translate(
      "MainWindow", tabName.toUtf8(), 0, QApplication::UnicodeUTF8));*/

  tvPkgFileList->setContextMenuPolicy(Qt::CustomContextMenu);

  //connect(tvPkgFileList, SIGNAL(customContextMenuRequested(QPoint)),
  //        this, SLOT(execContextMenuPkgFileList(QPoint)));
  //connect(tvPkgFileList, SIGNAL(clicked (const QModelIndex&)),
  //        this, SLOT(showFullPathOfObject(const QModelIndex&)));

  connect(tvPkgFileList, SIGNAL(doubleClicked (const QModelIndex&)),
          this, SLOT(openFile(const QModelIndex&)));
  //connect(tvPkgFileList, SIGNAL(activated(const QModelIndex)),
  //        this, SLOT(openFileOrDirectory(QModelIndex)));

  //connect(tvPkgFileList, SIGNAL(pressed (const QModelIndex&)),
  //        tvPkgFileList, SIGNAL(clicked (const QModelIndex&)));
}

/*
 * Retrieves the RSS news feed from "https://www.archlinux.org/feeds/news/"
 * If it fails to connect to the internet, uses the available "./.config/octopi/arch_rss.xml"
 * The result is a QString containing the RSS News Feed XML code
 */
QString MainWindow::retrieveArchNews(bool searchForLatestNews)
{
  QString res;
  QString tmpRssPath = QDir::homePath() + QDir::separator() + ".config/octopi/.tmp_arch_rss.xml";
  QString rssPath = QDir::homePath() + QDir::separator() + ".config/octopi/arch_rss.xml";
  QString contentsRss;

  QFile fileRss(rssPath);
  if (!fileRss.open(QIODevice::ReadOnly | QIODevice::Text)) res = "";
  QTextStream in2(&fileRss);
  contentsRss = in2.readAll();
  fileRss.close();

  if(searchForLatestNews && UnixCommand::hasInternetConnection())
  {
    QString curlCommand = "curl %1 -o %2";

    curlCommand = curlCommand.arg("https://www.archlinux.org/feeds/news/").arg(tmpRssPath);
    if (UnixCommand::runCurlCommand(curlCommand) == 0)
    {
      QFile fileTmpRss(tmpRssPath);
      QFile fileRss(rssPath);

      if (!fileRss.exists())
      {
        fileTmpRss.rename(tmpRssPath, rssPath);
      }
      else
      {
        //A rss file already exists. We have to make a SHA1 hash to compare the contents
        QString tmpRssSHA1;
        QString rssSHA1;
        QString contentsTmpRss;

        QFile fileTmpRss(tmpRssPath);
        if (!fileTmpRss.open(QIODevice::ReadOnly | QIODevice::Text)) res = "";
        QTextStream in(&fileTmpRss);
        contentsTmpRss = in.readAll();
        fileTmpRss.close();

        tmpRssSHA1 = QCryptographicHash::hash(contentsTmpRss.toAscii(), QCryptographicHash::Sha1);
        rssSHA1 = QCryptographicHash::hash(contentsRss.toAscii(), QCryptographicHash::Sha1);

        if (tmpRssSHA1 != rssSHA1){
          fileRss.remove();
          fileTmpRss.rename(tmpRssPath, rssPath);

          res = "*" + contentsTmpRss; //The asterisk indicates there is a MORE updated rss!
        }
        else
        {
          fileTmpRss.remove();
          res = contentsRss;
        }
      }
    }
  }
  //Either we don't have internet or we weren't asked to retrieve the latest news
  else
  {
    QFile fileRss(rssPath);

    //Maybe we have a file in "./.config/octopi/arch_rss.xml"
    if (fileRss.exists())
    {
      res = contentsRss;
    }
    else if (searchForLatestNews)
    {
      res = "<h3><font color=\"red\">" + StrConstants::getInternetUnavailableError() + "</font></h3>";
    }
    else
    {
      res = "<h3><font color=\"red\">" + StrConstants::getNewsErrorMessage() + "</font></h3>";
    }
  }

  return res;
}

/*
 * Parses the raw XML contents from the ArchLinux RSS news feed
 * Creates and returns a string containing a HTML code with latest 10 news
 */
QString MainWindow::parseArchNews()
{
  QString html = "<p align=\"center\"><h2>" + StrConstants::getArchLinuxNews() + "</h2></p><ul>";
  QString lastBuildDate;
  QString rssPath = QDir::homePath() + QDir::separator() + ".config/octopi/arch_rss.xml";
  QDomDocument doc("rss");
  int itemCounter=0;

  QFile file(rssPath);
  if (!file.open(QIODevice::ReadOnly)) return "";
  if (!doc.setContent(&file)) {
      file.close();
      return "";
  }
  file.close();

  QDomElement docElem = doc.documentElement(); //This is rss
  QDomNode n = docElem.firstChild(); //This is channel
  n = n.firstChild();

  while(!n.isNull()) {
    QDomElement e = n.toElement();

    if(!e.isNull())
    {
      if (e.tagName() == "lastBuildDate")
      {
        lastBuildDate = e.text();
      }
      else if(e.tagName() == "item")
      {
        //Let's iterate over the 10 lastest "item" news
        if (itemCounter == 10) break;

        QDomNode text = e.firstChild();

        while(!text.isNull())
        {
          QDomElement eText = text.toElement();

          if(!eText.isNull())
          {
            if (eText.tagName() == "title")
            {
              html += "<li><h3>" + eText.text() + "</h3><br>";
            }
            else if (eText.tagName() == "link")
            {
              html += Package::makeURLClickable(eText.text()) ;
            }
            else if (eText.tagName() == "description")
            {
              QString description = eText.text();
              description = description.remove(QRegExp("\\n"));
              html += description + "<br></li>";
            }
          }

          text = text.nextSibling();
        }

        itemCounter++;
      }
    }

    n = n.nextSibling();
  }

  html += "</ul>";
  return html;
}

/*
 * This is the high level method that orquestrates the ArchLinux RSS News printing in tabNews
 */
void MainWindow::refreshArchNews(bool searchForLatestNews)
{
  qApp->processEvents();
  CPUIntensiveComputing cic;
  QString archRSSXML = retrieveArchNews(searchForLatestNews);
  QString html;

  if (archRSSXML.count() >= 200)
  {
    if (archRSSXML.at(0)=='*')
    {
      //If this is an updated RSS, we must warn the user!
      ui->twProperties->setTabText(ctn_TABINDEX_NEWS, "** " + StrConstants::getTabNewsName() + " **");
      ui->twProperties->setCurrentIndex(ctn_TABINDEX_NEWS);
    }
    else
    {
      ui->twProperties->setTabText(ctn_TABINDEX_NEWS, StrConstants::getTabNewsName());
    }

    //First, we have to parse the raw RSS XML...
    html = parseArchNews();
  }
  else
  {
    ui->twProperties->setTabText(ctn_TABINDEX_NEWS, StrConstants::getTabNewsName());
    html = archRSSXML;
  }

  //Now that we have the html table code, let's put it into TextBrowser's News tab
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_NEWS)->findChild<QTextBrowser*>("textBrowser");
  if (text)
  {
    text->clear();
    text->setHtml(html);
  }
}

/*
 * This SLOT is called every time the user clicks a url inside newsTab textBrowser
 */
void MainWindow::onTabNewsSourceChanged(QUrl newSource)
{
  if(newSource.isRelative())
  {
    //If the user clicked a relative and impossible to display link...
    QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_NEWS)->findChild<QTextBrowser*>("textBrowser");
    if (text)
    {
      disconnect(text, SIGNAL(sourceChanged(QUrl)), this, SLOT(onTabNewsSourceChanged(QUrl)));
      text->setHtml(parseArchNews());
      connect(text, SIGNAL(sourceChanged(QUrl)), this, SLOT(onTabNewsSourceChanged(QUrl)));
    }
  }
}

/*
 * This is the TextBrowser News tab, which shows the latest news from Archlinux news feed
 */
void MainWindow::initTabNews()
{
  QString aux(StrConstants::getTabNewsName());
  QWidget *tabNews = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabNews);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setMargin(0);

  QTextBrowser *text = new QTextBrowser(tabNews);
  text->setObjectName("textBrowser");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
  gridLayoutX->addWidget(text, 0, 0, 1, 1);

  text->show();

  int tindex = ui->twProperties->insertTab(ctn_TABINDEX_NEWS, tabNews, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  ui->twProperties->setTabText(ui->twProperties->indexOf(tabNews), QApplication::translate(
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

  //QWidget *w = m_tabBar->tabButton(tindex, QTabBar::RightSide);
  //w->setToolTip(tr("Close tab"));
  //w->setObjectName("toolButton");

  //SearchBar *searchBar = new SearchBar(this);
  //MyHighlighter *highlighter = new MyHighlighter(text, "");

  /*
  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChanged(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosed()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findNextButtonClicked()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findPreviousButtonClicked()), this, SLOT(searchBarFindPrevious()));

  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);
  gridLayoutX->addWidget(new SyntaxHighlighterWidget(this, highlighter));
  */

  connect(text, SIGNAL(sourceChanged(QUrl)), this, SLOT(onTabNewsSourceChanged(QUrl)));

  text->show();
  ui->twProperties->setCurrentIndex(tindex);
  text->setFocus();
}

/*
 * This is the TextEdit output tab, which shows the output of pacman commands.
 */
void MainWindow::initTabOutput()
{
  QWidget *tabOutput = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabOutput);
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );

  QTextBrowser *text = new QTextBrowser(tabOutput);
  text->setObjectName("textOutputEdit");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  gridLayoutX->addWidget (text, 0, 0, 1, 1);

  QString aux(StrConstants::getTabOutputName());
  //QString translated_about = QApplication::translate ( "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 );

  ui->twProperties->removeTab(ctn_TABINDEX_OUTPUT);
  ui->twProperties->insertTab(ctn_TABINDEX_OUTPUT, tabOutput, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  //ui->twProperties->setTabText(ui->twProperties->indexOf(tabInfo), QApplication::translate(
  //    "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  text->show();
  text->setFocus();
}

/*
 * Initialize the Help tab with basic information about using Octopi
 */
void MainWindow::initTabHelpAbout()
{
  QString aux(StrConstants::getHelp());
  QWidget *tabHelpAbout = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabHelpAbout);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setMargin(0);

  QTextBrowser *text = new QTextBrowser(tabHelpAbout);
  text->setObjectName("textBrowser");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
  gridLayoutX->addWidget(text, 0, 0, 1, 1);

  QString url = "qrc:/resources/help/help_" + QLocale::system().name() + ".html";
  QResource resource(url);

  if(resource.isValid())
  {
    text->setSource(QUrl(url));
  }
  else
  {
    url = "qrc:/resources/help/help_en_US.html";
    text->setSource(QUrl(url));
  }

  text->show();

  int tindex = ui->twProperties->addTab(tabHelpAbout, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  ui->twProperties->setTabText(ui->twProperties->indexOf(tabHelpAbout), QApplication::translate(
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

  //QWidget *w = m_tabBar->tabButton(tindex, QTabBar::RightSide);
  //w->setToolTip(tr("Close tab"));
  //w->setObjectName("toolButton");

  //SearchBar *searchBar = new SearchBar(this);
  //MyHighlighter *highlighter = new MyHighlighter(text, "");

  /*
  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChanged(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosed()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findNextButtonClicked()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findPreviousButtonClicked()), this, SLOT(searchBarFindPrevious()));

  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);
  gridLayoutX->addWidget(new SyntaxHighlighterWidget(this, highlighter));
  */

  text->show();
  ui->twProperties->setCurrentIndex(tindex);
  text->setFocus();
}

/*
 * Slot to position twProperties at Help About tab
 */
void MainWindow::onHelpAbout()
{
  if (_isPropertiesTabWidgetVisible())
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_HELPABOUT);
  }
}

/*
 * Initialize QAction objects
 */
void MainWindow::initActions()
{
  connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
          SLOT(invalidateTabs()));

  connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
  connect(ui->actionNonInstalledPackages, SIGNAL(changed()), this, SLOT(changePackageListModel()));

  connect(ui->tvPackages, SIGNAL(activated(QModelIndex)), this, SLOT(changedTabIndex()));
  connect(ui->tvPackages, SIGNAL(clicked(QModelIndex)), this, SLOT(changedTabIndex()));
  connect(ui->tvPackages->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this,
          SLOT(headerViewPackageListSortIndicatorClicked(int,Qt::SortOrder)));
  connect(ui->tvPackages, SIGNAL(customContextMenuRequested(QPoint)), this,
          SLOT(execContextMenuPackages(QPoint)));

  connect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));

  connect(ui->actionSyncPackages, SIGNAL(triggered()), this, SLOT(doSyncDatabase()));
  connect(ui->actionSystemUpgrade, SIGNAL(triggered()), this, SLOT(doSystemUpgrade()));

  connect(ui->actionRemove, SIGNAL(triggered()), this, SLOT(insertIntoRemovePackage()));
  connect(ui->actionInstall, SIGNAL(triggered()), this, SLOT(insertIntoInstallPackage()));

  connect(ui->actionCommit, SIGNAL(triggered()), this, SLOT(doCommitTransaction()));
  connect(ui->actionRollback, SIGNAL(triggered()), this, SLOT(doRollbackTransaction()));
  connect(ui->actionHelpAbout, SIGNAL(triggered()), this, SLOT(onHelpAbout()));
  connect(ui->actionGetNews, SIGNAL(triggered()), this, SLOT(refreshArchNews()));

  connect(m_lblCounters, SIGNAL(linkActivated(QString)), this, SLOT(outputOutdatedPackageList()));

  ui->actionCommit->setEnabled(false);
  ui->actionRollback->setEnabled(false);
}
