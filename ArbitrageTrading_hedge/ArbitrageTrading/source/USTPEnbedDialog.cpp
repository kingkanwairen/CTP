#include "USTPEnbedDialog.h"
#include "USTPConfig.h"
#include "USTPMutexId.h"

USTPEnbedDialog::USTPEnbedDialog(const QString& firstIns, const QString& secIns, const double& orderPrice,  const int& direction, QWidget* parent)
:QDialog(parent)
{	
	mFirstInsName = firstIns;
	mSecInsName = secIns;
	mOrderPrice = orderPrice;
	mDirection = direction;
	mConfirmBtn = createButton(tr("确 定"), SLOT(createShortCutOrder()));;
	mEnbedBtn = createButton(tr("预 埋"), SLOT(createEnbedOrder()));
	mBtnLayout = new QHBoxLayout;
	mBtnLayout->addWidget(mConfirmBtn);
	mBtnLayout->addWidget(mEnbedBtn);

	mOrderGroup = createOrderGroup();

	mOrderLayout = new QVBoxLayout;
	mOrderLayout->addWidget(mOrderGroup);

	mViewLayout = new QVBoxLayout;
	mViewLayout->addLayout(mOrderLayout);
	mViewLayout->addStretch(1);
	mViewLayout->addSpacing(12);
	mViewLayout->addLayout(mBtnLayout);	
	setLayout(mViewLayout);
	setWindowTitle(STRATERGY_WINDOW_TITLE);
	initConnect(parent);
}

USTPEnbedDialog::~USTPEnbedDialog()
{

}

QGroupBox* USTPEnbedDialog::createOrderGroup()
{	
	QGroupBox *groupBox = new QGroupBox(tr("订单设置"));

	mFirstInsLabel = new QLabel(tr("合约一:"));
	mFirstInsEdit = new QLineEdit(mFirstInsName);
	mFirstInsLabel->setBuddy(mFirstInsEdit);

	mSecondInsLabel = new QLabel(tr("合约二:"));
	mSecondInsEdit = new QLineEdit(mSecInsName);
	mSecondInsLabel->setBuddy(mSecondInsEdit);

	mStyleLabel = new QLabel(tr("订单类型:"));
	QStringList list;
	list << tr("0-盯价套利") << tr("1-条件套利") << tr("2-条件对价套利") << tr("3-条件盯价套利") << tr("4-对价盯价套利");
	mStyleComboBox = createComboBox(list);
	mStyleComboBox->setCurrentIndex(2);
	mStyleLabel->setBuddy(mStyleComboBox);

	mDirectionLabel = new QLabel(tr("买/卖:"));
	QStringList bsList;
	bsList << tr("0-买入") << tr("1-卖出");
	mBSComboBox = createComboBox(bsList);
	mBSComboBox->setItemIcon(0, QIcon("../image/red.png"));
	mBSComboBox->setItemIcon(1, QIcon("../image/green.png"));
	mBSComboBox->setCurrentIndex(mDirection);
	mDirectionLabel->setBuddy(mBSComboBox);

	mOffsetLabel = new QLabel(tr("开/平:"));
	QStringList offsetList;
	offsetList << tr("0-开仓") << tr("1-平仓") << tr("2-平今") << tr("3-平昨");
	mOffsetComboBox = createComboBox(offsetList);
	mOffsetLabel->setBuddy(mOffsetComboBox);

	mHedgeLabel = new QLabel(tr("投机/套保:"));
	QStringList hedgeList;
	hedgeList << tr("0-投机") << tr("1-套利");
	mHedgeComboBox = createComboBox(hedgeList);
	mHedgeComboBox->setCurrentIndex(0);
	mHedgeLabel->setBuddy(mHedgeComboBox);

	mPriceLabel = new QLabel(tr("委托价差:"));
	mPriceSpinBox = new QDoubleSpinBox;
	mPriceSpinBox->setRange(-10000.0, 10000.0);
	double tick = USTPMutexId::getInsPriceTick(mFirstInsName);
	mPriceSpinBox->setDecimals(getPrcision(tick));
	mPriceSpinBox->setSingleStep(tick);
	mPriceSpinBox->setValue(mOrderPrice);
	mPriceLabel->setBuddy(mPriceSpinBox);

	mQtyLabel = new QLabel(tr("委托量:"));
	mQtySpinBox = new QSpinBox;
	mQtySpinBox->setRange(1, 1000);
	mQtySpinBox->setSingleStep(1);
	mQtySpinBox->setValue(1);
	mQtyLabel->setBuddy(mQtySpinBox);

	QGridLayout* vBox = new QGridLayout;
	vBox->addWidget(mFirstInsLabel, 0, 0, 1, 1);
	vBox->addWidget(mFirstInsEdit, 0, 1, 1, 2);
	vBox->addWidget(mSecondInsLabel, 0, 3, 1, 1);
	vBox->addWidget(mSecondInsEdit, 0, 4, 1, 2);

	vBox->addWidget(mStyleLabel, 1, 0, 1, 1);
	vBox->addWidget(mStyleComboBox, 1, 1, 1, 2);
	vBox->addWidget(mDirectionLabel, 1, 3, 1, 1);
	vBox->addWidget(mBSComboBox, 1, 4, 1, 2);

	vBox->addWidget(mOffsetLabel, 2, 0, 1, 1);
	vBox->addWidget(mOffsetComboBox, 2, 1, 1, 2);
	vBox->addWidget(mHedgeLabel, 2, 3, 1, 1);
	vBox->addWidget(mHedgeComboBox, 2, 4, 1, 2);

	vBox->addWidget(mQtyLabel, 3, 0, 1, 1);
	vBox->addWidget(mQtySpinBox, 3, 1, 1, 2);
	vBox->addWidget(mPriceLabel, 3, 3, 1, 1);
	vBox->addWidget(mPriceSpinBox, 3, 4, 1, 2);
	groupBox->setLayout(vBox);
	return groupBox;
}


