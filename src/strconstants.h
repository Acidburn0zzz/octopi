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

#ifndef STRCONSTANTS_H
#define STRCONSTANTS_H

#include "settingsmanager.h"
#include "wmhelper.h"
#include "unixcommand.h"

class StrConstants{
public:
  static QString getApplicationName(){
    return "Octopi";
  }

  static QString getApplicationVersion(){
    return "0.1.4.1";
  }

  static QString getApplicationCliHelp(){
    QString str =
        "\n" + QObject::tr("Octopi help usage:") + "\n\n" +
        "-version: " + QObject::tr("show application version.") + "\n" +
        "-style <Qt4-style>: " + QObject::tr("use a different Qt4 style (ex: -style gtk).") + "\n";

    return str;
  }

  static QString getForeignRepositoryName(){
    return "AUR";
  }

  static QString getArchLinuxNews(){
    return QObject::tr("Arch Linux news");
  }

  static QString getManjaroLinuxNews(){
    return QObject::tr("Manjaro Linux news");
  }

  static QString getNewsErrorMessage(){
    return QObject::tr("No news could be found! Press Ctrl+G to download the latest news.");
  }

  static QString getIncompatibleLinuxDistroError(){
    return QObject::tr("This Linux distro seems to be incompatible with Octopi!");
  }

  static QString getInternetUnavailableError(){
    return QObject::tr("Internet seems anavailable!");
  }

  static QString getAll(){
    return QObject::tr("Display all groups");
  }

  static QString getHelp(){
    return QObject::tr("About");
  }

  static QString getName(){
    return QObject::tr("Name");
  }

  static QString getVersion(){
    return QObject::tr("Version");
  }

  static QString getOutdatedVersion(){
    return QObject::tr("Outdated version");
  }

  static QString getAvailableVersion(){
    return QObject::tr("Available version");
  }

  static QString getURL(){
    return QObject::tr("URL");
  }

  static QString getRepository(){
    return QObject::tr("Repository");
  }

  static QString getLicenses(){
    return QObject::tr("Licenses");
  }

  static QString getGroup(){
    return QObject::tr("Group");
  }

  static QString getGroups(){
    return QObject::tr("Groups");
  }

  static QString getProvides(){
    return QObject::tr("Provides");
  }

  static QString getDependsOn(){
    return QObject::tr("Depends On");
  }

  static QString getOptionalDeps(){
    return QObject::tr("Optional Deps");
  }

  static QString getConflictsWith(){
    return QObject::tr("Conflicts With");
  }

  static QString getReplaces(){
    return QObject::tr("Replaces");
  }

  static QString getDownloadSize(){
    return QObject::tr("Download Size");
  }

  static QString getInstalledSize(){
    return QObject::tr("Installed Size");
  }

  static QString getPackager(){
    return QObject::tr("Packager");
  }

  static QString getPackage(){
    return QObject::tr("Package");
  }

  static QString getPackages(){
    return QObject::tr("Packages");
  }

  static QString getArchitecture(){
    return QObject::tr("Architecture");
  }

  static QString getBuildDate(){
    return QObject::tr("Build Date");
  }

  static QString getDescription(){
    return QObject::tr("Description");
  }

  static QString getAttention(){
    return QObject::tr("Attention");
  }

  static QString getAutomaticSuCommand(){
    return QObject::tr("automatic");
  }

  static QString getPassword(){
    return QObject::tr("Password");
  }

  static QString getTabInfoName(){
    return QObject::tr("Info");
  }

  static QString getTabFilesName(){
    return QObject::tr("Files");
  }

  static QString getTabTransactionName(){
    return QObject::tr("Transaction");
  }

  static QString getTabOutputName(){
    return QObject::tr("Output");
  }

  static QString getTabNewsName(){
    return QObject::tr("News");
  }

  static QString getContentsOf(){
    return QObject::tr("Contents of \"%1\"");
  }

  static QString getFind(){
    return QObject::tr("Find");
  }

  static QString getClear(){
    return QObject::tr("Clear");
  }

  static QString getOutdatedInstalledVersion(){
    return QObject::tr(" (outdated installed version is %1)");
  }

  static QString getBuildingPackageList(){
    return QObject::tr("Building package list...");
  }

  static QString getSearchingForArchLinuxNews(){
    return QObject::tr("Searching for Arch Linux latest news...");
  }

  static QString getSearchingForManjaroLinuxNews(){
    return QObject::tr("Searching for Manjaro Linux latest news...");
  }

