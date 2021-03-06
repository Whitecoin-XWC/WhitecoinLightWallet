#include "ExchangeModePage.h"
#include "ui_ExchangeModePage.h"

#include "ExchangePairSelectDialog.h"
#include "dialog/ErrorResultDialog.h"
#include "dialog/TransactionResultDialog.h"
#include "AddMyExchangePairsDialog.h"
#include "control/PriceDepthWidget.h"
#include "ExchangeBalancesWidget.h"
#include "ExchangeWithdrawWidget.h"
#include "ExchangeMyOrdersWidget.h"
#include "ExchangeContractFeeDialog.h"
#include "commondialog.h"
#include "KLineWidget.h"
#include "control/OrderDepthWidget.h"

ExchangeModePage::ExchangeModePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExchangeModePage)
{
    ui->setupUi(this);
    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->sellPositionTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->sellPositionTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->sellPositionTableWidget->setFocusPolicy(Qt::NoFocus);
//    ui->sellPositionTableWidget->setFrameShape(QFrame::NoFrame);
    ui->sellPositionTableWidget->setMouseTracking(true);
    ui->sellPositionTableWidget->setShowGrid(false);//隐藏表格线
    ui->sellPositionTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->sellPositionTableWidget->horizontalHeader()->setVisible(false);
    ui->sellPositionTableWidget->verticalHeader()->setVisible(false);
    ui->sellPositionTableWidget->setColumnWidth(0,216);

    ui->buyPositionTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->buyPositionTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->buyPositionTableWidget->setFocusPolicy(Qt::NoFocus);
//    ui->buyPositionTableWidget->setFrameShape(QFrame::NoFrame);
    ui->buyPositionTableWidget->setMouseTracking(true);
    ui->buyPositionTableWidget->setShowGrid(false);//隐藏表格线
    ui->buyPositionTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->buyPositionTableWidget->horizontalHeader()->setVisible(false);
    ui->buyPositionTableWidget->verticalHeader()->setVisible(false);
    ui->buyPositionTableWidget->setColumnWidth(0,216);

    setStyleSheet("QTableView{background-color:rgb(255,255,255);border:none;border-radius:0px;font: 11px \"微软雅黑\";color:rgb(52,37,90);}"
                  "QScrollBar:vertical{border:none; width:2px;background:transparent;padding:0px 0px 0px 0px;}"
                  "QScrollBar::handle:vertical{width:2px;background:rgb(202,196,223);border-radius:0px;min-height:14px;}"
                  "QScrollBar::add-line:vertical{height:0px;}"
                  "QScrollBar::sub-line:vertical{height:0px;}"
                  "QScrollBar::sub-page:vertical {background: transparent;}"
                  "QScrollBar::add-page:vertical {background: transparent;}"
                  );

    XWCWallet::getInstance()->mainFrame->installBlurEffect(ui->widget);
    XWCWallet::getInstance()->mainFrame->installBlurEffect(ui->widget2);
    XWCWallet::getInstance()->mainFrame->installBlurEffect(ui->positionComboBox);
    XWCWallet::getInstance()->mainFrame->installBlurEffect(ui->depthComboBox);
    XWCWallet::getInstance()->mainFrame->installBlurEffect(ui->sellPositionTableWidget);
    XWCWallet::getInstance()->mainFrame->installBlurEffect(ui->buyPositionTableWidget);

    ui->KLineBtn->setCheckable(true);
    ui->KLineBtn->setChecked(false);

    separatorLabel = new QLabel(this);
    separatorLabel->setGeometry(745, 0, 1, this->height());
    //separatorLabel->setStyleSheet("background-color:rgb(209,206,220);border:none;");
  	separatorLabel->setStyleSheet("background-color:rgb(220,20,60);border:none;");
	

//    closeKLineWidget = new QLabel(this);
//    closeKLineWidget->setGeometry(separatorLabel->x(), 0, 120, 30);
//    closeKLineWidget->setStyleSheet("QLabel{background-color:white;border:1px solid rgb(209,206,220);border-top:none;}");
//    closeKLineWidget->setAlignment(Qt::AlignCenter);
//    closeKLineWidget->setText(tr("K-Line  "));

//    closeKLineWidget->raise();
    separatorLabel->raise();

//    QToolButton* closeKLineBtn = new QToolButton(closeKLineWidget);
//    closeKLineBtn->setGeometry(closeKLineWidget->width() - 25, 7, 16, 16);
//    closeKLineBtn->setStyleSheet("QToolButton{background-image:url(:/ui/wallet_ui/closeBtn.png);background-repeat: no-repeat;background-position: center;border-style: flat;}"
//                                 "QToolButton:hover{background-image:url(:/ui/wallet_ui/closeBtn_hover.png);}");
//    connect(closeKLineBtn, &QToolButton::clicked, this, &ExchangeModePage::hideKLineWidget);

//    closeKLineWidget->hide();
    separatorLabel->hide();


//    ui->widget_2->hide();
#ifndef TEST_WALLET
//    ui->KLineBtn->hide();
#endif
    init();
}

ExchangeModePage::~ExchangeModePage()
{
    delete ui;


//    XWCWallet::getInstance()->mainFrame->extendToWidth(770);


}

void ExchangeModePage::init()
{
    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(239,242,245));
    setPalette(palette);

    QStringList accounts = XWCWallet::getInstance()->accountInfoMap.keys();
    ui->accountComboBox->addItems(accounts);
    connect( ui->accountComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(onAccountComboBoxCurrentIndexChanged(const QString &)));

    if(accounts.contains(XWCWallet::getInstance()->currentAccount))
    {
        ui->accountComboBox->setCurrentText(XWCWallet::getInstance()->currentAccount);
    }
    onAccountComboBoxCurrentIndexChanged(ui->accountComboBox->currentText());
}

void ExchangeModePage::refresh()
{
    getSellOrders(XWCWallet::getInstance()->currentExchangePair);
    getBuyOrders(XWCWallet::getInstance()->currentExchangePair);
    getRecentTransactions();

//    if(klw)
//    {
//        klw->refresh();
//    }
}


