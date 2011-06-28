#include "iannix.h"


IanniX::IanniX(QObject *parent, bool forceSettings) :
    NxObjectFactoryInterface(parent) {
    currentDocument = 0;
    projectScore = 0;
    freehandCurveId = 0;
    lastMessageAllow = true;
    splash = new UiSplash(0);
    splash->show();
    QTimer::singleShot(1500, this, SLOT(closeSplash()));
    scriptDir = QDir::current();

    //Update management
    updateManager = new QNetworkAccessManager(this);
    connect(updateManager, SIGNAL(finished(QNetworkReply*)), SLOT(checkForUpdatesFinished(QNetworkReply*)));

    //View
    view = new UiView(0);
    connect(view, SIGNAL(actionRouteFast_rewind()), SLOT(actionFast_rewind()));
    connect(view, SIGNAL(actionRoutePlay_pause()), SLOT(actionPlay_pause()));
    connect(view, SIGNAL(actionRouteToggle_Inspector()), SLOT(actionToggle_Inspector()));
    connect(view, SIGNAL(actionRouteToggle_Transport()), SLOT(actionToggle_Transport()));
    connect(view, SIGNAL(actionRouteToggle_Autosize()), SLOT(actionToggle_Autosize()));
    connect(view, SIGNAL(actionRouteDrawFreeCurve()), SLOT(actionDrawFreeCurve()));
    connect(view, SIGNAL(actionRouteDrawPointCurve()), SLOT(actionDrawPointCurve()));
    connect(view, SIGNAL(actionRouteDrawTriggers()), SLOT(actionDrawTriggers()));
    connect(view, SIGNAL(actionRouteAddFreeCursor()), SLOT(actionAddFreeCursor()));
    connect(view, SIGNAL(actionRouteCircleCurve()), SLOT(actionCircleCurve()));
    connect(view, SIGNAL(actionRouteGridChange(qreal)), SLOT(actionGridChange(qreal)));
    connect(view, SIGNAL(actionRouteGridOpacityChange(qreal)), SLOT(actionGridOpacityChange(qreal)));
    connect(view, SIGNAL(actionRouteSnapGrid()), SLOT(actionSnapGrid()));
    connect(view, SIGNAL(actionRouteCloseEvent(QCloseEvent*)), SLOT(actionCloseEvent(QCloseEvent*)));
    connect(view, SIGNAL(actionRouteAbout()), SLOT(actionLogo()));


    //Transport
    transport = view->getTransport();
    connect(transport, SIGNAL(actionRouteFast_rewind()), SLOT(actionFast_rewind()));
    connect(transport, SIGNAL(actionRoutePlay_pause()), SLOT(actionPlay_pause()));
    connect(transport, SIGNAL(actionRouteLogo()), SLOT(actionLogo()));
    connect(transport, SIGNAL(actionRouteGoto()), SLOT(actionGoto()));
    connect(transport, SIGNAL(actionRouteSetOpenGL()), SLOT(actionSetOpenGL()));
    connect(transport, SIGNAL(actionRouteSetScheduler()), SLOT(actionSetScheduler()));
    connect(transport, SIGNAL(actionRouteSpeed()), SLOT(actionSpeed()));
    connect(this, SIGNAL(updateTransport(QString,QString)), transport, SLOT(updateTransport(QString,QString)));

    //Inspector
    inspector = view->getInspector();
    inspector->setFactory(this);
    connect(inspector, SIGNAL(actionRouteTabChange(int)), SLOT(actionTabChange(int)));
    connect(inspector, SIGNAL(actionRouteViewChange()), SLOT(actionViewChange()));
    connect(inspector, SIGNAL(actionRouteCC()), SLOT(actionCC()));
    connect(inspector, SIGNAL(actionRouteCC2()), SLOT(actionCC2()));
    connect(inspector, SIGNAL(actionRouteProjectFiles()), SLOT(actionProjectFiles()));
    connect(inspector, SIGNAL(actionRouteProjectScripts()), SLOT(actionProjectScripts()));
    connect(inspector, SIGNAL(actionRouteProjectScriptsContext(QPoint)), SLOT(actionProjectScriptsContext(QPoint)));
    connect(inspector, SIGNAL(transportMessageChange(QString)), SLOT(transportMessageChange(QString)));

    //Render
    render = view->getRender();
    render->setFactory(this);
    connect(render, SIGNAL(setPerfOpenGL(QString)), transport, SLOT(setPerfOpenGL(QString)));
    connect(render, SIGNAL(mousePosChanged(QPointF)), inspector, SLOT(setMousePos(QPointF)));
    connect(render, SIGNAL(mouseZoomChanged(qreal)), inspector, SLOT(setMouseZoom(qreal)));
    connect(render, SIGNAL(selectionChanged()), inspector, SLOT(askRefresh()));
    connect(render, SIGNAL(actionRouteNew()), SLOT(actionNew()));
    connect(render, SIGNAL(actionRouteOpen()), SLOT(actionOpen()));
    connect(render, SIGNAL(actionRouteSave()), SLOT(actionSave()));
    connect(render, SIGNAL(actionRouteSave_as()), SLOT(actionSave_as()));
    connect(render, SIGNAL(actionRouteSave_all()), SLOT(actionSave_all()));
    connect(render, SIGNAL(actionRouteDuplicateScore()), SLOT(actionDuplicateScore()));
    connect(render, SIGNAL(actionRouteRename()), SLOT(actionRename()));
    connect(render, SIGNAL(actionRouteRemove()), SLOT(actionRemove()));
    connect(render, SIGNAL(actionRouteUndo()), SLOT(actionUndo()));
    connect(render, SIGNAL(actionRouteRedo()), SLOT(actionRedo()));
    connect(render, SIGNAL(editingStop()), SLOT(editingStop()));
    connect(render, SIGNAL(editingStart(QPointF)), SLOT(editingStart(QPointF)));
    connect(render, SIGNAL(editingMove(QPointF)), SLOT(editingMove(QPointF)));
    connect(render, SIGNAL(escFullscreen()), view, SLOT(escFullscreen()));
    render->zoom();
    render->loadTexture("background", scriptDir.absoluteFilePath("Tools/background.jpg"), QRectF(QPointF(0, 0), QPointF(0, 0)));
    render->loadTexture("trigger_active", scriptDir.absoluteFilePath("Tools/trigger.png"), QRectF(QPointF(-1, 1), QPointF(1, -1)));
    render->loadTexture("trigger_inactive", scriptDir.absoluteFilePath("Tools/trigger.png"), QRectF(QPointF(-1, 1), QPointF(1, -1)));
    render->loadTexture("trigger_active_message", scriptDir.absoluteFilePath("Tools/trigger_message.png"), QRectF(QPointF(-1, 1), QPointF(1, -1)));
    render->loadTexture("trigger_inactive_message", scriptDir.absoluteFilePath("Tools/trigger_message.png"), QRectF(QPointF(-1, 1), QPointF(1, -1)));

    //Objet transport
    transportObject = new NxTrigger(this, 0, render->getRenderOptions());

    //Editor
    editor = new UiEditor(0);

    //Other connections
    actionViewChange();
    inspector->setRender(render);

    //Other composents
    osc = new ExtOscManager(this);
    connect(inspector, SIGNAL(oscPortChange(quint16)), osc, SLOT(openPort(quint16)));
    connect(osc, SIGNAL(openPortStatus(bool)), inspector, SLOT(setOSCOk(bool)));

    tcp = new ExtTcpManager(this);

    udp = new ExtUdpManager(this);
    connect(inspector, SIGNAL(udpPortChange(quint16)), udp, SLOT(openPort(quint16)));
    connect(udp, SIGNAL(openPortStatus(bool)), inspector, SLOT(setUDPOk(bool)));

    http = new ExtHttpManager(this);

    serial = new ExtSerialManager(this);
    connect(inspector, SIGNAL(serialPortChange(QString)), serial, SLOT(openPort(QString)));
    connect(serial, SIGNAL(openPortStatus(bool)), inspector, SLOT(setSerialOk(bool)));

    midi = new ExtMidiManager(this);

    //File management
    //connect(&fileWatcher, SIGNAL(fileChanged(QString)), SLOT(fileWatcherChanged(QString)));
    connect(&fileWatcher, SIGNAL(directoryChanged(QString)), SLOT(fileWatcherChanged(QString)));

    //About
    about = new UiAbout(0);

    //Scheduler
    actionSpeed();
    timeLocal = 0;
    timeTransportRefresh = 0;
    timePerfRefresh = 0;
    timePerfCounter = 0;
    timer = new QTimer(this);
    timer->setInterval(5);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerEvent()));
    actionFast_rewind();
    actionTabChange(0);

    //Settings
    QSettings settings;
    //UDP
    if((forceSettings) || (!settings.childKeys().contains("oscPort")))
        settings.setValue("oscPort", 1234);
    if((forceSettings) || (!settings.childKeys().contains("udpPort")))
        settings.setValue("udpPort", 1235);
