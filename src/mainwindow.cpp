/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2013 Alexandre Albuquerque Arnt
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

#include "mainwindow.h"
#include "searchlineedit.h"
#include "ui_mainwindow.h"
#include "strconstants.h"
#include "package.h"
#include "uihelper.h"
#include "wmhelper.h"
#include "treeviewpackagesitemdelegate.h"
#include "searchbar.h"
#include "packagecontroller.h"

#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QString>
#include <QTextBrowser>
#include <QKeyEvent>
#include <QProgressBar>
#include <QTimer>
#include <QLabel>
#include <QMessageBox>
#include <QComboBox>
#include <QModelIndex>
#include <iostream>

/*
 * MainWindow's constructor: basic UI init
 */
MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  m_foundFilesInPkgFileList = new QList<QModelIndex>();
  m_indFoundFilesInPkgFileList = 0;
  ui->setupUi(this);
}

/*
 * MainWindow's destructor
 */
MainWindow::~MainWindow()
{
  //Let's garbage collect transaction files...
  m_unixCommand->removeTemporaryFiles();
  delete ui;
}

/*
 * The show() public SLOT, when the app is being drawn!!!
 * Init member variables and all UI widgets
 */
void MainWindow::show()
{
  loadSettings();
  restoreGeometry(SettingsManager::getWindowSize());

  m_initializationCompleted=false;
  m_commandExecuting=ectn_NONE;
  m_commandQueued=ectn_NONE;
  m_leFilterPackage = new SearchLineEdit(this);

  setWindowTitle(StrConstants::getApplicationName() + " " + StrConstants::getApplicationVersion());
  setMinimumSize(QSize(850, 600));

  initStatusBar();
  initTabOutput();
  initTabInfo();
  initTabFiles();
  initTabTransaction();
  initTabHelpAbout();
  initTabNews();
  initLineEditFilterPackages();
  initPackageTreeView();

  //Let's watch for changes in the Pacman db dir!
  m_pacmanDatabaseSystemWatcher =
      new QFileSystemWatcher(QStringList() << ctn_PACMAN_DATABASE_DIR, this);
  connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  qApp->setStyleSheet(StrConstants::getMenuCSS());

  loadPanelSettings();

  initActions();
  initAppIcon();
  initToolBar();
  initTabWidgetPropertiesIndex();
  refreshDistroNews(false);

  QMainWindow::show();

  /* This timer is needed to beautify GUI initialization... */
  timer = new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(buildPackageList()));
  timer->start(0.1);
}

/*
 * If we have some outdated packages, let's put an angry red face icon in this app!
 */
void MainWindow::refreshAppIcon()
{
  if(m_outdatedPackageList->count() > 0)
  {
    setWindowIcon(IconHelper::getIconOctopiRed());
  }
  else
  {
    setWindowIcon(IconHelper::getIconOctopiYellow());
  }
}

/*
 * Prints the list of outdated packages to the Output tab.
 */
void MainWindow::outputOutdatedPackageList()
{
  //We cannot output any list if there is a running transaction!
  if (m_commandExecuting != ectn_NONE) return;

  m_numberOfOutdatedPackages = m_outdatedPackageList->count();

  if(m_numberOfOutdatedPackages > 0)
  {
    QString html = "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
    QString anchorBegin = "anchorBegin";
    html += "<a id=\"" + anchorBegin + "\"></a>";

    clearTabOutput();

    if(m_outdatedPackageList->count()==1){
      html += "<h3>" + StrConstants::getOneOutdatedPackage() + "</h3>";
    }
    else
    {
      html += "<h3>" +
          StrConstants::getOutdatedPackages().arg(m_outdatedPackageList->count()) + "</h3>";
    }

    html += "<br><table border=\"0\">";
    html += "<tr><th width=\"25%\" align=\"left\">" + StrConstants::getName() +
        "</th><th width=\"18%\" align=\"right\">" +
        StrConstants::getOutdatedVersion() +
        "</th><th width=\"18%\" align=\"right\">" +
        StrConstants::getAvailableVersion() + "</th></tr>";

    for (int c=0; c < m_outdatedPackageList->count(); c++)
    {
      QString pkg = m_outdatedPackageList->at(c);
      QString outdatedVersion = getOutdatedPackageVersionByName(m_outdatedPackageList->at(c));
      QString availableVersion = getInstalledPackageVersionByName(m_outdatedPackageList->at(c));

      html += "<tr><td>" + pkg +
          "</td><td align=\"right\"><b><font color=\"#E55451\">" +
          outdatedVersion +
          "</b></font></td><td align=\"right\">" +
          availableVersion + "</td></tr>";
    }

    writeToTabOutput(html);

    QTextBrowser *text =
        ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");

    if (text)
    {
      text->scrollToAnchor(anchorBegin);
    }

    ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  }
}

/*
 * Removes all text inside the TabOutput editor
 */
void MainWindow::clearTabOutput()
{
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (text)
  {
    text->clear();
  }
}

/*
 * Searchs model modelInstalledPackages by a package name and returns its OUTDATED version
 */
QString MainWindow::getOutdatedPackageVersionByName(const QString &pkgName)
{
  QList<QStandardItem *> foundItems =
      m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);
  QString res;

  if (foundItems.count() > 0)
  {
    QStandardItem * si = foundItems.first();
    QStandardItem * siIcon = m_modelInstalledPackages->item(si->row(), ctn_PACKAGE_ICON_COLUMN);

    int mark = siIcon->text().indexOf('^');
    if (mark >= 0)
    {
      res = siIcon->text().right(siIcon->text().size()-mark-1);
    }
  }

  return res;
}

/*
 * Searchs model modelInstalledPackages by a package name and returns its AVAILABLE version
 */
QString MainWindow::getInstalledPackageVersionByName(const QString &pkgName)
{
  QList<QStandardItem *> foundItems =
      m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);
  QString res;

  if (foundItems.count() > 0)
  {
    QStandardItem * si = foundItems.first();
    QStandardItem * aux = m_modelInstalledPackages->item(si->row(), ctn_PACKAGE_VERSION_COLUMN);
    res = aux->text();
  }

  return res;
}

/*
 * Returns the QStandardItem available in the row that pkgName is found
 */
QStandardItem *MainWindow::getAvailablePackage(const QString &pkgName, const int index)
{
  QList<QStandardItem *> foundItems =
      m_modelPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);
  QStandardItem *res;

  if (foundItems.count() > 0)
  {
    QStandardItem * si = foundItems.first();
    QStandardItem * aux = m_modelPackages->item(si->row(), index);
    res = aux->clone();
  }

  return res;
}

/*
 * Searchs model modelInstalledPackages by a package name and returns if it is already installed
 */
bool MainWindow::isPackageInstalled(const QString &pkgName)
{
  QList<QStandardItem *> foundItems =
      m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);

  return (foundItems.count() > 0);
}

/*
 * User changed the search column used by the SortFilterProxyModel in tvPackages
 */
