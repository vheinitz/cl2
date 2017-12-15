#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fsm.h"
#include <QTimer>
#include <QDialog>
#include <QMessageBox>

FSM fsm;
FSM &_ = fsm;



State Red; State YellowRed; State Yellow; State Green; State
ANY(true); State STOP(false, true);
State Blind;
enum {Tick, Reset};

MainWindow *tp;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  tp = this;

  Red.onEnter =  ACTION{ tp->ui->cbRot->setChecked(true); };

  Yellow.onEnter =  ACTION{ tp->ui->cbYel->setChecked(true); };
  Yellow.onExit =   ACTION{ tp->ui->cbYel->setChecked(false); };

  YellowRed.onEnter = ACTION{ tp->ui->cbYel->setChecked(true); };
  YellowRed.onExit =  ACTION{ tp->ui->cbRot->setChecked(false);
                       tp->ui->cbYel->setChecked(false); };

  Green.onEnter =  ACTION{ tp->ui->cbGreen->setChecked(true); };
  Green.onExit =   ACTION{ tp->ui->cbGreen->setChecked(false); };


  //fsm.addTransition(StartTransition, EndTransition , OnEvent);

  _(ANY,       "  -->  ",  Red,        "ON:", Reset);
  _(Red,       "  -->  ",  YellowRed,  "ON:", Tick);
  _(YellowRed, "  -->  ",  Green,      "ON:", Tick);
  _(Green,     "  -->  ",  Yellow,     "ON:", Tick);
  _(Yellow,    "  -->  ",  Red,        "ON:", Tick);
  //unique test    _(Green,     "  -->  ",  Yellow,     "ON:", Tick);
  //ambigious test _(Green,     "  -->  ",  Red,     "ON:", Tick);
  //blind test
  _(Green,     "  -->  ",  Blind,     "ON:", Tick);
  _(Blind,     "  -->  ",  Blind,     "ON:", Tick);


  fsm.setStartState(Red);
  std::tuple<bool, std::string> checkResult = fsm.check();
  if ( std::get<0>(checkResult) == true )
  {
    fsm.start();
  }
  else
  {
    QMessageBox::warning(this,tr("FSM Error"),tr("FSM Errror:")+QString(std::get<1>(checkResult).c_str()) );
  }

  QTimer::singleShot(1000, this, SLOT( tick() )  );
}


MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_pushButton_clicked()
{
  fsm.processEvent(Tick);
}

void MainWindow::tick()
{
  fsm.processEvent(Tick);
  QTimer::singleShot(1000, this, SLOT( tick() )  );
}


void MainWindow::on_pushButton_2_clicked()
{
    fsm.processEvent(Reset);
}

void MainWindow::on_pushButton_3_clicked(bool checked)
{
    if (checked) fsm.pause();
    else fsm.resume();
}
