#include "USTParamDialog.h"
#include "USTPConfig.h"
#include "USTPStrategyWidget.h"

USTParamDialog::USTParamDialog(USTPStrategyWidget* pStrategyWidget, QWidget* parent)
:QDialog(parent)
{
	mCancelBtn = createButton(tr("ȡ ��"), SLOT(close()));;
	mConfirmBtn = createButton(tr("ȷ ��"), SLOT(createOrder()));
	mBtnLayout = new QHBoxLayout;
	mBtnLayout->addWidget(mConfirmBtn);
	mBtnLayout->addWidget(mCancelBtn);

	mOrderGroup = createOrderGroup();
	mSpotGroup = createSpotGroup();

	mOrderLayout = new QVBoxLayout;
	mOrderLayout->addWidget(mOrderGroup);
	mOrderLayout->addWidget(mSpotGroup);

	mViewLayout = new QVBoxLayout;
	mViewLayout->addLayout(mOrderLayout);
	mViewLayout->addStretch(1);
	mViewLayout->addSpacing(12);
	mViewLayout->addLayout(mBtnLayout);	
	setLayout(mViewLayout);
	initConnect(pStrategyWidget);
	setWindowTitle(STRATERGY_WINDOW_TITLE);
}

USTParamDialog::~USTParamDialog()
{

}