#ifdef Q_OS_MAC
    if((forceSettings) || (!settings.childKeys().contains("serialPort")))
        settings.setValue("serialPort", "/dev/tty.usbserial-A600afc5:BAUD115200:DATA_8:PAR_NONE:STOP_1:FLOW_OFF");
#endif
#ifdef Q_OS_WIN
    if((forceSettings) || (!settings.childKeys().contains("serialPort")))
        settings.setValue("serialPort", "COM1:BAUD115200:DATA_8:PAR_NONE:STOP_1:FLOW_OFF");
#endif
#ifdef Q_OS_LINUX
    if((forceSettings) || (!settings.childKeys().contains("serialPort")))
        settings.setValue("serialPort", "/dev/tty.usbserial-A600afc5:BAUD115200:DATA_8:PAR_NONE:STOP_1:FLOW_OFF");
#endif
    //if((forceSettings) || (!settings.childKeys().contains("édefaultMessageTrigger")))
        settings.setValue("defaultMessageTrigger", "osc://127.0.0.1:57120/trigger trigger_id trigger_xPos trigger_yPos cursor_id");
    //if((forceSettings) || (!settings.childKeys().contains("defaultMessageCursor")))
        settings.setValue("defaultMessageCursor", "osc://127.0.0.1:57120/cursor cursor_id cursor_value_x cursor_value_y");
    //if((forceSettings) || (!settings.childKeys().contains("defaultMessage")))
        settings.setValue("defaultMessage", "osc://127.0.0.1:57120/object trigger_id cursor_id");
    if((forceSettings) || (!settings.childKeys().contains("defaultMessageTransport")))
        settings.setValue("defaultMessageTransport", "osc://127.0.0.1:57120/transport status nb_triggers nb_cursors nb_curves");
    if((forceSettings) || (!settings.childKeys().contains("updatePeriod")))
        settings.setValue("updatePeriod", 1);

    inspector->setOSCPort(settings.value("oscPort").toUInt());
    inspector->setUDPPort(settings.value("udpPort").toUInt());
    inspector->setSerialPort(settings.value("serialPort").toString());
    inspector->setTransportMessage(settings.value("defaultMessageTransport").toString());
    defaultMessageTrigger = settings.value("defaultMessageTrigger").toString();
    defaultMessageCursor = settings.value("defaultMessageCursor").toString();

    bool settingsOk = settings.childKeys().contains("lastUpdate") && settings.childKeys().contains("updatePeriod") && settings.childKeys().contains("id");
    if(settingsOk) {
        QDateTime date = settings.value("lastUpdate").toDateTime();
        quint16 period = settings.value("updatePeriod").toUInt();
        qDebug("Last update : %s (each %d days)", qPrintable(date.toString("dd/MM/yyyy hh:mm:ss")), period);
        if(date.daysTo(QDateTime::currentDateTime()) >= period)
            checkForUpdates();
    }
    else {
        qDebug("First launch. Write settings.");
        srand(time(NULL));
        settings.setValue("id", rand());
        checkForUpdates();
    }


    //Files
    inspector->getProjectFiles()->clear();
    inspector->getProjectScripts()->clear();
    projectScore = new QTreeWidgetItem(inspector->getProjectFiles());
    projectScore->setFlags(Qt::ItemIsEnabled);
    projectScore->setText(0, tr("Current project"));
    projectScript = new QTreeWidgetItem(inspector->getProjectScripts());
    projectScript->setFlags(Qt::ItemIsEnabled);
    projectScript->setText(0, tr("Current project"));
    libScript = new QTreeWidgetItem(inspector->getProjectScripts());
    libScript->setFlags(Qt::ItemIsEnabled);
    libScript->setText(0, tr("Tools"));
    exampleScript = new QTreeWidgetItem(inspector->getProjectScripts());
    exampleScript->setFlags(Qt::ItemIsEnabled);
    exampleScript->setText(0, tr("Examples"));

    //Expand
    inspector->getProjectFiles()->expandAll();
    inspector->getProjectScripts()->expandAll();

    QDir examplesDir("./Examples/");
    QDir libDir("./Tools/");
    fileWatcherFolder(QStringList() << "*.nxscript" << "*.nxstyle", libDir, libScript, false);
    fileWatcherFolder(QStringList() << "*.nxscript" << "*.nxstyle", examplesDir, exampleScript, false);

    //Projet par défault
    loadProject("Project/root.root");
    actionFast_rewind();
}

qreal IanniX::getCpuUsage() {
    qreal cpu = 0;
    return cpu;
}

void IanniX::setScheduler(bool start) {
    if(start) {
        timer->start();
        renderMeasure.start();
    }
    else
        timer->stop();
    transport->setPlay_pause(getScheduler());
}

void IanniX::closeSplash() {
    splash->close();
    show();
}

void IanniX::show() {
    view->show();
    view->activateWindow();
}


