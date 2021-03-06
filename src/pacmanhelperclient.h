/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -v -c PacmanHelperClient -p pacmanhelperclient.h:pacmanhelperclient.cpp pacmanhelper/polkit/org.octopi.pacmanhelper.xml
 *
 * qdbusxml2cpp is Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef PACMANHELPERCLIENT_H_1370187937
#define PACMANHELPERCLIENT_H_1370187937

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.octopi.pacmanhelper
 */
class PacmanHelperClient: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.octopi.pacmanhelper"; }

public:
    PacmanHelperClient(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~PacmanHelperClient();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> syncdb()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("syncdb"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void syncdbcompleted();
};

namespace org {
  namespace octopi {
    typedef ::PacmanHelperClient pacmanhelper;
  }
}
#endif
