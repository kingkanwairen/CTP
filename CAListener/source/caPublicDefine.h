#include <QtCore/QString>

#define STATUS_DOCK_WIDGET tr("Engine״̬")
#define ORDER_DOCK_WIDGET tr("ί��״̬")
#define TRADE_DOCK_WIDGET tr("�ɽ�״̬")
#define POSITION_DOCK_WIDGET tr("�ֲ�״̬")

#define MAIN_WINDOW_TILTE tr("�ϻ��ڻ�APAMA�ն˳���")

#define  CONFIG_WINDOW_TITLE tr("ѡ��")
#define  DELETE_WINDOW_TITLE tr("ɾ��")
#define  EVENT_WINDOW_TITLE tr("����")
#define  MONITOR_WINDOW_TITLE tr("ע��")

#define  MAIN_WINDOW_TAB_TITLE tr("����״̬")

#define  OMS_ORDER_EVENT tr("OmsOrder")
#define  OMS_TRADE_EVENT tr("OmsTrade")
#define  OMS_POSITION_EVENT tr("OmsPosition")
#define  OMS_LOG_EVENT tr("OmsLog")


#define  TREE_WIDGET_TAB_HEAD_1 tr("Time")
#define  TREE_WIDGET_TAB_HEAD_2 tr("Message")

#define  ORDER_WIDGET_TAB_HEAD_1 tr("Symbol")
#define  ORDER_WIDGET_TAB_HEAD_2 tr("BS")
#define  ORDER_WIDGET_TAB_HEAD_3 tr("Price")
#define  ORDER_WIDGET_TAB_HEAD_4 tr("QtyTitle")
#define  ORDER_WIDGET_TAB_HEAD_5 tr("QtyRemain")
#define  ORDER_WIDGET_TAB_HEAD_6 tr("QtyExecuted")
#define  ORDER_WIDGET_TAB_HEAD_7 tr("OrderTime")
#define  ORDER_WIDGET_TAB_HEAD_8 tr("UpdateTime")
#define  ORDER_WIDGET_TAB_HEAD_9 tr("LogTime")
#define  ORDER_WIDGET_TAB_HEAD_10 tr("Key")
#define  ORDER_HEAD_LENGTH 10

#define  TRADE_WIDGET_TAB_HEAD_1 tr("TradeID")
#define  TRADE_WIDGET_TAB_HEAD_2 tr("Symbol")
#define  TRADE_WIDGET_TAB_HEAD_3 tr("BS")
#define  TRADE_WIDGET_TAB_HEAD_4 tr("Qty")
#define  TRADE_WIDGET_TAB_HEAD_5 tr("Price")
#define  TRADE_WIDGET_TAB_HEAD_6 tr("TradeTime")
#define  TRADE_WIDGET_TAB_HEAD_7 tr("LogTime")
#define  TRADE_WIDGET_TAB_HEAD_8 tr("Key")
#define  TRADE_HEAD_LENGTH 8

#define  POSITION_WIDGET_TAB_HEAD_1 tr("Symbol")
#define  POSITION_WIDGET_TAB_HEAD_2 tr("BS")
#define  POSITION_WIDGET_TAB_HEAD_3 tr("Qty")
#define  POSITION_WIDGET_TAB_HEAD_4 tr("AvgPrice")
#define  POSITION_WIDGET_TAB_HEAD_5 tr("LogTime")
#define  POSITION_WIDGET_TAB_HEAD_6 tr("Key")
#define  POSITION_HEAD_LENGTH 6


#define LINK_LABEL_WIDTH 16
#define LINK_LABEL_HEIGHT 16


#define STATUS_LABEL_WIDTH 360
#define STATUS_LABEL_HEIGHT 300
#define STATUS_WINDOW_MAX_WIDTH 360

#define ORDER_WINDOW_MIN_HEIGHT 200
#define TRADE_WINDOW_MIN_HEIGHT 200
#define POSITION_WINDOW_MIN_HEIGHT 200

#define MAIN_WINDOW_WIDTH 800
#define MAIN_WINDOW_HEIGHT 500

#define OMS_QUERY_CHANNEL "OMS_QUERY_CHANNEL"
#define OMS_LOG_CHANNEL "OMS_LOG_CHANNEL"
#define OMS_EXTEND_CHANNEL "OMS_EXTEND_CHANNEL"

#define  ShowWarning(msg) QMessageBox::warning(this, tr("��ʾ"), msg)
#define  ShowCritical(msg) QMessageBox::critical(NULL, tr("��ʾ"), msg)