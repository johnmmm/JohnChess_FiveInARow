#include "Chess.h"


Chess::Chess(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::Chess)
{
    ui->setupUi(this);

    hintNum = 0;

    connect(this, SIGNAL(addChess(int,int)), ui->gameboard, SLOT(addEnemysChess(int,int)));
}

Chess::~Chess()
{
    delete ui;
}



void Chess::on_createButton_clicked()
{
    dialog = new CreateDialog;
    dialog->show();

    connect(dialog, SIGNAL(getButton(int)), this, SLOT(createReturn(int)));
}

void Chess::on_connectButton_clicked()
{
    dialog1 = new ConnectDialog;
    dialog1->show();

    connect(dialog1, SIGNAL(getButton(int)), this, SLOT(connectReturn(int)));
}

void Chess::on_restartButton_clicked()
{
    ui->restartButton->setEnabled(false);
    ui->restartButton->setText(tr("waiting"));
    socket->write("restart ");
}

void Chess::on_hintButton_clicked()
{
    if(hintNum == 0)
    {
        hintNum = 1;

        ui->hintButton->setText(tr("Hide hints"));
        ui->gameboard->isHint = true;
        update();

        return;
    }
    if(hintNum == 1)
    {
        hintNum = 0;

        ui->hintButton->setText(tr("Hint"));
        ui->gameboard->isHint = false;
        update();

        return;
    }
}

void Chess::on_quitButton_clicked()
{
    ui->quitButton->setEnabled(false);
    ui->quitButton->setText(tr("waiting"));
    socket->write("quit ");
}

void Chess::createReturn(int num)
{
    if(num == 1)
    {
        server = new QTcpServer(this);
        server->listen(QHostAddress(dialog->getHostAddress()), 8888);
        qDebug() << dialog->getHostAddress();

        ui->createButton->setEnabled(false);
        ui->connectButton->setEnabled(false);

        QMessageBox *waiting = new QMessageBox(QMessageBox::NoIcon, tr("waiting"), dialog->getHostAddress()+tr("\waiting for new connection..."),QMessageBox::Cancel,this);

        bool isconnect = false;
        connect(server, &QTcpServer::newConnection, [&]()
        {
            socket = server->nextPendingConnection();
            ui->restartButton->setEnabled(true);
            ui->createButton->setEnabled(false);
            ui->connectButton->setEnabled(false);
            ui->hintButton->setEnabled(true);
            ui->gameboard->playerColor = Qt::black;
            ui->gameboard->enemyColor = Qt::white;
            ui->gameboard->inRound = true;
            ui->turnLabel->setText(tr("Player's turn"));

            connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
            connect(ui->gameboard, SIGNAL(addChess(QString)), this, SLOT(sendChessInfo(QString)));
            connect(ui->gameboard, SIGNAL(win()), this, SLOT(sendWin()));
//            connect(ui->restartButton, SIGNAL(clicked()), this, SLOT(on_restartButton_clicked()));
//            connect(ui->quitButton, SIGNAL(clicked()), this, SLOT(on_quitButton_clicked()));
            isconnect = true;
            waiting->close();
        });

        if(waiting->exec() == QMessageBox::Cancel && !isconnect)
        {
            server->close();
            ui->createButton->setEnabled(true);
            ui->connectButton->setEnabled(true);
        }
    }
}

void Chess::connectReturn(int num)
{
    if(num = 1)
    {
        socket = new QTcpSocket(this);
        socket->connectToHost(QHostAddress(dialog1->getHostIp()),8888);
        qDebug() << dialog1->getHostIp();

        if(socket->waitForConnected(10000))
        {
            ui->restartButton->setEnabled(true);
            ui->gameboard->playerColor = Qt::white;
            ui->gameboard->enemyColor = Qt::black;
            ui->createButton->setEnabled(false);
            ui->connectButton->setEnabled(false);
            ui->turnLabel->setText(tr("Enemy's turn"));

            connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
            connect(ui->gameboard, SIGNAL(addChess(QString)), this, SLOT(sendChessInfo(QString)));
            connect(ui->gameboard, SIGNAL(win()), this, SLOT(sendWin()));
        }
        else
        {
            QMessageBox::warning(this, tr("Error"), tr("Connection failed!"));
        }
    }
}