void MainWindow::tvPackagesSearchColumnChanged(QAction *actionSelected)
{
  //We are in the realm of tradictional NAME search
  if (actionSelected->objectName() == ui->actionSearchByName->objectName())
  {
    m_proxyModelPackages->setFilterKeyColumn(ctn_PACKAGE_NAME_COLUMN);
  }
  //We are talking about slower 'search by description'...
  else
  {
    m_proxyModelPackages->setFilterKeyColumn(ctn_PACKAGE_DESCRIPTION_COLUMN);
  }

  QModelIndex cIcon = m_proxyModelPackages->index(0, ctn_PACKAGE_ICON_COLUMN);
  QModelIndex cName = m_proxyModelPackages->index(0, ctn_PACKAGE_NAME_COLUMN);
  QModelIndex cVersion = m_proxyModelPackages->index(0, ctn_PACKAGE_VERSION_COLUMN);
  QModelIndex cRepository = m_proxyModelPackages->index(0, ctn_PACKAGE_REPOSITORY_COLUMN);

  ui->tvPackages->setCurrentIndex(cIcon);

  ui->tvPackages->scrollTo(cIcon, QAbstractItemView::PositionAtTop);

  if (m_leFilterPackage->text() == "")
  {
    ui->tvPackages->selectionModel()->setCurrentIndex(cIcon, QItemSelectionModel::Select);
  }
  else
  {
    ui->tvPackages->selectionModel()->setCurrentIndex(cIcon, QItemSelectionModel::SelectCurrent);
    ui->tvPackages->selectionModel()->setCurrentIndex(cName, QItemSelectionModel::Select);
    ui->tvPackages->selectionModel()->setCurrentIndex(cVersion, QItemSelectionModel::Select);
    ui->tvPackages->selectionModel()->setCurrentIndex(cRepository, QItemSelectionModel::Select);
  }

  QModelIndex mi = m_proxyModelPackages->index(0, ctn_PACKAGE_NAME_COLUMN);
  ui->tvPackages->setCurrentIndex(mi);
  ui->tvPackages->scrollTo(mi);

  changedTabIndex();
  m_leFilterPackage->setFocus();
}

/*
 * Populates the list of available packages from the given groupName
 */
void MainWindow::buildPackagesFromGroupList(const QString &groupName)
{
  m_progressWidget->show();

  if (m_cbGroups->currentIndex() == 0)
  {
    QStringList sl;
    m_modelPackagesFromGroup->setHorizontalHeaderLabels(
          sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());

    sl.clear();
    m_modelInstalledPackagesFromGroup->setHorizontalHeaderLabels(
          sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());

    if (m_leFilterPackage->text() != "") reapplyPackageFilter();

    if (ui->actionNonInstalledPackages->isChecked())
    {
      m_proxyModelPackages->setSourceModel(m_modelPackages);
    }
    else
    {
      m_proxyModelPackages->setSourceModel(m_modelInstalledPackages);
    }

    QModelIndex maux = m_proxyModelPackages->index(0, 0);
    ui->tvPackages->setCurrentIndex(maux);
    ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
    ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);

    refreshTabInfo();
    refreshTabFiles();
    ui->tvPackages->setFocus();

    return;
  }

  disconnect(m_pacmanDatabaseSystemWatcher,
             SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  CPUIntensiveComputing cic;

  m_modelPackagesFromGroup->clear();
  m_modelInstalledPackagesFromGroup->clear();

  QStringList sl;
  QList<QString> *list = Package::getPackagesOfGroup(groupName);

  QStandardItem *parentItemPackagesFromGroup = m_modelPackagesFromGroup->invisibleRootItem();
  QStandardItem *parentItemInstalledPackagesFromGroup = m_modelInstalledPackagesFromGroup->invisibleRootItem();

  QList<QString>::const_iterator it = list->begin();
  QList<QStandardItem*> lIcons, lNames, lVersions, lRepositories, lDescriptions;
  QList<QStandardItem*> lIcons2, lNames2, lVersions2, lRepositories2, lDescriptions2;

  m_progressWidget->setRange(0, list->count());
  m_progressWidget->setValue(0);

  int counter=0;
  while(it != list->end())
  {
    QString packageName = *it;

    QStandardItem *siIcon = getAvailablePackage(packageName, ctn_PACKAGE_ICON_COLUMN);
    QStandardItem *siName = getAvailablePackage(packageName, ctn_PACKAGE_NAME_COLUMN);
    QStandardItem *siVersion = getAvailablePackage(packageName, ctn_PACKAGE_VERSION_COLUMN);
    QStandardItem *siRepository = getAvailablePackage(packageName, ctn_PACKAGE_REPOSITORY_COLUMN);
    QStandardItem *siDescription = getAvailablePackage(packageName, ctn_PACKAGE_DESCRIPTION_COLUMN);

    lIcons << siIcon;
    lNames << siName;
    lVersions << siVersion;
    lRepositories << siRepository;
    lDescriptions << siDescription;

    //If this is an INSTALLED package, we add it to the model view of installed packages!
    if (siIcon->icon().pixmap(QSize(22,22)).toImage() !=
        IconHelper::getIconNonInstalled().pixmap(QSize(22,22)).toImage())
    {
      lIcons2 << lIcons.last()->clone();
      lNames2 << lNames.last()->clone();
      lVersions2 << lVersions.last()->clone();
      lRepositories2 << lRepositories.last()->clone();
      lDescriptions2 << lDescriptions.last()->clone();
    }

    counter++;
    m_progressWidget->setValue(counter);
    qApp->processEvents();
    it++;
  }

  parentItemPackagesFromGroup->insertColumn(ctn_PACKAGE_ICON_COLUMN, lIcons);
  parentItemPackagesFromGroup->insertColumn(ctn_PACKAGE_NAME_COLUMN, lNames);
  parentItemPackagesFromGroup->insertColumn(ctn_PACKAGE_VERSION_COLUMN, lVersions);
  parentItemPackagesFromGroup->insertColumn(ctn_PACKAGE_REPOSITORY_COLUMN, lRepositories);
  parentItemPackagesFromGroup->insertColumn(ctn_PACKAGE_DESCRIPTION_COLUMN, lDescriptions);

  parentItemInstalledPackagesFromGroup->insertColumn(ctn_PACKAGE_ICON_COLUMN, lIcons2);
  parentItemInstalledPackagesFromGroup->insertColumn(ctn_PACKAGE_NAME_COLUMN, lNames2);
  parentItemInstalledPackagesFromGroup->insertColumn(ctn_PACKAGE_VERSION_COLUMN, lVersions2);
  parentItemInstalledPackagesFromGroup->insertColumn(ctn_PACKAGE_REPOSITORY_COLUMN, lRepositories2);
  parentItemInstalledPackagesFromGroup->insertColumn(ctn_PACKAGE_DESCRIPTION_COLUMN, lDescriptions2);

  ui->tvPackages->setColumnWidth(ctn_PACKAGE_ICON_COLUMN, 24);
  ui->tvPackages->setColumnWidth(ctn_PACKAGE_NAME_COLUMN, 500);
  ui->tvPackages->setColumnWidth(ctn_PACKAGE_VERSION_COLUMN, 160);
  ui->tvPackages->header()->setSectionHidden(ctn_PACKAGE_DESCRIPTION_COLUMN, true);

  ui->tvPackages->sortByColumn(m_PackageListOrderedCol, m_PackageListSortOrder);

  m_modelPackagesFromGroup->setHorizontalHeaderLabels(
        sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository() << "");

  sl.clear();
  m_modelInstalledPackagesFromGroup->setHorizontalHeaderLabels(
        sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository() << "");

  if (m_leFilterPackage->text() != "") reapplyPackageFilter();

  if (ui->actionNonInstalledPackages->isChecked())
  {
    m_proxyModelPackages->setSourceModel(m_modelPackagesFromGroup);
  }
  else
  {
    m_proxyModelPackages->setSourceModel(m_modelInstalledPackagesFromGroup);
  }

  QModelIndex maux = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);

  list->clear();
  refreshTabInfo();
  refreshTabFiles();
  ui->tvPackages->setFocus();
  m_progressWidget->setValue(list->count());

  connect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  m_progressWidget->close();
}

