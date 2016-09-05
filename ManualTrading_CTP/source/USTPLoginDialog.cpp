#include "USTPLoginDialog.h"
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>
#include <QtCore/QThread>
#include <Windows.h>
#include "USTPMdApi.h"
#include "USTPTradeApi.h"
#include "USTPConfig.h"
#include "USTPMutexId.h"
#include "USTPCtpLoader.h"
#include "USTPLogger.h"

USTPLoginDialog::USTPLoginDialog(QWidget* parent)
:QDialog(parent)
{	
	mBrokerId = USTPCtpLoader::getBrokerId();
	mUserLabel = new QLabel(tr("用户账号:"));
	mPasswordLabel = new QLabel(tr("登录密码:"));
	mUserEdit = new QLineEdit(tr("016189"));
	mPasswordEdit = new QLineEdit(tr("zaq12wsx"));
	mPasswordEdit->setEchoMode(QLineEdit::Password);
	
	mLoginBtn =  new QPushButton(tr("登  录"));
    mCancelBtn = new QPushButton(tr("取  消"));

    mBtnLayout = new QHBoxLayout;	
	mBtnLayout->addWidget(mLoginBtn);
	mBtnLayout->addSpacing(20);
	mBtnLayout->addWidget(mCancelBtn);

	mGridLayout = new QGridLayout;
	mGridLayout->addWidget(mUserLabel, 0, 0, 1, 1);
	mGridLayout->addWidget(mUserEdit, 0, 1, 1, 2);
	mGridLayout->addWidget(mPasswordLabel, 1, 0, 1, 1);
	mGridLayout->addWidget(mPasswordEdit, 1, 1, 1, 2);

    mViewLayout = new QVBoxLayout;
	mViewLayout->setMargin(180);
    mViewLayout->addLayout(mGridLayout);
    mViewLayout->addStretch(5);
    mViewLayout->addSpacing(30);
    mViewLayout->addLayout(mBtnLayout);
    setLayout(mViewLayout);
	setWindowIcon(QIcon("../image/title.png"));
    setWindowTitle(LOGIN_WINDOW_TITLE);
	QDesktopWidget *pDesk = QApplication::desktop();
	resize(LOGIN_WINDOW_WIDTH, LOGIN_WINDOW_HEIGHT);
	move((pDesk->width() - width()) / 2, (pDesk->height() - height()) / 2);
	initConnect();
}

USTPLoginDialog::~USTPLoginDialog()
{
}

void USTPLoginDialog::initConnect()
{	
	connect(mLoginBtn, SIGNAL(clicked()), this, SLOT(doUserLogin()));
	connect(mCancelBtn, SIGNAL(clicked()), this, SLOT(close()));
	
	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPTradeRspUserLogin(const QString&, const QString&, const QString&, const int&, const int&, const int&, const int&, const QString&, bool)),
		this, SLOT(doUSTPTradeRspUserLogin(const QString&, const QString&, const QString&, const int&, const int&, const int&, const int&, const QString&, bool)));

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRspQryInstrument(const QString&, const QString&, const QString&, const double&, const int&, const int&, bool)),
		this, SLOT(doUSTPRspQryInstrument(const QString&, const QString&, const QString&, const double&, const int&, const int&, bool)));

	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPTradeSettlementInfoConfirm(const QString&, const QString&, const int&, const QString&)),
		this, SLOT(doUSTPTradeSettlementInfoConfirm(const QString&, const QString&, const int&, const QString&)));


	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRspQryInvestorPosition(const QString&, const char&, const int&, const int&, const char&, const QString&, const QString&, const QString&, bool)), 
		this, SLOT(doUSTPRspQryInvestorPosition(const QString&, const char&, const int&, const int&, const char&, const QString&, const QString&, const QString&, bool)));

	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPMdRspUserLogin(const QString&, const QString&, const int&, const QString&, bool)),
		this, SLOT(doUSTPMdRspUserLogin(const QString&, const QString&, const int&, const QString&, bool)));
}

void USTPLoginDialog::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.drawPixmap(rect(), QPixmap("../image/background.png"));
}