void IanniX::timerEvent() {
    qreal delta = renderMeasure.elapsed() / 1000.0F;
    if(forceTimeLocal) {
        delta = 0;
        timeTransportRefresh = 9999;
        setScheduler(false);
    }
    timeLocal += delta * timeFactor;
    if(timeLocal < 0) {
        forceTimeLocal = true;
        timeLocal = 0;
        return;
    }
    timeTransportRefresh += delta;
    timePerfRefresh += delta;
    timePerfCounter++;
    renderMeasure.start();
    getCpuUsage();

    //Browse documents
    QRect local;
    QRect cursorBoundingRectSearch;
    //QHashIterator<QString, NxDocument*> documentIterator(documents);
    /*while (documentIterator.hasNext())*/ {
        //documentIterator.next();
        NxDocument *document = currentDocument; //documentIterator.value();

        //Browse groups
        if(document) {
            QMapIterator<QString, NxGroup*> groupIterator(document->groups);
            while (groupIterator.hasNext()) {
                groupIterator.next();
                NxGroup *group = groupIterator.value();

                //Browse active/inactive objects
                for(quint16 activityIterator = 0 ; activityIterator < ObjectsActivityLenght ; activityIterator++) {

                    //Browse locals cursors
                    QHashIterator< QRect, QHash<quint16, NxObject*> > localIterator(group->objects[activityIterator][ObjectsTypeCursor]);
                    while (localIterator.hasNext()) {
                        localIterator.next();
                        local = localIterator.key();

                        //Browse cursors
                        QHashIterator<quint16, NxObject*> objectIterator(group->objects[activityIterator][ObjectsTypeCursor].value(local));
                        while (objectIterator.hasNext()) {
                            objectIterator.next();
                            NxCursor *cursor = (NxCursor*)objectIterator.value();

                            //Cursor reset
                            if(forceTimeLocal)
                                cursor->setTimeLocal(timeLocal);

                            //Set time for a cursor
                            cursor->setTime(delta * timeFactor);

                            //Is cursor active ?
                            if((!forceTimeLocal) && (cursor->getActive())) {
                                //Messages
                                cursor->trig();

                                //Bounding search
                                cursorBoundingRectSearch = cursor->getBoundingRectSearch();

                                //Browse groups
                                QMapIterator<QString, NxGroup*> groupIteratorTrigger(document->groups);
                                while (groupIteratorTrigger.hasNext()) {
                                    groupIteratorTrigger.next();
                                    NxGroup *group = groupIteratorTrigger.value();

                                    //Browse locals cursors
                                    QHashIterator< QRect, QHash<quint16, NxObject*> > localIterator(group->objects[ObjectsActivityActive][ObjectsTypeTrigger]);
                                    while (localIterator.hasNext()) {
                                        localIterator.next();
                                        local = localIterator.key();

                                        //Is cursor inside ?
                                        if((cursorBoundingRectSearch.contains(QPoint(local.x(), local.y()))) && (cursor->getLocal().width() == local.width())) {

                                            //Browse active triggers
                                            QHashIterator<quint16, NxObject*> objectIteratorTrigger(group->objects[ObjectsActivityActive][ObjectsTypeTrigger].value(local));
                                            while (objectIteratorTrigger.hasNext()) {
                                                objectIteratorTrigger.next();

                                                NxTrigger *trigger = (NxTrigger*)objectIteratorTrigger.value();

                                                //Check the collision
                                                if(cursor->contains(trigger)) {
                                                    trigger->trig(cursor);
                                                }
                                            }
                                        }
                                    }


                                    //Browse locals curves
                                    QHashIterator< QRect, QHash<quint16, NxObject*> > localIterator2(group->objects[ObjectsActivityActive][ObjectsTypeCurve]);
                                    while (localIterator2.hasNext()) {
                                        localIterator2.next();
                                        local = localIterator2.key();

                                        //Is cursor inside ?
                                        if(true/*(cursorBoundingRectSearch.contains(QPoint(local.x(), local.y()))) && (cursor->getLocal().width() == local.width())*/) {

                                            //Browse active triggers
                                            QHashIterator<quint16, NxObject*> objectIteratorCurve(group->objects[ObjectsActivityActive][ObjectsTypeCurve].value(local));
                                            while (objectIteratorCurve.hasNext()) {
                                                objectIteratorCurve.next();

                                                NxCurve *curve = (NxCurve*)objectIteratorCurve.value();

                                                //Check the collision
                                                cursor->trig(curve);
                                            }
                                        }
                                    }

                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if(timeTransportRefresh >= 0.06) {
        timeTransportRefresh = 0;
        QString timeStr = "";

        quint16 min = timeLocal / 60;
        if(min < 10) timeStr += "00";
        else if(min < 100) timeStr += "0";
        timeStr += QString().setNum(min) + ":";

        quint8 sec = (int)floor(timeLocal) % 60;
        if(sec < 10) timeStr += "0";
        timeStr += QString().setNum(sec) + ":";

        quint16 milli = (timeLocal - floor(timeLocal)) * 1000;
        if(milli < 10)       timeStr += "00";
        else if(milli < 100) timeStr += "0";
        timeStr += QString().setNum(milli);

        emit(updateTransport(timeStr, lastMessage));
        lastMessageAllow = true;
    }

    if(timePerfRefresh >= 1) {
        transport->setPerfScheduler(QString().setNum((quint16)qRound(1000.0F*timePerfRefresh/timePerfCounter)));
        timePerfRefresh = 0;
        timePerfCounter = 0;
    }

    if(forceTimeLocal)
        forceTimeLocal = false;
}


void IanniX::checkForUpdates() {
    QString url = "http://www.iannix.org/download/updates.php?id=" + QSettings().value("id").toString() + "&package=" + (QCoreApplication::applicationName() + "__" + QCoreApplication::applicationVersion()).toLower().replace(" ", "_").replace(".", "_");
    qDebug("Checking for updates %s", qPrintable(url));
    updateManager->get(QNetworkRequest(QUrl(url, QUrl::TolerantMode)));
}
void IanniX::checkForUpdatesFinished(QNetworkReply *reply) {
    if(reply->error() != QNetworkReply::NoError) {
        qDebug("Network error. %s", qPrintable(reply->errorString()));
    }
    else {
        QSettings().setValue("lastUpdate", QDateTime::currentDateTime());
        QString info = reply->readAll().trimmed();
        if(info.length() > 0) {
            QMessageBox message;
            message.setWindowTitle(tr("IanniX Update Center"));
            message.setText(tr("An update has been detected on the server."));
            message.setInformativeText(tr("Would you like to update IanniX with the new version ?"));
            message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            message.setDefaultButton(QMessageBox::Yes);
            message.setDetailedText(info);
            int rep = message.exec();
            if(rep == QMessageBox::Yes)
                QDesktopServices::openUrl(QUrl("http://www.iannix.org", QUrl::TolerantMode));
        }
    }
}

void IanniX::actionToggle_Inspector() {
    if(inspector->isVisible())
        inspector->parentWidget()->hide();
    else
        inspector->parentWidget()->show();
    view->activateWindow();;
}
void IanniX::actionToggle_Transport() {
    if(transport->isVisible())
        transport->parentWidget()->hide();
    else
        transport->parentWidget()->show();
    view->activateWindow();
}
void IanniX::actionToggle_Autosize() {
    if(render->getTriggerAutosize())
        render->setTriggerAutosize(false);
    else
        render->setTriggerAutosize(true);
}

void IanniX::actionPlay_pause() {
    if(getScheduler()) {
        setScheduler(false);
        sendMessage(transportObject, 0, 0, 0, QPointF(), QPointF(), "stop");
    }
    else {
        setScheduler(true);
        sendMessage(transportObject, 0, 0, 0, QPointF(), QPointF(), "play");
    }
    view->activateWindow();
}
void IanniX::actionFast_rewind() {
    forceTimeLocal = true;
    timeLocal = 0;
    setScheduler(true);
    sendMessage(transportObject, 0, 0, 0, QPointF(), QPointF(), "fastrewind");
    view->activateWindow();
}

void IanniX::actionLogo() {
    about->show();
}

void IanniX::actionGoto() {
    QStringList transportTime = transport->getTime().split(":", QString::SkipEmptyParts);
    if(transportTime.count() == 2) {
        QStringList transportTime2 = transportTime.at(1).split(".", QString::SkipEmptyParts);
        if(transportTime2.count() == 2) {
            qreal milli = transportTime2.at(1).toUInt();
            qreal sec = transportTime2.at(0).toUInt();
            qreal min = transportTime.at(0).toUInt();
            timeLocal = min * 60 + sec + milli / 1000.F;
            forceTimeLocal = true;
            setScheduler(true);
        }
    }
}

void IanniX::actionSetScheduler() {
    QString valStr = transport->getPerfScheduler();
    quint16 val = valStr.remove("ms").toUInt();
    if(val > 0)
        timer->setInterval(val);
}
void IanniX::actionSetOpenGL() {
    QString valStr = transport->getPerfOpenGL();
    quint16 val = valStr.remove("fps").toUInt();
    if(val > 0)
        render->setInterval(1000.0F/(qreal)val);
}

void IanniX::actionSpeed() {
    timeFactor = transport->getSpeed();
}

void IanniX::actionTabChange(int tab) {
    if(tab == 3)
        oscConsoleActive = true;
    else
        oscConsoleActive = false;
}
void IanniX::actionViewChange() {
    if(inspector->getViewTriggerCheck())
        render->getRenderOptions()->paintTriggers = true;
    else
        render->getRenderOptions()->paintTriggers = false;
    if(inspector->getViewCurveCheck())
        render->getRenderOptions()->paintCurves = true;
    else
        render->getRenderOptions()->paintCurves = false;
    if(inspector->getViewCursorCheck())
        render->getRenderOptions()->paintCursors = true;
    else
        render->getRenderOptions()->paintCursors = false;
    render->getRenderOptions()->paintZStart = inspector->getViewZStart();
    render->getRenderOptions()->paintZEnd = inspector->getViewZEnd();
}
void IanniX::actionCC() {
    QList<NxObject*> objects = inspector->getSelectedCCObject();
    QPointF center;
    quint16 centerCounter = 0;

    render->selectionClear(true);
    foreach(NxObject* object, objects) {
        render->selectionAdd(object);
        center += object->getPos();
        centerCounter++;
    }
    center /= centerCounter;
    render->centerOn(center);
}
void IanniX::actionCC2() {
    QList<NxGroup*> groups = inspector->getSelectedCC2Object();
    QPointF center;
    quint16 centerCounter = 0;

    render->selectionClear(true);
    foreach(NxGroup* group, groups) {
        //Browse active/inactive objects
        for(quint16 activityIterator = 0 ; activityIterator < ObjectsActivityLenght ; activityIterator++) {
            //Browse all types of objects
            for(quint16 typeIterator = 0 ; typeIterator < ObjectsTypeLength ; typeIterator++) {
                //Browse locals cursors
                QHashIterator< QRect, QHash<quint16, NxObject*> > localIterator(group->objects[activityIterator][typeIterator]);
                while (localIterator.hasNext()) {
                    localIterator.next();
                    QRect local = localIterator.key();
                    //Browse objects
                    QHashIterator<quint16, NxObject*> objectIterator(group->objects[activityIterator][typeIterator].value(local));
                    while (objectIterator.hasNext()) {
                        objectIterator.next();
                        NxObject *object = objectIterator.value();
                        render->selectionAdd(object);
                        center += object->getPos();
                        centerCounter++;
                    }
                }
            }
        }
    }
    center /= centerCounter;
    render->centerOn(center);
}


void IanniX::actionNew() {
    if(projectScore) {
        bool ok;
        QString text = QInputDialog::getText(0, tr("New score"), tr("Enter the name of your new score:"), QLineEdit::Normal, "New Document " + QDateTime::currentDateTime().toString("MMddhhmmss"), &ok);
        if((ok) && (text != "")) {
            if(!QFile::exists(scriptDir.absoluteFilePath(text +  ".nxscore"))) {
                QFile newFile(scriptDir.absoluteFilePath(text +  ".nxscore"));
                if(newFile.open(QIODevice::WriteOnly)) {
                    newFile.write("");
                    newFile.close();
                }
            }
            else {
                QMessageBox::information(0, tr("Filename conflict"), tr("The file can't be created! A file with this name exists in your project."), QMessageBox::Ok);
            }
        }
    }
}
void IanniX::actionOpen() {
    QString fileName = QFileDialog::getExistingDirectory(0, tr("Open IanniX Project Folder"), QString("./scores/"));
    if(fileName != "")
        loadProject(fileName + QString(QDir::separator()));
}
void IanniX::actionSave() {
    if(currentDocument) {
        if(!currentDocument->save(render->getRenderOptions()))
            actionSave_as();
    }
}
void IanniX::actionSave_as() {

}
void IanniX::actionRename() {
    if(currentDocument) {
        bool ok;
        QString text = QInputDialog::getText(0, tr("File rename"), tr("Enter the desired name of your score:"), QLineEdit::Normal, currentDocument->getScriptFile().baseName(), &ok);
        if((ok) && (text != ""))
            currentDocument->rename(currentDocument->getScriptFile().absoluteFilePath().replace(currentDocument->getScriptFile().baseName()+".", text+"."));
    }
}
void IanniX::actionRemove() {
    if(currentDocument) {
        int rep = QMessageBox::question(0, tr("File remove"), tr("The file will be removed from your disk. Are you sure?"), QMessageBox::Yes | QMessageBox::No);
        if(rep == QMessageBox::Yes)
            currentDocument->remove();
    }
}
void IanniX::actionDuplicateScore() {
    if(currentDocument) {
        bool ok;
        QString text = QInputDialog::getText(0, tr("File duplication"), tr("Enter the desired name of the duplicated score"), QLineEdit::Normal, currentDocument->getScriptFile().baseName(), &ok);
        if((ok) && (text != "")) {
            currentDocument->setNewFilename(currentDocument->getScriptFile().absoluteFilePath().replace(currentDocument->getScriptFile().baseName()+".", text+"."));
            currentDocument->save(render->getRenderOptions());
        }
    }
}

void IanniX::actionSave_all() {
    QHashIterator<QString, NxDocument*> documentIterator(documents);
    while (documentIterator.hasNext()) {
        documentIterator.next();
        NxDocument *document = documentIterator.value();
        document->save(render->getRenderOptions());
    }
}
void IanniX::actionProjectFiles() {
    if(inspector->getProjectFiles()->currentItem()->type() == 1024) {
        NxDocument *scriptList = (NxDocument*)inspector->getProjectFiles()->currentItem();
        if(scriptList) {
            currentDocument = scriptList;
            render->setDocument(currentDocument);
            view->setWindowTitle(tr("IanniX — ") + currentDocument->getScriptFile().baseName());
            currentDocument->load();
            render->selectionClear(true);
            //actionFast_rewind();
        }
    }
}
void IanniX::actionProjectScripts() {
    if((inspector->getProjectScripts()->currentItem()->type() == 1024) && (currentDocument)) {
        ExtScriptManager *scriptList = (ExtScriptManager*)inspector->getProjectScripts()->currentItem();
        if(scriptList) {
            pushSnapshot();
            scriptList->parseScript();
            if(!activeScripts.contains(scriptList))
                activeScripts.append(scriptList);
            render->selectionClear(true);
            actionFast_rewind();
            editor->openFile(scriptList->getScriptFile());
            view->activateWindow();
        }
    }
}
void IanniX::actionProjectScriptsContext(const QPoint & point) {
    if((inspector->getProjectScripts()->currentItem()->type() == 1024) && (currentDocument)) {
        ExtScriptManager *scriptList = (ExtScriptManager*)inspector->getProjectScripts()->currentItem();
        if(scriptList) {
            QMenu menu(inspector);
            QAction *openScript = menu.addAction(tr("Open in default external editor"));
            QAction *ret = menu.exec(render->mapToGlobal(point));
            if(ret == openScript) {
                QDesktopServices::openUrl(QUrl("file:///" + scriptList->getScriptFile().absoluteFilePath().replace("\\", "/"), QUrl::TolerantMode));
            }
        }
    }
}




NxGroup* IanniX::addGroup(const QString & documentId, const QString & groupId) {
    NxGroup *group = new NxGroup(this, inspector->getViewGroup());
    group->setId(groupId);
    documents.value(documentId)->groups[group->getId()] = group;
    documents.value(documentId)->setCurrentGroup(group);
    return group;
}


void IanniX::setObjectActivity(void *_object, quint8 activeOld) {
    //Extract object
    NxObject *object = (NxObject*)_object;

    //Add a group if not allocated
    if(!documents.value(object->getDocumentId())->groups.contains(object->getGroupId()))
        addGroup(object->getDocumentId(), object->getGroupId());

    //Move object
    documents.value(object->getDocumentId())->groups.value(object->getGroupId())->objects[activeOld][object->getType()][object->getLocal()].remove(object->getId());
    documents.value(object->getDocumentId())->groups.value(object->getGroupId())->objects[object->getActive()][object->getType()][object->getLocal()].insert(object->getId(), object);
}
void IanniX::setObjectLocal(void *_object, QRect localOld) {
    //Extract object
    NxObject *object = (NxObject*)_object;

    //Add a group if not allocated
    if(!documents.value(object->getDocumentId())->groups.contains(object->getGroupId()))
        addGroup(object->getDocumentId(), object->getGroupId());

    //Move object
    documents.value(object->getDocumentId())->groups.value(object->getGroupId())->objects[object->getActive()][object->getType()][localOld].remove(object->getId());
    documents.value(object->getDocumentId())->groups.value(object->getGroupId())->objects[object->getActive()][object->getType()][object->getLocal()].insert(object->getId(), object);
}
void IanniX::setObjectGroupId(void *_object, const QString & groupIdOld) {
    //Extract object
    NxObject *object = (NxObject*)_object;

    //Add a group if not allocated
    if(!documents.value(object->getDocumentId())->groups.contains(object->getGroupId()))
        addGroup(object->getDocumentId(), object->getGroupId());

    //Move object
    documents.value(object->getDocumentId())->groups.value(object->getGroupId())->objects[object->getActive()][object->getType()][object->getLocal()].insert(object->getId(), object);

    //Remove a group if empty
    if(documents.value(object->getDocumentId())->groups.contains(groupIdOld)) {
        documents.value(object->getDocumentId())->groups.value(groupIdOld)->objects[object->getActive()][object->getType()][object->getLocal()].remove(object->getId());
        if(documents.value(object->getDocumentId())->groups.value(groupIdOld)->getCount() == 0) {
            NxGroup *group = documents.value(object->getDocumentId())->groups.value(groupIdOld);
            documents.value(object->getDocumentId())->groups.remove(groupIdOld);
            delete group;
        }
    }
}

void IanniX::removeObject(NxObject *object) {
    if(object) {
        if(object->getType() == ObjectsTypeCurve) {
            NxCurve *curve = (NxCurve*)object;
            QStringList commands;
            foreach(NxObject *object, curve->getCursors())
                commands << COMMAND_REMOVE + " " + QString().setNum(object->getId()) + COMMAND_END;
            foreach(const QString & command, commands) {
                execute(command);
            }
        }
        else if(object->getType() == ObjectsTypeCursor) {
            NxCurve *curve = ((NxCursor*)object)->getCurve();
            if(curve)
                curve->removeCursor(object);
        }

        //Remove the object
        documents.value(object->getDocumentId())->groups.value(object->getGroupId())->objects[object->getActive()][object->getType()][object->getLocal()].remove(object->getId());
        documents.value(object->getDocumentId())->objects.remove(object->getId());

        //Remove groups
        if(documents.value(object->getDocumentId())->groups.value(object->getGroupId())->getCount() == 0) {
            NxGroup *group = documents.value(object->getDocumentId())->groups.value(object->getGroupId());
            documents.value(object->getDocumentId())->groups.remove(object->getGroupId());
            delete group;
        }
        render->selectionClear(true);

        delete object;
    }
}


const QVariant IanniX::execute(const QString & command, bool createNewObjectIfExists) {
    QStringList arguments = command.split(" ", QString::SkipEmptyParts);

    if((arguments.count() > 0) && (currentDocument)) {
        QString commande = arguments.at(0).toLower();
        if((commande == COMMAND_ADD) && (arguments.count() >= 3)) {
            bool ok = false;
            qint16 id = arguments.at(2).toUInt(&ok);
            NxObject *parentObject = 0;
            if(ok) {
                parentObject = currentDocument->getObject(id);
                if(parentObject) {
                    if(createNewObjectIfExists) {
                        id = currentDocument->nextAvailableId();
                    }
                    else {
                        id = parentObject->getId();
                        removeObject(parentObject);
                        parentObject = 0;
                    }
                }
            }
            else
                id = currentDocument->nextAvailableId();

            NxObject *object = 0;
            QString type = arguments.at(1).toLower();
            if(type == "curve")
                object = new NxCurve(this, inspector->getCurveItem(), render->getRenderOptions());
            else if(type == "cursor")
                object = new NxCursor(this, inspector->getCursorItem(), render->getRenderOptions());
            else
                object = new NxTrigger(this, inspector->getTriggerItem(), render->getRenderOptions());

            if(object) {
                object->dispatchProperty("documentId", currentDocument->getId());
                object->dispatchProperty("groupId", "");
                object->dispatchProperty("id", id);
                object->dispatchProperty("active", ObjectsActivityActive);
                object->dispatchProperty("pos", QPointF(0, 0));
                object->dispatchProperty("z", 0);
                object->setParentObject(parentObject);

                if(parentObject) {
                    QPointF posOffset(0.5, -0.5);
                    /*
                    if(parentObject->getParentObject())
                        posOffset = parentObject->getPos() - parentObject->getParentObject()->getPos();
                    */
                    object->setPosOffset(posOffset);
                }
                currentDocument->objects[id] = object;
                currentDocument->setCurrentObject(object);

                return object->getId();
            }
            else
                return 0;
        }
        else if((commande == COMMAND_TEXTURE) && (arguments.count() >= 7)) {
            QString filename = command.mid(command.indexOf(arguments.at(6), command.indexOf(arguments.at(5))+arguments.at(5).length())).trimmed();
            if(!QFile().exists(filename))
                filename = scriptDir.absoluteFilePath(filename);
            render->loadTexture(arguments.at(1).trimmed(), filename, QRectF(QPointF(arguments.at(2).toDouble(), arguments.at(3).toDouble()), QPointF(arguments.at(4).toDouble(), arguments.at(5).toDouble())));
        }
        else if((commande == COMMAND_GLOBAL_COLOR) && (arguments.count() >= 6)) {
            render->getRenderOptions()->colors[arguments.at(1)] = QColor(arguments.at(2).toDouble(), arguments.at(3).toDouble(), arguments.at(4).toDouble(), arguments.at(5).toDouble());
        }
        else if((commande == COMMAND_GLOBAL_COLOR2) && (arguments.count() >= 6)) {
            QColor color;
            color.setHsv(arguments.at(2).toDouble(), arguments.at(3).toDouble(), arguments.at(4).toDouble(), arguments.at(5).toDouble());
            render->getRenderOptions()->colors[arguments.at(1)] = color;
        }
        else if((commande == COMMAND_ZOOM) && (arguments.count() >= 2)) {
            render->zoom(arguments.at(1).toDouble());
        }
        else if((commande == COMMAND_CENTER) && (arguments.count() >= 3)) {
            render->getRenderOptions()->axisCenter = QPointF(-arguments.at(1).toDouble(), -arguments.at(2).toDouble());
            render->zoom();
        }
        else if((commande == COMMAND_PLAY) && (arguments.count() >= 2)) {
            if(arguments.at(1).toDouble() != 0) {
                transport->setPlay_pause(true);
                transport->setSpeed(arguments.at(1).toDouble());
                setScheduler(true);
                sendMessage(transportObject, 0, 0, 0, QPointF(), QPointF(), "play");
            }
            else {
                transport->setPlay_pause(false);
                setScheduler(false);
                sendMessage(transportObject, 0, 0, 0, QPointF(), QPointF(), "stop");
            }
            actionSpeed();
        }
        else if((commande == COMMAND_STOP) && (arguments.count() >= 1)) {
            transport->setPlay_pause(false);
            setScheduler(false);
            sendMessage(transportObject, 0, 0, 0, QPointF(), QPointF(), "stop");
            actionSpeed();
        }
        else if((commande == COMMAND_FF) && (arguments.count() >= 1)) {
            actionFast_rewind();
        }
        else if((commande == COMMAND_LOG) && (arguments.count() >= 2)) {
            lastMessage = command.mid(command.indexOf(arguments.at(1), command.indexOf(arguments.at(0))+1)).trimmed();
        }
        else if((commande == COMMAND_CLEAR) && (arguments.count() >= 1)) {
            if(currentDocument) {
                pushSnapshot();
                currentDocument->clear();
            }
        }
        else if((commande == COMMAND_SNAP_POP) && (arguments.count() >= 2)) {
            if(currentDocument) {
                render->selectionClear(true);
                currentDocument->popSnapshot(command.mid(command.indexOf(arguments.at(1), command.indexOf(arguments.at(0))+arguments.at(0).length())).trimmed());
            }
        }
        else if((commande == COMMAND_SNAP_PUSH) && (arguments.count() >= 2)) {
            if(currentDocument)
                currentDocument->pushSnapshot(command.mid(command.indexOf(arguments.at(1), command.indexOf(arguments.at(0))+arguments.at(0).length())).trimmed());
        }
        else if((commande == COMMAND_SNAP_POP) && (arguments.count() >= 1)) {
            if(currentDocument) {
                render->selectionClear(true);
                currentDocument->popSnapshot();
            }
        }
        else if((commande == COMMAND_SNAP_PUSH) && (arguments.count() >= 1)) {
            pushSnapshot();
        }
        else if((commande == COMMAND_SPEED) && (arguments.count() >= 2)) {
            transport->setSpeed(arguments.at(1).toDouble());
        }
        else if((commande == COMMAND_MESSAGE_SEND) && (arguments.count() >= 4)) {

        }
        else if((commande == COMMAND_AUTOSIZE) && (arguments.count() >= 2)) {
            render->setTriggerAutosize(arguments.at(1).toDouble());
        }

        else if(arguments.count() >= 2) {
            NxObjectDispatchProperty *object = getObject(qPrintable(arguments.at(1)));

            if(object) {
                if((commande == COMMAND_REMOVE) && (arguments.count() >= 2)) {
                    if((object->getType() == ObjectsTypeCursor) || (object->getType() == ObjectsTypeCurve) || (object->getType() == ObjectsTypeTrigger)) {
                        NxObject *objectObj = (NxObject*)object;
                        removeObject(objectObj);
                    }
                    return 0;
                }
                else if((commande == COMMAND_GROUP) && (arguments.count() >= 3)) {
                    if((object->getType() == ObjectsTypeCursor) || (object->getType() == ObjectsTypeCurve) || (object->getType() == ObjectsTypeTrigger)) {
                        NxObject *objectObj = (NxObject*)object;
                        QString groupId = arguments.at(2);
                        if(groupId == "")
                            groupId = documents[objectObj->getDocumentId()]->getCurrentGroup()->getId();
                        objectObj->dispatchProperty("groupId", groupId);
                    }
                }

                else if((commande == COMMAND_CURSOR_CURVE) && (arguments.count() >= 2)) {
                    if(object->getType() == ObjectsTypeCursor) {
                        NxCursor *cursor = (NxCursor*)object;
                        NxObject *object2 = (NxObject*)getObject(qPrintable(arguments.at(2)), false);
                        if((object2) && (object2->getType() == ObjectsTypeCurve)) {
                            NxCurve *curve = (NxCurve*)object2;
                            cursor->setCurve(curve);
                            cursor->calculate();
                            return curve->getPathLength();
                        }
                    }
                }

                else if((commande == COMMAND_LABEL) && (arguments.count() >= 3)) {
                    object->dispatchProperty("label", command.mid(command.indexOf(arguments.at(2), command.indexOf(arguments.at(1))+arguments.at(1).length())).trimmed());
                }

                else if((commande == COMMAND_MESSAGE_SEND) && (arguments.count() >= 2)) {
                    object->dispatchProperty("sendosc", 0);
                }

                else if((commande == COMMAND_RESIZE) && (arguments.count() >= 4)) {
                    object->dispatchProperty("resize", QSizeF(arguments.at(2).toDouble(), arguments.at(3).toDouble()));
                }
                else if((commande == COMMAND_RESIZEF) && (arguments.count() >= 3)) {
                    object->dispatchProperty("resizeF", arguments.at(2).toDouble());
                }

                else if((commande == COMMAND_CURSOR_WIDTH) && (arguments.count() >= 3)) {
                    object->dispatchProperty("cursorWidth", arguments.at(2).toDouble());
                }

                else if((commande == COMMAND_LINE) && (arguments.count() >= 4)) {
                    object->dispatchProperty("lineStipple", arguments.at(2).toDouble());
                    object->dispatchProperty("lineFactor", arguments.at(3).toDouble());
                }
                else if((commande == COMMAND_SIZE) && (arguments.count() >= 3)) {
                    object->dispatchProperty("size", arguments.at(2).toDouble());
                }

                else if((commande == COMMAND_CURSOR_START) && (arguments.count() >= 5)) {
                    object->dispatchProperty("easingStart", arguments.at(2).toDouble());
                    object->dispatchProperty("easingStartDuration", arguments.at(3).toDouble());
                    object->dispatchProperty("start", command.mid(command.indexOf(arguments.at(4), command.indexOf(arguments.at(3))+arguments.at(3).length())).trimmed());
                }

                else if((commande == COMMAND_CURSOR_SPEED) && (arguments.count() >= 4)) {
                    if(arguments.at(2) == "auto")
                        object->dispatchProperty("timeFactorAuto", arguments.at(3).toDouble());
                }
                else if((commande == COMMAND_CURSOR_SPEED) && (arguments.count() >= 3)) {
                    object->dispatchProperty("timeFactor", arguments.at(2).toDouble());
                }
                else if((commande == COMMAND_CURSOR_SPEEDF) && (arguments.count() >= 3)) {
                    object->dispatchProperty("timeFactorF", arguments.at(2).toDouble());
                }
                else if((commande == COMMAND_CURSOR_OFFSET) && (arguments.count() >= 5)) {
                    object->dispatchProperty("timeInitialOffset", arguments.at(2).toDouble());
                    object->dispatchProperty("timeStartOffset", arguments.at(3).toDouble());
                    if(arguments.at(4).toLower() == "end")
                        object->dispatchProperty("timeEndOffset", -1);
                    else
                        object->dispatchProperty("timeEndOffset", arguments.at(4).toDouble());
                }
                else if((commande == COMMAND_CURSOR_BOUNDS_SOURCE) && (arguments.count() >= 6)) {
                    object->dispatchProperty("boundsSource", command.mid(command.indexOf(arguments.at(2), command.indexOf(arguments.at(1))+arguments.at(1).length())).trimmed());
                }
                else if((commande == COMMAND_CURSOR_BOUNDS_TARGET) && (arguments.count() >= 6)) {
                    object->dispatchProperty("boundsTarget", command.mid(command.indexOf(arguments.at(2), command.indexOf(arguments.at(1))+arguments.at(1).length())).trimmed());
                }
                else if((commande == COMMAND_CURSOR_TIME) && (arguments.count() >= 3)) {
                    object->dispatchProperty("timeLocal", arguments.at(2).toDouble());
                }



                else if((commande == COMMAND_CURVE_POINT) && (arguments.count() >= 5)) {
                    if(object->getType() == ObjectsTypeCurve) {
                        NxCurve *curve = (NxCurve*)object;
                        if(arguments.count() >= 9)
                            curve->setPointAt(arguments.at(2).toDouble(), QPointF(arguments.at(3).toDouble(), arguments.at(4).toDouble()), QPointF(arguments.at(5).toDouble(), arguments.at(6).toDouble()), QPointF(arguments.at(7).toDouble(), arguments.at(8).toDouble()));
                        else if(arguments.count() >= 7)
                            curve->setPointAt(arguments.at(2).toDouble(), QPointF(arguments.at(3).toDouble(), arguments.at(4).toDouble()), QPointF(arguments.at(5).toDouble(), arguments.at(6).toDouble()));
                        else
                            curve->setPointAt(arguments.at(2).toDouble(), QPointF(arguments.at(3).toDouble(), arguments.at(4).toDouble()));
                        return curve->getPathLength();
                    }
                }

                else if((commande == COMMAND_CURVE_TXT) && (arguments.count() >= 5)) {
                    if(object->getType() == ObjectsTypeCurve) {
                        NxCurve *curve = (NxCurve*)object;
                        curve->setText(arguments.at(4), arguments[3].replace("_", " ").trimmed());
                        curve->setResizeF(arguments.at(2).toDouble());
                        return curve->getPathLength();
                    }
                }
                else if((commande == COMMAND_CURVE_SVG) && (arguments.count() == 4)) {
                    if(object->getType() == ObjectsTypeCurve) {
                        NxCurve *curve = (NxCurve*)object;
                        curve->setSVG(command.mid(command.indexOf(arguments.at(3), command.indexOf(arguments.at(2))+arguments.at(2).length())).trimmed());
                        curve->setResizeF(arguments.at(2).toDouble());
                        return curve->getPathLength();
                    }
                }
                else if((commande == COMMAND_CURVE_SVG2) && (arguments.count() >= 4)) {
                    if(object->getType() == ObjectsTypeCurve) {
                        NxCurve *curve = (NxCurve*)object;
                        curve->setSVG2(command.mid(command.indexOf(arguments.at(3), command.indexOf(arguments.at(2))+arguments.at(2).length())).trimmed());
                        curve->setResizeF(arguments.at(2).toDouble());
                        return curve->getPathLength();
                    }
                }
                else if((commande == COMMAND_CURVE_IMG) && (arguments.count() >= 4)) {
                    if(object->getType() == ObjectsTypeCurve) {
                        NxCurve *curve = (NxCurve*)object;
                        curve->setImage(command.mid(command.indexOf(arguments.at(3), command.indexOf(arguments.at(2))+arguments.at(2).length())).trimmed());
                        curve->setResizeF(arguments.at(2).toDouble());
                        return curve->getPathLength();
                    }
                }

                //Compatibility
                else if((commande == COMMAND_CURVE_SVG) && (arguments.count() >= 5)) {
                    if(object->getType() == ObjectsTypeCurve) {
                        NxCurve *curve = (NxCurve*)object;
                        curve->setSVG(command.mid(command.indexOf(arguments.at(4), command.indexOf(arguments.at(3))+arguments.at(2).length())).trimmed());
                        curve->setResize(QSizeF(arguments.at(2).toDouble(), arguments.at(3).toDouble()));
                        return curve->getPathLength();
                    }
                }

                else if((commande == COMMAND_CURVE_ELL) && (arguments.count() >= 4)) {
                    if(object->getType() == ObjectsTypeCurve) {
                        NxCurve *curve = (NxCurve*)object;
                        curve->setEllipse(QSizeF(arguments.at(2).toDouble(), arguments.at(3).toDouble()));
                        return curve->getPathLength();
                    }
                }

                else if((commande == COMMAND_POS) && (arguments.count() >= 5)) {
                    object->dispatchProperty("pos", QPointF(arguments.at(2).toDouble(), arguments.at(3).toDouble()));
                    object->dispatchProperty("z", arguments.at(4).toDouble());
                }
                else if((commande == COMMAND_ACTIVE) && (arguments.count() >= 3)) {
                    object->dispatchProperty("active", arguments.at(2).toDouble());
                }

                else if((commande == COMMAND_COLOR_ACTIVE) && (arguments.count() >= 3)) {
                    object->dispatchProperty("colorActive", command.mid(command.indexOf(arguments.at(2), command.indexOf(arguments.at(1))+arguments.at(1).length())).trimmed());
                }
                else if((commande == COMMAND_COLOR_INACTIVE) && (arguments.count() >= 3)) {
                    object->dispatchProperty("colorInactive", command.mid(command.indexOf(arguments.at(2), command.indexOf(arguments.at(1))+arguments.at(1).length())).trimmed());
                }
                else if((commande == COMMAND_COLOR_ACTIVE_MESSAGE) && (arguments.count() >= 3)) {
                    object->dispatchProperty("colorActiveMessage", command.mid(command.indexOf(arguments.at(2), command.indexOf(arguments.at(1))+arguments.at(1).length())).trimmed());
                }
                else if((commande == COMMAND_COLOR_INACTIVE_MESSAGE) && (arguments.count() >= 3)) {
                    object->dispatchProperty("colorInactiveMessage", command.mid(command.indexOf(arguments.at(2), command.indexOf(arguments.at(1))+arguments.at(1).length())).trimmed());
                }

                else if((commande == COMMAND_COLOR_ACTIVE2) && (arguments.count() >= 3)) {
                    object->dispatchProperty("colorActive2", command.mid(command.indexOf(arguments.at(2), command.indexOf(arguments.at(1))+arguments.at(1).length())).trimmed());
                }
                else if((commande == COMMAND_COLOR_INACTIVE2) && (arguments.count() >= 3)) {
                    object->dispatchProperty("colorInactive2", command.mid(command.indexOf(arguments.at(2), command.indexOf(arguments.at(1))+arguments.at(1).length())).trimmed());
                }
                else if((commande == COMMAND_COLOR_ACTIVE_MESSAGE2) && (arguments.count() >= 3)) {
                    object->dispatchProperty("colorActiveMessage2", command.mid(command.indexOf(arguments.at(2), command.indexOf(arguments.at(1))+arguments.at(1).length())).trimmed());
                }
                else if((commande == COMMAND_COLOR_INACTIVE_MESSAGE) && (arguments.count() >= 3)) {
                    object->dispatchProperty("colorInactiveMessage2", command.mid(command.indexOf(arguments.at(2), command.indexOf(arguments.at(1))+arguments.at(1).length())).trimmed());
                }

                else if((commande == COMMAND_MESSAGE) && (arguments.count() >= 3)) {
                    object->dispatchProperty("messagePatterns", command.mid(command.indexOf(arguments.at(2), command.indexOf(arguments.at(1))+arguments.at(1).length())).trimmed());
                }
                else if((commande == COMMAND_TRIGGER_OFF) && (arguments.count() >= 3)) {
                    object->dispatchProperty("triggerOff", arguments.at(2));
                }

                else if((commande == COMMAND_TEXTURE_ACTIVE) && (arguments.count() >= 3)) {
                    object->dispatchProperty("textureActive", arguments.at(2));
                }
                else if((commande == COMMAND_TEXTURE_INACTIVE) && (arguments.count() >= 3)) {
                    object->dispatchProperty("textureInactive", arguments.at(2));
                }
                else if((commande == COMMAND_TEXTURE_ACTIVE_MESSAGE) && (arguments.count() >= 3)) {
                    object->dispatchProperty("textureActiveMessage", arguments.at(2));
                }
                else if((commande == COMMAND_TEXTURE_INACTIVE_MESSAGE) && (arguments.count() >= 3)) {
                    object->dispatchProperty("textureInactiveMessage", arguments.at(2));
                }

                return object->getProperty("id").toDouble();
            }
        }
    }
    return 0;
}

void IanniX::sendMessage(void *_object, void *_trigger, void *_cursor, void *_collisionCurve, const QPointF & collisionPoint, const QPointF & collisionValue, const QString & status) {
    NxObject *object = (NxObject*)_object;
    if(object) {
        NxTrigger *trigger = (NxTrigger*)_trigger;
        NxCursor *cursor = (NxCursor*)_cursor;
        NxCurve* collisionCurve = (NxCurve*)_collisionCurve;
        NxCurve *curve = 0;
        if(cursor)
            curve = cursor->getCurve();

        QStringList sentMessages;
        foreach(const QVector<QByteArray> & messagePattern, object->getMessagePatterns()) {
            if(messagesCache.contains(messagePattern.at(0)))
                message = messagesCache.value(messagePattern.at(0));
            else {
                message.setUrl(QUrl(messagePattern.at(0), QUrl::TolerantMode));
                messagesCache.insert(messagePattern.at(0), message);
            }
            if(message.parse(messagePattern, trigger, cursor, curve, collisionCurve, collisionPoint, collisionValue, status, inspector->nbTriggers, inspector->nbCursors, inspector->nbCurves)) {
                if(message.getType() == MessagesTypeDirect)
                    send(message);
                else if((message.getType() == MessagesTypeOsc) && (osc))
                    osc->send(message);
                else if((message.getType() == MessagesTypeHttp) && (http))
                    http->send(message);
                else if((message.getType() == MessagesTypeTcp) && (tcp))
                    tcp->send(message);
                else if((message.getType() == MessagesTypeUdp) && (udp))
                    udp->send(message);
                else if((message.getType() == MessagesTypeSerial) && (serial))
                    serial->send(message);
                else if((message.getType() == MessagesTypeMidi) && (midi))
                    midi->send(message);

                if(object->getSelectedHover())
                    sentMessages << message.getVerboseMessage();
            }
        }
        if((object->getSelectedHover()) && (sentMessages.count() > 0))
            object->setMessageLabel(sentMessages);
    }
}
void IanniX::send(const ExtMessage & message) {
    //Launch
    execute(message.getAsciiMessage());

    //Log in the OSC console
    logOscReceive(message.getVerboseMessage());
}

void IanniX::onOscReceive(const QString & message) {
    foreach(ExtScriptManager *document, activeScripts)
        document->onOSCCall(message);
}
void IanniX::onDraw() {
    foreach(ExtScriptManager *document, activeScripts)
        document->onDrawCall(timeLocal);
}

void IanniX::askNxObject() {
    inspector->actionMessages();
}
void IanniX::actionDrawFreeCurve() {
    if((render->getEditingMode() == EditingModeFree) && (render->getEditing()))
        editingStop();
    else {
        view->unToogleDraw(2);
        view->unToogleDraw(3);
        view->unToogleDraw(4);
        render->setEditingMode(EditingModeFree);
    }
}
void IanniX::actionDrawPointCurve() {
    if((render->getEditingMode() == EditingModePoint) && (render->getEditing()))
        editingStop();
    else {
        view->unToogleDraw(1);
        view->unToogleDraw(3);
        view->unToogleDraw(4);
        render->setEditingMode(EditingModePoint);
    }
}
void IanniX::actionDrawTriggers() {
    if((render->getEditingMode() == EditingModeTriggers) && (render->getEditing()))
        editingStop();
    else {
        view->unToogleDraw(1);
        view->unToogleDraw(2);
        view->unToogleDraw(4);
        render->setEditingMode(EditingModeTriggers);
    }
}
void IanniX::actionAddFreeCursor() {
    bool freeCursor = true;
    view->unToogleDraw(1);
    view->unToogleDraw(2);
    view->unToogleDraw(3);
    view->unToogleDraw(4);
    editingStop();
    pushSnapshot();
    foreach(NxObject *object, *(render->getSelection())) {
        if(object->getType() == ObjectsTypeCurve) {
            freeCursor = false;
            NxCurve *curve = (NxCurve*)object;
            quint16 cursorId = execute(QString("add cursor auto")).toUInt();
            execute(QString("setCurve %1 %2").arg(cursorId).arg(curve->getId()));
            execute(QString("setMessage %1 20, %2").arg(cursorId).arg(defaultMessageCursor));
            execute(QString("setPattern %1 0 0 1").arg(cursorId));
            execute(QString("setOffset %1 %2 0 end").arg(cursorId).arg(curve->getMaxOffset() / 2));
        }
    }
    if(freeCursor) {
        quint16 cursorId = execute(QString("add cursor auto")).toUInt();
        execute(QString("setMessage %1 20, %2").arg(cursorId).arg(defaultMessageCursor));
        execute(QString("setPattern %1 0 0 1").arg(cursorId));
    }
}
void IanniX::actionCircleCurve() {
    if((render->getEditingMode() == EditingModeCircle) && (render->getEditing()))
        editingStop();
    else {
        view->unToogleDraw(1);
        view->unToogleDraw(2);
        view->unToogleDraw(3);
        render->setEditingMode(EditingModeCircle);
    }
}

void IanniX::editingStart(const QPointF & point) {
    if(render->getEditing()) {
        if((render->getEditingMode() == EditingModeFree) || (render->getEditingMode() == EditingModePoint)) {
            editingStartPoint = point;
            freehandCurveIndex = 1;
            pushSnapshot();
            freehandCurveId = execute("add curve auto").toUInt();
            execute(QString("setPos %1 %2 %3 0").arg(freehandCurveId).arg(editingStartPoint.x()).arg(editingStartPoint.y()));
        }
        else if(render->getEditingMode() == EditingModeTriggers) {
            pushSnapshot();
            quint16 triggerId = execute("add trigger auto").toUInt();
            execute(QString("setPos %1 %2 %3 0").arg(triggerId).arg(point.x()).arg(point.y()));
            execute(QString("setMessage %1 1, %2").arg(triggerId).arg(defaultMessageTrigger));
        }
        else if(render->getEditingMode() == EditingModeCircle) {
            pushSnapshot();
            quint16 curveId = execute(QString("add curve auto")).toUInt();
            execute(QString("setPos %1 %2 %3 0").arg(curveId).arg(point.x()).arg(point.y()));
            execute(QString("setPointsEllipse %1 2 2").arg(curveId));
            quint16 cursorId = execute(QString("add cursor auto")).toUInt();
            execute(QString("setCurve %1 %2").arg(cursorId).arg(curveId));
            execute(QString("setMessage %1 20, %2").arg(cursorId).arg(defaultMessageCursor));
        }
    }
}
void IanniX::editingStop() {
    if((render->getEditing()) && ((render->getEditingMode() == EditingModeFree) || (render->getEditingMode() == EditingModePoint))) {
        if(freehandCurveIndex > 1) {
            quint16 cursorId = execute(QString("add cursor auto")).toUInt();
            execute(QString("setCurve %1 %2").arg(cursorId).arg(freehandCurveId));
            execute(QString("setMessage %1 20, %2").arg(cursorId).arg(defaultMessageCursor));
            execute(QString("setPattern %1 0 0 1").arg(cursorId));
        }
        else
            execute(QString("remove %1").arg(freehandCurveId));
    }
    view->unToogleDraw(0);
    render->unsetEditing();
}
void IanniX::editingMove(const QPointF & point) {
    if((render->getEditing()) && ((render->getEditingMode() == EditingModeFree) || (render->getEditingMode() == EditingModePoint))) {
        QPointF newPoint = point - editingStartPoint;
        execute(QString("setPointAt %1 %2 %3 %4").arg(freehandCurveId).arg(freehandCurveIndex++).arg(newPoint.x()).arg(newPoint.y()));
    }
}
void IanniX::actionGridChange(qreal val) {
    render->getRenderOptions()->axisGrid = val;
}
void IanniX::actionGridOpacityChange(qreal val) {
    render->getRenderOptions()->colors["grid"] = QColor(255*val, 255*val, 255*val, 255);
    render->getRenderOptions()->colors["axis"] = render->getRenderOptions()->colors["grid"].lighter(150);
}
void IanniX::actionSnapGrid() {
    render->actionSnapGrid();
}
void IanniX::transportMessageChange(const QString & message) {
    transportObject->setMessagePatterns("1," + message);
}

void IanniX::actionCloseEvent(QCloseEvent *event) {
    quint16 nbFileNoSave = 0;
    QHashIterator<QString, NxDocument*> documentIterator(documents);
    while (documentIterator.hasNext()) {
        documentIterator.next();
        NxDocument *document = documentIterator.value();
        if(document->getHasChanged())
            nbFileNoSave++;
    }

    if(nbFileNoSave > 0) {
        QString ess = " has";
        if(nbFileNoSave > 1)
            ess = "s have";
        int rep = QMessageBox::question(0, tr("Score save"), QString().setNum(nbFileNoSave) + tr(" document") + ess + tr(" been changed without saving.\n\nDo you want to save modifications?"), QMessageBox::SaveAll | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
        if(rep == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        else if(rep == QMessageBox::SaveAll)
            actionSave_all();
    }

    if(editor)
        editor->close();
    if(about)
        about->close();

    QSettings settings;
    settings.setValue("oscPort", inspector->getOSCPort());
    settings.setValue("udpPort", inspector->getUDPPort());
    settings.setValue("serialPort", inspector->getSerialPort());
    settings.setValue("defaultMessageTransport", inspector->getTransportMessage());

    event->accept();
}


void IanniX::actionUndo() {
    render->selectionClear(true);
    currentDocument->popSnapshot();
}
void IanniX::actionRedo() {
    render->selectionClear(true);
    currentDocument->popSnapshot(true);
}

void IanniX::loadProject(const QString & projectFile) {
    if(fileWatcher.directories().contains(scriptDir.path()))
        fileWatcher.removePath(scriptDir.path());
    scriptDir = QFileInfo(projectFile).absoluteDir();
    fileWatcher.addPath(scriptDir.path() + QString(QDir::separator()));

    //Clear
    render->setDocument(0);
    activeScripts.clear();

    //projectScore->setText(0, scriptDir.dirName());
    //projectScript->setText(0, scriptDir.dirName());

    //Files
    fileWatcherChanged(scriptDir.path());
    if(projectScore->childCount() == 0)
        actionNew();
    else {
        inspector->getProjectFiles()->setCurrentItem(projectScore->child(0));
        actionProjectFiles();
    }
}
void IanniX::fileWatcherChanged(const QString & path) {
    QDir fileDir(path);

    //Scores
    fileWatcherFolder(QStringList() << "*.nxscore", fileDir, projectScore, true);
    fileWatcherFolder(QStringList() << "*.nxscript" << "*.nxstyle", fileDir, projectScript, false);

    if(currentDocument) {
        for(quint16 indexChild = 0 ; indexChild < projectScore->childCount() ; indexChild++) {
            ExtScriptManager *document = (ExtScriptManager*)projectScore->child(indexChild);
            if(document->getScriptFile() == currentDocument->getScriptFile())
                inspector->getProjectFiles()->setCurrentItem(document);
        }
        view->setWindowTitle(tr("IanniX — ") + currentDocument->getScriptFile().baseName());
    }
}

void IanniX::fileWatcherFolder(QStringList extension, QDir dir, QTreeWidgetItem *parentList, bool isDocument) {
    QFileInfoList fileList = dir.entryInfoList(extension, QDir::Files | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
    QList<ExtScriptManager*> removeList;
    for(quint16 indexChild = 0 ; indexChild < parentList->childCount() ; indexChild++)
        if(parentList->child(indexChild)->type() == 1024)
            removeList.append((ExtScriptManager*)parentList->child(indexChild));
    foreach(const QFileInfo & file, fileList) {
        bool create = true;

        for(quint16 indexChild = 0 ; indexChild < parentList->childCount() ; indexChild++) {
            if(parentList->child(indexChild)->type() == 1024) {
                ExtScriptManager *document = (ExtScriptManager*)parentList->child(indexChild);
                if(document->getScriptFile() == file) {
                    document->updateTitle();
                    create = false;
                    removeList.removeOne(document);
                }
            }
        }
        if(create) {
            if(isDocument) {
                NxDocument *document = new NxDocument(this, file, parentList);
                documents[document->getId()] = document;
            }
            else
                new ExtScriptManager(this, file, parentList);
        }
    }
    foreach(ExtScriptManager *document, removeList) {
        QString destroyKey = "";
        QHashIterator<QString, NxDocument*> documentIterator(documents);
        while (documentIterator.hasNext()) {
            documentIterator.next();
            if(documentIterator.value() == document)
                destroyKey = documentIterator.key();
        }
        if(destroyKey != "")
            documents.remove(destroyKey);
        parentList->removeChild(document);
    }
    parentList->sortChildren(0, Qt::AscendingOrder);
}

void IanniX::logOscSend(const QString & message) {
    if(lastMessageAllow) {
        lastMessage = message;
        lastMessageAllow = false;
    }
    if(oscConsoleActive)
        inspector->logOscSend(message);
}
void IanniX::logOscReceive(const QString & message) {
    if(oscConsoleActive)
        inspector->logOscReceive(message);
}
void IanniX::pushSnapshot() {
    if(currentDocument)
        currentDocument->pushSnapshot();
}
