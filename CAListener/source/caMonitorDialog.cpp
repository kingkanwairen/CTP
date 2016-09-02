#include "caMonitorDialog.h"
#include "caPublicDefine.h"
#include <QtGui/QtGui>
#include <vector>
using namespace  std;

caMonitorDialog::caMonitorDialog(CaiApi* pCaiApi, QWidget* parent)
{	
	m_pCaiApi = pCaiApi;
	mPathGroup = new QGroupBox(tr("Monitor路径"));
	mPathLabel = new QLabel(tr("Path:"));
	mPathLineEdit = new QLineEdit(this);
	mPathLabel->setBuddy(mPathLineEdit);
	mFindBtn = createButton(tr("&Browse..."), SLOT(browse()));
	
	mPathLayout = new QGridLayout;
	mPathLayout->addWidget(mPathLabel, 0, 0, 1, 1);
	mPathLayout->addWidget(mPathLineEdit, 0, 1, 1, 3);
	mPathLayout->addWidget(mFindBtn, 0, 4, 1, 1);
	mPathGroup->setLayout(mPathLayout);
	

	mInsertBtn = createButton(tr("注入"), SLOT(insertMonitor()));
	mCancelBtn = createButton(tr("关闭"), SLOT(close()));
	mViewLayout = new QGridLayout;
	mViewLayout->addWidget(mPathGroup, 0, 0, 1, 5);
	mViewLayout->addWidget(mInsertBtn, 1, 1, 1, 1);
	mViewLayout->addWidget(mCancelBtn, 1, 3, 1, 1);
	setLayout(mViewLayout);
	setWindowTitle(MONITOR_WINDOW_TITLE);
}

caMonitorDialog::~caMonitorDialog()
{
	if (mPathLabel){
		delete mPathLabel;
		mPathLabel = NULL;
	}

	if (mPathLineEdit){
		delete mPathLineEdit;
		mPathLineEdit = NULL;
	}

	if (mFindBtn){
		delete mFindBtn;
		mFindBtn = NULL;
	}

	if (mInsertBtn){
		delete mInsertBtn;
		mInsertBtn = NULL;
	}

	if (mCancelBtn){
		delete mCancelBtn;
		mCancelBtn = NULL;
	}

	if (mPathLayout){
		delete mPathLayout;
		mPathLayout = NULL;
	}
	
	if (mViewLayout){
		delete mViewLayout;
		mViewLayout = NULL;
	}
}

void caMonitorDialog::insertMonitor()
{	
	QString path = mPathLineEdit->text();
	if (!path.isEmpty()){
		QFile file(path);  
		if (!file.open(QFile::ReadOnly)) { 
			ShowWarning(tr("文件读取失败."));
			return;
		}
		QTextStream stream(&file);
		QString monitor;
		QString line;
		do {
			line = stream.readLine();
			monitor += line;
		} while (!line.isNull());
		string stdMonitor = monitor.toStdString();
		m_pCaiApi->CreateMonitorScript(const_cast<char*>(stdMonitor.c_str()));
		mPathLineEdit->clear();
	}
}


QPushButton *caMonitorDialog::createButton(const QString &text, const char *member)
{
	QPushButton *button = new QPushButton(text);
	connect(button, SIGNAL(clicked()), this, member);
	return button;
}

void caMonitorDialog::browse()
{
	QFileDialog::Options options;
	options |= QFileDialog::DontUseNativeDialog;
	QString selectedFilter;
	QString fileName = QFileDialog::getOpenFileName(this, tr("获取monitor文件"),
		mPathLineEdit->text(),
		tr("All Files (*);;Text Files (*.txt);;Monitor Files (*.mon)"),
		&selectedFilter,
		options);
	if (!fileName.isEmpty())
		mPathLineEdit->setText(fileName);
}

#include "moc_caMonitorDialog.cpp"