/*
 * Populates the list of available packages (installed [+ non-installed])
 *
 * It's called Only: when the selected group is <All> !
 */
void MainWindow::buildPackageList()
{
  m_progressWidget->show();
  disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  static bool firstTime = true;
  timer->stop();

  CPUIntensiveComputing cic;  

  //Refresh the list of Group names
  refreshComboBoxGroups();

  m_modelPackages->clear();
  m_modelInstalledPackages->clear();
  m_modelInstalledPackagesFromGroup->clear();
  m_modelPackagesFromGroup->clear();

  if (ui->actionNonInstalledPackages->isChecked())
  {
    m_proxyModelPackages->setSourceModel(m_modelPackages);
  }
  else
  {
    m_proxyModelPackages->setSourceModel(m_modelInstalledPackages);
  }

  QStringList sl;

  if(!firstTime) //If it's not the starting of the app...
  {
    //Let's get outdatedPackages list again!
    m_outdatedPackageList = Package::getOutdatedPackageList();
    m_numberOfOutdatedPackages = m_outdatedPackageList->count();
  }

  QStringList *unrequiredPackageList = Package::getUnrequiredPackageList();
  QList<PackageListData> *list = Package::getPackageList();
  QList<PackageListData> *listForeign = Package::getForeignPackageList();
  QList<PackageListData>::const_iterator itForeign = listForeign->begin();

  m_progressWidget->setRange(0, list->count() + listForeign->count());
  m_progressWidget->setValue(0);

  int counter=0;
  while (itForeign != listForeign->end())
  {
    counter++;
    m_progressWidget->setValue(counter);

    PackageListData pld = PackageListData(
          itForeign->name, itForeign->repository, itForeign->version,
          itForeign->name + " " + Package::getInformationDescription(itForeign->name, true),
          ectn_FOREIGN);

    list->append(pld);
    itForeign++;
  }

  QStandardItem *parentItem = m_modelPackages->invisibleRootItem();
  QStandardItem *parentItemInstalledPackages = m_modelInstalledPackages->invisibleRootItem();
  QList<PackageListData>::const_iterator it = list->begin();
  QList<QStandardItem*> lIcons, lNames, lVersions, lRepositories, lDescriptions;
  QList<QStandardItem*> lIcons2, lNames2, lVersions2, lRepositories2, lDescriptions2;

  while(it != list->end())
  {
    PackageListData pld = *it;
    //If this is an installed package, it can be also outdated!
    switch (pld.status)
    {
      case ectn_FOREIGN:
        lIcons << new QStandardItem(IconHelper::getIconForeign(), "_Foreign");
        break;
      case ectn_OUTDATED:
        lIcons << new QStandardItem(IconHelper::getIconOutdated(), "_OutDated^"+pld.outatedVersion);
        break;
      case ectn_INSTALLED:
        //Is this package unrequired too?
        if (unrequiredPackageList->contains(pld.name))
        {
          lIcons << new QStandardItem(IconHelper::getIconUnrequired(), "_Unrequired");
        }
        else
        {
          lIcons << new QStandardItem(IconHelper::getIconInstalled(), "_Installed");
        }
        break;
      case ectn_NON_INSTALLED:
        lIcons << new QStandardItem(IconHelper::getIconNonInstalled(), "_NonInstalled");
        break;
      default:;
    }

    lNames << new QStandardItem(pld.name);
    lVersions << new QStandardItem(pld.version);

    //Let's put package description in UTF-8 format
    QString pkgDescription = pld.description;
    pkgDescription = pkgDescription.fromUtf8(pkgDescription.toAscii().data());

    lDescriptions << new QStandardItem(pkgDescription);

    if (pld.repository.isEmpty())
    {
      lRepositories << new QStandardItem(StrConstants::getForeignRepositoryName());
    }
    else
      lRepositories << new QStandardItem(pld.repository);

    //If this is an INSTALLED package, we add it to the model view of installed packages!
    if (pld.status != ectn_NON_INSTALLED)
    {
      lIcons2 << lIcons.last()->clone();
      lNames2 << lNames.last()->clone();
      lVersions2 << lVersions.last()->clone();
      lRepositories2 << lRepositories.last()->clone();
      lDescriptions2 << lDescriptions.last()->clone();
    }

    counter++;
    m_progressWidget->setValue(counter);
    it++;
  }

  parentItem->insertColumn(0, lIcons);
  parentItem->insertColumn(1, lNames);
  parentItem->insertColumn(2, lVersions);
  parentItem->insertColumn(3, lRepositories);
  parentItem->insertColumn(4, lDescriptions);

  parentItemInstalledPackages->insertColumn(ctn_PACKAGE_ICON_COLUMN, lIcons2);
  parentItemInstalledPackages->insertColumn(ctn_PACKAGE_NAME_COLUMN, lNames2);
  parentItemInstalledPackages->insertColumn(ctn_PACKAGE_VERSION_COLUMN, lVersions2);
  parentItemInstalledPackages->insertColumn(ctn_PACKAGE_REPOSITORY_COLUMN, lRepositories2);
  parentItemInstalledPackages->insertColumn(ctn_PACKAGE_DESCRIPTION_COLUMN, lDescriptions2);

  ui->tvPackages->setColumnWidth(ctn_PACKAGE_ICON_COLUMN, 24);
  ui->tvPackages->setColumnWidth(ctn_PACKAGE_NAME_COLUMN, 500);
  ui->tvPackages->setColumnWidth(ctn_PACKAGE_VERSION_COLUMN, 160);
  ui->tvPackages->header()->setSectionHidden(ctn_PACKAGE_DESCRIPTION_COLUMN, true);
  ui->tvPackages->sortByColumn(m_PackageListOrderedCol, m_PackageListSortOrder);

  m_modelPackages->sort(m_PackageListOrderedCol, m_PackageListSortOrder);
  m_modelInstalledPackages->sort(m_PackageListOrderedCol, m_PackageListSortOrder);

  m_modelPackages->setHorizontalHeaderLabels(
        sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository() << "");

  sl.clear();
  m_modelInstalledPackages->setHorizontalHeaderLabels(
        sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository() << "");

  if (m_leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex maux = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);    

  list->clear();
  refreshTabInfo();
  refreshTabFiles();

  if (_isPackageTreeViewVisible())
  {
    ui->tvPackages->setFocus();
  }

  //Refresh counters
  m_numberOfInstalledPackages = m_modelInstalledPackages->invisibleRootItem()->rowCount(); 
  m_numberOfAvailablePackages = m_modelPackages->invisibleRootItem()->rowCount() - m_numberOfInstalledPackages;

  //Refresh statusbar widget
  refreshStatusBar();

  //Refresh application icon
  refreshAppIcon();

  connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  if (firstTime)
  {
    if (_isPackageTreeViewVisible()) m_leFilterPackage->setFocus();
    m_initializationCompleted = true;
  }

  counter = list->count() + listForeign->count();
  m_progressWidget->setValue(counter);

  firstTime = false;
  m_progressWidget->close();
}