void ExchangeModePage::jsonDataUpdated(QString id)
{
    if( id == "ExchangeModePage+invoke_contract_offline+getUserBalances+" + ui->accountComboBox->currentText())
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
//        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            result.prepend("{");
            result.append("}");

            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QString resultStr = parse_doucment.object().value("result").toString();
            QJsonObject resultObject = QJsonDocument::fromJson(resultStr.toLatin1()).object();
            QStringList assetSymbols = resultObject.keys();

            foreach(QString assetSymbol, assetSymbols)
            {
                QJsonObject object = resultObject.value(assetSymbol).toObject();
                ExchangeBalance balance;
                if(object.contains("locked"))   balance.locked = jsonValueToULL( object.value("locked"));
                if(object.contains("available"))   balance.available = jsonValueToULL( object.value("available"));

                XWCWallet::getInstance()->assetExchangeBalanceMap.insert(assetSymbol, balance);
            }

            onPairSelected(XWCWallet::getInstance()->currentExchangePair);
        }

        return;
    }

    if( id == "ExchangeModePage+invoke_contract_offline+getRecentTransactions")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
//        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            result.prepend("{");
            result.append("}");

            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QString resultStr = parse_doucment.object().value("result").toString();
            QJsonObject resultObject = QJsonDocument::fromJson(resultStr.toLatin1()).object();

            QString str = XWCWallet::getInstance()->currentExchangePair.first + "/" + XWCWallet::getInstance()->currentExchangePair.second;
            if(resultObject.contains(str))
            {
                QJsonObject object = resultObject.value(str).toObject();
                QString orderStr = object.value("order").toString();
                QStringList orderStrList = orderStr.split(",");
                if(orderStrList.size() == 8)
                {
                    const AssetInfo& baseAssetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.first));
                    const AssetInfo& quoteAssetInfo  = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.second));
                    unsigned long long baseAmount = orderStrList.at(6).toULongLong();
                    unsigned long long quoteAmount = orderStrList.at(7).toULongLong();
                    double price = double(quoteAmount) / qPow(10,quoteAssetInfo.precision) / baseAmount * qPow(10,baseAssetInfo.precision);
                    ui->currentPriceLabel->setText( QString::number(price,'f', XWCWallet::getInstance()->getExchangePairPrecision(XWCWallet::getInstance()->currentExchangePair))
                                                    + " " + revertERCSymbol(XWCWallet::getInstance()->currentExchangePair.second));

                    const ExchangeBalance& assetAmount2 = XWCWallet::getInstance()->assetExchangeBalanceMap.value(XWCWallet::getInstance()->currentExchangePair.second);
                    unsigned long long ableToBuyAmount = assetAmount2.available * baseAmount / quoteAmount;
                    QString str = QString::number( getBigNumberString(ableToBuyAmount, baseAssetInfo.precision).toDouble(), 'f', 4);
                    ui->ableToBuyLabel->setText( QString("%1 %2").arg(str).arg( revertERCSymbol( baseAssetInfo.symbol)));

                    const ExchangeBalance& assetAmount1 = XWCWallet::getInstance()->assetExchangeBalanceMap.value(XWCWallet::getInstance()->currentExchangePair.first);
                    unsigned long long ableToSellAmount = assetAmount1.available * quoteAmount / baseAmount;
                    QString str2 = QString::number( getBigNumberString(ableToSellAmount, quoteAssetInfo.precision).toDouble(), 'f', 4);
                    ui->ableToSellLabel->setText( QString("%1 %2").arg(str2).arg( revertERCSymbol( quoteAssetInfo.symbol)));
                }
            }


        }

        return;
    }

    if( id == "ExchangeModePage+invoke_contract_offline+getSellOrders+" +
            XWCWallet::getInstance()->currentExchangePair.first + "+" + XWCWallet::getInstance()->currentExchangePair.second)
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
//        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            result.prepend("{");
            result.append("}");

            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QString resultStr = parse_doucment.object().value("result").toString();
            QJsonArray resultArray = QJsonDocument::fromJson(resultStr.toLatin1()).array();

            sellOrdersVector.clear();
            foreach (QJsonValue v, resultArray)
            {
                QString str = v.toString();
                QStringList strList = str.split(",");
                OrderInfo orderInfo;
                orderInfo.baseAmount = strList.at(0).toULongLong();
                orderInfo.quoteAmount = strList.at(1).toULongLong();
                orderInfo.address = strList.at(2);
                orderInfo.trxId = strList.at(3);
                sellOrdersVector.append(orderInfo);
            }

//            calculateAbleToBuy();

            mergeDepth(sellOrdersVector, XWCWallet::getInstance()->currentExchangePair);

            const AssetInfo& baseAssetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.first));
            const AssetInfo& quoteAssetInfo  = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.second));

            int size = sellOrdersVector.size();
            ui->sellPositionTableWidget->setRowCount(0);
            ui->sellPositionTableWidget->setRowCount(size);

            for(int i = 0; i < size; i++)
            {
                ui->sellPositionTableWidget->setRowHeight( size - i - 1, 22);

                const OrderInfo& orderInfo = sellOrdersVector.at(i);
//                ui->sellPositionTableWidget->setItem( size - i - 1,0, new QTableWidgetItem(tr("Sell %1").arg(i + 1)));
//                ui->sellPositionTableWidget->item(size - i - 1,0)->setTextColor(QColor(44,202,148));

                double price = (double)orderInfo.quoteAmount / qPow(10,quoteAssetInfo.precision) / orderInfo.baseAmount * qPow(10,baseAssetInfo.precision);
//                QTableWidgetItem* item = new QTableWidgetItem(QString::number(price,'f', XWCWallet::getInstance()->getExchangePairPrecision(XWCWallet::getInstance()->currentExchangePair)));
//                item->setTextColor(QColor(90,115,227));
//                ui->sellPositionTableWidget->setItem( size - i - 1,1,item);

//                ui->sellPositionTableWidget->setItem( size - i - 1,2, new QTableWidgetItem( getBigNumberString(orderInfo.baseAmount, baseAssetInfo.precision)));
//                ui->sellPositionTableWidget->item(size - i - 1,2)->setTextColor(QColor(90,115,227));
//                ui->sellPositionTableWidget->item(size - i - 1,2)->setData(Qt::UserRole, orderInfo.baseAmount);

                OrderDepthWidget* odw = new OrderDepthWidget;
                odw->setType(true);
                odw->setNumber(i + 1);
                odw->setPrice(QString::number(price,'f', XWCWallet::getInstance()->getExchangePairPrecision(XWCWallet::getInstance()->currentExchangePair)));
                odw->setAmount(getBigNumberString(orderInfo.baseAmount, baseAssetInfo.precision));
                odw->setBaseAmount(orderInfo.baseAmount);
                ui->sellPositionTableWidget->setCellWidget(size - i - 1, 0, odw);
            }
            tableWidgetSetItemZebraColor(ui->sellPositionTableWidget);

            ui->sellPositionTableWidget->scrollToBottom();
        }

        return;
    }

    if( id == "ExchangeModePage+invoke_contract_offline+getBuyOrders+" +
            XWCWallet::getInstance()->currentExchangePair.first + "+" + XWCWallet::getInstance()->currentExchangePair.second)
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
//        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            result.prepend("{");
            result.append("}");

            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QString resultStr = parse_doucment.object().value("result").toString();
            QJsonArray resultArray = QJsonDocument::fromJson(resultStr.toLatin1()).array();

            buyOrdersVector.clear();
            foreach (QJsonValue v, resultArray)
            {
                QString str = v.toString();
                QStringList strList = str.split(",");
                OrderInfo orderInfo;
                orderInfo.baseAmount = strList.at(0).toULongLong();
                orderInfo.quoteAmount = strList.at(1).toULongLong();
                orderInfo.address = strList.at(2);
                orderInfo.trxId = strList.at(3);
                buyOrdersVector.append(orderInfo);
            }

