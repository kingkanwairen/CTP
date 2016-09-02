#include "caEngineRsp.h"
#include "caLogManager.h"
#include "caPublicDefine.h"
#include <QtCore/QStringList>

caEngineRsp::caEngineRsp(void)
{
}

caEngineRsp::~caEngineRsp(void)
{
}

void caEngineRsp::OnRecvMsg(char* pText)
{	
	if(pText != NULL){
		QString recvEvent = QString(pText);
		int iNamePos = recvEvent.indexOf("(", 0);
		QString remain, key, remainItem;
		QStringList items;

		if(iNamePos != -1){
			QString name = recvEvent.mid(0, iNamePos).trimmed();
			if(OMS_ORDER_EVENT == name){	//委托
				remain = recvEvent.mid(iNamePos + 1);
				remainItem = remain.mid(0, remain.length() - 1);
				items = remainItem.split(",");
				if (items.size() == ORDER_HEAD_LENGTH - 1){
					key = items[0] + tr("_") + items[1];
					items.append(key);
					emit onUpdateOrder(items);
				}

			}else if(OMS_TRADE_EVENT == name){	//成交
				remain = recvEvent.mid(iNamePos + 1);
				remainItem = remain.mid(0, remain.length() - 1);
				items = remainItem.split(",");
				if (items.size() == TRADE_HEAD_LENGTH - 1){
					key = items[1] + tr("_") + items[2];
					items.append(key);
					emit onUpdateTrade(items);
				}

			}else if(OMS_POSITION_EVENT == name){	//持仓
				remain = recvEvent.mid(iNamePos + 1);
				remainItem = remain.mid(0, remain.length() - 1);
				items = remainItem.split(",");
				if (items.size() == POSITION_HEAD_LENGTH - 1){
					key = items[0] + tr("_") + items[1];
					items.append(key);
					emit onUpdatePosition(items);
				}
			
			}else if(OMS_LOG_EVENT == name){	//策略
				remain = recvEvent.mid(iNamePos + 1);
				remainItem = remain.mid(0, remain.length() - 1);
				items = remainItem.split(",");
				emit onUpdateLog(items);
			}
		}
		caLogManager::saveData(QString(pText));
	}
}


void caEngineRsp::OnCorrelatorDisconnect(char* pReason)
{	
	if(pReason != NULL){	
		emit onDisconnetEngine(QString(pReason));
		caLogManager::saveData(QString(pReason));
	}
	
}

#include "moc_caEngineRsp.cpp"