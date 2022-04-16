﻿#include "repeatermodel.h"

rbManager::rbManager(QObject *parent)
    : QAbstractListModel(parent)
{
    found = false;

    prov = new QGeoServiceProvider("osm");
    prov->setLocale(QLocale::English);
    geoCoder = prov->geocodingManager();

    source = QGeoPositionInfoSource::createDefaultSource(this);

    connect(source,
            SIGNAL(positionUpdated(QGeoPositionInfo)),
            this,
            SLOT(positionUpdated(QGeoPositionInfo)));
    source->startUpdates();

    nm = new QNetworkAccessManager(this);
    connect(nm,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(parseNetworkResponse(QNetworkReply*)));
}

int rbManager::rowCount(const QModelIndex &parent) const
{
    return database.size();
}

QVariant rbManager::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {

        switch(role)
        {
            case call: return database[index.row()].call;
            case lati: return database[index.row()].lat;
            case lond: return database[index.row()].lon;
            case dist: return database[index.row()].distance;
            case freq: return database[index.row()].frequency;
            case shif: return database[index.row()].shift;
            case tone: return database[index.row()].tone;
            case mode: return database[index.row()].modes;
            case city: return database[index.row()].city;
        }
    }
    return QVariant();
}

QHash<int, QByteArray> rbManager::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[call] = "call";
    roles[lati] = "lati";
    roles[lond] = "lond";
    roles[dist] = "dist";
    roles[freq] = "freq";
    roles[shif] = "shif";
    roles[tone] = "tone";
    roles[mode] = "mode";
    roles[city] = "city";
    return roles;
}

void rbManager::positionUpdated(const QGeoPositionInfo &info)
{
    qDebug() << "Position updated:" << info;
    reply = geoCoder->reverseGeocode(info.coordinate());
    connect(reply, &QGeoCodeReply::finished, this, &rbManager::positionDecoded);
}

void rbManager::positionDecoded()
{
    if(reply->locations().length() == 1) {
        QGeoLocation loc = reply->locations().at(0);
        QGeoAddress addr = loc.address();
        country = addr.country();
        coord = loc.coordinate();
        qDebug() << country << coord.latitude() << coord.longitude();
        found = true;
        getRepeaters(country);
    }

    delete reply;
}

// ----

void rbManager::getRepeaters(QString country)
{
    QString url = "https://www.repeaterbook.com/api/exportROW.php?country="+country;
    nm->get(QNetworkRequest(QUrl(url)));
}

bool rbManager::filter(double rLat, double rLon, double radius)
{
    QGeoCircle circ(coord, 1000.0 * radius);
    QGeoCoordinate repeater(rLat,rLon);
    return circ.contains(repeater);
}

double rbManager::distance(double rLat, double rLon)
{
    QGeoCoordinate repeater(rLat,rLon);
    return repeater.distanceTo(coord)/1000.0;
}

void rbManager::parseNetworkResponse(QNetworkReply *nreply)
{
    QString rawJson = nreply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(rawJson.toUtf8());
    QJsonObject json = jsonResponse.object();

    QJsonArray repeaters = json["results"].toArray();

    for(auto repeater : repeaters) {
        relais r;
        double rlat = repeater.toObject()["Lat"].toString().toDouble();
        double rlon = repeater.toObject()["Long"].toString().toDouble();

        double radius = settings.value("rbRadius").toString().toDouble();

        if(filter(rlat, rlon, radius)) {
            r.call      = repeater.toObject()["Callsign"].toString();
            r.frequency = repeater.toObject()["Frequency"].toString();
            r.lat       = repeater.toObject()["Lat"].toString();
            r.lon       = repeater.toObject()["Long"].toString();
            r.shift     = QString::number(repeater.toObject()["Input Freq"].toString().toDouble() - r.frequency.toDouble());
            r.distance  = QString::number(distance(rlat, rlon));
            r.tone      = repeater.toObject()["PL"].toString();
            r.city      = repeater.toObject()["Nearest City"].toString();
            r.modes     = "";

            r.modes += repeater.toObject()["FM Analog"].toString() == "Yes" ? "FM "     : "";
            r.modes += repeater.toObject()["DMR"].toString() == "Yes" ? "DMR "    : "";
            r.modes += repeater.toObject()["D-Star"].toString() == "Yes" ? "D-Star " : "";
            r.modes += repeater.toObject()["System Fusion"].toString() == "Yes" ? "Fusion " : "";
            r.modes += repeater.toObject()["Wires Node"].toString() != "" ? "Wires " : "";

            qDebug() << r.call << r.lat << r.lon << r.frequency << r.shift << r.distance << r.tone << r.modes;

            database.append(r);
        }
    }

    std::sort(database.begin(), database.end(), [](relais a, relais b) {
        return a.distance < b.distance;
    });
}
