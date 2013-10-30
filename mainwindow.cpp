#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDesktopWidget>
#include <QDebug>
#include <QtMultimedia/QSoundEffect>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QFile stylesheetFile("/home/mike/TediumRemedy/stylesheet.qss");
    if(!stylesheetFile.open(QFile::ReadOnly)) {

    }

    QStatusBar *sb = this->statusBar();

    chatModeLabel = new QLabel(this);
    //chatModeLabel->setText("R");
    sb->addPermanentWidget(chatModeLabel);


    /*QImage typingImagePng("/home/mike/TediumRemedy/typing.png");
    typingImagePng = typingImagePng.scaled(20,20,Qt::KeepAspectRatio);
    typingImage = new QLabel(this);
    typingImage->setPixmap(QPixmap::fromImage(typingImagePng));
    sb->addPermanentWidget(typingImage);*/

    typingLabel = new QLabel(this);
    //typingLabel->setText("1 & 2");
    sb->addPermanentWidget(typingLabel);



    //return;

    QString stylesheetString = QLatin1String(stylesheetFile.readAll());
    setStyleSheet(stylesheetString);

    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));
    QObject::connect(ui->typingBox, SIGNAL(enterPressed()), this, SLOT(enterPressed()));
    QObject::connect(ui->typingBox, SIGNAL(escapePressed()), this, SLOT(escapePressed()));    
    QObject::connect(ui->typingBox, SIGNAL(SwitchMode()), this, SLOT(SwitchMode()));

    stranger = new Stranger(this);
    spy = new Spy(this);
    incomingMessageSound = new QSoundEffect(this);
    incomingMessageSound->setSource(QUrl("/home/mike/TediumRemedy/bell2.wav"));
    incomingMessageSound->play();

    QObject::connect(stranger, SIGNAL(ReceivedMessage(const QString &)), this, SLOT(ReceivedMessage(const QString &)));
    QObject::connect(stranger, SIGNAL(StrangerDisconnected()), this, SLOT(StrangerDisconnected()));
    QObject::connect(stranger, SIGNAL(ConversationStarted()), this, SLOT(StrangerConnected()));
    QObject::connect(stranger, SIGNAL(ConversationStartedWithQuestion(QString)), this, SLOT(StrangerConnectedWithQuestion(QString)));

    QObject::connect(stranger, SIGNAL(StrangerStartsTyping()), this, SLOT(StrangerStartsTyping()));
    QObject::connect(stranger, SIGNAL(StrangerStopsTyping()), this, SLOT(StrangerStopsTyping()));

    QObject::connect(spy, SIGNAL(ReceivedMessage(const QString &,const QString &)), this, SLOT(SpymodeReceivedMessage(const QString &,const QString &)));
    QObject::connect(spy, SIGNAL(StrangerDisconnected(const QString &)), this, SLOT(SpymodeStrangerDisconnected(const QString &)));
    QObject::connect(spy, SIGNAL(ConversationStarted()), this, SLOT(SpymodeStrangersConnected()));
    QObject::connect(spy, SIGNAL(ConversationStartedWithQuestion(QString)), this, SLOT(StrangerConnectedWithQuestion(QString)));
    QObject::connect(spy, SIGNAL(StrangerStartsTyping()), this, SLOT(SpymodeStrangerStartsTyping(const QString &)));
    QObject::connect(spy, SIGNAL(StrangerStopsTyping()), this, SLOT(SpymodeStrangerStopsTyping(const QString &)));

    chatMode = Spying;
    SwitchMode(); //switch it to regular

    this->escapePressed();
    //stranger->StartConversation("en", "");
    //spy->StartConversation("This is a test question");
}

void MainWindow::SwitchMode() {
    if(chatMode==Regular) {
        chatMode = AnsweringQuestions;
        chatModeLabel->setText("Ans");
    }
    else if(chatMode==AnsweringQuestions) {
            chatMode = Spying;
        chatModeLabel->setText("Q");
    }
    else if(chatMode==Spying) {
            chatMode = Regular;
        chatModeLabel->setText("Regular");
}
}

void MainWindow::enterPressed() {
    QString messageText = ui->typingBox->toPlainText();
    ui->chatlogBox->append("You: "+messageText);
    ui->typingBox->clear();
    stranger->SendMessage(messageText);
}

void MainWindow::escapePressed() {
    ui->chatlogBox->clear();
    //spy->StartConversation(ui->typingBox->toPlainText());
    if(chatMode == Regular)
        stranger->StartConversation("en", "", false);
    else if(chatMode == Spying)
        spy->StartConversation(ui->typingBox->toPlainText());
    else if(chatMode == AnsweringQuestions)
            stranger->StartConversation("en", "", true);
}

void MainWindow::ReceivedMessage(const QString &messageText) {
    ui->chatlogBox->append("Stranger: "+messageText);
    incomingMessageSound->play();
    typingLabel->setText("");
}

void MainWindow::StrangerDisconnected() {
    ui->chatlogBox->append("Stranger disconnected");
}

void MainWindow::StrangerConnected() {

    ui->chatlogBox->append("Stranger connected");
}

void MainWindow::StrangerConnectedWithQuestion(QString questionText) {
    ui->chatlogBox->append(questionText);
}

void MainWindow::StrangerStartsTyping() {
    //ui->chatlogBox->append("Stranger typing");
    typingLabel->setText("T");
}

void MainWindow::StrangerStopsTyping() {
    typingLabel->setText("");
    //ui->chatlogBox->append("Stranger stopped typing");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QWidget *focusedWidget = QApplication::focusWidget();
    //let's not redirect Control/Alt modified key presses to the typing box (Ctrl+C for ex)
    if(focusedWidget != ui->typingBox && !(event->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
        ui->typingBox->setFocus();
        QKeyEvent *eventDuplicate = new QKeyEvent(event->type(), event->key(), event->modifiers(), event->text(), false, event->count());
        QCoreApplication::postEvent(ui->typingBox, eventDuplicate);
    }
    //event->key();
    qDebug() << "Inside MyWindow keypress";

}

//spy mode

void MainWindow::SpymodeReceivedMessage(const QString &strangerID, const QString &messageText){
    ui->chatlogBox->append(strangerID+": "+messageText);
    typingLabel->setText("");
}

void MainWindow::SpymodeStrangerDisconnected(const QString &strangerID) {
    ui->chatlogBox->append(strangerID+" disconnected");
}

void MainWindow::SpymodeStrangersConnected() {
    ui->chatlogBox->append("Conversation started");
}

void MainWindow::SpymodeStrangerStartsTyping(const QString &strangerID) {
    //ui->chatlogBox->append(strangerID+" types");
    typingLabel->setText(strangerID);
}

void MainWindow::SpymodeStrangerStopsTyping(const QString &strangerID) {

}

void MainWindow::PlaySoundFile(QString filename) {
    //QSoundEffect s()
    /*QFile inputFile;     // class member.
    static QAudioOutput* audio = NULL; // class member.
    inputFile.setFileName("/home/mike/TediumRemedy/bell.wav");
    inputFile.open(QIODevice::ReadOnly);

    QAudioFormat format;
    // Set up the format, eg.
    format.setFrequency(8000);
    format.setChannels(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        qWarning()<<"raw audio format not supported by backend, cannot play audio.";
        return;
    }

    if(!audio)
        audio = new QAudioOutput(format, this);

    //connect(audio,SIGNAL(stateChanged(QAudio::State)),SLOT(finishedPlaying(QAudio::State)));
    audio->start(&inputFile);*/
}