/*
 * Decides which SLOT to call: buildPackageList or buildPackagesFromGroupList
 */
void MainWindow::metaBuildPackageList()
{
  qApp->processEvents();
  if (m_cbGroups->currentIndex() != 0)
  {
    buildPackagesFromGroupList(m_cbGroups->currentText());
  }
  else
  {
    buildPackageList();
  }
}

/*
 * Prints the values of the package counters at the right of the statusBar
 */
void MainWindow::refreshStatusBar()
{
  QString text;

  if(m_numberOfOutdatedPackages > 0)
  {
    text = " | " + StrConstants::getNumberInstalledPackages().arg(m_numberOfInstalledPackages) +
        " | <b><font color=\"#E55451\"><a href=\"dummy\" style=\"color:\'#E55451\'\">" +
        StrConstants::getNumberOutdatedPackages().arg(m_numberOfOutdatedPackages) + "</a></font></b>";
        //StrConstants::getNumberAvailablePackages().arg(m_numberOfAvailablePackages);
  }
  else
  {
    text = StrConstants::getNumberInstalledPackages().arg(m_numberOfInstalledPackages); // +
        //" | " + StrConstants::getNumberAvailablePackages().arg(m_numberOfAvailablePackages);
  }

  m_lblTotalCounters->setText(text);
  //ui->statusBar->addPermanentWidget(m_lblTotalCounters);
  ui->statusBar->addWidget(m_lblTotalCounters);
}

/*
 * Whenever the user changes the checkbox menu to show non installed packages,
 * we have to change the model from the Packages treeview...
 */
void MainWindow::changePackageListModel()
{
  QStringList sl;

  if (ui->actionNonInstalledPackages->isChecked())
  {
    if(m_cbGroups->currentIndex() == 0)
    {
      m_modelPackages->setHorizontalHeaderLabels(
          sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());     
      m_proxyModelPackages->setSourceModel(m_modelPackages);
    }
    else
    {
      m_modelPackagesFromGroup->setHorizontalHeaderLabels(
            sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());
      m_proxyModelPackages->setSourceModel(m_modelPackagesFromGroup);
    }
  }
  else
  {
    if(m_cbGroups->currentIndex() == 0)
    {
      m_modelInstalledPackages->setHorizontalHeaderLabels(
          sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());
      m_proxyModelPackages->setSourceModel(m_modelInstalledPackages);
    }
    else
    {
      m_modelInstalledPackagesFromGroup->setHorizontalHeaderLabels(
            sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());
      m_proxyModelPackages->setSourceModel(m_modelInstalledPackagesFromGroup);
    }
  }

  if (m_leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex cIcon = m_proxyModelPackages->index(0, ctn_PACKAGE_ICON_COLUMN);
  QModelIndex cName = m_proxyModelPackages->index(0, ctn_PACKAGE_NAME_COLUMN);
  QModelIndex cVersion = m_proxyModelPackages->index(0, ctn_PACKAGE_VERSION_COLUMN);
  QModelIndex cRepository = m_proxyModelPackages->index(0, ctn_PACKAGE_REPOSITORY_COLUMN);

  ui->tvPackages->setCurrentIndex(cIcon);

  ui->tvPackages->scrollTo(cIcon, QAbstractItemView::PositionAtTop);

  if (m_leFilterPackage->text() == "")
  {
    ui->tvPackages->selectionModel()->setCurrentIndex(cIcon, QItemSelectionModel::Select);
  }
  else
  {
    ui->tvPackages->selectionModel()->setCurrentIndex(cIcon, QItemSelectionModel::SelectCurrent);
    ui->tvPackages->selectionModel()->setCurrentIndex(cName, QItemSelectionModel::Select);
    ui->tvPackages->selectionModel()->setCurrentIndex(cVersion, QItemSelectionModel::Select);
    ui->tvPackages->selectionModel()->setCurrentIndex(cRepository, QItemSelectionModel::Select);
  }

  changedTabIndex();
}

/*
 * Returns the current selected StandardItemModel, based on groups and installed packages switches
 */
QStandardItemModel *MainWindow::_getCurrentSelectedModel()
{
  QStandardItemModel *sim=0;

  if(ui->actionNonInstalledPackages->isChecked())
  {
    if(m_cbGroups->currentIndex() == 0)
      sim = m_modelPackages;
    else
      sim = m_modelPackagesFromGroup;
  }
  else
  {
    if(m_cbGroups->currentIndex() == 0)
      sim = m_modelInstalledPackages;
    else
      sim = m_modelInstalledPackagesFromGroup;
  }

  return sim;
}

/*
 * Brings the context menu when the user clicks the right button above the package list
 */
void MainWindow::execContextMenuPackages(QPoint point)
{
  if(ui->tvPackages->selectionModel()->selectedRows().count() > 0)
  {
    QStandardItemModel * sim = _getCurrentSelectedModel();
    bool allSameType = true;
    bool allInstallable = true;
    bool allRemovable = true;

    foreach(QModelIndex item, ui->tvPackages->selectionModel()->selectedRows())
    {
      QModelIndex mi = m_proxyModelPackages->mapToSource(item);
      QStandardItem *si = sim->item(mi.row(), ctn_PACKAGE_ICON_COLUMN);

      if((si->icon().pixmap(QSize(22,22)).toImage()) ==
         IconHelper::getIconForeign().pixmap(QSize(22,22)).toImage())
      {
        allInstallable = false;
      }
      else if((si->icon().pixmap(QSize(22,22)).toImage()) ==
              IconHelper::getIconNonInstalled().pixmap(QSize(22,22)).toImage())
      {
        allRemovable = false;
      }
    }

    if (allSameType)
    {
      QMenu *menu = new QMenu(this);

      if(allInstallable)
      {
        menu->addAction(ui->actionInstall);

        if (m_cbGroups->currentIndex() != 0)
        {
          menu->addAction(ui->actionInstallGroup);
        }
      }

      if(allRemovable)
      {
        menu->addAction(ui->actionRemove);

        if (m_cbGroups->currentIndex() != 0)
        {
          //Is this group already installed?
          if (m_modelInstalledPackagesFromGroup->rowCount() == m_modelPackagesFromGroup->rowCount())
          {
            menu->addAction(ui->actionRemoveGroup);
          }
        }
      }

      if(menu->actions().count() > 0)
      {
        QPoint pt2 = ui->tvPackages->mapToGlobal(point);
        pt2.setY(pt2.y() + ui->tvPackages->header()->height());
        menu->exec(pt2);
      }
    }
  }
}

/*
 * This SLOT collapses all treeview items
 */
void MainWindow::collapseAllContentItems(){
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if (tv != 0)
    tv->collapseAll();
}

/*
 * This SLOT collapses only the currently selected item
 */
void MainWindow::collapseThisContentItems(){
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if ( tv != 0 ){
    tv->repaint(tv->rect());
    QCoreApplication::processEvents();
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tv->model());
    QModelIndex mi = tv->currentIndex();
    if (sim->hasChildren(mi))	_collapseItem(tv, sim, mi);
  }
}

