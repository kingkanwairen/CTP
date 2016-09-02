#include "USTPEnbedDialog.h"
#include "USTPConfig.h"
#include "USTPMutexId.h"

USTPEnbedDialog::USTPEnbedDialog(const int& row, const QString& selIns, const double& orderPrice,  const int& direction, QWidget* parent)
:QDialog(parent)
{	
	mRow = row;
	mSelInsName = selIns;
	mOrderPrice = orderPrice;
	mDirection = direction;
	mConfirmBtn = createButton(tr("ȷ ��"), SLOT(createShortCutOrder()));;
	mCancelBtn = createButton(tr("ȡ ��"), SLOT(close()));;
	mBtnLayout = new QHBoxLayout;
	mBtnLayout->addWidget(mConfirmBtn);
	mBtnLayout->addWidget(mCancelBtn);

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
	QGroupBox *groupBox = new QGroupBox(tr("��������"));

	mFirstInsLabel = new QLabel(tr("��Լ:"));
	mFirstInsEdit = new QLineEdit(mSelInsName);
	mFirstInsLabel->setBuddy(mFirstInsEdit);

	mDirectionLabel = new QLabel(tr("��/��:"));
	QStringList bsList;
	bsList << tr("0-����") << tr("1-����");
	mBSComboBox = createComboBox(bsList);
	mBSComboBox->setItemIcon(0, QIcon("../image/red.png"));
	mBSComboBox->setItemIcon(1, QIcon("../image/green.png"));
	mBSComboBox->setCurrentIndex(mDirection);
	mDirectionLabel->setBuddy(mBSComboBox);

	mOffsetLabel = new QLabel(tr("��/ƽ:"));
	QStringList offsetList;
	offsetList << tr("0-����") << tr("1-ƽ��") << tr("2-ƽ��") << tr("3-ƽ��");
	mOffsetComboBox = createComboBox(offsetList);
	mOffsetComboBox->setCurrentIndex(0);
	mOffsetLabel->setBuddy(mOffsetComboBox);

	mQtyLabel = new QLabel(tr("ί����:"));
	mQtySpinBox = new QSpinBox;
	mQtySpinBox->setRange(1, 1000);
	mQtySpinBox->setSingleStep(1);
	mQtySpinBox->setValue(1);
	mQtyLabel->setBuddy(mQtySpinBox);

	mPriceLabel = new QLabel(tr("�۸�:"));
	mPriceSpinBox = new QDoubleSpinBox;
	mPriceSpinBox->setRange(0, 100000.0);
	double tick = USTPMutexId::getInsPriceTick(mSelInsName);
	mPriceSpinBox->setDecimals(getPrcision(tick));
	mPriceSpinBox->setSingleStep(tick);
	mPriceSpinBox->setValue(mOrderPrice);
	mPriceLabel->setBuddy(mPriceSpinBox);

	mOpenSuperLabel = new QLabel(tr("���ֳ��۵�:"));
	mOpenSuperBox = new QSpinBox;
	mOpenSuperBox->setRange(0, 1000);
	mOpenSuperBox->setSingleStep(1);
	mOpenSuperBox->setValue(1);
	mOpenSuperLabel->setBuddy(mOpenSuperBox);
	
	mStopProfitLabel = new QLabel(tr("ֹӯ��:"));
	mStopProfitBox = new QSpinBox;
	mStopProfitBox->setRange(0, 1000);
	mStopProfitBox->setSingleStep(1);
	mStopProfitBox->setValue(2);
	mStopProfitLabel->setBuddy(mStopProfitBox);

	mStopLossLabel = new QLabel(tr("ֹ��:"));
	mStopLossBox = new QSpinBox;
	mStopLossBox->setRange(0, 1000);
	mStopLossBox->setSingleStep(1);
	mStopLossBox->setValue(3);
	mStopLossLabel->setBuddy(mStopLossBox);

	QGridLayout* vBox = new QGridLayout;
	vBox->addWidget(mFirstInsLabel, 0, 0, 1, 1);
	vBox->addWidget(mFirstInsEdit, 0, 1, 1, 2);
	vBox->addWidget(mDirectionLabel, 0, 3, 1, 1);
	vBox->addWidget(mBSComboBox, 0, 4, 1, 2);

	vBox->addWidget(mOffsetLabel, 1, 0, 1, 1);
	vBox->addWidget(mOffsetComboBox, 1, 1, 1, 2);
	vBox->addWidget(mQtyLabel, 1, 3, 1, 1);
	vBox->addWidget(mQtySpinBox, 1, 4, 1, 2);
	
	vBox->addWidget(mPriceLabel, 2, 0, 1, 1);
	vBox->addWidget(mPriceSpinBox, 2, 1, 1, 2);
	vBox->addWidget(mOpenSuperLabel, 2, 3, 1, 1);
	vBox->addWidget(mOpenSuperBox, 2, 4, 1, 2);

	vBox->addWidget(mStopProfitLabel, 3, 0, 1, 1);
	vBox->addWidget(mStopProfitBox, 3, 1, 1, 2);
	vBox->addWidget(mStopLossLabel, 3, 3, 1, 1);
	vBox->addWidget(mStopLossBox, 3, 4, 1, 2);
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
	connect(this, SIGNAL(onCreateEnbedOrder(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const int&, const int&)), 
		widget, SLOT(doCreateEnbedOrder(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const int&, const int&)));
}


void USTPEnbedDialog::createShortCutOrder()
{
	QString firstName = mFirstInsEdit->text();
	if(firstName.isEmpty()){
		ShowWarning(tr("��Լ���ò���Ϊ��."));
	}else{
		QString bs = mBSComboBox->currentText();
		QString offset = mOffsetComboBox->currentText();
		double price = mPriceSpinBox->value();
		int qty = mQtySpinBox->value();
		int openSuperNum = mOpenSuperBox->value();
		int stopProfitNum = mStopProfitBox->value();
		int stopLossNum = mStopLossBox->value();
		emit onCreateEnbedOrder(firstName, bs, offset, price, qty, openSuperNum, stopProfitNum, stopLossNum, mRow);
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