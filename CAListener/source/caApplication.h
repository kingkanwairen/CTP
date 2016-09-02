#ifndef caApplication_h
#define caApplication_h

#include <QtGui/QApplication>


class caApplication : public QApplication
{
	Q_OBJECT

public:
	
	caApplication(int& argc, char** argv);

	~caApplication();

	bool Initialize();

	bool Finish();

private:
};

#endif