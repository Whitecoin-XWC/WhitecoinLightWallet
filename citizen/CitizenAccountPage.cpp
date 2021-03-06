#include "CitizenAccountPage.h"
#include "ui_CitizenAccountPage.h"

#include "wallet.h"
#include "control/AssetIconItem.h"
#include "CreateCitizenDialog.h"
#include "commondialog.h"
#include "ChangePayBackDialog.h"
#include "poundage/PageScrollWidget.h"

static const int ROWNUMBER = 7;
CitizenAccountPage::CitizenAccountPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CitizenAccountPage)
{
    ui->setupUi(this);

    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->lockBalanceTableWidget->installEventFilter(this);
    ui->lockBalanceTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->lockBalanceTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->lockBalanceTableWidget->setFocusPolicy(Qt::NoFocus);
//    ui->lockBalanceTableWidget->setFrameShape(QFrame::NoFrame);
    ui->lockBalanceTableWidget->setMouseTracking(true);
    ui->lockBalanceTableWidget->setShowGrid(false);//隐藏表格线

    ui->lockBalanceTableWidget->horizontalHeader()->setSectionsClickable(true);
//    ui->lockBalanceTableWidget->horizontalHeader()->setFixedHeight(40);
    ui->lockBalanceTableWidget->horizontalHeader()->setVisible(true);
    ui->lockBalanceTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->lockBalanceTableWidget->setColumnWidth(0,140);
    ui->lockBalanceTableWidget->setColumnWidth(1,490);
    ui->lockBalanceTableWidget->setStyleSheet(TABLEWIDGET_STYLE_1);

    ui->newCitizenBtn->setStyleSheet(TOOLBUTTON_STYLE_1);
    ui->startMineBtn->setStyleSheet(TOOLBUTTON_STYLE_1);
    ui->changeFeeBtn->setStyleSheet(TOOLBUTTON_STYLE_1);

    XWCWallet::getInstance()->mainFrame->installBlurEffect(ui->lockBalanceTableWidget);


    pageWidget = new PageScrollWidget();
    ui->stackedWidget->addWidget(pageWidget);
    connect(pageWidget,&PageScrollWidget::currentPageChangeSignal,this,&CitizenAccountPage::pageChangeSlot);

#ifdef LIGHT_MODE
    ui->startMineBtn->hide();
    ui->changeFeeBtn->hide();
#endif
    init();
}

CitizenAccountPage::~CitizenAccountPage()
{
    delete ui;
}

void CitizenAccountPage::init()
{
    inited = false;
    XWCWallet::getInstance()->fetchCitizenPayBack();

    ui->accountComboBox->clear();
    QStringList accounts = XWCWallet::getInstance()->getMyCitizens();
    if(accounts.size() > 0)
    {
        ui->accountComboBox->addItems(accounts);

        if(accounts.contains(XWCWallet::getInstance()->currentAccount))
        {
            ui->accountComboBox->setCurrentText(XWCWallet::getInstance()->currentAccount);
        }
    }
    else
    {
        ui->label->hide();
        ui->accountComboBox->hide();
        ui->idLabel->hide();
        ui->idLabel2->hide();
        ui->totalProducedLabel->hide();
        ui->totalProducedLabel2->hide();
        ui->lastBlockLabel->hide();
        ui->lastBlockLabel2->hide();
        ui->startMineBtn->hide();
        ui->changeFeeBtn->hide();
        ui->effectiveTimeLabel->hide();

        QLabel* label = new QLabel(this);
        label->setGeometry(QRect(ui->label->pos(), QSize(300,30)));
        label->setText(tr("There are no miner accounts in the wallet"));
        label->setStyleSheet(NOACCOUNT_TIP_LABEL);
    }

    inited = true;

    on_accountComboBox_currentIndexChanged(ui->accountComboBox->currentText());
    queryActiveMiners();
}

void CitizenAccountPage::refresh()
{
    init();
}

void CitizenAccountPage::jsonDataUpdated(QString id)
{
    if( id == "CitizenAccountPage+dump_private_key+" + ui->accountComboBox->currentText())
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        result.prepend("{");
        result.append("}");

        QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
        QJsonObject jsonObject = parse_doucment.object();
        QJsonArray array = jsonObject.take("result").toArray();
        if(array.size() > 0)
        {
            QJsonArray array2 = array.at(0).toArray();
            QString privateKey = array2.at(1).toString();
            AccountInfo accountInfo = XWCWallet::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText());
            XWCWallet::getInstance()->witnessConfig->addPrivateKey( accountInfo.pubKey, privateKey);
            XWCWallet::getInstance()->witnessConfig->setProductionEnabled(true);

            MinerInfo minerInfo = XWCWallet::getInstance()->minerMap.value(ui->accountComboBox->currentText());
            XWCWallet::getInstance()->witnessConfig->addMiner(minerInfo.minerId);
            XWCWallet::getInstance()->witnessConfig->save();

            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("Mining configuration has been written. This miner account will start mining when the wallet is launched next time."));
            commonDialog.pop();
        }

        return;
    }

    if( id == "CitizenAccountPage-list_active_miners")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        result.prepend("{");
        result.append("}");
        QJsonArray array = QJsonDocument::fromJson(result.toLatin1()).object().value("result").toArray();
        activeMiners.clear();
        for(QJsonValue v : array)
        {
            activeMiners << v.toString();
        }
        on_accountComboBox_currentIndexChanged(ui->accountComboBox->currentText());

        return;
    }
}

void CitizenAccountPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(239,242,245),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(239,242,245),Qt::SolidPattern));

    painter.drawRect(rect());
}