/*
 * This SLOT expands all treeview items
 */
void MainWindow::expandAllContentItems(){
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if ( tv != 0 ){
    tv->repaint(tv->rect());
    QCoreApplication::processEvents();
    tv->expandAll();
  }
}

/*
 * This SLOT expands only the currently selected item
 */
void MainWindow::expandThisContentItems(){
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if ( tv != 0 ){
    tv->repaint(tv->rect());
    QCoreApplication::processEvents();
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tv->model());
    QModelIndex mi = tv->currentIndex();
    if (sim->hasChildren(mi))	_expandItem(tv, sim, &mi);
  }
}

/*
 * This method does the job of collapsing the given item and its children
 */
void MainWindow::_collapseItem(QTreeView* tv, QStandardItemModel* sim, QModelIndex mi){
  for (int i=0; i<sim->rowCount(mi); i++){
    if (sim->hasChildren(mi)){
      QCoreApplication::processEvents();
      tv->collapse(mi);
      QModelIndex mi2 = mi.child(i, 0);
      _collapseItem(tv, sim, mi2);
    }
  }
}

/*
 * This method does the job of expanding the given item and its children
 */
void MainWindow::_expandItem(QTreeView* tv, QStandardItemModel* sim, QModelIndex* mi){
  for (int i=0; i<sim->rowCount(*mi); i++){
    if (sim->hasChildren(*mi)){
      tv->expand(*mi);
      QModelIndex mi2 = mi->child(i, 0);
      _expandItem(tv, sim, &mi2);
    }
  }
}

/*
 * Brings the context menu when the user clicks the right button
 * above the Files treeview in "Files" Tab
 */
void MainWindow::execContextMenuPkgFileList(QPoint point)
{
  QTreeView *tvPkgFileList =
      ui->twProperties->currentWidget()->findChild<QTreeView*>("tvPkgFileList");

  if (tvPkgFileList == 0)
  {
    return;
  }

  QModelIndex mi = tvPkgFileList->currentIndex();
  QString selectedPath = PackageController::showFullPathOfItem(mi);

  QMenu menu(this);
  QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
  QStandardItem *si = sim->itemFromIndex(mi);
  if (si == 0) return;
  if (si->hasChildren() && (!tvPkgFileList->isExpanded(mi)))
    menu.addAction(ui->actionExpandItem);

  if (si->hasChildren() && (tvPkgFileList->isExpanded(mi)))
    menu.addAction(ui->actionCollapseItem);

  if (menu.actions().count() > 0)
    menu.addSeparator();

  menu.addAction(ui->actionCollapseAllItems);
  menu.addAction(ui->actionExpandAllItems);
  menu.addSeparator();

  QDir d;
  QFile f(selectedPath);

  if (si->icon().pixmap(QSize(22,22)).toImage() ==
      IconHelper::getIconFolder().pixmap(QSize(22,22)).toImage())
  {
    if (d.exists(selectedPath))
    {
      menu.addAction(ui->actionOpenDirectory);
      menu.addAction(ui->actionOpenTerminal);
    }
  }
  else if (f.exists())
  {
    menu.addAction(ui->actionOpenFile);
  }
  if (f.exists() && UnixCommand::isTextFile(selectedPath))
  {
    menu.addAction(ui->actionEditFile);
  }

  QPoint pt2 = tvPkgFileList->mapToGlobal(point);
  pt2.setY(pt2.y() + tvPkgFileList->header()->height());
  menu.exec(pt2);
}

/*
 * Brings the context menu when the user clicks the right button
 * above the Transaction treeview in "Transaction" Tab
 */
void MainWindow::execContextMenuTransaction(QPoint point)
{
  QTreeView *tvTransaction = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvTransaction");
  if (!tvTransaction) return;

  if ((tvTransaction->currentIndex() == getRemoveTransactionParentItem()->index() &&
      tvTransaction->model()->hasChildren(tvTransaction->currentIndex())) ||
      (tvTransaction->currentIndex() == getInstallTransactionParentItem()->index() &&
            tvTransaction->model()->hasChildren(tvTransaction->currentIndex())))
  {
    QMenu menu(this);
    menu.addAction(ui->actionRemoveTransactionItems);
    QPoint pt2 = tvTransaction->mapToGlobal(point);
    pt2.setY(pt2.y() + tvTransaction->header()->height());
    menu.exec(pt2);
  }
  else if (tvTransaction->currentIndex() != getRemoveTransactionParentItem()->index() &&
           tvTransaction->currentIndex() != getInstallTransactionParentItem()->index() &&
           tvTransaction->currentIndex().isValid())
  {
    QMenu menu(this);

    if (tvTransaction->selectionModel()->selectedIndexes().count() == 1)
    {
      ui->actionRemoveTransactionItem->setText(StrConstants::getRemoveItem());
    }
    else
    {
      ui->actionRemoveTransactionItem->setText(StrConstants::getRemoveItems());
    }

    menu.addAction(ui->actionRemoveTransactionItem);
    QPoint pt2 = tvTransaction->mapToGlobal(point);
    pt2.setY(pt2.y() + tvTransaction->header()->height());
    menu.exec(pt2);
  }
}

/*
 * Returns true if tabWidget height is greater than 0. Otherwise, returns false.
 */
bool MainWindow::_isPropertiesTabWidgetVisible()
{
  QList<int> rl;
  rl = ui->splitterHorizontal->sizes();

  return (rl[1] > 0);
}

/*
 * Returns true if tvPackages height is greater than 0. Otherwise, returns false.
 */
bool MainWindow::_isPackageTreeViewVisible()
{
  QList<int> rl;
  rl = ui->splitterHorizontal->sizes();

  return (rl[0] > 0);
}

/*
 * Re-populates the HTML view with selected package's information (tab ONE)
 */