//            calculateAbleToSell();

            mergeDepth(buyOrdersVector, XWCWallet::getInstance()->currentExchangePair);

            const AssetInfo& baseAssetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.first));
            const AssetInfo& quoteAssetInfo  = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.second));

            int size = buyOrdersVector.size();
            ui->buyPositionTableWidget->setRowCount(0);
            ui->buyPositionTableWidget->setRowCount(size);

            for(int i = 0; i < size; i++)
            {
                ui->buyPositionTableWidget->setRowHeight( i, 22);

                const OrderInfo& orderInfo = buyOrdersVector.at(i);
//                ui->buyPositionTableWidget->setItem(i,0, new QTableWidgetItem(tr("Buy %1").arg(i + 1)));
//                ui->buyPositionTableWidget->item(i,0)->setTextColor(QColor(235,0,94));

                double price = (double)orderInfo.quoteAmount / qPow(10,quoteAssetInfo.precision) / orderInfo.baseAmount * qPow(10,baseAssetInfo.precision);
//                QTableWidgetItem* item = new QTableWidgetItem(QString::number(price,'f', XWCWallet::getInstance()->getExchangePairPrecision(XWCWallet::getInstance()->currentExchangePair)));
//                item->setTextColor(QColor(90,115,227));
//                ui->buyPositionTableWidget->setItem(i,1,item);

//                ui->buyPositionTableWidget->setItem(i,2, new QTableWidgetItem( getBigNumberString(orderInfo.baseAmount, baseAssetInfo.precision)));
//                ui->buyPositionTableWidget->item(i,2)->setTextColor(QColor(90,115,227));
//                ui->buyPositionTableWidget->item(i,2)->setData(Qt::UserRole, orderInfo.baseAmount);

                OrderDepthWidget* odw = new OrderDepthWidget;
                odw->setType(false);
                odw->setNumber(i + 1);
                odw->setPrice(QString::number(price,'f', XWCWallet::getInstance()->getExchangePairPrecision(XWCWallet::getInstance()->currentExchangePair)));
                odw->setAmount(getBigNumberString(orderInfo.baseAmount, baseAssetInfo.precision));
                odw->setBaseAmount(orderInfo.baseAmount);
                ui->buyPositionTableWidget->setCellWidget(i, 0, odw);
            }
            tableWidgetSetItemZebraColor(ui->buyPositionTableWidget);

            showDepth();    // 后查的买单 所以在查完后显示深度
            on_positionComboBox_currentIndexChanged(ui->positionComboBox->currentIndex());
        }

        return;
    }

    if( id == "ExchangeModePage+invoke_contract+putOnBuyOrder")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Transaction of buy-order has been sent out!"));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();

            getUserBalances();
        }
        else if(result.startsWith("\"error\":"))
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to create buy-order!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        return;
    }

    if( id.startsWith("ExchangeModePage+invoke_contract_testing+putOnBuyOrder+"))
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        XWCWallet::TotalContractFee totalFee = XWCWallet::getInstance()->parseTotalContractFee(result);
        unsigned long long totalAmount = totalFee.baseAmount + ceil(totalFee.step * 1.2 * XWCWallet::getInstance()->contractFee / 100.0);

        ExchangeContractFeeDialog exchangeContractFeeDialog(totalAmount, ui->accountComboBox->currentText());
        if(exchangeContractFeeDialog.pop())
        {
            QString params = id.mid( QString("ExchangeModePage+invoke_contract_testing+putOnBuyOrder+").size());
            exchangeContractFeeDialog.updatePoundageID();
            XWCWallet::getInstance()->postRPC( "ExchangeModePage+invoke_contract+putOnBuyOrder",
                                             toJsonFormat( "invoke_contract",
                                                           QJsonArray() << ui->accountComboBox->currentText() << XWCWallet::getInstance()->currentContractFee()
                                                           << ceil(totalFee.step * 1.2) << EXCHANGE_MODE_CONTRACT_ADDRESS
                                                           << "putOnBuyOrder"  << params));
        }
    }

    if( id == "ExchangeModePage+invoke_contract+putOnSellOrder")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Transaction of sell-order has been sent out!"));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();

            getUserBalances();
        }
        else if(result.startsWith("\"error\":"))
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to create sell-order!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        return;
    }

    if( id.startsWith("ExchangeModePage+invoke_contract_testing+putOnSellOrder+"))
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        XWCWallet::TotalContractFee totalFee = XWCWallet::getInstance()->parseTotalContractFee(result);
        unsigned long long totalAmount = totalFee.baseAmount + ceil(totalFee.step * 1.2 * XWCWallet::getInstance()->contractFee / 100.0);

        ExchangeContractFeeDialog exchangeContractFeeDialog(totalAmount, ui->accountComboBox->currentText());
        if(exchangeContractFeeDialog.pop())
        {
            QString params = id.mid( QString("ExchangeModePage+invoke_contract_testing+putOnSellOrder+").size());
            exchangeContractFeeDialog.updatePoundageID();
            XWCWallet::getInstance()->postRPC( "ExchangeModePage+invoke_contract+putOnSellOrder",
                                             toJsonFormat( "invoke_contract",
                                                           QJsonArray() << ui->accountComboBox->currentText() << XWCWallet::getInstance()->currentContractFee()
                                                           << ceil(totalFee.step * 1.2) << EXCHANGE_MODE_CONTRACT_ADDRESS
                                                           << "putOnSellOrder"  << params));
        }
    }
}