  static QString getOneOutdatedPackage(){
    return QObject::tr("There is one outdated package in your system:");
  }

  static QString getOutdatedPackages(){
    return QObject::tr("There are %1 outdated packages in your system:");
  }

  static QString getNewVersionAvailable(){
    return QObject::tr("(version %1 is available)");
  }

  static QString getSelectedPackages(){
    return QObject::tr("%1 (%2) packages");
  }

  static QString getNumberInstalledPackages(){
    return QObject::tr("%1 installed");
  }

  static QString getNumberOutdatedPackages(){
    return QObject::tr("%1 outdated");
  }

  static QString getNumberAvailablePackages(){
    return QObject::tr("%1 available");
  }

  static QString getCleaningPackageCache(){
    return QObject::tr("Cleaning package cache...");
  }

  static QString getSynchronizing(){
    return QObject::tr("Synchronizing");
  }

  static QString getSyncDatabases(){
    return QObject::tr("Synchronizing databases...");
  }
  static QString getSystemUpgrade(){
    return QObject::tr("Starting full system upgrade...");
  }

  static QString getInstallingPackages(){
    return QObject::tr("Installing selected packages...");
  }

  static QString getRemovingPackages(){
    return QObject::tr("Removing selected packages...");
  }

  static QString getCommandFinishedOK(){
    return QObject::tr("Command finished OK!");
  }

  static QString getCommandFinishedWithErrors(){
    return QObject::tr("Command finished with errors!");
  }

  static QString geRetrievingTarget(){
    return QObject::tr("Retrieving %1");
  }

  static QString getTotalDownloadSize(){
    return QObject::tr("Total download size: %1 KB");
  }

  static QString getRetrieveTarget(){
    return QObject::tr("The following target needs to be retrieved");
  }

  static QString getRemoveTarget(){
    return QObject::tr("The following target will be removed");
  }

  static QString getRetrieveTargets(){
    return QObject::tr("The following %1 targets need to be retrieved");
  }

  static QString getRemoveTargets(){
    return QObject::tr("The following %1 targets will be removed");
  }

  static QString getWarnHoldPkgFound() {
    return QObject::tr("There are forbidden packages in the removal list!");
  }

  static QString getNoNewUpdatesAvailable(){
    return QObject::tr("There are no new updates available!");
  }

  static QString getConfirmationQuestion(){
    return QObject::tr("Confirm?");
  }

  static QString getWarning(){
    return QObject::tr("Warning!!!");
  }

  static QString getConfirmation(){
    return QObject::tr("Confirmation");
  }

  static QString getCleanCacheConfirmation(){
    return QObject::tr("Do you really want to clean the package cache?");
  }

  static QString getRollbackTransactionConfirmation(){
    return QObject::tr("Do you really want to rollback the transaction?");
  }

  static QString getEnterAdministratorsPassword(){
    return QObject::tr(
          "Please, enter the administrator's password");
  }

  static QString getErrorNoSuCommand(){
    return
      QObject::tr("There are no means to get administrator's credentials.");
  }

  static QString getYoullNeedSuFrontend(){
    return QObject::tr("You'll need to install a su frontend like gksu or kdesu.");
  }

  static QString getErrorRunningWithRoot(){
    return QObject::tr("You can not run Octopi with administrator's credentials.");
  }

  static QString getThereIsAPendingTransaction(){
    return QObject::tr("There is a pending transaction");
  }

  static QString getDoYouReallyWantToQuit(){
    return QObject::tr("Do you really want to quit?");
  }

  static QString getExecutingCommand(){
    return QObject::tr("Executing command");
  }

  static QString getNeedsAppRestart(){
    return QObject::tr("Needs application restart to take effect");
  }

  static QString getWarnNeedsAppRestart(){
    return QObject::tr("These changes need application restart to take effect!");
  }

  /*static QString getForMoreInformation(){
    return "<small>" + QObject::tr("For more information about QTGZManager visit:") +
        " <a href=\"http://qtgzmanager.wordpress.com\" target=\"_blank\">" +
        "http://qtgzmanager.wordpress.com</a></small>";
  }*/

  static QString getThisIsNotATextFile(){
    return QObject::tr("This file does not appear to be a simple text.\n"
                       "Are you sure you want to open it?");
  }

  static QString getTransactionInstallText(){
    return QObject::tr("To be installed");
  }

  static QString getTransactionRemoveText(){
    return QObject::tr("To be removed");
  }

