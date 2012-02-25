/*
    IanniX — a graphical real-time open-source sequencer for digital art
    Copyright (C) 2010-2012 — IanniX Association

    This program is free software: you can redistribute it and/or modify
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

#ifndef EXTMIDIMANAGER_H
#define EXTMIDIMANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include "extmessage.h"
#include "qrtmidi/RtMidi.h"

#define MASK_CHANNEL      0x0f
#define MASK_STATUS       0xf0
#define MASK_SAFETY       0x7f

#define STATUS_NOTEOFF    0x80
#define STATUS_NOTEON     0x90
#define STATUS_CTLCHG     0xB0
#define STATUS_PROGRAM    0xC0
#define STATUS_BEND       0xE0

void midiCallback(double deltatime, std::vector< unsigned char > *receivedMessage, void *userData);


class ExtMidiManager : public QObject, public ExtMessageManager {
    Q_OBJECT

private:
    QHash<QString, RtMidiIn*> portIn;
    QHash<QString, RtMidiOut*> portOut;

public:
    ExtMidiManager(NxObjectFactoryInterface *_factory);

public:
    void send(const ExtMessage & message);
    void sendNote(const QString & portname, quint8 channel, quint16 note, quint16 velocity);
    void sendCC(const QString & portname, quint8 channel, quint16 controller, quint16 value);
    void sendPGM(const QString & portname, quint8 channel, quint16 program);
    void sendBend(const QString & portname, quint8 channel, quint16 bendvalue);

public slots:
    void receivedMessage(const QString & destination, const QStringList &arguments);
};


class ExtMidiNoteOff : public QObject {
    Q_OBJECT

private:
    QString portname;
    quint8 channel;
    quint16 note;
    ExtMidiManager *manager;

public:
    ExtMidiNoteOff(ExtMidiManager *_manager, const QString & _portname, quint8 _channel, quint16 _note, quint16 delay) {
        manager = _manager;
        portname = _portname;
        channel = _channel;
        note = _note;
        startTimer(delay);
    }
protected:
    void timerEvent(QTimerEvent *e) {
        manager->sendNote(portname, channel, note, 0);
        killTimer(e->timerId());
        delete this;
    }
};

#endif // EXTMIDIMANAGER_H
