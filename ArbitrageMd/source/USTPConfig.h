#include <QtCore/QString>

#define MARKET_DOCK_WIDGET tr("������Ϣ")
#define MAIN_WINDOW_TILTE tr("���������ʾϵͳ")
#define LOGIN_WINDOW_TITLE tr("��¼")
#define PARAM_WINDOW_TITLE tr("��������")

#define COMBO_MARKET_WINDOW_TITLE tr("��Ϻ�Լ����")
#define COMBO_MARKET_WINDOW_WIDTH 560
#define COMBO_MARKET_WINDOW_HEIGHT 300


#define  DEPTH_WIDGET_TAB_HEAD_0 tr("Key")
#define  DEPTH_WIDGET_TAB_HEAD_1 tr("��һ��")
#define  DEPTH_WIDGET_TAB_HEAD_2 tr("�ڶ���")
#define  DEPTH_WIDGET_TAB_HEAD_3 tr("��-���")
#define  DEPTH_WIDGET_TAB_HEAD_4 tr("��-����")
#define  DEPTH_WIDGET_TAB_HEAD_5 tr("��-���")
#define  DEPTH_WIDGET_TAB_HEAD_6 tr("��-����")
#define  DEPTH_WIDGET_TAB_HEAD_7 tr("��-����")
#define  DEPTH_WIDGET_TAB_HEAD_8 tr("��-����")
#define  DEPTH_WIDGET_TAB_HEAD_9 tr("��-����")
#define  DEPTH_WIDGET_TAB_HEAD_10 tr("��-����")
#define  DEPTH_HEAD_LENGTH 11

#define LINK_LABEL_WIDTH 16
#define LINK_LABEL_HEIGHT 16

#define STATUS_LABEL_WIDTH 360
#define STATUS_LABEL_HEIGHT 300
#define STATUS_WINDOW_MAX_WIDTH 360
#define MARKET_WINDOW_MIN_WIDTH 550

#define LOGIN_WINDOW_HEIGHT 350
#define LOGIN_WINDOW_WIDTH 600

#define  ShowInfo(msg) QMessageBox::information(this, tr("��ʾ"), msg)
#define  ShowWarning(msg) QMessageBox::warning(this, tr("��ʾ"), msg)
#define  ShowCritical(msg) QMessageBox::critical(NULL, tr("��ʾ"), msg)


#define LOG_FILE "../log/trade_"
#define SYSTEM_FILE_PATH  "../config/config.xml"
#define SYSTEM_LICESE_PATH  "../config/license.txt"
#define PARAM_FILE_PATH "../config/param.txt"

#define CTP_INITINAL_ERROR -1
#define CTP_LOAD_ERROR -2
#define CTP_STOP_ERROR -3
#define CTP_FINALIZE_ERROR -4

#define VALUE_1 1.0

#define  INIT_VALUE 10000000

