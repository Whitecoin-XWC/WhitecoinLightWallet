#include "depositexchangecontractdialog.h"
#include "ui_depositexchangecontractdialog.h"

#include "wallet.h"
#include "commondialog.h"
#include "FeeChooseWidget.h"
#include "dialog/ErrorResultDialog.h"
#include "dialog/TransactionResultDialog.h"
#include <QtCore/qmath.h>

DepositExchangeContractDialog::DepositExchangeContractDialog(bool _isExchangeMode, QWidget *parent) :
    QDialog(parent),
    isExchangeMode(_isExchangeMode),
    ui(new Ui::DepositExchangeContractDialog)
{
    ui->setupUi(this);

    setParent(XWCWallet::getInstance()->mainFrame->containerWidget);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet(BACKGROUNDWIDGET_STYLE);
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet(CONTAINERWIDGET_STYLE);

    ui->okBtn->setStyleSheet(OKBTN_STYLE);
    ui->cancelBtn->setStyleSheet(CANCELBTN_STYLE);
    ui->closeBtn->setStyleSheet(CLOSEBTN_STYLE);
    ui->assetComboBox->setStyleSheet(COMBOBOX_BORDER_STYLE);

    feeChoose = new FeeChooseWidget(0,XWCWallet::getInstance()->feeType);
    ui->stackedWidget->addWidget(feeChoose);
    feeChoose->resize(ui->stackedWidget->size());
    ui->stackedWidget->setCurrentIndex(0);


    connect( XWCWallet::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    init();
}

DepositExchangeContractDialog::~DepositExchangeContractDialog()
{
    delete ui;
}

void DepositExchangeContractDialog::pop()
{
    move(0,0);
    exec();
}

void DepositExchangeContractDialog::init()
{
    ui->accountNameLabel->setText(XWCWallet::getInstance()->currentAccount);

    QStringList assetIds = XWCWallet::getInstance()->assetInfoMap.keys();
    foreach (QString assetId, assetIds)
    {
        ui->assetComboBox->addItem( revertERCSymbol( XWCWallet::getInstance()->assetInfoMap.value(assetId).symbol), assetId);
    }
    
    QStringList assets = XWCWallet::getInstance()->getAllExchangeAssets();
    int size = assets.size();
    for(int i = 0; i < size; i++)
    {
        QString symbol = assets.at(i);
        ui->assetComboBox->addItem( symbol);

    QStringList assets = XWCWallet::getInstance()->getAllExchangeAssets();
    int size = assets.size();
    for(int i = 0; i < size; i++)
    {
        QString symbol = assets.at(i);
        ui->assetComboBox->addItem( symbol);

    }
}

void DepositExchangeContractDialog::setCurrentAsset(QString _assetSymbol)
{
    ui->assetComboBox->setCurrentText( revertERCSymbol( _assetSymbol));
}

void DepositExchangeContractDialog::jsonDataUpdated(QString id)
{
    if( id.startsWith( "id-unlock-DepositExchangeContractDialog") )
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if( result == "\"result\":null")
        {

            QString isExchangeModeString = isExchangeMode ? "EXCHANGE_MODE" : "XWCWallet-GetExchangeContractAddress";

            QString contractAddress = isExchangeMode ? EXCHANGE_MODE_CONTRACT_ADDRESS
                                                     : XWCWallet::getInstance()->getExchangeContractAddress(ui->accountNameLabel->text());

            feeChoose->updatePoundageID();
            if (ui->assetComboBox->currentText() == "XWC") {

            XWCWallet::getInstance()->postRPC( "id-transfer_to_contract", toJsonFormat( "transfer_to_contract",
                                                                                   QJsonArray() << ui->accountNameLabel->text() << contractAddress
                                                                                   << ui->amountLineEdit->text() << getRealAssetSymbol( ui->assetComboBox->currentText())
                                                                                   << "deposit to exchange contract"
                                                                                   << XWCWallet::getInstance()->currentContractFee() << stepCount
                                                                                   << true
                                                                                   ));

            }
            else //????????????????????????XDTT WNTT?????????????????????????????????XWC Exchange ?????????????????????????????? ??????????????????XDTT WNTT???????????????
            {
                if (ui->assetComboBox->currentText() == "XDTT")
                {
                    //qlonglong tmp = ui->amountLineEdit->text().toInt();
                    double tmp = ui->amountLineEdit->text().toDouble();
                    tmp = tmp * qPow(10,ASSET_PRECISION);

                    /*transactionResultDialog.setInfoText(tr("tmp * qPow(10,ASSET_PRECISION)"));
                    transactionResultDialog.setDetailText(QString::number(tmp));
                    transactionResultDialog.pop();*/

                    QString params = QString("%1,%2,%3").arg(EXCHANGE_MODE_XDTT_TOKEN_OFFICIALWALLET).arg(QString::number(tmp,10,8)).arg("memo");
                    //transactionResultDialog.setInfoText(tr("QString(%1,%2,%3)"));
                    //transactionResultDialog.setDetailText(params);
                    //transactionResultDialog.pop();

                    XWCWallet::getInstance()->postRPC( "id-transfer_to_contract_token", toJsonFormat( "invoke_contract",
                                                         QJsonArray() << ui->accountNameLabel->text()
                                                         << XWCWallet::getInstance()->currentContractFee() << stepCount
                                                         << EXCHANGE_MODE_XDTT_TOKENCONTRACT_ADDRESS
                                                         << "transfer"  << params));
                }
                else if (ui->assetComboBox->currentText() == "WNTT")
                {
                    //qlonglong tmp = ui->amountLineEdit->text().toInt();
                    double tmp = ui->amountLineEdit->text().toDouble();
                    tmp = tmp * qPow(10,ASSET_PRECISION);
                    /*transactionResultDialog.setInfoText(tr("tmp * qPow(10,ASSET_PRECISION)"));
                    transactionResultDialog.setDetailText(QString::number(tmp));
                    transactionResultDialog.pop();*/


                    QString params = QString("%1,%2,%3").arg(EXCHANGE_MODE_WNTT_TOKEN_OFFICIALWALLET).arg(QString::number(tmp,10,8)).arg("memo");
//                    transactionResultDialog.setInfoText(tr("QString(%1,%2,%3)"));
//                    transactionResultDialog.setDetailText(params);
//                    transactionResultDialog.pop();

                    XWCWallet::getInstance()->postRPC( "id-transfer_to_contract_token", toJsonFormat( "invoke_contract",
                                                         QJsonArray() << ui->accountNameLabel->text()
                                                         << XWCWallet::getInstance()->currentContractFee() << stepCount
                                                         << EXCHANGE_MODE_WNTT_TOKENCONTRACT_ADDRESS
                                                         << "transfer"  << params));
                }
            }

        }
        else if(result.startsWith("\"error\":"))
        {
            ui->okBtn->setEnabled(true);
            ui->tipLabel->setText("<body><font style=\"font-size:12px\" color=#ff224c>" + tr("Wrong password!") + "</font></body>" );
        }

        return;
    }


    if( id == "id-transfer_to_contract_token")
    {

        QString contractAddress = isExchangeMode ? EXCHANGE_MODE_CONTRACT_ADDRESS
                                                 : XWCWallet::getInstance()->getExchangeContractAddress(ui->accountNameLabel->text());

        TransactionResultDialog transactionResultDialog;

        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;
        /*transactionResultDialog.setInfoText(tr("id-transfer_to_contract_token  result"));
        transactionResultDialog.setDetailText(result);
        transactionResultDialog.pop();
        transactionResultDialog.setInfoText(tr("ui->amountLineEdit->text()"));
        transactionResultDialog.setDetailText(ui->amountLineEdit->text());
        transactionResultDialog.pop();*/

        if(result.startsWith("\"result\":"))
        {
            close();

            if (ui->assetComboBox->currentText() == "XDTT")
            {
                XWCWallet::getInstance()->postRPC( "id-transfer_to_contract", toJsonFormat( "invoke_contract",
                                                     QJsonArray() << ui->accountNameLabel->text()
                                                     << XWCWallet::getInstance()->currentContractFee() << stepCount
                                                     << contractAddress
                                                     << "on_deposit_xdt"  << ui->amountLineEdit->text()));


            } else if (ui->assetComboBox->currentText() == "WNTT") {
                XWCWallet::getInstance()->postRPC( "id-transfer_to_contract", toJsonFormat( "invoke_contract",
                                                     QJsonArray() << ui->accountNameLabel->text()
                                                     << XWCWallet::getInstance()->currentContractFee() << stepCount
                                                     << contractAddress
                                                     << "on_deposit_wnt"  << ui->amountLineEdit->text()));

            }

        }
        else if(result.startsWith("\"error\":"))
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to deposit to the contract!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();

        }

        return;
    }

    if( id == "id-transfer_to_contract")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;
        TransactionResultDialog transactionResultDialog;

        if(result.startsWith("\"result\":"))
        {
            close();

            if (ui->assetComboBox->currentText() == "XWC"){
                transactionResultDialog.setInfoText(tr("XWC Transaction of deposit has been sent out!"));
                transactionResultDialog.setDetailText(result);
                transactionResultDialog.pop();
            } else if (ui->assetComboBox->currentText() == "XDTT") {
                transactionResultDialog.setInfoText(tr("XDTT Transaction of deposit has been sent out!"));
                transactionResultDialog.setDetailText(result);
                transactionResultDialog.pop();
            } else if (ui->assetComboBox->currentText() == "WNTT") {
                transactionResultDialog.setInfoText(tr("WNTT Transaction of deposit has been sent out!"));
                transactionResultDialog.setDetailText(result);
                transactionResultDialog.pop();
            }

        }
        else if(result.startsWith("\"error\":"))
        {            
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to deposit to the contract!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();

            ui->okBtn->setEnabled(true);
        }

        return;
    }

    if( id == "id-transfer_to_contract_testing-DepositExchangeContractDialog")
    {
        QString result = XWCWallet::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            XWCWallet::TotalContractFee totalFee = XWCWallet::getInstance()->parseTotalContractFee(result);
            stepCount = totalFee.step;
            unsigned long long totalAmount = totalFee.baseAmount + ceil(totalFee.step * XWCWallet::getInstance()->contractFee / 100.0);

            feeChoose->updateFeeNumberSlots(getBigNumberString(totalAmount, ASSET_PRECISION).toDouble());
            feeChoose->updateAccountNameSlots(ui->accountNameLabel->text(),true);
        }


        return;
    }
}