  static QString getRemoveItem(){
    return QObject::tr("Remove item");
  }

  static QString getRemoveItems(){
    return QObject::tr("Remove items");
  }

  //Style Sheets ---------------------------------

  static QString getToolTipNormalCSS(){
    return QString("QToolTip {"
                   "background-color: #f7fcdc;"
                   "color: #000000;"
                   "border: 0.6px solid #337700;" //#effd72;"
                   "padding: 0.2px;"
                   "border-radius: 3px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}"
        );
  }

  static QString getToolTipBlankCSS(){
    return QString("QToolTip {"
                   "background-color: white;"
                   "color: #444444;"
                   "border: 0.5px solid #cccccc;"
                   "padding: 1.5px;"
                   "border-radius: 3px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}"
        );
  }

  static QString getToolTipRedCSS(){
    return QString("QToolTip {"
                   "background-color: #FF7A7A;" //"background-color: #FF8A8A;"
                   "color: #550000;"
                   "border: 0.6px solid #990000;"
                   "padding: 1.5px;"
                   "border-radius: 3px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}");
  }

  static QString getToolTipGreenCSS(){
    return QString("QToolTip {"
                   "background-color: #BDF4CB;"
                   "color: #000000;"
                   "border: 0.6px solid #006600;"
                   "padding: 1.5px;"
                   "border-radius: 3px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}"
                   );
  }

  static QString getToolTipBlueCSS(){
    return QString("QToolTip {"
                   "background-color: #CFECEC;"
                   "color: #000000;"
                   "border: 0.6px solid #3090C7;"
                   "padding: 1.5px;"
                   "border-radius: 3px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}"
                   );
  }

  static QString getToolTipYellowCSS(){
    return QString(" QToolTip {"
                   "background-color: #FFFF66;" //#FFCC00;"
                   "color: #000000;"
                   "border: 0.6px solid #FF9900;"
                   "padding: 1.5px;"
                   "border-radius: 3px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}"
                   );
  }

  static QString getToolTipRemovedCSS(){
    return QString(" QToolTip {"
                   "background-color: #FFFFDD;"
                   "color: #000000;"
                   "border: 0.6px solid #000000;"
                   "padding: 0.2px;"
                   "border-radius: 1px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}"
                   );
  }

  static QString getToolBarCSS(){
    return QString("QToolBar { border: 5px; } "
                   "QToolTip {}"
                   );
  }

  static QString getFilterPackageNotFoundCSS(){
    return QString("QLineEdit{ color: white; "
                   "background-color: rgb(207, 135, 142);"
                   "border-color: rgb(206, 204, 197);}"
                   );
  }

  static QString getFilterPackageFoundCSS(){
    return QString("QLineEdit, SearchLineEdit{ color: black; "
                   "background-color: rgb(255, 255, 200);"
                   "border-color: rgb(206, 204, 197);}"
                   );
  }

  static QString getDockWidgetTitleCSS(){
    return QString("QDockWidget::title { "
                   "text-align: right;"
                   "background: transparent;"
                   "padding-right: 5px;}"
                   );
  }

  static QString getTabBarCSS(){
    return QString("QTabBar::close-button {"
                   "image: url(:/resources/images/window-close.png);"
                   "border-radius: 4px}"
                   "QTabBar::close-button:hover {"
                   "background-color: palette(light)}"
                   "QTabBar::close-button:pressed {"
                   "background-color: palette(mid)}"
                   );
  }

  static QString getMenuCSS(){
    QString res;

    if (UnixCommand::getLinuxDistro() == ectn_MANJAROLINUX && !WMHelper::isKDERunning())
    {
      res = "QMenu::item { padding: 2px 25px 2px 20px; }"
          "QListView::item:selected { background: #81ac44; color: #ffffff; }"
          "QMenu::item:selected, QMenuBar::item:selected, QMenu::selected {"
          "background: #81ac44; color: #ffffff;}"
          " QTabBar::tab { /* The tab widget frame */"
          //"background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
          //                            "stop: 0 #E1E1E1, stop: 0.4 #DDDDDD,"
          //                            "stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3);"
          "border: 1px solid #C4C4C3;"
          "border-bottom-color: #C2C7CB; /* same as the pane color */"
          "border-top-left-radius: 4px;"
          "border-top-right-radius: 4px;"
          "min-width: 8ex;"
          "padding: 2px;"
          "margin-left: 1px;"
          "margin-right: 1px;}"

          "QTabBar::tab:selected, QTabBar::tab:hover {"
          "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                      "stop: 0 #fafafa, stop: 0.4 #f4f4f4,"
                                      "stop: 0.5 #e7e7e7, stop: 1.0 #fafafa); border: 2px solid #C4C4C3; color: #000000;}"
      "QProgressBar::chunk {"
          "background-color: #81ac44;}"
          //"width: 20px;}";
      "QProgressBar {"
          "text-align: center;"
          "border: 2px;}";
    }

    else res = "";

    return res;
  }