void CitizenAccountPage::showLockBalance()
{
    MinerInfo minerInfo = XWCWallet::getInstance()->minerMap.value(ui->accountComboBox->currentText());
    int size = minerInfo.lockBalances.size();
    ui->lockBalanceTableWidget->setRowCount(0);
    ui->lockBalanceTableWidget->setRowCount(size);
    for(int i = 0; i < size; i++)
    {
        AssetAmount aa = minerInfo.lockBalances.at(i);
        AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(aa.assetId);
        ui->lockBalanceTableWidget->setItem( i, 0, new QTableWidgetItem( assetInfo.symbol));
        ui->lockBalanceTableWidget->setItem( i, 1, new QTableWidgetItem( getBigNumberString(aa.amount, assetInfo.precision)));

        AssetIconItem* assetIconItem = new AssetIconItem();
        assetIconItem->setAsset(ui->lockBalanceTableWidget->item(i,0)->text());
        ui->lockBalanceTableWidget->setCellWidget(i, 0, assetIconItem);
    }

    int page = (ui->lockBalanceTableWidget->rowCount()%ROWNUMBER==0 && ui->lockBalanceTableWidget->rowCount() != 0) ?
                ui->lockBalanceTableWidget->rowCount()/ROWNUMBER : ui->lockBalanceTableWidget->rowCount()/ROWNUMBER+1;
    pageWidget->SetTotalPage(page);
    pageWidget->setShowTip(ui->lockBalanceTableWidget->rowCount(),ROWNUMBER);
    pageChangeSlot(pageWidget->GetCurrentPage());

    pageWidget->setVisible(0 != ui->lockBalanceTableWidget->rowCount());

    tableWidgetSetItemZebraColor(ui->lockBalanceTableWidget);

}

void CitizenAccountPage::queryActiveMiners()
{
    XWCWallet::getInstance()->postRPC( "CitizenAccountPage-list_active_miners",
                                     toJsonFormat( "list_active_miners", QJsonArray() << 0 << 1000 ));
}

void CitizenAccountPage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    if(!inited || ui->accountComboBox->currentText().isEmpty())  return;
    XWCWallet::getInstance()->currentAccount = ui->accountComboBox->currentText();

    MinerInfo minerInfo = XWCWallet::getInstance()->minerMap.value(ui->accountComboBox->currentText());
    ui->idLabel->setText(minerInfo.minerId);
    ui->totalProducedLabel->setText(QString::number(minerInfo.totalProduced));
    ui->lastBlockLabel->setText(QString::number(minerInfo.lastBlock));

    if(!activeMiners.isEmpty() && !activeMiners.contains(minerInfo.minerId))
    {
        ui->totalProducedLabel->hide();
        ui->totalProducedLabel2->hide();
        ui->lastBlockLabel->hide();
        ui->lastBlockLabel2->hide();

        QDateTime utcTime = QDateTime::currentDateTimeUtc();
        int offset = utcTime.toTime_t();
        offset = offset - offset % (3600 * 24) + 3600 * 24;
        QDateTime effectiveTimeUtc;
        effectiveTimeUtc.setTime_t(offset);
        ui->effectiveTimeLabel->setText(tr("This miner will take effect at %1").arg(effectiveTimeUtc.toString("yyyy-MM-dd hh:mm:ss")));
        ui->effectiveTimeLabel->show();
    }
    else
    {
        ui->totalProducedLabel->show();
        ui->totalProducedLabel2->show();
        ui->lastBlockLabel->show();
        ui->lastBlockLabel2->show();
        ui->effectiveTimeLabel->hide();
    }

#ifndef LIGHT_MODE
    if(!minerInfo.minerId.isEmpty())
    {
        if(XWCWallet::getInstance()->witnessConfig->isProductionEnabled() && XWCWallet::getInstance()->witnessConfig->getMiners().contains(minerInfo.minerId))
        {
            ui->startMineBtn->hide();
        }
        else
        {
            ui->startMineBtn->show();
        }
        ui->changeFeeBtn->show();
    }
    else
    {
        ui->startMineBtn->hide();
        ui->changeFeeBtn->hide();
    }
#endif
    showLockBalance();
}

void CitizenAccountPage::on_newCitizenBtn_clicked()
{
    CreateCitizenDialog createCitizenDialog;
    createCitizenDialog.pop();
}

void CitizenAccountPage::on_startMineBtn_clicked()
{
//#ifdef LIGHT_MODE
//    CommonDialog commonDialog(CommonDialog::OkOnly);
//    commonDialog.setText(tr("Citizens can not start mining in light mode wallet!"));
//    commonDialog.pop();

//#else

    CommonDialog commonDialog(CommonDialog::OkAndCancel);
    commonDialog.setText(tr("Sure to open the mining function of this miner account?"));
    if(commonDialog.pop())
    {
        XWCWallet::getInstance()->postRPC( "CitizenAccountPage+dump_private_key+" + ui->accountComboBox->currentText(),
                                         toJsonFormat( "dump_private_key", QJsonArray() << ui->accountComboBox->currentText() ));

    }
//#endif
}

void CitizenAccountPage::on_changeFeeBtn_clicked()
{
    if(!ui->accountComboBox->currentText().isEmpty())
    {
        ChangePayBackDialog dia(ui->accountComboBox->currentText());
        dia.exec();
    }


}

void CitizenAccountPage::pageChangeSlot(unsigned int page)
{
    for(int i = 0;i < ui->lockBalanceTableWidget->rowCount();++i)
    {
        if(i < page*ROWNUMBER)
        {
            ui->lockBalanceTableWidget->setRowHidden(i,true);
        }
        else if(page * ROWNUMBER <= i && i < page*ROWNUMBER + ROWNUMBER)
        {
            ui->lockBalanceTableWidget->setRowHidden(i,false);
        }
        else
        {
            ui->lockBalanceTableWidget->setRowHidden(i,true);
        }
    }

}