void ExchangeModePage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(239,242,245),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(239,242,245),Qt::SolidPattern));

    painter.drawRect(rect());
}

void ExchangeModePage::on_favoriteMarketBtn_clicked()
{
    ExchangePairSelectDialog dialog("");
    connect(&dialog, &ExchangePairSelectDialog::pairSelected, this, &ExchangeModePage::onPairSelected);
    connect(&dialog, &ExchangePairSelectDialog::pairSelected, this, &ExchangeModePage::pairChanged);
    connect(&dialog, &ExchangePairSelectDialog::addFavoriteClicked, this, &ExchangeModePage::onAddFavoriteClicked);
    dialog.move(ui->favoriteMarketBtn->mapToGlobal( QPoint(ui->favoriteMarketBtn->width() / 2 - dialog.width() / 2,ui->favoriteMarketBtn->height())));

    dialog.exec();
}

void ExchangeModePage::on_marketBtn1_clicked()
{
    ExchangePairSelectDialog dialog("XWC");
    connect(&dialog, &ExchangePairSelectDialog::pairSelected, this, &ExchangeModePage::onPairSelected);
    connect(&dialog, &ExchangePairSelectDialog::pairSelected, this, &ExchangeModePage::pairChanged);
    dialog.move(ui->marketBtn1->mapToGlobal( QPoint(ui->marketBtn1->width() / 2 - dialog.width() / 2,ui->marketBtn1->height())));

    dialog.exec();
}

//void ExchangeModePage::on_marketBtn2_clicked()
//{
//    ExchangePairSelectDialog dialog("BTC");
//    connect(&dialog, &ExchangePairSelectDialog::pairSelected, this, &ExchangeModePage::onPairSelected);
//    dialog.move(ui->marketBtn2->mapToGlobal( QPoint(ui->marketBtn2->width() / 2 - dialog.width() / 2,ui->marketBtn2->height())));

//    dialog.exec();
//}

//void ExchangeModePage::on_marketBtn3_clicked()
//{
//    ExchangePairSelectDialog dialog("ERCPAX");
//    connect(&dialog, &ExchangePairSelectDialog::pairSelected, this, &ExchangeModePage::onPairSelected);
//    connect(&dialog, &ExchangePairSelectDialog::pairSelected, this, &ExchangeModePage::pairChanged);
//    dialog.move(ui->marketBtn3->mapToGlobal( QPoint(ui->marketBtn3->width() / 2 - dialog.width() / 2,ui->marketBtn3->height())));
//
//    dialog.exec();
//}

void ExchangeModePage::onPairSelected(const ExchangePair &_pair)
{
    double balance_available_first = 0.0;
    double balance_available_second = 0.0;

    XWCWallet::getInstance()->currentExchangePair = _pair;
    ui->currentPairLabel->setText( QString("%1 / %2").arg(revertERCSymbol(_pair.first)).arg(revertERCSymbol(_pair.second)));

    ui->buyPriceLabel->setText(tr("Price (%1)").arg(revertERCSymbol( _pair.second)));
    ui->sellPriceLabel->setText(tr("Price (%1)").arg(revertERCSymbol( _pair.second)));
    ui->buyAmountLabel->setText(tr("Amount (%1)").arg(revertERCSymbol( _pair.first)));
    ui->sellAmountLabel->setText(tr("Amount (%1)").arg(revertERCSymbol( _pair.first)));

    const AssetInfo& assetInfo1 = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(_pair.first));
    const AssetInfo& assetInfo2 = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(_pair.second));
    const ExchangeBalance& assetAmount1 = XWCWallet::getInstance()->assetExchangeBalanceMap.value(_pair.first);
    const ExchangeBalance& assetAmount2 = XWCWallet::getInstance()->assetExchangeBalanceMap.value(_pair.second);

    balance_available_first = assetAmount1.available / qPow(10,ASSET_PRECISION);
    balance_available_second = assetAmount2.available ;

    ui->availableLabel1->setText( QString("%1 %2").arg(getBigNumberString(balance_available_second, assetInfo2.precision)).arg(revertERCSymbol( assetInfo2.symbol)));
    ui->availableLabel2->setText( QString("%1 %2").arg(getBigNumberString(balance_available_first, assetInfo1.precision)).arg(revertERCSymbol( assetInfo1.symbol)));

