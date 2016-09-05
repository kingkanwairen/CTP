#include "USTPUnilateralDialog.h"
#include "USTPConfig.h"
#include "USTPUnilateralWidget.h"

USTPUnilateralDialog::USTPUnilateralDialog(USTPUnilateralWidget* pUnilateralWidget, QWidget* parent)
:QDialog(parent)
{
	mCancelBtn = createButton(tr("取 消"), SLOT(close()));;
	mConfirmBtn = createButton(tr("确 定"), SLOT(createOrder()));
	mBtnLayout = new QHBoxLayout;
	mBtnLayout->addWidget(mConfirmBtn);
	mBtnLayout->addWidget(mCancelBtn);

	mOrderGroup = createOrderGroup();

	mViewLayout = new QVBoxLayout;
	mViewLayout->addWidget(mOrderGroup);
	mViewLayout->addStretch(1);
	mViewLayout->addSpacing(12);
	mViewLayout->addLayout(mBtnLayout);	
	setLayout(mViewLayout);
	initConnect(pUnilateralWidget);
	setWindowTitle(UNILATERAL_WINDOW_TITLE);
}

USTPUnilateralDialog::~USTPUnilateralDialog()
{

}

QGroupBox* USTPUnilateralDialog::createOrderGroup()
{
	QGroupBox *groupBox = new QGroupBox(tr("订单设置"));
	mInsLabel = new QLabel(tr("单边合约:"));
	mInsEdit = new QLineEdit;
	mInsLabel->setBuddy(mInsEdit);

	mDirectionLabel = new QLabel(tr("买/卖:"));
	QStringList bsList;
	bsList << tr("0-买入") << tr("1-卖出");
	mBSComboBox = createComboBox(bsList);
	mDirectionLabel->setBuddy(mBSComboBox);

	mOffsetLabel = new QLabel(tr("开/平:"));
	QStringList offsetList;
	offsetList << tr("0-开仓") << tr("1-平仓") << tr("2-平今") << tr("3-平昨");
	mOffsetComboBox = createComboBox(offsetList);
	mOffsetLabel->setBuddy(mOffsetComboBox);

	mOrderPriceLabel = new QLabel(tr("委托价格:"));
	mOrderPriceSpinBox = new QDoubleSpinBox;
	mOrderPriceSpinBox->setRange(-999999.0, 999999.0);
	mOrderPriceSpinBox->setDecimals(3);
	mOrderPriceSpinBox->setSingleStep(1.0);
	mOrderPriceSpinBox->setValue(100.0);
	mOrderPriceLabel->setBuddy(mOrderPriceSpinBox);

	mOrderQtyLabel = new QLabel(tr("委托量:"));
	mOrderQtySpinBox = new QSpinBox;
	mOrderQtySpinBox->setRange(1, 1000);
	mOrderQtySpinBox->setSingleStep(1);
	mOrderQtySpinBox->setValue(1);
	mOrderQtyLabel->setBuddy(mOrderQtySpinBox);

	mCyclePointLabel = new QLabel(tr("循环档:"));
	mCyclePointSpinBox = new QSpinBox;
	mCyclePointSpinBox->setRange(0, 99999);
	mCyclePointSpinBox->setSingleStep(1);
	mCyclePointSpinBox->setValue(1);
	mCyclePointLabel->setBuddy(mCyclePointSpinBox);

	mCycleNumLabel = new QLabel(tr("循环次数:"));
	mCycleNumSpinBox = new QSpinBox;
	mCycleNumSpinBox->setRange(0, 99999);
	mCycleNumSpinBox->setSingleStep(1);
	mCycleNumSpinBox->setValue(1);
	mCycleNumLabel->setBuddy(mCycleNumSpinBox);

	QGridLayout* vBox = new QGridLayout;
	vBox->addWidget(mInsLabel, 0, 0, 1, 1);
	vBox->addWidget(mInsEdit, 0, 1, 1, 2);

	vBox->addWidget(mDirectionLabel, 0, 3, 1, 1);
	vBox->addWidget(mBSComboBox, 0, 4, 1, 2);

	vBox->addWidget(mOrderQtyLabel, 1, 0, 1, 1);
	vBox->addWidget(mOrderQtySpinBox, 1, 1, 1, 2);

	vBox->addWidget(mOffsetLabel, 1, 3, 1, 1);
	vBox->addWidget(mOffsetComboBox, 1, 4, 1, 2);

	vBox->addWidget(mOrderPriceLabel, 2, 0, 1, 1);
	vBox->addWidget(mOrderPriceSpinBox, 2, 1, 1, 2);

	vBox->addWidget(mOrderQtyLabel, 2, 3, 1, 1);
	vBox->addWidget(mOrderQtySpinBox, 2, 4, 1, 2);

	vBox->addWidget(mCyclePointLabel, 3, 0, 1, 1);
	vBox->addWidget(mCyclePointSpinBox, 3, 1, 1, 2);

	vBox->addWidget(mCycleNumLabel, 3, 3, 1, 1);
	vBox->addWidget(mCycleNumSpinBox, 3, 4, 1, 2);

	groupBox->setLayout(vBox);
	return groupBox;
}


QComboBox* USTPUnilateralDialog::createComboBox(const QStringList &itemList)
{
	QComboBox *comboBox = new QComboBox;
	foreach(QString item, itemList){
		comboBox->addItem(item);
	}
	comboBox->setEditable(true);
	comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	return comboBox;
}

QPushButton *USTPUnilateralDialog::createButton(const QString &text, const char *member)
{
	QPushButton *button = new QPushButton(text);
	connect(button, SIGNAL(clicked()), this, member);
	return button;
}

void USTPUnilateralDialog::initConnect(USTPUnilateralWidget* pUnilateralWidget)
{	
	connect(this, SIGNAL(onCreateUnilateralOrder(const QString&, const QString&, const QString&, const double&, const int&, const int&,
		const int&)), pUnilateralWidget, SLOT(doCreateUnilateralOrder(const QString&, const QString&, const QString&, const double&, const int&,
		const int&, const int&)));
}


void USTPUnilateralDialog::createOrder()
{
	QString insName = mInsEdit->text();
	if(insName.isEmpty()){
		ShowWarning(tr("合约设置不能为空."));
	}else{
		QString bs = mBSComboBox->currentText();
		QString offset = mOffsetComboBox->currentText();
		double orderPrice = mOrderPriceSpinBox->value();
		int qty = mOrderQtySpinBox->value();
		int cyclePoint = mCyclePointSpinBox->value();
		int cycleNum = mCycleNumSpinBox->value();
		emit onCreateUnilateralOrder(insName, bs, offset, orderPrice, qty, cyclePoint, cycleNum);
		accept();
	}
}

#include "moc_USTPUnilateralDialog.cpp"