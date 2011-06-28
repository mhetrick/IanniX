#include "uieditor.h"
#include "ui_uieditor.h"

UiEditor::UiEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UiEditor) {
    ui->setupUi(this);

    QRect screen = QApplication::desktop()->geometry();
    move(screen.bottomRight().x() - rect().width(), 20);

    connect(ui->actionSave, SIGNAL(triggered()), SLOT(save()));
    connect(ui->actionClose, SIGNAL(triggered()), SLOT(close()));

    ui->jsEditor->setTextWrapEnabled(false);
    ui->jsEditor->setLineNumbersVisible(true);
    ui->jsEditor->setCodeFoldingEnabled(true);
    ui->jsEditor->setBracketsMatchingEnabled(true);

    /*
    ui->jsEditor->setWindowTitle(QFileInfo(fileName).fileName());
    ui->jsEditor->setFrameShape(JSEdit::NoFrame);
    ui->jsEditor->setWordWrapMode(QTextOption::NoWrap);
    ui->jsEditor->setTabStopWidth(4);
    ui->jsEditor->resize(QApplication::desktop()->availableGeometry().size() / 2);
    */
    /*
    ui->jsEditor->setColor(JSEdit::Background,    QColor("#0C152B"));
    ui->jsEditor->setColor(JSEdit::Normal,        QColor("#FFFFFF"));
    ui->jsEditor->setColor(JSEdit::Comment,       QColor("#666666"));
    ui->jsEditor->setColor(JSEdit::Number,        QColor("#DBF76C"));
    ui->jsEditor->setColor(JSEdit::String,        QColor("#5ED363"));
    ui->jsEditor->setColor(JSEdit::Operator,      QColor("#FF7729"));
    ui->jsEditor->setColor(JSEdit::Identifier,    QColor("#FFFFFF"));
    ui->jsEditor->setColor(JSEdit::Keyword,       QColor("#FDE15D"));
    ui->jsEditor->setColor(JSEdit::BuiltIn,       QColor("#9CB6D4"));
    ui->jsEditor->setColor(JSEdit::Cursor,        QColor("#1E346B"));
    ui->jsEditor->setColor(JSEdit::Marker,        QColor("#DBF76C"));
    ui->jsEditor->setColor(JSEdit::BracketMatch,  QColor("#1AB0A6"));
    ui->jsEditor->setColor(JSEdit::BracketError,  QColor("#A82224"));
    ui->jsEditor->setColor(JSEdit::FoldIndicator, QColor("#555555"));
    */
}

UiEditor::~UiEditor() {
    delete ui;
}

void UiEditor::openFile(const QFileInfo & _scriptFile) {
    scriptFile = _scriptFile;

    QFile file(scriptFile.absoluteFilePath());
    if(file.open(QFile::ReadOnly)) {
        QString contents = file.readAll();
        file.close();
        //contents.replace("\t", "  ");

        ui->jsEditor->setPlainText(contents);
        setWindowTitle(tr("IanniX — Script Editor — ") + scriptFile.baseName());
        show();
    }
}
void UiEditor::save() {
    QFile file(scriptFile.absoluteFilePath());
    if(file.open(QFile::WriteOnly)) {
        QString contents = ui->jsEditor->toPlainText();
        //contents.replace("  ", "\t");
        file.write(contents.toLatin1());
        file.close();
    }
}


void UiEditor::changeEvent(QEvent *e) {
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
