#include "trayicon.h"

TrayIcon::TrayIcon(ClipboardHistory *history, QWidget *parent) : QWidget(parent)
{
  if(history)
    _history=history;
  constructIcon();
}

void TrayIcon::constructIcon()
{
   _icon=new QSystemTrayIcon(this);
   _menu=new QMenu(this);
   connect(_icon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

  _icon->setIcon(QIcon(Resources::logo16));

  _historyMenu=new Menu(_menu);
  _historyMenu->setTitle("history");
  _historyMenu->setIcon(QIcon(Resources::history16));
  _menu->addMenu(_historyMenu);

  _showHideAction=new QAction(_menu);
  _showHideAction->setIcon(QIcon(Resources::logo16));
  _showHideAction->setText("show clipboard manager");
  _menu->addAction(_showHideAction);

  _clearAction=new QAction(_menu);
  _clearAction->setIcon(QIcon(Resources::clrea16));
  _clearAction->setText("clear hisory");
  _menu->addAction(_clearAction);

  _settingsAction=new QAction(_menu);
  _settingsAction->setIcon(QIcon(Resources::settings16));
  _settingsAction->setText("settings");
  _settingsAction->setToolTip("open settings dialog");
  _menu->addAction(_settingsAction);

  _pauseResumeAction=new QAction(_menu);
  _pauseResumeAction->setIcon(QIcon(Resources::pause16));
  _pauseResumeAction->setText("pause");
  _pauseResumeAction->setToolTip("pause collecting clipboard changes");
  _menu->addAction(_pauseResumeAction);

  _onOffAction=new QAction(_menu);
  _onOffAction->setText("turn off");
  _onOffAction->setIcon(QIcon(Resources::on16));
  _onOffAction->setToolTip("turn off genius . new items will not added to the list until turn on");
  _menu->addAction(_onOffAction);

  _aboutAction=new QAction(_menu);
  _aboutAction->setIcon(QIcon(Resources::info16));
  _aboutAction->setText("about");
  _aboutAction->setToolTip("about genius ");
  _menu->addAction(_aboutAction);

  _exitAction=new QAction(_menu);
  _exitAction->setIcon(QIcon(Resources::exit16));
  _exitAction->setText("exit");
  _exitAction->setToolTip("exit from Genius");
  _menu->addAction(_exitAction);

  connect(_historyMenu,SIGNAL(triggered(QAction*)),this,SLOT(historyMenuActionTriggered(QAction*)));
  connect(_historyMenu,SIGNAL(keyPressed(int)),this,SLOT(historyMenuKeyPressed(int)));
  connect(_menu,SIGNAL(triggered(QAction*)),this,SLOT(menuActionTrigered(QAction*)));
  _icon->setContextMenu(_menu);
  _icon->setToolTip("system tray icon of genius ");
}

void TrayIcon::managerHidden()
{
   _showHideAction->setText("show clipboard manager");
}

void TrayIcon::managerShown()
{
  _showHideAction->setText("hide clipboard manager");
}

void TrayIcon::show()
{
  _icon->show();
}


void TrayIcon::addImageAction(QString *text, QIcon *icon, int reference, int index)
{
  MenuItem *action=new MenuItem(_historyMenu);
  action->setIcon(*icon);
  action->setTxt(*text);
  action->setData(QVariant(reference));
  if(_historyMenu->actions().isEmpty())
    _historyMenu->addAction(action);
  else
  {
    if(index==0)
    {
       _historyMenu->insertAction(_historyMenu->actions().first(),action);
    }
    else
    {
      if(_historyMenu->actions().length()>=index)
      {
        _historyMenu->insertAction(_historyMenu->actions().at(index),action);
      }
      else
      {
       _historyMenu->insertAction(_historyMenu->actions().first(),action);
      }
    }
  }

  checkLimit();
  setIndexes();
}

void TrayIcon::addTextAction(QString *text, QString *tooltipText, int reference, int index)
{

  MenuItem *action=new MenuItem(_historyMenu);
  action->setTxt(*text);
  action->setToolTip(*tooltipText);
  action->setData(QVariant(reference));
  if(_historyMenu->actions().isEmpty())
    _historyMenu->addAction(action);
  else
  {
    if(index==0)
    {
       _historyMenu->insertAction(_historyMenu->actions().first(),action);
    }
    else
    {
      if(_historyMenu->actions().length()>=index)
      {
        _historyMenu->insertAction(_historyMenu->actions().at(index),action);
      }
      else
      {
       _historyMenu->insertAction(_historyMenu->actions().first(),action);
      }
    }
  }

  checkLimit();
  setIndexes();
}

void TrayIcon::removeItem(int reference)
{
  int length=_historyMenu->actions().length();
  if(length>0)
  {
    QAction *action;
    for(int i=0;i<length;i++)
    {
      action=_historyMenu->actions().at(i);
      if(action)
      {
        int ref=action->data().toInt();
        if(ref==reference)
        {
          _historyMenu->actions().removeAt(i);
          delete action;
          setIndexes();
          return;
        }
      }
    }
  }
}

void TrayIcon::clearHistoryList()
{
  if(_historyMenu)
  {
    int length=_historyMenu->actions().length();
    if(length>0)
    {

      foreach (QAction *action, _historyMenu->actions())
      {
        delete action;
      }
      _historyMenu->actions().clear();
    }
  }
}

void TrayIcon::menuActionTrigered(QAction *action)
{
  if(action==_exitAction)
  {
    emit exitRequested();
  }
  else if(action==_showHideAction)
  {
    emit showHideManagerTriggerd();
  }
  else if(action==_clearAction)
  {
    if(_history && _history->isEmpty()==false)
      _history->clear();
  }
  else if(action==_settingsAction)
  {
    emit settingsDialogRequested();
  }
  else if(action==_pauseResumeAction)
  {
      if(_pauseResumeAction->text()=="pause")
      {
          emit pause();
          _pauseResumeAction->setText("resume");
          _pauseResumeAction->setIcon(QIcon(Resources::play16));
          _pauseResumeAction->setToolTip("resume collecting clipboard changes ");
      }
      else
      {
          emit resume();
          _pauseResumeAction->setText("pause");
          _pauseResumeAction->setIcon(QIcon(Resources::pause16));
          _pauseResumeAction->setToolTip("pause collecting clipboard changes ");
      }
  }
  else if(action==_onOffAction)
  {
    if(_onOffAction->text()=="turn off")
    {
        emit turnOffGenius();
      _onOffAction->setText("turn on");
      _onOffAction->setIcon(QIcon(Resources::off16));
      _onOffAction->setToolTip("turn on genius . new items will saved in the history");

    }
    else
    {
        emit turnOnGenius();
      _onOffAction->setText("turn off");
      _onOffAction->setIcon(QIcon(Resources::on16));
      _onOffAction->setToolTip("turn off genius . new items will not added to the list until turn on");
    }
  }
  else if(action==_aboutAction)
  {
    About ab(0);
    ab.exec();
  }
}

void TrayIcon::historyMenuActionTriggered(QAction *action)
{
  if(action)
  {
    int reference=action->data().toInt();
    emit itemSelected(reference);
    if(_pasteWhenSelected)
    {
      QThread::msleep(50);
      FakeKey::simulatePaste();
    }
  }
}

void TrayIcon::showMessage(QString title, QString message, QSystemTrayIcon::MessageIcon icon, int duration)
{
  if(_icon)
  {
    _icon->showMessage(title,message,icon,duration);
  }
}


void TrayIcon::checkLimit()
{
  if(GSettings::historyItemLimit<_historyMenu->actions().count())
  {
    while (_historyMenu->actions().count()>GSettings::historyItemLimit)
    {
      delete _historyMenu->actions().takeLast();
    }
  }
}

void TrayIcon::showHistoryMenu()
{
  if(_historyMenu->actions().count()>0)
  {
    _pasteWhenSelected=true;
    //QPoint point=_historyMenu->mapFromGlobal(QCursor::pos());
    _historyMenu->exec(QCursor::pos());
    _pasteWhenSelected=false;
  }
}

void TrayIcon::exchangeLocation(int ref1, int ref2)
{
  QAction *action1=NULL;
  QAction *action2=NULL;

  foreach (QAction* action, _historyMenu->actions())
  {
    if(action1 && action2) break;
    if(action )
    {
      int data=action->data().toInt();
      if(data==ref1)
        action1=action;
      else if(data==ref2)
        action2=action;
    }
  }
  if(action1 && action2)
  {
    QIcon ico=action2->icon();
    action2->setIcon(action1->icon());
    action1->setIcon(ico);

    QVariant data=action2->data();
    action2->setData(action1->data());
    action1->setData(data);

    QString txt=action2->text();
    action2->setText(action1->text());
    action1->setText(txt);

    QString iText=action2->iconText();
    action2->setText(action1->iconText());
    action1->setText(iText);

    QString TT=action2->toolTip();
    action2->setToolTip(action1->toolTip());
    action1->setToolTip(TT);
  }

  setIndexes();
}

void TrayIcon::iconActivated(QSystemTrayIcon::ActivationReason reson)
{
    if(reson==QSystemTrayIcon::DoubleClick)
        emit showHideManagerTriggerd();
}


void TrayIcon::setIndexes()
{
  bool indxed=GSettings::showIndexesInHistoryMenu;
  if(indxed)
  {
    if(_historyMenu && _historyMenu->actions().isEmpty()==false && _history->isEmpty()==false)
    {
      int index=1;
      foreach (QAction *action , _historyMenu->actions())
      {
        MenuItem *mi=dynamic_cast<MenuItem*>(action);
        mi->indexed(indxed);
        mi->index(index);
        index++;
      }
    }
  }
  else
  {
    foreach (QAction *action , _historyMenu->actions())
    {
      MenuItem *mi=dynamic_cast<MenuItem*>(action);
      if(mi->indexed())
        mi->indexed(false);
    }
  }
}

void TrayIcon::historyMenuKeyPressed(int key)
{
   if(key>=0x31 && key<0x3a)
   {
     int number=key-0x30;
     if(_historyMenu->actions().count()>number-1)
     {
       QAction *act=_historyMenu->actions().at(number-1);
       int reference=act->data().toInt();
       _historyMenu->close();
       itemSelected(reference);
       if(_pasteWhenSelected)
         FakeKey::simulatePaste();
     }
   }
}
