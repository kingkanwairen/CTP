#include "caDeleteDialog.h"
#include "caPublicDefine.h"
#include <QtGui/QtGui>
#include <vector>
using namespace  std;

caDeleteDialog::caDeleteDialog(CaiApi* pCaiApi, QWidget* parent)
{	
	m_pCaiApi = pCaiApi;
	mEventGroup = new QGroupBox(tr("¶ÔÏó"));
	mEventLabel = new QLabel(tr("Monitor or Event:"));
	mEventLineEdit = new QLineEdit(this);
	mEventLabel->setBuddy(mEventLineEdit);

	mEventLayout = new QGridLayout;
	mEventLayout->addWidget(mEventLabel, 0, 0, 1, 1);
	mEventLayout->addWidget(mEventLineEdit, 0, 1, 1, 3);
	mEventGroup->setLayout(mEventLayout);

	mDeleteBtn = createButton(tr("É¾³ý"), SLOT(deleteEvent()));
	mCancelBtn = createButton(tr("¹Ø±Õ"), SLOT(close()));
	mViewLayout = new QGridLayout;
	mViewLayout->addWidget(mEventGroup, 0, 0, 1, 5);
	mViewLayout->addWidget(mDeleteBtn, 1, 1, 1, 1);
	mViewLayout->addWidget(mCancelBtn, 1, 3, 1, 1);
	setLayout(mViewLayout);
	setWindowTitle(DELETE_WINDOW_TITLE);
}

caDeleteDialog::~caDeleteDialog()
{
	if (mEventLabel){
		delete mEventLabel;
		mEventLabel = NULL;
	}

	if (mEventLineEdit){
		delete mEventLineEdit;
		mEventLineEdit = NULL;
	}

	if (mDeleteBtn){
		delete mDeleteBtn;
		mDeleteBtn = NULL;
	}

	if (mCancelBtn){
		delete mCancelBtn;
		mCancelBtn = NULL;
	}

	if (mEventLayout){
		delete mEventLayout;
		mEventLayout = NULL;
	}

	if (mViewLayout){
		delete mViewLayout;
		mViewLayout = NULL;
	}
}

void caDeleteDialog::deleteEvent()
{	
	QString name = mEventLineEdit->text();
	if (!name.isEmpty()){
		string stdEvent = name.toStdString();
		m_pCaiApi->DeleteMonitorScript(const_cast<char*>(stdEvent.c_str()));
		mEventLineEdit->clear();
	}
}


QPushButton *caDeleteDialog::createButton(const QString &text, const char *member)
{
	QPushButton *button = new QPushButton(text);
	connect(button, SIGNAL(clicked()), this, member);
	return button;
}

#include "moc_caDeleteDialog.cpp"