QGroupBox* USTParamDialog::createOrderGroup()
{	
	QGroupBox *groupBox = new QGroupBox(tr("��������"));

	mFirstInsLabel = new QLabel(tr("��Լһ:"));
	mFirstInsEdit = new QLineEdit;
	mFirstInsLabel->setBuddy(mFirstInsEdit);

	mSecondInsLabel = new QLabel(tr("��Լ��:"));
	mSecondInsEdit = new QLineEdit;
	mSecondInsLabel->setBuddy(mSecondInsEdit);

	mStyleLabel = new QLabel(tr("��������:"));
	QStringList list;
	list << tr("0-��������") << tr("1-��������") << tr("2-�����Լ�����") << tr("3-������������") << tr("4-�Լ۶�������");
	mStyleComboBox = createComboBox(list);
	mStyleComboBox->setCurrentIndex(0);
	mStyleLabel->setBuddy(mStyleComboBox);

	mDirectionLabel = new QLabel(tr("��/��:"));
	QStringList bsList;
	bsList << tr("0-����") << tr("1-����");
	mBSComboBox = createComboBox(bsList);
	mDirectionLabel->setBuddy(mBSComboBox);

	mOffsetLabel = new QLabel(tr("��/ƽ:"));
	QStringList offsetList;
	offsetList << tr("0-����") << tr("1-ƽ��") << tr("2-ƽ��") << tr("3-ƽ��");
	mOffsetComboBox = createComboBox(offsetList);
	mOffsetLabel->setBuddy(mOffsetComboBox);

	mHedgeLabel = new QLabel(tr("Ͷ��/�ױ�:"));
	QStringList hedgeList;
	hedgeList << tr("0-Ͷ��") << tr("1-����");
	mHedgeComboBox = createComboBox(hedgeList);
	mHedgeComboBox->setCurrentIndex(1);
	mHedgeLabel->setBuddy(mHedgeComboBox);

	mPriceLabel = new QLabel(tr("ί�м۲�:"));
	mPriceSpinBox = new QDoubleSpinBox;
	mPriceSpinBox->setRange(-10000.0, 10000.0);
	mPriceSpinBox->setDecimals(3);
	mPriceSpinBox->setSingleStep(1.0);
	mPriceSpinBox->setValue(20.0);
	mPriceLabel->setBuddy(mPriceSpinBox);

	mQtyLabel = new QLabel(tr("ί����:"));
	mQtySpinBox = new QSpinBox;
	mQtySpinBox->setRange(1, 1000);
	mQtySpinBox->setSingleStep(1);
	mQtySpinBox->setValue(1);
	mQtyLabel->setBuddy(mQtySpinBox);

	mFirstTimeLabel = new QLabel(tr("��Լһʱ��:"));
	mFirstTimeSpinBox = new QSpinBox;
	mFirstTimeSpinBox->setRange(1, 60000);
	mFirstTimeSpinBox->setSingleStep(1);
	mFirstTimeSpinBox->setValue(400);
	mFirstTimeLabel->setBuddy(mFirstTimeSpinBox);

	mSecondTimeLabel = new QLabel(tr("��Լ��ʱ��:"));
	mSecondTimeSpinBox = new QSpinBox;
	mSecondTimeSpinBox->setRange(1, 60000);
	mSecondTimeSpinBox->setSingleStep(1);
	mSecondTimeSpinBox->setValue(1000);
	mSecondTimeLabel->setBuddy(mSecondTimeSpinBox);

	mReferTickLabel = new QLabel(tr("�ο�Tick��:"));
	mReferTickSpinBox = new QSpinBox;
	mReferTickSpinBox->setRange(1, 3);
	mReferTickSpinBox->setSingleStep(1);
	mReferTickSpinBox->setValue(1);
	mReferTickLabel->setBuddy(mReferTickSpinBox);

	mSuperPriceLabel = new QLabel(tr("���۵�:"));
	mSuperPriceSpinBox = new QSpinBox;
	mSuperPriceSpinBox->setRange(-5000, 5000);
	mSuperPriceSpinBox->setSingleStep(1);
	mSuperPriceSpinBox->setValue(50);
	mSuperPriceLabel->setBuddy(mSuperPriceSpinBox);

	mActionReferTickLabel = new QLabel(tr("�����ο�Tick��:"));
	mActionReferTickSpinBox = new QSpinBox;
	mActionReferTickSpinBox->setRange(1, 10);
	mActionReferTickSpinBox->setSingleStep(1);
	mActionReferTickSpinBox->setValue(3);
	mActionReferTickLabel->setBuddy(mActionReferTickSpinBox);

	mActionSuperSlipLabel = new QLabel(tr("�������۵�:"));
	mActionSlipSpinBox = new QSpinBox;
	mActionSlipSpinBox->setRange(-100, 100);
	mActionSlipSpinBox->setSingleStep(1);
	mActionSlipSpinBox->setValue(1);
	mActionSuperSlipLabel->setBuddy(mActionSlipSpinBox);


	mCycleLabel = new QLabel(tr("ѭ����:"));
	mCycleSpinBox = new QSpinBox;
	mCycleSpinBox->setRange(1, 1000);
	mCycleSpinBox->setSingleStep(1);
	mCycleSpinBox->setValue(10);
	mCycleLabel->setBuddy(mCycleSpinBox);

	mFirstTimeCheckBox = new QCheckBox(tr("��Լһ��ʱ����"));
	mFirstTimeCheckBox->setChecked(false);

	mSecondTimeCheckBox = new QCheckBox(tr("��Լ����ʱ����"));
	mSecondTimeCheckBox->setChecked(false);

	mDefineOrderCheckBox = new QCheckBox(tr("��ʱ����"));
	mDefineOrderCheckBox->setChecked(false);
	mDefineOrderCheckBox->setEnabled(false);

	mOppoentPriceCheckBox = new QCheckBox(tr("�Լ�"));
	mOppoentPriceCheckBox->setChecked(false);
	mOppoentPriceCheckBox->setEnabled(false);

	mCycleCheckBox = new QCheckBox(tr("ѭ��"));
	mCycleCheckBox->setChecked(false);

	mActionReferTickCheckBox = new QCheckBox(tr("�����ο�Tick"));
	mActionReferTickCheckBox->setChecked(false);


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

	vBox->addWidget(mFirstTimeLabel, 4, 0, 1, 1);
	vBox->addWidget(mFirstTimeSpinBox, 4, 1, 1, 2);
	vBox->addWidget(mSecondTimeLabel, 4, 3, 1, 1);
	vBox->addWidget(mSecondTimeSpinBox, 4, 4, 1, 2);

	vBox->addWidget(mReferTickLabel, 5, 0, 1, 1);
	vBox->addWidget(mReferTickSpinBox, 5, 1, 1, 2);
	vBox->addWidget(mSuperPriceLabel, 5, 3, 1, 1);
	vBox->addWidget(mSuperPriceSpinBox, 5, 4, 1, 2);

	vBox->addWidget(mActionReferTickLabel, 6, 0, 1, 1);
	vBox->addWidget(mActionReferTickSpinBox, 6, 1, 1, 2);
	vBox->addWidget(mActionReferTickCheckBox, 6, 3, 1, 1);

	vBox->addWidget(mActionSuperSlipLabel, 7, 0, 1, 1);
	vBox->addWidget(mActionSlipSpinBox, 7, 1, 1, 2);

	vBox->addWidget(mCycleLabel, 8, 0, 1, 1);
	vBox->addWidget(mCycleSpinBox, 8, 1, 1, 2);
	vBox->addWidget(mCycleCheckBox, 8, 3, 1, 1);

	vBox->addWidget(mFirstTimeCheckBox, 9, 0, 1, 1);
	vBox->addWidget(mSecondTimeCheckBox, 9, 1, 1, 1);
	vBox->addWidget(mDefineOrderCheckBox, 9, 3, 1, 1);
	vBox->addWidget(mOppoentPriceCheckBox, 9, 4, 1, 1);
	groupBox->setLayout(vBox);
	return groupBox;
}

QGroupBox* USTParamDialog::createSpotGroup()
{
	QGroupBox *groupBox = new QGroupBox(tr("�������"));
	mFirstOpenLabel = new QLabel(tr("��Լһ���۵�:"));
	mFirstPriceSpinBox = new QSpinBox;
	mFirstPriceSpinBox->setRange(-1000, 1000);
	mFirstPriceSpinBox->setSingleStep(1);
	mFirstPriceSpinBox->setValue(1);
	mFirstOpenLabel->setBuddy(mFirstPriceSpinBox);

	mSecondOpenLabel = new QLabel(tr("��Լ�����۵�:"));
	mSecondPriceSpinBox = new QSpinBox;
	mSecondPriceSpinBox->setRange(-1000, 1000);
	mSecondPriceSpinBox->setSingleStep(1);
	mSecondPriceSpinBox->setValue(1);
	mSecondOpenLabel->setBuddy(mSecondPriceSpinBox);


	QGridLayout* vBox = new QGridLayout;
	vBox->addWidget(mFirstOpenLabel, 0, 0, 1, 1);
	vBox->addWidget(mFirstPriceSpinBox, 0, 1, 1, 2);
	vBox->addWidget(mSecondOpenLabel, 0, 3, 1, 1);
	vBox->addWidget(mSecondPriceSpinBox, 0, 4, 1, 2);
	groupBox->setLayout(vBox);
	return groupBox;
}