void USTPLoginDialog::doUserLogin()
{	
	mUserId = mUserEdit->text();
	mPassword = mPasswordEdit->text();
	if ((!mUserId.isEmpty()) && (!mPassword.isEmpty())){
		QString authUser = USTPCtpLoader::getAuthUser();
		if(authUser != mUserId){
			ShowWarning(tr("用户无登录权限."));
			return;
		}
		USTPMutexId::initialize();
		int nMdResult = USTPMdApi::reqUserLogin(mBrokerId, mUserId, mPassword);
		if(nMdResult != 0){
			ShowWarning(tr("行情登录请求发送失败."));
		}
	}else{
		ShowWarning(tr("用户账号或密码为空."));
	}
}

void USTPLoginDialog::doUSTPMdRspUserLogin(const QString& brokerId, const QString& userId, const int& errorId, const QString& errorMsg, bool bIsLast)
{	
	if (errorId >= 0){
		int nTradeResult = USTPTradeApi::reqUserLogin(USTPMutexId::getMutexId(), mBrokerId, mUserId, mPassword);
		if(nTradeResult != 0)
			ShowWarning(tr("交易登录请求发送失败."));
	}
}

void USTPLoginDialog::doUSTPTradeRspUserLogin(const QString& tradingDate, const QString& brokerId, const QString& userId, const int& maxLocalId, const int& frontId,
											  const int& sessionId, const int& errorId, const QString& errorMsg, bool bIsLast)
{
	if (errorId == 0){
		QString nextSecDay = USTPCtpLoader::getDateTime();
		if(tradingDate.toInt() > nextSecDay.toInt()){
			ShowWarning(tr("系统时间不合法."));
			return;
		}
		USTPMutexId::setUserInfo(userId, mPassword, maxLocalId, frontId, sessionId);
		int nResult = USTPTradeApi::reqSettlementInfoConfirm(USTPMutexId::getMutexId(), brokerId, userId);
		if(nResult != 0)
			ShowWarning(tr("请求结算失败."));
	}else
		ShowWarning(errorMsg);
}


void USTPLoginDialog::doUSTPTradeSettlementInfoConfirm(const QString& brokerId, const QString& investorId, const int& errorId, const QString& errorMsg)
{
	if (errorId == 0){
		int nResult = USTPTradeApi::reqQryInstrument(USTPMutexId::getMutexId(), "", "", "");
		if(nResult != 0)
			ShowWarning(tr("请求查询合约失败."));
	}else
		ShowWarning(errorMsg);
}

void USTPLoginDialog::doUSTPRspQryInstrument(const QString& exchangeId, const QString& productId, const QString& instrumentId, const double& priceTick, 
											 const int& volumeMultiple, const int& maxMarketVolume, bool bIsLast)
{	
	USTPMutexId::setInsPriceTick(instrumentId, exchangeId, priceTick, volumeMultiple);
	USTPMutexId::setInsMarketMaxVolume(instrumentId, maxMarketVolume);

#ifdef _DEBUG
	QString data =  QString(tr(" [MarketVolume]  Instrument: ")) + instrumentId +  QString(tr("   MaxMarketVolume: ")) + QString::number(maxMarketVolume) +
		tr("  Exh: ") + exchangeId;
	USTPLogger::saveData(data);
#endif

	if(productId == tr("IF") && maxMarketVolume > 0)
		USTPMutexId::setReferenceIns(instrumentId);
	if(bIsLast){
		::Sleep(1000);
		int nResult = USTPTradeApi::reqQryInvestorPosition(USTPMutexId::getMutexId(), mBrokerId, USTPMutexId::getUserId(), "");
	}
}

void  USTPLoginDialog::doUSTPRspQryInvestorPosition(const QString& instrumentId, const char& direction, const int& position, const int& yPosition, const char& hedgeFlag,
													const QString& brokerId, const QString& tradingDay, const QString& investorId, bool bIsLast)
{
	if(direction == '2' && instrumentId != ""){
		if (position > 0) 
			USTPMutexId::addBidPosition(instrumentId, position);
	}else if(direction == '3' && instrumentId != ""){
		if (position > 0) 
			USTPMutexId::addAskPosition(instrumentId, position);
	}
	if(bIsLast){
		accept();
	}
}

#include "moc_USTPLoginDialog.cpp"