//  ui->availableLabel1->setText( QString("%1 %2").arg(getBigNumberString(assetAmount2.available, assetInfo2.precision)).arg(revertERCSymbol( assetInfo2.symbol)));
//  ui->availableLabel2->setText( QString("%1 %2").arg(getBigNumberString(assetAmount1.available, assetInfo1.precision)).arg(revertERCSymbol( assetInfo1.symbol)));

    ui->ableToBuyLabel->setText( revertERCSymbol( assetInfo1.symbol));
    ui->ableToSellLabel->setText( revertERCSymbol( assetInfo2.symbol));

    QRegExp rx1(QString("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg( XWCWallet::getInstance()->getExchangePairPrecision(_pair)));
    QRegExpValidator* validator = new QRegExpValidator(rx1, this);
    ui->sellPriceLineEdit->setValidator(validator);
    ui->sellPriceLineEdit->clear();
    ui->buyPriceLineEdit->setValidator(validator);
    ui->buyPriceLineEdit->clear();

    QRegExp rx2(QString("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg( XWCWallet::getInstance()->getExchangeAmountPrecision(_pair.first)));
    QRegExpValidator* validator2 = new QRegExpValidator(rx2, this);
    ui->sellAmountLineEdit->setValidator(validator2);
    ui->sellAmountLineEdit->clear();
    ui->buyAmountLineEdit->setValidator(validator2);
    ui->buyAmountLineEdit->clear();

    ui->buyAmountLineEdit->setEnabled(true);
    ui->buyPriceLineEdit->setEnabled(true);
    ui->sellAmountLineEdit->setEnabled(true);
    ui->sellPriceLineEdit->setEnabled(true);

    ui->estimatedBuyLabel->setText( tr("<html><head/><body><p>Estimated: <span style=\" color:#eb005e;\">%1</span> %2</p></body></html>")
                                    .arg(QString::number( 0, 'f', assetInfo2.precision))
                                    .arg( revertERCSymbol( _pair.second))
                                    );

    ui->estimateSellLabel->setText( tr("<html><head/><body><p>Estimated: <span style=\" color:#eb005e;\">%1</span> %2</p></body></html>")
                                    .arg(QString::number( 0, 'f', assetInfo2.precision))
                                    .arg( revertERCSymbol( _pair.second))
                                    );

    refresh();
}

void ExchangeModePage::onAddFavoriteClicked()
{
    AddMyExchangePairsDialog dialog;
    dialog.pop();
}

void ExchangeModePage::showDepth()
{
    unsigned long long benchmark = getBenchmarkOfOrders();

    unsigned long long amount = 0;
    for(int i = 0; i < ui->sellPositionTableWidget->rowCount(); i++)
    {
//        unsigned long long baseAmount = ui->sellPositionTableWidget->item(i,2)->data(Qt::UserRole).toULongLong();
//        PriceDepthWidget* pdw = new PriceDepthWidget;
//        pdw->setType(0);
//        pdw->setLength( (double)baseAmount / maxAmount);
//        ui->sellPositionTableWidget->setCellWidget( i, 3, pdw);

        OrderDepthWidget* odw = static_cast<OrderDepthWidget*>(ui->sellPositionTableWidget->cellWidget(ui->sellPositionTableWidget->rowCount() - i - 1,0));
        if(odw)
        {
            amount += odw->baseAmount;
            odw->setLength( (double)amount / benchmark);
            qDebug() << odw->baseAmount << amount << benchmark;
        }
    }

    amount = 0;
    for(int i = 0; i < ui->buyPositionTableWidget->rowCount(); i++)
    {
//        unsigned long long baseAmount = ui->buyPositionTableWidget->item(i,2)->data(Qt::UserRole).toULongLong();
//        PriceDepthWidget* pdw = new PriceDepthWidget;
//        pdw->setType(1);
//        pdw->setLength( (double)baseAmount / maxAmount);
//        ui->buyPositionTableWidget->setCellWidget( i, 3, pdw);

        OrderDepthWidget* odw = static_cast<OrderDepthWidget*>(ui->buyPositionTableWidget->cellWidget(i,0));
        if(odw)
        {
            amount += odw->baseAmount;
            odw->setLength( (double)amount / benchmark);
            qDebug() << odw->baseAmount << amount << benchmark;
        }
    }
}

void ExchangeModePage::showPosition(int num)
{
    for(int i = 0; i < ui->sellPositionTableWidget->rowCount(); i++)
    {
        ui->sellPositionTableWidget->setRowHidden(i, i < ui->sellPositionTableWidget->rowCount() - num);
    }

    for(int i = 0; i < ui->buyPositionTableWidget->rowCount(); i++)
    {
        ui->buyPositionTableWidget->setRowHidden(i, i >= num);
    }
}

void ExchangeModePage::showSeparator()
{
//    closeKLineWidget->raise();
    separatorLabel->raise();
    separatorLabel->show();
//    closeKLineWidget->show();
}

void ExchangeModePage::hideSeparator()
{
    separatorLabel->hide();
//    closeKLineWidget->hide();
}

void ExchangeModePage::showKLineWidget_____()
{
    klw = new KLineWidget(this);
    connect(klw, &KLineWidget::pairChanged, this, &ExchangeModePage::onPairSelected);
    connect(this, &ExchangeModePage::pairChanged, klw, &KLineWidget::onPairChanged);
    connect(klw, &KLineWidget::onClose, this, &ExchangeModePage::hideKLineWidget);

    klw->setAttribute(Qt::WA_DeleteOnClose);
    klw->setWindowTitle("K-Line");
    klw->show();
    klw->raise();
//        klw->move( XWCWallet::getInstance()->mainFrame->x() + XWCWallet::getInstance()->mainFrame->width(), XWCWallet::getInstance()->mainFrame->y());

    klw->move(this->width(), 0);
    this->setGeometry(this->x(), this->y(), 1490, this->height());
    XWCWallet::getInstance()->mainFrame->extendToWidth(1490);

    showSeparator();
}

void ExchangeModePage::hideKLineWidget()
{
    if(klw)
    {
        klw->close();
        klw = nullptr;
    }

    this->setGeometry(this->x(), this->y(), 770, this->height());
    XWCWallet::getInstance()->mainFrame->extendToWidth(770);

    hideSeparator();

    ui->KLineBtn->setChecked(false);
}

// on_accountComboBox_currentIndexChanged is the slot function for accountComboBox
void ExchangeModePage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    XWCWallet::getInstance()->currentAccount = ui->accountComboBox->currentText();
    XWCWallet::getInstance()->assetExchangeBalanceMap.clear();

    ui->buyAmountLineEdit->setEnabled(false);
    ui->buyPriceLineEdit->setEnabled(false);
    ui->sellAmountLineEdit->setEnabled(false);
    ui->sellPriceLineEdit->setEnabled(false);

    getUserBalances();
}

void ExchangeModePage::onAccountComboBoxCurrentIndexChanged(const QString &arg1)
{
    XWCWallet::getInstance()->currentAccount = ui->accountComboBox->currentText();
    XWCWallet::getInstance()->assetExchangeBalanceMap.clear();

    ui->buyAmountLineEdit->setEnabled(false);
    ui->buyPriceLineEdit->setEnabled(false);
    ui->sellAmountLineEdit->setEnabled(false);
    ui->sellPriceLineEdit->setEnabled(false);

    getUserBalances();
}

void ExchangeModePage::getUserBalances()
{
    AccountInfo accountInfo = XWCWallet::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText());
    //XWCWallet::getInstance()->postRPC( "ExchangeModePage+invoke_contract_offline+getUserBalances+" + ui->accountComboBox->currentText(),
    //                                 toJsonFormat( "invoke_contract_offline",
    //                                 QJsonArray() << ui->accountComboBox->currentText() << EXCHANGE_MODE_CONTRACT_ADDRESS
    //                                               << "getUserBalances"  << accountInfo.address));
												   
    XWCWallet::getInstance()->postRPC( "ExchangeModePage+invoke_contract_offline+getUserBalances+" + ui->accountComboBox->currentText(),
                                     toJsonFormat( "invoke_contract_offline",
                                     QJsonArray() << ui->accountComboBox->currentText() << EXCHANGE_MODE_CONTRACT_ADDRESS
                                                   << "getUserBalances"  << accountInfo.address));
}

void ExchangeModePage::getRecentTransactions()
{
    XWCWallet::getInstance()->postRPC( "ExchangeModePage+invoke_contract_offline+getRecentTransactions",
                                     toJsonFormat( "invoke_contract_offline",
                                     QJsonArray() << ui->accountComboBox->currentText() << EXCHANGE_MODE_CONTRACT_ADDRESS
                                                   << "getRecentTransactions"  << ""));
}

void ExchangeModePage::getSellOrders(const ExchangePair &_pair)
{
    QString params = QString("%1,%2,%3,%4,%5").arg(_pair.first).arg(_pair.second).arg(100).arg(0).arg("sell");
    XWCWallet::getInstance()->postRPC( "ExchangeModePage+invoke_contract_offline+getSellOrders+" + _pair.first + "+" + _pair.second,
                                     toJsonFormat( "invoke_contract_offline",
                                     QJsonArray() << ui->accountComboBox->currentText() << EXCHANGE_MODE_CONTRACT_ADDRESS
                                                   << "getOrders"  << params));

}

unsigned long long ExchangeModePage::getMaxOrderAmount()
{
    unsigned long long result = 0;
    foreach (const OrderInfo& order, sellOrdersVector)
    {
        if(order.baseAmount > result)   result = order.baseAmount;
    }
    foreach (const OrderInfo& order, buyOrdersVector)
    {
        if(order.baseAmount > result)   result = order.baseAmount;
    }

    return result;
}

unsigned long long ExchangeModePage::getBenchmarkOfOrders()
{
    unsigned long long sellSumAmount = 0;
    foreach (const OrderInfo& order, sellOrdersVector)
    {
        sellSumAmount += order.baseAmount;
    }

    unsigned long long buySumAmount = 0;
    foreach (const OrderInfo& order, buyOrdersVector)
    {
        buySumAmount += order.baseAmount;
    }

    return sellSumAmount > buySumAmount ? sellSumAmount : buySumAmount;
}


void ExchangeModePage::getBuyOrders(const ExchangePair &_pair)
{
    QString params = QString("%1,%2,%3,%4,%5").arg(_pair.first).arg(_pair.second).arg(100).arg(0).arg("buy");
    XWCWallet::getInstance()->postRPC( "ExchangeModePage+invoke_contract_offline+getBuyOrders+" + _pair.first + "+" + _pair.second,
                                     toJsonFormat( "invoke_contract_offline",
                                     QJsonArray() << ui->accountComboBox->currentText() << EXCHANGE_MODE_CONTRACT_ADDRESS
                                                   << "getOrders"  << params));
}

void ExchangeModePage::mergeDepth(QVector<OrderInfo> &orders, const ExchangePair& pair)
{
    const AssetInfo& baseAssetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(pair.first));
    const AssetInfo& quoteAssetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(pair.second));
    int precision = XWCWallet::getInstance()->getExchangePairPrecision(pair);
    QString lastPrice;
    QVector<OrderInfo> mergedOrders;
    for(int i = 0; i < orders.size(); i++)
    {
        OrderInfo orderInfo = orders.at(i);

        double price = (double)orderInfo.quoteAmount / qPow(10,quoteAssetInfo.precision) / orderInfo.baseAmount * qPow(10,baseAssetInfo.precision);
        QString priceStr = QString::number( price, 'f', precision);
        if(priceStr == lastPrice)
        {
            mergedOrders.last().baseAmount += orderInfo.baseAmount;
            mergedOrders.last().quoteAmount += orderInfo.quoteAmount;
        }
        else
        {
            mergedOrders.append(orderInfo);
            lastPrice = priceStr;
        }
    }

    orders = mergedOrders;
}