QComboBox* USTParamDialog::createComboBox(const QStringList &itemList)
{
	QComboBox *comboBox = new QComboBox;
	foreach(QString item, itemList){
		comboBox->addItem(item);
	}
	comboBox->setEditable(true);
	comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	return comboBox;
}

QPushButton *USTParamDialog::createButton(const QString &text, const char *member)
{
	QPushButton *button = new QPushButton(text);
	connect(button, SIGNAL(clicked()), this, member);
	return button;
}

void USTParamDialog::initConnect(USTPStrategyWidget* pStrategyWidget)
{	
	connect(mStyleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(doEnableBox(int)));

	connect(this, SIGNAL(onCreateNewOrder(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const double&, 
		const int&, const int&, const int&, const int&, const int&, const int&, const int&, const int&, const int&, const int&, bool, bool, bool, bool, bool, bool)), 
		pStrategyWidget, SLOT(doCreateNewOrder(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const double&, 
		const int&, const int&, const int&, const int&, const int&, const int&, const int&, const int&, const int&, const int&, bool, bool, bool, bool, bool, bool)));
}

void USTParamDialog::doEnableBox(int selIndex)
{
	switch (selIndex)
	{
	case 0:
		mOppoentPriceCheckBox->setChecked(false);
		mOppoentPriceCheckBox->setEnabled(false);	
		mDefineOrderCheckBox->setChecked(false);
		mDefineOrderCheckBox->setEnabled(false);
		break;
	case 1:
		mOppoentPriceCheckBox->setChecked(false);
		mOppoentPriceCheckBox->setEnabled(true);
		mDefineOrderCheckBox->setChecked(false);
		mDefineOrderCheckBox->setEnabled(true);
		break;
	case 2:
		mOppoentPriceCheckBox->setChecked(false);
		mOppoentPriceCheckBox->setEnabled(true);
		mDefineOrderCheckBox->setChecked(false);
		mDefineOrderCheckBox->setEnabled(true);
		break;
	case 3:
		mOppoentPriceCheckBox->setChecked(false);
		mOppoentPriceCheckBox->setEnabled(false);	
		mDefineOrderCheckBox->setChecked(false);
		mDefineOrderCheckBox->setEnabled(false);
		break;
	case 4:
		mOppoentPriceCheckBox->setChecked(false);
		mOppoentPriceCheckBox->setEnabled(true);
		mDefineOrderCheckBox->setChecked(false);
		mDefineOrderCheckBox->setEnabled(true);
		break;
	}
}


void USTParamDialog::createOrder()
{
	QString firstName, secondName;
	firstName = mFirstInsEdit->text();
	secondName = mSecondInsEdit->text();
	if(firstName.isEmpty() || secondName.isEmpty()){
		ShowWarning(tr("��Լ���ò���Ϊ��."));
	}else{
		QString style = mStyleComboBox->currentText();
		QString bs = mBSComboBox->currentText();
		QString offset = mOffsetComboBox->currentText();
		QString hedge = mHedgeComboBox->currentText();
		double price = mPriceSpinBox->value();
		int qty = mQtySpinBox->value();
		int firtTime = mFirstTimeSpinBox->value();
		int secTime = mSecondTimeSpinBox->value();
		int firstPrice = mFirstPriceSpinBox->value();
		int secondPrice = mSecondPriceSpinBox->value();
		int referTickNum = mReferTickSpinBox->value();
		int superTickNum = mSuperPriceSpinBox->value();
		int actionReferTickNum = mActionReferTickSpinBox->value();
		int actionSuperSlipNum = mActionSlipSpinBox->value();
		int cyclePrice = mCycleSpinBox->value();
		bool isDefineOrderCheck = mDefineOrderCheckBox->isChecked();
		bool isFirstTimeCheck = mFirstTimeCheckBox->isChecked();
		bool isSecondTimeCheck = mSecondTimeCheckBox->isChecked();
		bool isOppoentPriceCheck = mOppoentPriceCheckBox->isChecked();
		bool isCycle = mCycleCheckBox->isChecked();
		bool isActionReferTick = mActionReferTickCheckBox->isChecked();
		emit onCreateNewOrder(firstName, secondName, style, bs, offset, hedge, price, qty, firtTime, secTime, firstPrice, secondPrice, referTickNum,
			superTickNum, cyclePrice, actionReferTickNum, actionSuperSlipNum, isDefineOrderCheck, isOppoentPriceCheck, isFirstTimeCheck, isSecondTimeCheck,
			isCycle, isActionReferTick);
		accept();
	}
}

#include "moc_USTParamDialog.cpp"