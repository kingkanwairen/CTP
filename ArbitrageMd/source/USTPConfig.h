#include <QtCore/QString>

#define MARKET_DOCK_WIDGET tr("行情信息")
#define MAIN_WINDOW_TILTE tr("组合行情显示系统")
#define LOGIN_WINDOW_TITLE tr("登录")
#define PARAM_WINDOW_TITLE tr("订单设置")

#define COMBO_MARKET_WINDOW_TITLE tr("组合合约设置")
#define COMBO_MARKET_WINDOW_WIDTH 560
#define COMBO_MARKET_WINDOW_HEIGHT 300


#define  DEPTH_WIDGET_TAB_HEAD_0 tr("Key")
#define  DEPTH_WIDGET_TAB_HEAD_1 tr("第一腿")
#define  DEPTH_WIDGET_TAB_HEAD_2 tr("第二腿")
#define  DEPTH_WIDGET_TAB_HEAD_3 tr("买-买价")
#define  DEPTH_WIDGET_TAB_HEAD_4 tr("买-卖价")
#define  DEPTH_WIDGET_TAB_HEAD_5 tr("卖-买价")
#define  DEPTH_WIDGET_TAB_HEAD_6 tr("卖-卖价")
#define  DEPTH_WIDGET_TAB_HEAD_7 tr("买-买量")
#define  DEPTH_WIDGET_TAB_HEAD_8 tr("买-卖量")
#define  DEPTH_WIDGET_TAB_HEAD_9 tr("卖-买量")
#define  DEPTH_WIDGET_TAB_HEAD_10 tr("卖-卖量")
#define  DEPTH_HEAD_LENGTH 11

#define LINK_LABEL_WIDTH 16
#define LINK_LABEL_HEIGHT 16

#define STATUS_LABEL_WIDTH 360
#define STATUS_LABEL_HEIGHT 300
#define STATUS_WINDOW_MAX_WIDTH 360
#define MARKET_WINDOW_MIN_WIDTH 550

#define LOGIN_WINDOW_HEIGHT 350
#define LOGIN_WINDOW_WIDTH 600

#define  ShowInfo(msg) QMessageBox::information(this, tr("提示"), msg)
#define  ShowWarning(msg) QMessageBox::warning(this, tr("提示"), msg)
#define  ShowCritical(msg) QMessageBox::critical(NULL, tr("提示"), msg)


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