void DepositExchangeContractDialog::on_okBtn_clicked()
{
    if(!XWCWallet::getInstance()->ValidateOnChainOperation())     return;

    if(ui->amountLineEdit->text().toDouble() <= 0)  return;
    if(ui->pwdLineEdit->text().isEmpty())           return;

    ui->okBtn->setEnabled(false);
    XWCWallet::getInstance()->postRPC( "id-unlock-DepositExchangeContractDialog", toJsonFormat( "unlock", QJsonArray() << ui->pwdLineEdit->text()
                                               ));

}

void DepositExchangeContractDialog::on_cancelBtn_clicked()
{
    close();
}

void DepositExchangeContractDialog::on_assetComboBox_currentIndexChanged(const QString &arg1)
{
    AssetAmountMap map = XWCWallet::getInstance()->accountInfoMap.value(ui->accountNameLabel->text()).assetAmountMap;
    AssetInfo assetInfo = XWCWallet::getInstance()->assetInfoMap.value(XWCWallet::getInstance()->getAssetId( getRealAssetSymbol( ui->assetComboBox->currentText())));

    ui->balanceLabel->setText(getBigNumberString(map.value(assetInfo.id).amount, assetInfo.precision) + " " + revertERCSymbol( assetInfo.symbol) );

    QRegExp rx1(QString("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg(assetInfo.precision));
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->amountLineEdit->setValidator(pReg1);
    ui->amountLineEdit->clear();
}

void DepositExchangeContractDialog::on_closeBtn_clicked()
{
    close();
}

void DepositExchangeContractDialog::estimateContractFee()
{
    QString contractAddress = isExchangeMode ? EXCHANGE_MODE_CONTRACT_ADDRESS
                                             : XWCWallet::getInstance()->getExchangeContractAddress(ui->accountNameLabel->text());

    XWCWallet::getInstance()->postRPC( "id-transfer_to_contract_testing-DepositExchangeContractDialog", toJsonFormat( "transfer_to_contract_testing",
                                                                           QJsonArray() << ui->accountNameLabel->text() << contractAddress
                                                                           << ui->amountLineEdit->text() << getRealAssetSymbol( ui->assetComboBox->currentText())
                                                                           << "deposit to exchange contract"
                                                                           ));


}

void DepositExchangeContractDialog::on_amountLineEdit_textChanged(const QString &arg1)
{
    estimateContractFee();
}