void ExchangeModePage::calculateAbleToBuy()
{
    if(sellOrdersVector.size() < 1)
    {
        ui->ableToBuyLabel->setText("--");
    }
    else
    {
        OrderInfo lowestOrder = sellOrdersVector.first();


        const AssetInfo& assetInfo1 = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.first));
        const ExchangeBalance& assetAmount2 = XWCWallet::getInstance()->assetExchangeBalanceMap.value(XWCWallet::getInstance()->currentExchangePair.second);

        unsigned long long ableToBuyAmount = assetAmount2.available * lowestOrder.baseAmount / lowestOrder.quoteAmount;
        QString str = QString::number( getBigNumberString(ableToBuyAmount, assetInfo1.precision).toDouble(), 'f', 4);
        ui->ableToBuyLabel->setText( QString("%1 %2").arg(str).arg( revertERCSymbol( assetInfo1.symbol)));

    }
}

void ExchangeModePage::calculateAbleToSell()
{
    if(buyOrdersVector.size() < 1)
    {
        ui->ableToSellLabel->setText("--");
    }
    else
    {
        OrderInfo highestOrder = buyOrdersVector.first();

        const AssetInfo& assetInfo2 = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.second));
        const ExchangeBalance& assetAmount1 = XWCWallet::getInstance()->assetExchangeBalanceMap.value(XWCWallet::getInstance()->currentExchangePair.first);

        unsigned long long ableToSellAmount = assetAmount1.available * highestOrder.quoteAmount / highestOrder.baseAmount;
        QString str = QString::number( getBigNumberString(ableToSellAmount, assetInfo2.precision).toDouble(), 'f', 4);
        ui->ableToSellLabel->setText( QString("%1 %2").arg(str).arg( revertERCSymbol( assetInfo2.symbol)));
    }
}

