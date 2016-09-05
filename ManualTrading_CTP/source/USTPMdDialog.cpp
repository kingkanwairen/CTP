#include "USTPMdDialog.h"
#include "USTPMarketWidget.h"
#include "USTPSubmitWidget.h"
#include "USTPConfig.h"

USTPMdDialog::USTPMdDialog(USTPMarketWidget* pWidget, USTPSubmitWidget* pSubmitWidget, const QString& inss, QWidget* parent)
:QDialog(parent)
{	
	mSelIndex = -1;
    mListWidget = new QListWidget;
	mFirstInsLabel = new QLabel(tr("��Լ"));
	mFirstInsEdit = new QLineEdit;
	mAddBtn = new QPushButton(tr("���>>"));
	mUpBtn = new QPushButton(tr("���ơ�"));
	mDownBtn =  new QPushButton(tr("���ơ�"));
	mDelBtn =  new QPushButton(tr("ɾ��ȫ��"));

	mGridLayout = new QGridLayout;
	mGridLayout->addWidget(mListWidget, 0, 4, 4, 4);
	mGridLayout->addWidget(mFirstInsLabel, 1, 0, 1, 1);
	mGridLayout->addWidget(mFirstInsEdit, 1, 1, 1, 2);
	mGridLayout->addWidget(mAddBtn, 0, 3, 1, 1);
	mGridLayout->addWidget(mUpBtn, 1, 3, 1, 1);
	mGridLayout->addWidget(mDownBtn, 2, 3, 1, 1);
	mGridLayout->addWidget(mDelBtn, 3, 3, 1, 1);
	
	mConfirmBtn = new QPushButton(tr("ע ��"));
	mCancelBtn =  new QPushButton(tr("ȡ ��"));
    mHorizonLayout = new QHBoxLayout;
    mHorizonLayout->addWidget(mConfirmBtn);
	mHorizonLayout->addSpacing(40);
    mHorizonLayout->addWidget(mCancelBtn);

    mViewLayout = new QVBoxLayout;
	mViewLayout->setMargin(20);
    mViewLayout->addLayout(mGridLayout);
    mViewLayout->addLayout(mHorizonLayout);
    setLayout(mViewLayout);	
	initConnect(pWidget, pSubmitWidget);
	initList(inss);
	resize(COMBO_MARKET_WINDOW_WIDTH, COMBO_MARKET_WINDOW_HEIGHT);
    setWindowTitle(COMBO_MARKET_WINDOW_TITLE);
}

USTPMdDialog::~USTPMdDialog()
{

}


void USTPMdDialog::initConnect(USTPMarketWidget* pWidget, USTPSubmitWidget* pSubmitWidget)
{	
	connect(mAddBtn, SIGNAL(clicked()), this, SLOT(doAddInstrument()));
	connect(mDelBtn, SIGNAL(clicked()), this, SLOT(doDelAllInstruemnt()));

	connect(mUpBtn, SIGNAL(clicked()), this, SLOT(doUpInstrument()));
	connect(mDownBtn, SIGNAL(clicked()), this, SLOT(doDownInstruemnt()));

	connect(mConfirmBtn, SIGNAL(clicked()), this, SLOT(doSubscribeMarket()));
	connect(mCancelBtn, SIGNAL(clicked()), this, SLOT(close()));
	connect(mListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(doSelInstrument(QListWidgetItem*)));
	connect(mListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(doDelInstrument(QListWidgetItem*)));
	connect(this, SIGNAL(onSubscribeMd(const QStringList&)), pWidget, SLOT(doSubscribeMd(const QStringList&)));
	connect(this, SIGNAL(onSubscribeMd(const QStringList&)), pSubmitWidget, SLOT(doSubscribeMd(const QStringList&)));
}

void USTPMdDialog::initList(const QString& list)
{	
	if (list.isEmpty())
		return;
	QStringList insCombo = list.split(",");
	if(insCombo.size() > 0)
		mListWidget->addItems(insCombo);
}

void USTPMdDialog::doAddInstrument()
{
	QString firstIns = mFirstInsEdit->text();
	if(firstIns.isEmpty()){
		ShowWarning(tr("��Լ����Ϊ��."));
	}else{
		QList<QListWidgetItem *> items = mListWidget->findItems(firstIns, Qt::MatchRecursive);
		if(items.size() > 0){
			mListWidget->setCurrentItem(items[0]);
		}else{
			QListWidgetItem *showItem = new QListWidgetItem(mListWidget);
			showItem->setText(firstIns);
			mListWidget->addItem(showItem);
		}
	}
}

void USTPMdDialog::doUpInstrument()
{
	if (mSelIndex < 0)
		return;
	QListWidgetItem* it = mListWidget->takeItem(mSelIndex);
	int nSize = mListWidget->count();
	mSelIndex--;
	if(mSelIndex < 0)
		mSelIndex = nSize;
	mListWidget->insertItem(mSelIndex, it);	
	mListWidget->setCurrentRow(mSelIndex, QItemSelectionModel::ClearAndSelect);
}

void USTPMdDialog::doDownInstruemnt()
{
	if (mSelIndex < 0)
		return;
	int nSize = mListWidget->count();
	QListWidgetItem* it = mListWidget->takeItem(mSelIndex);
	mSelIndex++;
	if(mSelIndex >= nSize)
		mSelIndex = 0;
	mListWidget->insertItem(mSelIndex, it);	
	mListWidget->setCurrentRow(mSelIndex, QItemSelectionModel::ClearAndSelect);
}

void USTPMdDialog::doDelAllInstruemnt()
{	
	for(int n = 0; n < mListWidget->count(); n++){
		QListWidgetItem* item = mListWidget->item(n);
		delete item;
		item = NULL;
	}
	mListWidget->clear();
}

void USTPMdDialog::doDelInstrument(QListWidgetItem* item)
{	
	int nRow = mListWidget->row(item);
	QListWidgetItem* it = mListWidget->takeItem(nRow);
	delete it;
	it = NULL;
}

void USTPMdDialog::doSelInstrument(QListWidgetItem* item)
{	
	mSelIndex = mListWidget->row(item);
}

void USTPMdDialog::doSubscribeMarket()
{	
	QStringList items;
	for(int n = 0; n < mListWidget->count(); n++){
		QListWidgetItem* item = mListWidget->item(n);
		items << item->text();
	}
	if (items.size() > 0){
		emit onSubscribeMd(items);
		QString saveLists = items.at(0);
		for (int index = 1; index < items.size(); index++) {
			saveLists += tr(",");
			saveLists += items.at(index);
		}
		saveToFile(saveLists);
		accept();
	}
}

bool USTPMdDialog::saveToFile(const QString& list)
{
	QFile file("../config/param.txt");  
	if (!file.open(QIODevice::ReadOnly)) { 
		return false;
	}
	QTextStream stream(&file);
	QString saveLine;
	QString line;
	do {
		line = stream.readLine();
		if(line == tr("[MdItem]")){
			saveLine += line;
			saveLine += tr("\n");
			line = stream.readLine();
			line.replace(line, list, Qt::CaseSensitive);
		}
		saveLine += line;
		saveLine += tr("\n");
	} while (!line.isNull());
	file.close();

	QFile saveFile("../config/param.txt");  
	if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) { 
		return false;
	}
	QTextStream saveStream(&saveFile);
	saveStream << saveLine;
	saveFile.close();
	return true;
}

#include "moc_USTPMdDialog.cpp"
