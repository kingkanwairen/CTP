#include "USTPMdDialog.h"
#include "USTPMarketWidget.h"
#include <QtGui/QDoubleValidator>
#include "USTPConfig.h"

USTPMdDialog::USTPMdDialog(USTPMarketWidget* pWidget, const QString& inss, QWidget* parent)
:QDialog(parent)
{	
	mSelIndex = -1;
    mListWidget = new QListWidget;
	mFirstInsLabel = new QLabel(tr("第一腿"));
	mSecondInsLabel = new QLabel(tr("第二腿"));
	mFirstInsFactorLabel = new QLabel(tr("第一腿系数"));
	mSecondInsFactorLabel = new QLabel(tr("第二腿系数"));
	mFirstInsEdit = new QLineEdit;
	mSecondInsEdit = new QLineEdit;
	mFirstInsFactorEdit = new QLineEdit(tr("1.0"));
	QDoubleValidator * dbValidator = new QDoubleValidator(this);
	dbValidator->setRange(-9999999, 99999999, 3);
	mFirstInsFactorEdit->setValidator(dbValidator);
	mSecondInsFactorEdit = new QLineEdit(tr("1.0"));
	mSecondInsFactorEdit->setValidator(dbValidator);

	mConstantValueLabel = new QLabel(tr("常量"));
	mConstantValueEdit = new QLineEdit(tr("0.0"));
	mConstantValueEdit->setValidator(dbValidator);

	mAddBtn = new QPushButton(tr("添加>>"));
	mUpBtn = new QPushButton(tr("上移↑"));
	mDownBtn =  new QPushButton(tr("下移↓"));
	mDelBtn =  new QPushButton(tr("删除全部"));

	mGridLayout = new QGridLayout;
	mGridLayout->addWidget(mListWidget, 0, 5, 4, 2);
	mGridLayout->addWidget(mFirstInsFactorLabel, 0, 0, 1, 1);
	mGridLayout->addWidget(mFirstInsFactorEdit, 0, 1, 1, 1);
	mGridLayout->addWidget(mFirstInsLabel, 0, 2, 1, 1);
	mGridLayout->addWidget(mFirstInsEdit, 0, 3, 1, 1);
	
	mGridLayout->addWidget(mSecondInsFactorLabel, 1, 0, 1, 1);
	mGridLayout->addWidget(mSecondInsFactorEdit, 1, 1, 1, 1);	
	mGridLayout->addWidget(mSecondInsLabel, 1, 2, 1, 1);
	mGridLayout->addWidget(mSecondInsEdit, 1, 3, 1, 1);	

	mGridLayout->addWidget(mConstantValueLabel, 2, 0, 1, 1);
	mGridLayout->addWidget(mConstantValueEdit, 2, 1, 1, 1);	

	mGridLayout->addWidget(mAddBtn, 0, 4, 1, 1);
	mGridLayout->addWidget(mUpBtn, 1, 4, 1, 1);
	mGridLayout->addWidget(mDownBtn, 2, 4, 1, 1);
	mGridLayout->addWidget(mDelBtn, 3, 4, 1, 1);
	
	
	mConfirmBtn = new QPushButton(tr("注 册"));
	mCancelBtn =  new QPushButton(tr("取 消"));
    mHorizonLayout = new QHBoxLayout;
    mHorizonLayout->addWidget(mConfirmBtn);
	mHorizonLayout->addSpacing(40);
    mHorizonLayout->addWidget(mCancelBtn);

    mViewLayout = new QVBoxLayout;
	mViewLayout->setMargin(20);
    mViewLayout->addLayout(mGridLayout);
    mViewLayout->addLayout(mHorizonLayout);
    setLayout(mViewLayout);	
	initConnect(pWidget);
	initList(inss);
	resize(COMBO_MARKET_WINDOW_WIDTH, COMBO_MARKET_WINDOW_HEIGHT);
    setWindowTitle(COMBO_MARKET_WINDOW_TITLE);
}

USTPMdDialog::~USTPMdDialog()
{

}


void USTPMdDialog::initConnect(USTPMarketWidget* pWidget)
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
	QString secIns = mSecondInsEdit->text();
	QString firstInsFactor = mFirstInsFactorEdit->text();
	QString secInsFactor = mSecondInsFactorEdit->text();
	QString constantValue = mConstantValueEdit->text();
	if(firstIns.isEmpty() || secIns.isEmpty()){
		ShowWarning(tr("合约设置不能为空."));
	}else{
		QString zSecInsFactor, zConstantValue;
		if(secInsFactor.toFloat() > 0.0)
			zSecInsFactor = tr("+") + secInsFactor + tr("*") + secIns;
		else
			zSecInsFactor = tr("+(") + secInsFactor + tr(")") + tr("*") + secIns;

		if(zConstantValue.toFloat() > 0.0)
			zConstantValue = tr("+") + constantValue;
		else
			zConstantValue = tr("+(") + constantValue + tr(")");

		QString comIns = firstInsFactor + tr("*") + firstIns + zSecInsFactor + zConstantValue;
		QList<QListWidgetItem *> items = mListWidget->findItems(comIns, Qt::MatchRecursive);
		if(items.size() > 0){
			mListWidget->setCurrentItem(items[0]);
		}else{
			QListWidgetItem *showItem = new QListWidgetItem(mListWidget);
			showItem->setText(comIns);
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