void ExchangeModePage::estimateBuyValue()
{
    double price = ui->buyPriceLineEdit->text().toDouble();
    double amount = ui->buyAmountLineEdit->text().toDouble();
    double estimatedValue = price * amount;

    const AssetInfo& assetInfo2 = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.second));
    double available = getBigNumberString( XWCWallet::getInstance()->assetExchangeBalanceMap.value( XWCWallet::getInstance()->currentExchangePair.second).available, assetInfo2.precision).toDouble();

    int precision = XWCWallet::getInstance()->getExchangePairPrecision( XWCWallet::getInstance()->currentExchangePair) + XWCWallet::getInstance()->getExchangeAmountPrecision(XWCWallet::getInstance()->currentExchangePair.first);
    if(precision > assetInfo2.precision)    precision = assetInfo2.precision;
    if(available >= estimatedValue)
    {
        ui->estimatedBuyLabel->setText( tr("<html><head/><body><p>Estimated: <span style=\" color:#eb005e;\">%1</span> %2</p></body></html>")
                                        .arg(QString::number( roundDown(price * amount, precision), 'f', precision))
                                        .arg( revertERCSymbol( XWCWallet::getInstance()->currentExchangePair.second))
                                        );
    }
    else
    {
        double maxAmount = roundDown( available / price, XWCWallet::getInstance()->getExchangeAmountPrecision(XWCWallet::getInstance()->currentExchangePair.first));
        QString maxAmountStr = QString::number( maxAmount, 'f', XWCWallet::getInstance()->getExchangeAmountPrecision(XWCWallet::getInstance()->currentExchangePair.first));

        ui->buyAmountLineEdit->setText(maxAmountStr);

        ui->estimatedBuyLabel->setText( tr("<html><head/><body><p>Estimated: <span style=\" color:#eb005e;\">%1</span> %2</p></body></html>")
                                        .arg(QString::number( roundDown(price * maxAmountStr.toDouble(), precision), 'f', precision))
                                        .arg( revertERCSymbol( XWCWallet::getInstance()->currentExchangePair.second))
                                        );
    }

}

void ExchangeModePage::estimateSellValue()
{
    double price = ui->sellPriceLineEdit->text().toDouble();
    double amount = ui->sellAmountLineEdit->text().toDouble();

    const AssetInfo& assetInfo1 = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.first));
    const AssetInfo& assetInfo2 = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.second));
    double available = getBigNumberString( XWCWallet::getInstance()->assetExchangeBalanceMap.value( XWCWallet::getInstance()->currentExchangePair.first).available, assetInfo1.precision).toDouble();

    int precision = XWCWallet::getInstance()->getExchangePairPrecision( XWCWallet::getInstance()->currentExchangePair) + XWCWallet::getInstance()->getExchangeAmountPrecision(XWCWallet::getInstance()->currentExchangePair.first);
    if(precision > assetInfo2.precision)    precision = assetInfo2.precision;
    if(available >= amount)
    {
        qDebug() <<"aaaaaaaaaaaaaaaaaa " <<price << amount << precision << price * amount << roundDown(price * amount, precision);
        ui->estimateSellLabel->setText( tr("<html><head/><body><p>Estimated: <span style=\" color:#2cca94;\">%1</span> %2</p></body></html>")
                                        .arg(QString::number( roundDown(price * amount, precision), 'f', precision))
                                        .arg( revertERCSymbol( XWCWallet::getInstance()->currentExchangePair.second))
                                        );
    }
    else
    {
        QString avaiableStr = QString::number( roundDown( available), 'f', XWCWallet::getInstance()->getExchangeAmountPrecision(XWCWallet::getInstance()->currentExchangePair.first));
        ui->sellAmountLineEdit->setText(avaiableStr);

        ui->estimateSellLabel->setText( tr("<html><head/><body><p>Estimated: <span style=\" color:#eb005e;\">%1</span> %2</p></body></html>")
                                        .arg(QString::number( roundDown(price * avaiableStr.toDouble(), precision), 'f', precision))
                                        .arg( revertERCSymbol( XWCWallet::getInstance()->currentExchangePair.second))
                                        );
    }
}


void ExchangeModePage::on_buyBtn_clicked()
{
    if(ui->buyPriceLineEdit->text().toDouble() <= 0 || ui->buyAmountLineEdit->text().toDouble() <= 0)   return;

    hideKLineWidget();

    const AssetInfo& baseAssetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.first));
    const AssetInfo& quoteAssetInfo  = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.second));

    QString baseAmountStr = decimalToIntegerStr(ui->buyAmountLineEdit->text(), baseAssetInfo.precision);
    QString quoteAmountStr = decimalToIntegerStr(QString::number(ui->buyAmountLineEdit->text().toDouble() * ui->buyPriceLineEdit->text().toDouble(), 'f', quoteAssetInfo.precision), quoteAssetInfo.precision);

    PairInfo pairInfo = XWCWallet::getInstance()->pairInfoMap.value(XWCWallet::getInstance()->currentExchangePair);
    if(baseAmountStr.toULongLong() < pairInfo.leastBaseAmount || quoteAmountStr.toULongLong() < pairInfo.leastQuoteAmount)
    {
        CommonDialog dialog(CommonDialog::OkOnly);
        dialog.setText(tr("The order amount is too small!"));
        dialog.pop();
        return;
    }

    QString params = QString("%1,%2,%3,%4").arg(XWCWallet::getInstance()->currentExchangePair.first).arg(baseAmountStr).arg(XWCWallet::getInstance()->currentExchangePair.second).arg(quoteAmountStr);
//    XWCWallet::getInstance()->postRPC( "ExchangeModePage+invoke_contract+putOnBuyOrder",
//                                     toJsonFormat( "invoke_contract",
//                                     QJsonArray() << ui->accountComboBox->currentText() << "0.00001" << 1000000 << EXCHANGE_MODE_CONTRACT_ADDRESS
//                                                   << "putOnBuyOrder"  << params));

    XWCWallet::getInstance()->postRPC( "ExchangeModePage+invoke_contract_testing+putOnBuyOrder+" + params,
                                     toJsonFormat( "invoke_contract_testing",
                                     QJsonArray() << ui->accountComboBox->currentText() << EXCHANGE_MODE_CONTRACT_ADDRESS
                                                   << "putOnBuyOrder"  << params));
}

