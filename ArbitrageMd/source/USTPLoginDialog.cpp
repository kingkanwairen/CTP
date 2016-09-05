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
		int nResult = USTPTradeApi::reqQryInstrument(USTPMutexId::getMutexId(), "", "", "");
			if(nResult != 0)
				ShowWarning(tr("查询合约请求发送失败."));
	}else
		ShowWarning(errorMsg);
}


void USTPLoginDialog::doUSTPRspQryInstrument(const QString& exchangeId, const QString& productId, const QString& instrumentId, const double& priceTick, 
											 const int& volumeMultiple, const int& maxMarketVolume, bool bIsLast)
{	
	USTPMutexId::setInsPriceTick(instrumentId, exchangeId, priceTick, volumeMultiple);
	if(bIsLast){
		accept();
	}
}

#include "moc_USTPLoginDialog.cpp"