void MainWindow::refreshTabInfo(bool clearContents, bool neverQuit)
{
  static QString strSelectedPackage;

  if(neverQuit == false &&
     (ui->twProperties->currentIndex() != ctn_TABINDEX_INFORMATION || !_isPropertiesTabWidgetVisible())) return;

  if (clearContents || ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTextBrowser *text =
        ui->twProperties->widget(ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");

    if (text)
    {
      text->clear();
    }

    strSelectedPackage="";
    return;
  }

  QModelIndex item = ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).first();
  QModelIndex mi = m_proxyModelPackages->mapToSource(item);

  QStandardItem *siIcon;
  QStandardItem *siName;
  QStandardItem *siRepository;
  QStandardItem *siVersion;

  if (ui->actionNonInstalledPackages->isChecked())
  {
    if (m_cbGroups->currentIndex() == 0)
    {
      siIcon = m_modelPackages->item( mi.row(), ctn_PACKAGE_ICON_COLUMN);
      siName = m_modelPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
    else
    {
      siIcon = m_modelPackagesFromGroup->item( mi.row(), ctn_PACKAGE_ICON_COLUMN);
      siName = m_modelPackagesFromGroup->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelPackagesFromGroup->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelPackagesFromGroup->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
  }
  else
  {
    if (m_cbGroups->currentIndex() == 0)
    {
      siIcon = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_ICON_COLUMN);
      siName = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
    else
    {
      siIcon = m_modelInstalledPackagesFromGroup->item( mi.row(), ctn_PACKAGE_ICON_COLUMN);
      siName = m_modelInstalledPackagesFromGroup->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelInstalledPackagesFromGroup->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelInstalledPackagesFromGroup->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
  }
  //If we are trying to refresh an already displayed package...
  if (strSelectedPackage == siRepository->text()+"#"+siName->text()+"#"+siVersion->text())
  {
    if (neverQuit)
    {
      _changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
    }

    return;
  }

  CPUIntensiveComputing cic;

  /* Appends all info from the selected package! */
  QString pkgName=siName->text();
  PackageInfoData pid;

  if (siRepository->text() != StrConstants::getForeignRepositoryName()){
    pid = Package::getInformation(pkgName);
  }
  else
  {
    pid = Package::getInformation(pkgName, true); //This is a foreign package!!!
  }

  //Let's put package description in UTF-8 format
  QString pkgDescription = pid.description;
  pkgDescription = pkgDescription.fromUtf8(pkgDescription.toAscii().data());

  QString version = StrConstants::getVersion();
  QString url = StrConstants::getURL();
  QString licenses = StrConstants::getLicenses();
  QString groups = StrConstants::getGroups();
  QString provides = StrConstants::getProvides();
  QString dependsOn = StrConstants::getDependsOn();
  QString optionalDeps = StrConstants::getOptionalDeps();
  QString conflictsWith = StrConstants::getConflictsWith();
  QString replaces = StrConstants::getReplaces();
  QString downloadSize = StrConstants::getDownloadSize();
  QString installedSize = StrConstants::getInstalledSize();
  QString packager = StrConstants::getPackager();
  QString architecture = StrConstants::getArchitecture();
  QString buildDate = StrConstants::getBuildDate();

  QTextBrowser *text = ui->twProperties->widget(
        ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");

  if (text)
  {
    QString html;
    QString valDownloadSize =
        QString("%1 KB").arg(pid.downloadSize, 6, 'f', 2);

    QString valInstalledSize =
        QString("%1 KB").arg(pid.installedSize, 6, 'f', 2);

    text->clear();
    QString anchorBegin = "anchorBegin";

    html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
    html += "<a id=\"" + anchorBegin + "\"></a>";

    html += "<h2>" + pkgName + "</h2>";
    html += "<a style=\"font-size:16px;\">" + pkgDescription + "</a>";

    html += "<table border=\"0\">";

    html += "<tr><th width=\"20%\"></th><th width=\"80%\"></th></tr>";
    html += "<tr><td>" + url + "</td><td style=\"font-size:14px;\">" + pid.url + "</td></tr>";

    int mark = siIcon->text().indexOf('^');
    if (mark >= 0)
    {
      QString outdatedVersion = siIcon->text().right(siIcon->text().size()-mark-1);
      html += "<tr><td>" + version + "</td><td>" + siVersion->text() + "<b><font color=\"#E55451\">"
                       + StrConstants::getOutdatedInstalledVersion().arg(outdatedVersion) +
                       "</b></font></td></tr>";
    }
    else
    {
      html += "<tr><td>" + version + "</td><td>" + siVersion->text() + "</td></tr>";
    }

    //This is needed as packager names could be encoded in different charsets, resulting in an error
    QString packagerName = pid.packager;
    packagerName = packagerName.replace("<", "&lt;");
    packagerName = packagerName.replace(">", "&gt;");
    packagerName = packagerName.fromUtf8(packagerName.toAscii().data());

    html += "<tr><td>" + licenses + "</td><td>" + pid.license + "</td></tr>";

    //Show this info only if there's something to show
    if(! pid.group.contains("None"))
      html += "<tr><td>" + groups + "</td><td>" + pid.group + "</td></tr>";
    if(! pid.provides.contains("None"))
      html += "<tr><td>" + provides + "</td><td>" + pid.provides + "</td></tr>";
    if(! pid.dependsOn.contains("None"))
      html += "<tr><td>" + dependsOn + "</td><td>" + pid.dependsOn + "</td></tr>";
    if(! pid.optDepends.contains("None"))
      html += "<tr><td>" + optionalDeps + "</td><td>" + pid.optDepends + "</td></tr>";
    if(! pid.conflictsWith.contains("None"))
      html += "<tr><td><b>" + conflictsWith + "</b></td><td><b>" + pid.conflictsWith + "</b></font></td></tr>";
    if(! pid.replaces.contains("None"))
      html += "<tr><td>" + replaces + "</td><td>" + pid.replaces + "</td></tr>";

    html += "<tr><td>" + downloadSize + "</td><td>" + valDownloadSize + "</td></tr>";
    html += "<tr><td>" + installedSize + "</td><td>" + valInstalledSize + "</td></tr>";
    html += "<tr><td>" + packager + "</td><td>" + packagerName + "</td></tr>";
    html += "<tr><td>" + architecture + "</td><td>" + pid.arch + "</td></tr>";
    html += "<tr><td>" + buildDate + "</td><td>" +
        pid.buildDate.toString("ddd - dd/MM/yyyy hh:mm:ss") + "</td></tr>";

    html += "</table>";
    text->setHtml(html);
    text->scrollToAnchor(anchorBegin);
  }

  strSelectedPackage = siRepository->text()+"#"+siName->text()+"#"+siVersion->text();

  if (neverQuit)
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
  }
}

/*
 * Selects the very first item in the tvPkgFileList treeview
 */
void MainWindow::_selectFirstItemOfPkgFileList()
{
  QTreeView *tvPkgFileList = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");
  if(tvPkgFileList)
  {
    tvPkgFileList->setFocus();
    QModelIndex maux = tvPkgFileList->model()->index(0, 0);
    tvPkgFileList->setCurrentIndex(maux);
  }
}

/*
 * Re-populates the treeview which contains the file list of selected package (tab TWO)
 */
void MainWindow::refreshTabFiles(bool clearContents, bool neverQuit)
{
  static QString strSelectedPackage;

  if(neverQuit == false &&
     (ui->twProperties->currentIndex() != ctn_TABINDEX_FILES || !_isPropertiesTabWidgetVisible()))
  {
    return;
  }

  if (clearContents ||
      ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTreeView *tvPkgFileList =
        ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");

    if(tvPkgFileList)
    {
      QStandardItemModel *modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
      modelPkgFileList->clear();
      strSelectedPackage="";
      return;
    }
  }

  QModelIndex item = ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).first();
  QModelIndex mi = m_proxyModelPackages->mapToSource(item);
  QStandardItem *siName;
  QStandardItem *siRepository;
  QStandardItem *siVersion;

  if (ui->actionNonInstalledPackages->isChecked())
  {
    if (m_cbGroups->currentIndex() == 0)
    {
      siName = m_modelPackages->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelPackages->item(mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
    else
    {
      siName = m_modelPackagesFromGroup->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelPackagesFromGroup->item(mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelPackagesFromGroup->item(mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
  }
  else
  {
    if (m_cbGroups->currentIndex() == 0)
    {
      siName = m_modelInstalledPackages->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelInstalledPackages->item(mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelInstalledPackages->item(mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
    else
    {
      siName = m_modelInstalledPackagesFromGroup->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelInstalledPackagesFromGroup->item(mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelInstalledPackagesFromGroup->item(mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
  }

  //If we are trying to refresh an already displayed package...
  if (strSelectedPackage == siRepository->text()+"#"+siName->text()+"#"+siVersion->text())
  {
    if (neverQuit)
    {
      _changeTabWidgetPropertiesIndex(ctn_TABINDEX_FILES);
      _selectFirstItemOfPkgFileList();
    }
    else
    {
      QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
      if (tv)
        tv->scrollTo(tv->currentIndex());
    }

    return;
  }

  //Maybe this is a non-installed package...
  bool nonInstalled = ((ui->actionNonInstalledPackages->isChecked() &&
                        m_cbGroups->currentIndex() == 0) &&
                       (m_modelPackages->item(mi.row(), ctn_PACKAGE_ICON_COLUMN)->text() == "_NonInstalled"));

  if (!nonInstalled)
  {
    //Let's try another test...
    nonInstalled = ((ui->actionNonInstalledPackages->isChecked() &&
                            m_cbGroups->currentIndex() != 0) &&
                           (m_modelPackagesFromGroup->item(
                              mi.row(), ctn_PACKAGE_ICON_COLUMN)->text() == "_NonInstalled"));
  }

  QTreeView *tvPkgFileList = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");
  if (tvPkgFileList){
    QString pkgName = siName->text();
    QStringList fileList = Package::getContents(pkgName);

    QStandardItemModel *fakeModelPkgFileList = new QStandardItemModel(this);
    QStandardItemModel *modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
    modelPkgFileList->clear();
    QStandardItem *fakeRoot = fakeModelPkgFileList->invisibleRootItem();
    QStandardItem *root = modelPkgFileList->invisibleRootItem();
    QStandardItem *bkpDir, *item, *bkpItem=root, *parent;
    bool first=true;
    bkpDir = root;

    if (nonInstalled){
      strSelectedPackage="";
      return;
    }

    CPUIntensiveComputing cic;

    foreach ( QString file, fileList ){
      QFileInfo fi ( file );

      if(fi.isDir()){
        if ( first == true ){
          item = new QStandardItem ( IconHelper::getIconFolder(), file );
          item->setAccessibleDescription("directory " + item->text());
          fakeRoot->appendRow ( item );
        }
        else{
          if ( file.indexOf ( bkpDir->text() ) != -1 ){
            item = new QStandardItem ( IconHelper::getIconFolder(), file );
            item->setAccessibleDescription("directory " + item->text());
            bkpDir->appendRow ( item );
          }
          else{
            parent = bkpItem->parent();
            do{
              if ( parent == 0 || file.indexOf ( parent->text() ) != -1 ) break;
              parent = parent->parent();
            }
            while ( parent != fakeRoot );

            item = new QStandardItem ( IconHelper::getIconFolder(), file );
            item->setAccessibleDescription("directory " + item->text());
            if ( parent != 0 ) parent->appendRow ( item );
            else fakeRoot->appendRow ( item );
          }
        }
        bkpDir = item;
      }
      else{
        item = new QStandardItem ( IconHelper::getIconBinary(), fi.fileName() );
        item->setAccessibleDescription("file " + item->text());
        parent = bkpDir;

        do{
          if ( parent == 0 || file.indexOf ( parent->text() ) != -1 ) break;
          parent = parent->parent();
        }
        while ( parent != fakeRoot );

        parent->appendRow ( item );
      }

      bkpItem = item;
      first = false;
    }

    root = fakeRoot;
    fakeModelPkgFileList->sort(0);
    modelPkgFileList = fakeModelPkgFileList;
    tvPkgFileList->setModel(modelPkgFileList);
    tvPkgFileList->header()->setDefaultAlignment( Qt::AlignCenter );
    modelPkgFileList->setHorizontalHeaderLabels( QStringList() <<
                                                 StrConstants::getContentsOf().arg(pkgName));

    QList<QStandardItem*> lit = modelPkgFileList->findItems( "/", Qt::MatchStartsWith | Qt::MatchRecursive );

    foreach( QStandardItem* it, lit ){
      QFileInfo fi( it->text() );
      if ( fi.isFile() == false ){
        QString s( it->text() );
        s.remove(s.size()-1, 1);
        s = s.right(s.size() - s.lastIndexOf('/') -1);
        it->setText( s );
      }
    }
  } 

  strSelectedPackage = siRepository->text()+"#"+siName->text()+"#"+siVersion->text();

  if (neverQuit)
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_FILES);
    _selectFirstItemOfPkgFileList();
  }

  SearchBar *searchBar = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<SearchBar*>("searchbar");
  if (searchBar)
  {
    if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    {
      searchBar->close();
    }
  }
}

/*
 * Whenever user double clicks the package list items, app shows the contents of the selected package
 */
void MainWindow::onDoubleClickPackageList()
{
  QStandardItemModel *sim=_getCurrentSelectedModel();
  QModelIndex mi = m_proxyModelPackages->mapToSource(ui->tvPackages->currentIndex());
  QStandardItem *siName = sim->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);

  if (isPackageInstalled(siName->text()))
  {
    refreshTabFiles(false, true);
  }
  else
  {
    refreshTabInfo(false, true);
  }
}

/*
 * When the user changes the current selected tab, we must take care of data refresh.
 */
void MainWindow::changedTabIndex()
{
  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION)
    refreshTabInfo();
  else if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    refreshTabFiles();

  if(m_initializationCompleted)
    saveSettings(ectn_CurrentTabIndex);
}

/*
 * Clears the information showed on the current tab (Info or Files).
 */
void MainWindow::invalidateTabs()
{
  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION) //This is TabInfo
  {
    refreshTabInfo(true);
    return;
  }
  else if(ui->twProperties->currentIndex() == ctn_TABINDEX_FILES) //This is TabFiles
  {
    refreshTabFiles(true);
    return;
  }
}

/*
 * Maximizes/de-maximizes the upper pane (tvPackages)
 */
void MainWindow::maximizePackagesTreeView(bool pSaveSettings)
{
  QList<int> savedSizes;
  savedSizes << 200 << 235;

  QList<int> l, rl;
  rl = ui->splitterHorizontal->sizes();

  if ( rl[1] != 0 )
  {
    ui->splitterHorizontal->setSizes( l << ui->tvPackages->maximumHeight() << 0);
    if(!ui->tvPackages->hasFocus())
      ui->tvPackages->setFocus();

    if(pSaveSettings)
      saveSettings(ectn_MAXIMIZE_PACKAGES);
  }
  else
  {
    ui->splitterHorizontal->setSizes(savedSizes);
    ui->tvPackages->scrollTo(ui->tvPackages->currentIndex());
    ui->tvPackages->setFocus();

    if(pSaveSettings)
      saveSettings(ectn_NORMAL);
  }
}

/*
 * Maximizes/de-maximizes the lower pane (tabwidget)
 */
void MainWindow::maximizePropertiesTabWidget(bool pSaveSettings)
{
  QList<int> savedSizes;
  savedSizes << 200 << 235;

  QList<int> l, rl;
  rl = ui->splitterHorizontal->sizes();

  if ( rl[0] != 0 )
  {
    ui->splitterHorizontal->setSizes( l << 0 << ui->twProperties->maximumHeight());
    ui->twProperties->currentWidget()->childAt(1,1)->setFocus();

    if(pSaveSettings)
      saveSettings(ectn_MAXIMIZE_PROPERTIES);
  }
  else
  {
    ui->splitterHorizontal->setSizes(savedSizes);
    ui->tvPackages->scrollTo(ui->tvPackages->currentIndex());
    ui->tvPackages->setFocus();

    if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    {
      QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
      if (tv)
        tv->scrollTo(tv->currentIndex());
    }

    if(pSaveSettings)
      saveSettings(ectn_NORMAL);
  }
}

/*
 * This SLOT is called every time we press a key at FilterLineEdit
 */
void MainWindow::reapplyPackageFilter()
{
  CPUIntensiveComputing cic;

  bool isFilterPackageSelected = m_leFilterPackage->hasFocus();
  QString search = Package::parseSearchString(m_leFilterPackage->text());
  QRegExp regExp(search, Qt::CaseInsensitive, QRegExp::RegExp);

  m_proxyModelPackages->setFilterRegExp(regExp);
  int numPkgs = m_proxyModelPackages->rowCount();

  if (m_leFilterPackage->text() != ""){
    if (numPkgs > 0)
      m_leFilterPackage->setFoundStyle();
    else m_leFilterPackage->setNotFoundStyle();
  }
  else{
    m_leFilterPackage->initStyleSheet();;
    m_proxyModelPackages->setFilterRegExp("");
  }

  if (isFilterPackageSelected) m_leFilterPackage->setFocus();
  m_proxyModelPackages->sort(m_PackageListOrderedCol, m_PackageListSortOrder);

  ui->tvPackages->selectionModel()->clear();
  QModelIndex mi = m_proxyModelPackages->index(0, ctn_PACKAGE_NAME_COLUMN);
  ui->tvPackages->setCurrentIndex(mi);
  ui->tvPackages->scrollTo(mi);

  //We need to call this method to refresh package selection counters
  tvPackagesSelectionChanged(QItemSelection(),QItemSelection());
  invalidateTabs();
}

/*
 * Whenever a user clicks on the Sort indicator of the package treeview, we keep values to mantain his choices
 */
void MainWindow::headerViewPackageListSortIndicatorClicked( int col, Qt::SortOrder order )
{
  m_PackageListOrderedCol = col;
  m_PackageListSortOrder = order;

  saveSettings(ectn_PackageList);
}

/*
 * Helper method to position the text cursor always in the end of doc
 */
void MainWindow::_positionTextEditCursorAtEnd()
{
  QTextBrowser *textEdit =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (textEdit){
    QTextCursor tc = textEdit->textCursor();
    tc.clearSelection();
    tc.movePosition(QTextCursor::End);
    textEdit->setTextCursor(tc);
  }
}

/*
 * Ensures the given index tab is visible
 */
void MainWindow::_ensureTabVisible(const int index)
{
  QList<int> rl;
  rl = ui->splitterHorizontal->sizes();

  if(rl[1] <= 50)
  {
    rl.clear();
    rl << 200 << 235;
    ui->splitterHorizontal->setSizes(rl);

    saveSettings(ectn_NORMAL);
  }

  ui->twProperties->setCurrentIndex(index);
}

/*
 * Helper method to find the given "findText" in the Output TextEdit
 */
bool MainWindow::_textInTabOutput(const QString& findText)
{
  bool res;
  QTextBrowser *text =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (text)
  {
    _positionTextEditCursorAtEnd();
    res = text->find(findText, QTextDocument::FindBackward | QTextDocument::FindWholeWords);
    _positionTextEditCursorAtEnd();
  }

  return res;
}

/*
 * Helper method that opens an existing file using the available program/DE.
 */
void MainWindow::openFile()
{
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if (tv)
  {
    QString path = PackageController::showFullPathOfItem(tv->currentIndex());
    QFileInfo file(path);
    if (file.isFile())
    {
      WMHelper::openFile(path);
    }
  }
}

/*
 * Helper method that edits an existing file using the available program/DE.
 */
void MainWindow::editFile()
{
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if (tv)
  {
    QString path = PackageController::showFullPathOfItem(tv->currentIndex());
    WMHelper::editFile(path);
  }
}

/*
 * Helper method that opens a terminal in the selected directory, using the available program/DE.
 */
void MainWindow::openTerminal()
{
  QString dir = getSelectedDirectory();

  if (!dir.isEmpty())
  {
    WMHelper::openTerminal(dir);
  }
}

/*
 * Helper method that opens an existing directory using the available program/DE.
 */
void MainWindow::openDirectory(){
  QString dir = getSelectedDirectory();

  if (!dir.isEmpty())
  {
    WMHelper::openDirectory(dir);
  }
}

/*
 * Returns the current selected directory of the FileList treeview in FilesTab
 * In case nothing is selected, returns an empty string
 */
QString MainWindow::getSelectedDirectory()
{
  QString targetDir;

  if (_isPropertiesTabWidgetVisible() && ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
  {
    QTreeView *t = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
    if(t && t->currentIndex().isValid())
    {
      QString itemPath = PackageController::showFullPathOfItem(t->currentIndex());
      QFileInfo fi(itemPath);

      if (fi.isDir())
        targetDir = itemPath;
      else targetDir = fi.path();
    }
  }

  return targetDir;
}

/*
 * Changes the number of selected items in tvPackages: YY in XX(YY) Packages
 */
void MainWindow::tvPackagesSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  QString newMessage = StrConstants::getSelectedPackages().arg(
        QString::number(m_proxyModelPackages->rowCount())).
      arg(QString::number(ui->tvPackages->selectionModel()->selectedRows().count()));

  m_lblSelCounter->setText(newMessage);
}