void ExchangeModePage::on_sellBtn_clicked()
{
    if(ui->sellPriceLineEdit->text().toDouble() <= 0 || ui->sellAmountLineEdit->text().toDouble() <= 0)   return;

    hideKLineWidget();

    const AssetInfo& baseAssetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.first));
    const AssetInfo& quoteAssetInfo  = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId(XWCWallet::getInstance()->currentExchangePair.second));

    QString baseAmountStr = decimalToIntegerStr(ui->sellAmountLineEdit->text(), baseAssetInfo.precision);
    QString quoteAmountStr = decimalToIntegerStr(QString::number(ui->sellAmountLineEdit->text().toDouble() * ui->sellPriceLineEdit->text().toDouble(), 'f', quoteAssetInfo.precision), quoteAssetInfo.precision);

    PairInfo pairInfo = XWCWallet::getInstance()->pairInfoMap.value(XWCWallet::getInstance()->currentExchangePair);
    if(baseAmountStr.toULongLong() < pairInfo.leastBaseAmount || quoteAmountStr.toULongLong() < pairInfo.leastQuoteAmount)
    {
        CommonDialog dialog(CommonDialog::OkOnly);
        dialog.setText(tr("The order amount is too small!"));
        dialog.pop();
        return;
    }

    QString params = QString("%1,%2,%3,%4").arg(XWCWallet::getInstance()->currentExchangePair.first).arg(baseAmountStr).arg(XWCWallet::getInstance()->currentExchangePair.second).arg(quoteAmountStr);
//    XWCWallet::getInstance()->postRPC( "ExchangeModePage+invoke_contract+putOnSellOrder",
//                                     toJsonFormat( "invoke_contract",
//                                     QJsonArray() << ui->accountComboBox->currentText() << "0.00001" << 1000000 << EXCHANGE_MODE_CONTRACT_ADDRESS
//                                                   << "putOnSellOrder"  << params));

    XWCWallet::getInstance()->postRPC( "ExchangeModePage+invoke_contract_testing+putOnSellOrder+" + params,
                                     toJsonFormat( "invoke_contract_testing",
                                     QJsonArray() << ui->accountComboBox->currentText() << EXCHANGE_MODE_CONTRACT_ADDRESS
                                                   << "putOnSellOrder"  << params));
}




void ExchangeModePage::on_sellPositionTableWidget_cellPressed(int row, int column)
{
    OrderDepthWidget* odw = static_cast<OrderDepthWidget*>(ui->sellPositionTableWidget->cellWidget(row,0));
    if(odw)
    {
        ui->sellPriceLineEdit->setText( odw->price);
        ui->buyPriceLineEdit->setText( odw->price);
        on_sellPriceLineEdit_textEdited(ui->sellPriceLineEdit->text());
        on_buyPriceLineEdit_textEdited(ui->buyPriceLineEdit->text());
    }
}

void ExchangeModePage::on_buyPositionTableWidget_cellPressed(int row, int column)
{
    OrderDepthWidget* odw = static_cast<OrderDepthWidget*>(ui->buyPositionTableWidget->cellWidget(row,0));
    if(odw)
    {
        ui->sellPriceLineEdit->setText( odw->price);
        ui->buyPriceLineEdit->setText( odw->price);
        on_sellPriceLineEdit_textEdited(ui->sellPriceLineEdit->text());
        on_buyPriceLineEdit_textEdited(ui->buyPriceLineEdit->text());
    }
}

void ExchangeModePage::on_buyPriceLineEdit_textEdited(const QString &arg1)
{
    estimateBuyValue();
}

void ExchangeModePage::on_buyAmountLineEdit_textEdited(const QString &arg1)
{
    estimateBuyValue();
}

void ExchangeModePage::on_sellPriceLineEdit_textEdited(const QString &arg1)
{
    estimateSellValue();
}

void ExchangeModePage::on_sellAmountLineEdit_textEdited(const QString &arg1)
{
    estimateSellValue();
}

void ExchangeModePage::on_balanceBtn_clicked()
{
    hideKLineWidget();

    Q_EMIT backBtnVisible(true);

    ExchangeBalancesWidget* exchangeBalancesWidget = new ExchangeBalancesWidget(this);
//    connect(exchangeBalancesWidget, SIGNAL(back()), this, SLOT(refresh()));
//    connect(exchangeBalancesWidget, &ExchangeBalancesWidget::back, [this]{ Q_EMIT backBtnVisible(false);});
    exchangeBalancesWidget->setAttribute(Qt::WA_DeleteOnClose);
    exchangeBalancesWidget->show();
    exchangeBalancesWidget->raise();
}

void ExchangeModePage::on_myOrdersBtn_clicked()
{
    hideKLineWidget();

    Q_EMIT backBtnVisible(true);

    ExchangeMyOrdersWidget* exchangeMyOrdersWidget = new ExchangeMyOrdersWidget(this);
    exchangeMyOrdersWidget->setAttribute(Qt::WA_DeleteOnClose);
    exchangeMyOrdersWidget->show();
    exchangeMyOrdersWidget->raise();
}

void ExchangeModePage::on_positionComboBox_currentIndexChanged(int index)
{
    if(index == 0 || index == 1)
    {
        showPosition(5);
    }
    else if(index == 2)
    {
        showPosition(10);
    }
    else if(index == 3)
    {
        showPosition(20);
    }
}

void ExchangeModePage::on_KLineBtn_clicked____()
{
	showKLineWidget();
}

void ExchangeModePage::showKLineWidget()
{
    Q_EMIT backBtnVisible(true);

    KLineWidget* klw = new KLineWidget(this);
    klw->setAttribute(Qt::WA_DeleteOnClose);
    //klw->show();
    //klw->raise();

    klw->hide();
}

void ExchangeModePage::on_KLineBtn_clicked()
{
	showKLineWidget();
}

void ExchangeModePage::showKLineWidget()
{
    Q_EMIT backBtnVisible(true);

    KLineWidget* klw = new KLineWidget(this);
    klw->setAttribute(Qt::WA_DeleteOnClose);
    //klw->show();
    //klw->raise();

    klw->hide();
}
void ExchangeModePage::on_withdrawBtn_clicked()
{
    hideKLineWidget();

    Q_EMIT backBtnVisible(true);

    ExchangeWithdrawWidget* exchangeWithdrawWidget = new ExchangeWithdrawWidget(this);
    exchangeWithdrawWidget->setAttribute(Qt::WA_DeleteOnClose);
    exchangeWithdrawWidget->show();
    exchangeWithdrawWidget->raise();
}