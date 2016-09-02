#include "caPage.h"

caNormalPage::caNormalPage(const QString& ip, const int& nPort, QWidget *parent)
    : QWidget(parent)
{
    mLinkGroup = new QGroupBox(tr("连接参数"));
	mIpLabel = new QLabel(tr("Ip:"));
	mIpLineEdit = new QLineEdit(ip, this);
	mIpLineEdit->setInputMask(QString(tr("000.000.000.000; ")));
	mIpLabel->setBuddy(mIpLineEdit);

	mPortLabel = new QLabel(tr("Port:"));
	mPortLineEdit = new QLineEdit(QString::number(nPort), this);
	mPortLabel->setBuddy(mPortLineEdit);

    mLinkLayout = new QGridLayout;
    mLinkLayout->addWidget(mIpLabel, 0, 0, 1, 1);
    mLinkLayout->addWidget(mIpLineEdit, 0, 1, 1, 3);
	mLinkLayout->addWidget(mPortLabel, 1, 0, 1, 1);
	mLinkLayout->addWidget(mPortLineEdit, 1, 1, 1, 3);
	mLinkGroup->setLayout(mLinkLayout);

    mViewLayout = new QVBoxLayout;
    mViewLayout->addWidget(mLinkGroup);
    mViewLayout->addStretch(1);
    setLayout(mViewLayout);
}


caNormalPage::~caNormalPage()
{	
	if (mIpLabel)
	{
		delete mIpLabel;
		mIpLabel = NULL;
	}

	if (mIpLineEdit)
	{
		delete mIpLineEdit;
		mIpLineEdit = NULL;
	}

	if (mPortLabel)
	{
		delete mPortLabel;
		mPortLabel = NULL;
	}

	if (mPortLineEdit)
	{
		delete mPortLineEdit;
		mPortLineEdit = NULL;
	}

	if (mLinkLayout)
	{
		delete mLinkLayout;
		mLinkLayout = NULL;
	}

	if (mViewLayout)
	{
		delete mViewLayout;
		mViewLayout = NULL;
	}
}

int caNormalPage::GetPort()
{
	QString strPort = mPortLineEdit->text();

	if (!strPort.isEmpty())
	{
		return strPort.toInt();
	}
	return -1;
}

QString caNormalPage::GetIp()
{
	QString strIp = mIpLineEdit->text();
	if (!strIp.isEmpty()){
		return strIp;
	}
	return QDir::currentPath();
}

caFilePage::caFilePage(const QString& path, const int& status, QWidget *parent)
    : QWidget(parent)
{
	mPathGroup = new QGroupBox(tr("路径"));
	mPathGroup->setCheckable(true);
	mPathGroup->setChecked(status);

	mPathLabel = new QLabel(tr("存放路径:"));
	mBrowseButton = createButton(tr("&Browse..."), SLOT(browse()));
	mDirectoryComboBox = createComboBox(path);
	mPathLabel->setBuddy(mDirectoryComboBox);

	mPathLayout = new QGridLayout;
	mPathLayout->addWidget(mPathLabel, 0, 0, 1, 1);
	mPathLayout->addWidget(mDirectoryComboBox, 0, 1, 1, 3);
	mPathLayout->addWidget(mBrowseButton, 0, 4, 1, 1);
	mPathGroup->setLayout(mPathLayout);

	mViewLayout = new QVBoxLayout;
	mViewLayout->addWidget(mPathGroup);
	mViewLayout->addStretch(1);
	setLayout(mViewLayout);
}

caFilePage::~caFilePage()
{
	if (mPathLabel)
	{
		delete mPathLabel;
		mPathLabel = NULL;
	}

	if (mPathLayout)
	{
		delete mPathLayout;
		mPathLayout = NULL;
	}

	if (mViewLayout)
	{
		delete mViewLayout;
		mViewLayout = NULL;
	}
}

void caFilePage::browse()
{
	QFileDialog::Options options;
	options |= QFileDialog::DontUseNativeDialog;
	QString selectedFilter;
	QString fileName = QFileDialog::getOpenFileName(this, tr("日志文件"),
		mDirectoryComboBox->currentText(),
		tr("All Files (*);;Text Files (*.txt)"),
		&selectedFilter, options);
	if (!fileName.isEmpty()) {
		if (mDirectoryComboBox->findText(fileName) == -1)
			mDirectoryComboBox->addItem(fileName);
		mDirectoryComboBox->setCurrentIndex(mDirectoryComboBox->findText(fileName));
	}
}

QComboBox* caFilePage::createComboBox(const QString &text)
{
	QComboBox *comboBox = new QComboBox;
	comboBox->setEditable(true);
	comboBox->addItem(text);
	comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	return comboBox;
}

QPushButton *caFilePage::createButton(const QString &text, const char *member)
{
	QPushButton *button = new QPushButton(text);
	connect(button, SIGNAL(clicked()), this, member);
	return button;
}

bool caFilePage::GetSaveStatus()
{
	return mPathGroup->isChecked();

}
QString caFilePage::GetPath()
{	
	return mDirectoryComboBox->currentText();
}

#include "moc_caPage.cpp"