void Chess::readData()
{
    //qDebug() << "receive!!";
    auto messages = QString(socket->readAll()).split(' ');
//    for(int i = 0; i < messages.length(); i++)
//        qDebug() << messages[i];
    bool finished = false;

    while(!finished)
    {
        if(messages[0] == "add")
        {
            //qDebug() << "receive add";
            emit addChess(messages[1].toInt(), messages[2].toInt());

            ui->turnLabel->setText(tr("Player's turn"));
            ui->restartButton->setEnabled(true);
            ui->hintButton->setEnabled(true);

            if(!messages[3].isEmpty())
            {
                auto temp = messages;
                messages.clear();
                for(int i = 3; i < temp.length(); i++)
                    messages << temp[i];
            }
            else
                finished = true;
        }

        else if(messages[0] == "win")
        {
            //QSound::play(":/..");

            QMessageBox *mmm;
            mmm = new QMessageBox(QMessageBox::NoIcon,tr("Lose"),tr("<strong>YOU LOSE!!!</strong>"),QMessageBox::Ok,this);
            mmm->show();

            if(!messages[1].isEmpty())
            {
                auto temp = messages;
                messages.clear();
                for(int i = 1; i < temp.length(); i++)
                    messages << temp[i];
            }
            else
                finished = true;
        }

        else if(messages[0] == "restart")
        {
            //wait for
            if(!messages[1].isEmpty())
            {
                auto temp = messages;
                messages.clear();
                for(int i = 1; i < temp.length(); i++)
                    messages << temp[i];
            }
            else
                finished = true;
        }

        else if(messages[0] == "quit")
        {
            auto quit = QMessageBox::question(this,tr("Quit"),tr("Do you want to quit the game?"),QMessageBox::Yes|QMessageBox::No);

            if(quit == QMessageBox::Yes)
            {
                socket->write("agree quit ");
                socket->waitForBytesWritten();
                qApp->quit();
            }
            else
                socket->write("reject quit ");

            if(!messages[1].isEmpty())
            {
                auto temp = messages;
                messages.clear();
                for(int i = 1; i < temp.length(); i++)
                    messages << temp[i];
            }
            else
                finished = true;
        }

        else if(messages[0] == "agree")
        {
            if(messages[1] == "restart")
            {
                //wait for...
                if(!messages[2].isEmpty())
                {
                    auto temp = messages;
                    messages.clear();
                    for(int i = 2; i < temp.length(); i++)
                        messages << temp[i];
                }
                else
                    finished = true;
            }
            else if(messages[1] == "quit")
            {
                qApp->quit();

                if(!messages[2].isEmpty())
                {
                    auto temp = messages;
                    messages.clear();
                    for(int i = 2; i < temp.length(); i++)
                        messages << temp[i];
                }
                else
                    finished = true;
            }
        }

        else if(messages[0] == "reject")
        {
            if(messages[1] == "restart")
            {
                ui->restartButton->setText("Restart");
                ui->restartButton->setEnabled(true);

                if(!messages[2].isEmpty())
                {
                    auto temp = messages;
                    messages.clear();
                    for(int i = 2; i < temp.length(); i++)
                        messages << temp[i];
                }
                else
                    finished = true;
            }
            else if(messages[2] == "quit")
            {
                ui->quitButton->setText("Quit");
                ui->quitButton->setEnabled(true);

                if(!messages[2].isEmpty())
                {
                    auto temp = messages;
                    messages.clear();
                    for(int i = 2; i < temp.length(); i++)
                        messages << temp[i];
                }
                else
                    finished = true;
            }
        }
    }


}

void Chess::sendChessInfo(QString chess)
{
    ui->turnLabel->setText(tr("Enemy's turn"));
    ui->restartButton->setEnabled(false);
    ui->hintButton->setEnabled(false);

    socket->write((QString("add ") + chess).toStdString().c_str());//读取的时候会多一个空格..
}

void Chess::sendWin()
{
//    QSound::play(":/...");
    ui->restartButton->setEnabled(true);

    QMessageBox *message;
    message = new QMessageBox(QMessageBox::NoIcon, tr("Win"), tr("<strong>YOU WIN!</strong>"),QMessageBox::Ok,this);

    message->show();
    qDebug() << "send win";
    socket->write("win ");
}