  static QString getTreeViewCSS(){
    QString res;    

    if (UnixCommand::getLinuxDistro() == ectn_MANJAROLINUX && !WMHelper::isKDERunning())
    {
      res = "QTreeView::branch:has-siblings:!adjoins-item {"
                   "   border-image: url(:/resources/styles/vline.png) 0;}"
                   "QTreeView::branch:has-siblings:adjoins-item {"
                   "    border-image: url(:/resources/styles/branch-more.png) 0;}"
                   "QTreeView::branch:!has-children:!has-siblings:adjoins-item {"
                   "   border-image: url(:/resources/styles/branch-end.png) 0;}"
                   "QTreeView::branch:has-children:!has-siblings:closed,"
                   "QTreeView::branch:closed:has-children:has-siblings {"
                   "       border-image: none;"
                   "        image: url(:/resources/styles/branch-closed_BW.png);}"
                   "QTreeView::branch:open:has-children:!has-siblings,"
                   "QTreeView::branch:open:has-children:has-siblings  {"
                   "       border-image: none;"
                   "       image: url(:/resources/styles/branch-open_BW.png);}"
                   "QTreeView {"
                       "selection-background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #81ac44, stop: 1 #6b9039);"
                       "show-decoration-selected: 1;"
                   "}"
                   "QTreeView::item {"
                       "border: 1px solid #d9d9d9;"
                       "border-top-color: transparent;"
                       "border-bottom-color: transparent;"
                   "}"
                   "QTreeView::item:hover {"
                       "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #a7c976, stop: 1 #9bc264);"
                       "border: 1px solid #95be5a;"
                       "color: #191919;"
                   "}"
                   "QTreeView::item:selected {"
                       "border: 1px solid #6d9039;"
                   "}"
                   "QTreeView::item:selected:active{"
                       "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #81ac44, stop: 1 #6b9039)"
                   "}"
                   "QTreeView::item:selected:!active {"
                       "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #81ac44, stop: 1 #6c9039);"
                       "color: #ebebeb;"
                   "}";
    }
    else
    {
      res = "QTreeView::branch:has-siblings:!adjoins-item {"
                     " border-image: url(:/resources/styles/vline.png) 0;}"
                     "QTreeView::branch:has-siblings:adjoins-item {"
                     " border-image: url(:/resources/styles/branch-more.png) 0;}"
                     "QTreeView::branch:!has-children:!has-siblings:adjoins-item {"
                     " border-image: url(:/resources/styles/branch-end.png) 0;}"
                     "QTreeView::branch:has-children:!has-siblings:closed,"
                     "QTreeView::branch:closed:has-children:has-siblings {"
                     " border-image: none;"
                     " image: url(:/resources/styles/branch-closed_BW.png);}"
                     "QTreeView::branch:open:has-children:!has-siblings,"
                     "QTreeView::branch:open:has-children:has-siblings {"
                     " border-image: none;"
                     " image: url(:/resources/styles/branch-open_BW.png);}"

                     "QTreeView {"
                         "selection-background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6ea1f1, stop: 1 #567dbc);"
                         "show-decoration-selected: 1;"
                     "}"
                     "QTreeView::item {"
                         "border: 1px solid #d9d9d9;"
                         "border-top-color: transparent;"
                         "border-bottom-color: transparent;"
                     "}"
                     "QTreeView::item:hover {"
                         "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #e7effd, stop: 1 #cbdaf1);"
                         "border: 1px solid #bfcde4;"
                         "color: #000000;"
                     "}"
                     "QTreeView::item:selected {"
                         "border: 1px solid #567dbc;"
                     "}"
                     "QTreeView::item:selected:active{"
                         "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6ea1f1, stop: 1 #567dbc)"
                     "}"
                     "QTreeView::item:selected:!active {"
                     " background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6b9be8, stop: 1 #577fbf)"
                     "}";
    }

    return res;
  }
};

#endif // STRCONSTANTS_H
