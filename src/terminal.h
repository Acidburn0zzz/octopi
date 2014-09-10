/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2014 Alexandre Albuquerque Arnt
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

#ifndef TERMINAL_H
#define TERMINAL_H

#include "utils.h"

#include <QString>
#include <QObject>
#include <QProcess>

class TerminalType : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString terminal READ terminal WRITE setTerminal NOTIFY terminalChanged )

  QString m_terminal;

public:
  QString terminal() const { return m_terminal; }

  void setTerminal(const QString &terminal) { m_terminal = terminal; }

  TerminalType(QString terminal){
    setTerminal(terminal);
  }

  Q_SIGNALS:
    void terminalChanged();

};

class Terminal : public QObject
{
  Q_OBJECT

private:
  QString m_selectedTerminal;
  QProcess *m_process;
  utils::ProcessWrapper *m_processWrapper;

public:
  Terminal(QObject *parent, const QString& selectedTerminal);
  virtual ~Terminal();

  void openTerminal(const QString& dirName);
  void openRootTerminal();
  void runCommandInTerminal(const QStringList& commandList);
  void runCommandInTerminalAsNormalUser(const QStringList& commandList);

  static QStringList getListOfAvailableTerminals();

signals:
  void started();
  void finished(int, QProcess::ExitStatus);
  void startedTerminal();
  void finishedTerminal(int, QProcess::ExitStatus);
};

#endif // TERMINAL_H
