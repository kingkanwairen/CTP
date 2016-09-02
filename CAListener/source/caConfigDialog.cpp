#include "caConfigDialog.h"
#include "caPublicDefine.h"
#include <QtCore/QTextStream>
#include "calogManager.h"

caConfigDialog::caConfigDialog(QString ip, int iPort, QString path, int status, QWidget* parent)
:QDialog(parent)
{	
	caLogManager::setFilePath(status, path);
    mListWidget = new QListWidget;
    mListWidget->setViewMode(QListView::IconMode);
    mListWidget->setIconSize(QSize(60, 50));
    mListWidget->setMovement(QListView::Static);
    mListWidget->setMaximumWidth(96);
    mListWidget->setSpacing(10);
	InitIconItem();
	mListWidget->setCurrentRow(0);

    mStackWidget = new QStackedWidget;
	mNormalPage = new caNormalPage(ip, iPort);
	mFilePage = new caFilePage(path, status);
	mStackWidget->addWidget(mNormalPage);
    mStackWidget->addWidget(mFilePage);
   

    mCancelBtn = new QPushButton(tr("取消"));
	mConfirmBtn =  new QPushButton(tr("确定"));
	mApplicationBtn = new QPushButton(tr("应用"));

    mHorizonLayout = new QHBoxLayout;
    mHorizonLayout->addWidget(mListWidget);
    mHorizonLayout->addWidget(mStackWidget, 1);

    mBtnLayout = new QHBoxLayout;
	mBtnLayout->addWidget(mConfirmBtn);
	mBtnLayout->addWidget(mApplicationBtn);
    mBtnLayout->addWidget(mCancelBtn);

    mViewLayout = new QVBoxLayout;
    mViewLayout->addLayout(mHorizonLayout);
    mViewLayout->addStretch(1);
    mViewLayout->addSpacing(12);
    mViewLayout->addLayout(mBtnLayout);
    setLayout(mViewLayout);
	
	InitConnect();
    setWindowTitle(CONFIG_WINDOW_TITLE);
}

caConfigDialog::~caConfigDialog()
{
	if (mConfirmBtn)
	{
		delete mConfirmBtn;
		mConfirmBtn = NULL;
	}

	if (mApplicationBtn)
	{
		delete mApplicationBtn;
		mApplicationBtn = NULL;
	}

	if (mCancelBtn)
	{
		delete mCancelBtn;
		mCancelBtn = NULL;
	}

	if (mNormalItem)
	{
		delete mNormalItem;
		mNormalItem = NULL;
	}

	if (mFileItem)
	{	
		delete mFileItem;
		mFileItem = NULL;
	}

	if (mNormalPage)
	{
		delete mNormalPage;
		mNormalPage = NULL;
	}

	if (mFilePage)
	{
		delete mFilePage;
		mFilePage = NULL;
	}

	if (mHorizonLayout)
	{
		delete mHorizonLayout;
		mHorizonLayout = NULL;
	}

	if (mStackWidget)
	{
		delete mStackWidget;
		mStackWidget = NULL;
	}

	if (mListWidget)
	{
		delete mListWidget;
		mListWidget = NULL;
	}

	if (mBtnLayout)
	{
		delete mBtnLayout;
		mBtnLayout = NULL;
	}

	if (mViewLayout)
	{
		delete mViewLayout;
		mViewLayout = NULL;
	}
}

void caConfigDialog::InitIconItem()
{	
	mNormalItem = new QListWidgetItem(mListWidget);
	mNormalItem->setIcon(QIcon("../rc/update.png"));
	mNormalItem->setText(tr("常规"));
	mNormalItem->setTextAlignment(Qt::AlignHCenter);
	mNormalItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    mFileItem = new QListWidgetItem(mListWidget);
    mFileItem->setIcon(QIcon("../rc/config.png"));
    mFileItem->setText(tr("文件"));
    mFileItem->setTextAlignment(Qt::AlignHCenter);
    mFileItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

}

void caConfigDialog::InitConnect()
{	
	connect(mApplicationBtn, SIGNAL(clicked()), this, SLOT(ApplyParam()));
	connect(mConfirmBtn, SIGNAL(clicked()), this, SLOT(ConfirmParam()));
	connect(mCancelBtn, SIGNAL(clicked()), this, SLOT(close()));
	connect(mListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
		this, SLOT(ChangePage(QListWidgetItem*, QListWidgetItem*)));
}

void caConfigDialog::ChangePage(QListWidgetItem *currentItem, QListWidgetItem *previousItem)
{
    if (!currentItem)
        currentItem = previousItem;
    mStackWidget->setCurrentIndex(mListWidget->row(currentItem));
}

void caConfigDialog::ApplyParam()
{	
	int linkPort = mNormalPage->GetPort();	
	QString linkIp = mNormalPage->GetIp();
	QString filePath = mFilePage->GetPath();
	bool save = mFilePage->GetSaveStatus();
	emit onSetCofig(linkIp, linkPort);

	//设置日志路径
	if(save){
		if (filePath.isEmpty()){
			ShowWarning(tr("Path 不能为空."));
			return;
		}else{
			caLogManager::setFilePath(save, filePath);
		}
	}
	
	//存储数据
	QFile data("../config/ca.ini");
	if (data.open(QFile::WriteOnly | QFile::Truncate)){
		QTextStream out(&data);
		out << "Link:" << linkIp << "," << linkPort << endl << "Log:" << save << "," << filePath;
		data.close();
	}else{
		ShowWarning(tr("日志文件无法打开."));
	}
}

void caConfigDialog::ConfirmParam()
{	
	ApplyParam();
	close();
}

#include "moc_caConfigDialog.cpp"