QComboBox* USTPEnbedDialog::createComboBox(const QStringList &itemList)
{
	QComboBox *comboBox = new QComboBox;
	foreach(QString item, itemList){
		comboBox->addItem(item);
	}
	comboBox->setEditable(true);
	comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	return comboBox;
}

QPushButton *USTPEnbedDialog::createButton(const QString &text, const char *member)
{
	QPushButton *button = new QPushButton(text);
	connect(button, SIGNAL(clicked()), this, member);
	return button;
}

void USTPEnbedDialog::initConnect(QWidget* widget)
{	
	connect(this, SIGNAL(onCreateEnbedOrder(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const double&, const int&, bool)), 
		widget, SLOT(doCreateEnbedOrder(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const double&, const int&, bool)));
}


void USTPEnbedDialog::createShortCutOrder()
{
	QString firstName, secondName;
	firstName = mFirstInsEdit->text();
	secondName = mSecondInsEdit->text();
	if(firstName.isEmpty() || secondName.isEmpty()){
		ShowWarning(tr("合约设置不能为空."));
	}else{
		QString style = mStyleComboBox->currentText();
		QString bs = mBSComboBox->currentText();
		QString offset = mOffsetComboBox->currentText();
		QString hedge = mHedgeComboBox->currentText();
		double price = mPriceSpinBox->value();
		int qty = mQtySpinBox->value();
		emit onCreateEnbedOrder(firstName, secondName, style, bs, offset, hedge, price, qty, false);
		accept();
	}
}

void USTPEnbedDialog::createEnbedOrder()
{
	QString firstName, secondName;
	firstName = mFirstInsEdit->text();
	secondName = mSecondInsEdit->text();
	if(firstName.isEmpty() || secondName.isEmpty()){
		ShowWarning(tr("合约设置不能为空."));
	}else{
		QString style = mStyleComboBox->currentText();
		QString bs = mBSComboBox->currentText();
		QString offset = mOffsetComboBox->currentText();
		QString hedge = mHedgeComboBox->currentText();
		double price = mPriceSpinBox->value();
		int qty = mQtySpinBox->value();
		emit onCreateEnbedOrder(firstName, secondName, style, bs, offset, hedge, price, qty, true);
		accept();
	}
}

int USTPEnbedDialog::getPrcision(const double& value)
{
	if (value >= VALUE_1){
		return 0;
	}else if(value * 10 >= VALUE_1){
		return 1;
	}else if(value * 100 >= VALUE_1){
		return 2;
	}else if(value * 1000 >= VALUE_1){
		return 3;
	}
	return 0;
}

#include "moc_USTPEnbedDialog.cpp"