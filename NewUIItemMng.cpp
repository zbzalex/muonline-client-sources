// NewUIItemMng.cpp: implementation of the CNewUIItemMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIItemMng.h"
#include "CSItemOption.h"
#include "GIPetManager.h"
#include "ZzzInfomation.h"
#ifdef SOCKET_SYSTEM
#include "SocketSystem.h"
#endif	// SOCKET_SYSTEM

using namespace SEASON3B;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIItemMng::CNewUIItemMng() 
{
	m_dwAlternate = 0;
	m_dwAvailableKeyStream = 0x80000000;
	m_UpdateTimer.SetTimer(1000);
}

SEASON3B::CNewUIItemMng::~CNewUIItemMng() 
{ 
	DeleteAllItems(); 
}

ITEM* SEASON3B::CNewUIItemMng::CreateItem(BYTE* pbyItemPacket)
{
	size_t ItemPacketSize = 5;
	ItemPacketSize++;
	ItemPacketSize++;

	if(IsBadReadPtr(pbyItemPacket, ItemPacketSize))
		return NULL;

	WORD wType = ExtractItemType(pbyItemPacket);
	BYTE byOption380 = 0, byOptionHarmony = 0;
	
	byOption380 = pbyItemPacket[5];
	byOptionHarmony = pbyItemPacket[6];

#ifdef MODIFY_SOCKET_PROTOCOL
	// 家南 酒捞袍 可记 (0x00~0xF9: 可记绊蜡锅龋, 0xFE: 后 家南, 0xFF: 阜腮 家南)
	BYTE bySocketOption[5] = { pbyItemPacket[7], pbyItemPacket[8], pbyItemPacket[9], pbyItemPacket[10], pbyItemPacket[11] };
#else	// MODIFY_SOCKET_PROTOCOL
	BYTE bySocketOption[5] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
#endif	// MODIFY_SOCKET_PROTOCOL
	
	return CNewUIItemMng::CreateItem(wType/MAX_ITEM_INDEX, wType%MAX_ITEM_INDEX, pbyItemPacket[1], pbyItemPacket[2], pbyItemPacket[3], 
		pbyItemPacket[4], byOption380, byOptionHarmony
#ifdef SOCKET_SYSTEM
		, bySocketOption
#endif	// SOCKET_SYSTEM
		);
}

ITEM* SEASON3B::CNewUIItemMng::CreateItem(BYTE byType, BYTE bySubType, BYTE byLevel /* = 0 */, BYTE byDurability /* = 255 */, 
								BYTE byOption1 /* = 0 */, BYTE byOptionEx /* = 0 */, BYTE byOption380 /* = 0 */, 
								BYTE byOptionHarmony /* = 0 */
#ifdef SOCKET_SYSTEM
								, BYTE * pbySocketOptions /*= NULL*/
#endif	// SOCKET_SYSTEM
								)
{
	ITEM* pNewItem = new ITEM;
	memset(pNewItem, 0, sizeof(ITEM));

	WORD wType = byType * MAX_ITEM_INDEX + bySubType;
	pNewItem->Key = GenerateItemKey();	//. 虐甫 积己茄促.
	pNewItem->Type = wType;
	pNewItem->Durability = byDurability;
	pNewItem->Option1 = byOption1;
	pNewItem->ExtOption = byOptionEx;
	if( (((byOption380 & 0x08) << 4) >> 7) > 0)
		pNewItem->option_380 = true;
	else
		pNewItem->option_380 = false;
	pNewItem->Jewel_Of_Harmony_Option = (byOptionHarmony & 0xf0) >> 4;	//可记 辆幅
	pNewItem->Jewel_Of_Harmony_OptionLevel = byOptionHarmony & 0x0f;	//可记 饭骇( 蔼捞 酒丛 )

#ifdef SOCKET_SYSTEM
	if (pbySocketOptions == NULL)
	{
		pNewItem->SocketCount = 0;
		assert(!"咯扁肺 坷绰 版快俊 家南 可记 包访 犬牢 鞘夸");
	}
	else
	{
		pNewItem->SocketCount = MAX_SOCKETS;

		for (int i = 0; i < MAX_SOCKETS; ++i)
		{
			pNewItem->bySocketOption[i] = pbySocketOptions[i];	// 辑滚俊辑 罐篮 郴侩 归诀
		}

#ifdef _VS2008PORTING
		for (int i = 0; i < MAX_SOCKETS; ++i)
#else // _VS2008PORTING
		for (i = 0; i < MAX_SOCKETS; ++i)
#endif // _VS2008PORTING
		{
			if (pbySocketOptions[i] == 0xFF)		// 家南捞 阜塞 (DB惑俊绰 0x00 栏肺 登绢乐澜)
			{
				pNewItem->SocketCount = i;
				break;
			}
			else if (pbySocketOptions[i] == 0xFE)	// 家南捞 厚绢乐澜 (DB惑俊绰 0xFF 栏肺 登绢乐澜)
			{
				pNewItem->SocketSeedID[i] = SOCKET_EMPTY;
			}
			else	// 0x00~0xF9 鳖瘤 家南 绊蜡锅龋肺 荤侩, MAX_SOCKET_OPTION(50)窜困肺 唱穿绢 绊蜡锅龋肺 钎矫 (DB惑俊绰 0x01~0xFA肺 登绢乐澜)
			{
				pNewItem->SocketSeedID[i] = pbySocketOptions[i] % SEASON4A::MAX_SOCKET_OPTION;
				pNewItem->SocketSphereLv[i] = int(pbySocketOptions[i] / SEASON4A::MAX_SOCKET_OPTION) + 1;
			}
		}
		
  		if (g_SocketItemMgr.IsSocketItem(pNewItem))	// 家南 酒捞袍捞搁
		{
			pNewItem->SocketSeedSetOption = byOptionHarmony;	// 炼拳狼焊籍可记蔼栏肺 矫靛技飘可记阑 钎泅窃
			pNewItem->Jewel_Of_Harmony_Option = 0;
			pNewItem->Jewel_Of_Harmony_OptionLevel = 0;
		}
		else
		{
			pNewItem->SocketSeedSetOption = SOCKET_EMPTY;
		}
	}
#endif	// SOCKET_SYSTEM

	pNewItem->byColorState = ITEM_COLOR_NORMAL;

	pNewItem->RefCount = 1;
	
// 扁埃力 酒捞袍
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
	if( ((byOption380&0x02) >> 1) > 0 )
	{
		pNewItem->bPeriodItem = true;		// 扁埃力 酒捞袍
	}
	else 
	{
		pNewItem->bPeriodItem = false;		// 老馆 酒捞袍
	}

	if( ((byOption380&0x04) >> 2) > 0 )
	{
		pNewItem->bExpiredPeriod = true;	// 扁埃父丰
	}
	else 
	{
		pNewItem->bExpiredPeriod = false;	// 扁埃郴 捞芭唱, 老馆酒捞袍
	}
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM

	SetItemAttr(pNewItem, byLevel, byOption1, byOptionEx);

	m_listItem.push_back(pNewItem);
	return pNewItem;
}

ITEM* SEASON3B::CNewUIItemMng::CreateItem(ITEM* pItem)
{
	pItem->RefCount++;
	return pItem;
}

ITEM* SEASON3B::CNewUIItemMng::DuplicateItem(ITEM* pItem)
{
	ITEM* pNewItem = new ITEM;
	memcpy(pNewItem, pItem, sizeof(ITEM));
	pNewItem->Key = GenerateItemKey();	//. 虐甫 积己茄促.
	pNewItem->RefCount = 1;

	// 葛电 酒捞袍阑 包府窍绰 listItem俊档 眠啊 钦聪促. 皋葛府 包府 瞒盔
#ifdef LDS_FIX_MEMORYLEAK_DUPLICATEDITEMS_INVENTORY_SAMEPOS
	m_listItem.push_back(pNewItem);
#endif // LDS_FIX_MEMORYLEAK_DUPLICATEDITEMS_INVENTORY_SAMEPOS
	
	return pNewItem;
}

void SEASON3B::CNewUIItemMng::DeleteItem(ITEM* pItem)
{
	if(pItem == NULL)
		return;
	
#ifdef LDS_FIX_MEMORYLEAK_DUPLICATEDITEMS_INVENTORY_SAMEPOS
	if(--pItem->RefCount <= 0)
#else // LDS_FIX_MEMORYLEAK_DUPLICATEDITEMS_INVENTORY_SAMEPOS
	if(--pItem->RefCount == 0)
#endif // LDS_FIX_MEMORYLEAK_DUPLICATEDITEMS_INVENTORY_SAMEPOS
	{
		type_list_item::iterator li = m_listItem.begin();
		for(; li != m_listItem.end(); li++)
		{
			if((*li) == pItem)
			{
				SAFE_DELETE(*li);
				m_listItem.erase(li);
				break;
			}
		}
	}
}

#ifdef YDG_FIX_MEMORY_LEAK_0905_2ND
void SEASON3B::CNewUIItemMng::DeleteDuplicatedItem(ITEM* pItem)
{
	if(pItem == NULL)
		return;

#ifdef LDS_FIX_MEMORYLEAK_DUPLICATEDITEMS_INVENTORY_SAMEPOS
	if(--pItem->RefCount <= 0)
#else // LDS_FIX_MEMORYLEAK_DUPLICATEDITEMS_INVENTORY_SAMEPOS
	if(--pItem->RefCount == 0)
#endif // LDS_FIX_MEMORYLEAK_DUPLICATEDITEMS_INVENTORY_SAMEPOS
	{
#ifdef LDS_FIX_MEMORYLEAK_0908_DUPLICATEITEM
		DeleteItem( pItem );
#endif // LDS_FIX_MEMORYLEAK_0908_DUPLICATEITEM
		
#ifndef LDS_FIX_MEMORYLEAK_DUPLICATEDITEMS_INVENTORY_SAMEPOS	// !!林狼 #ifndef
		SAFE_DELETE(pItem);
#endif // LDS_FIX_MEMORYLEAK_DUPLICATEDITEMS_INVENTORY_SAMEPOS
	}
}
#endif	// YDG_FIX_MEMORY_LEAK_0905_2ND

void SEASON3B::CNewUIItemMng::DeleteAllItems()
{
	type_list_item::iterator li = m_listItem.begin();
	for(; li != m_listItem.end(); li++)
	{
		SAFE_DELETE(*li);
	}
	m_listItem.clear();

	m_dwAlternate = 0;
	m_dwAvailableKeyStream = 0x80000000;
}

bool SEASON3B::CNewUIItemMng::IsEmpty()
{ 
	return m_listItem.empty(); 
}

void SEASON3B::CNewUIItemMng::Update()
{
	m_UpdateTimer.UpdateTime();
	if(m_UpdateTimer.IsTime())
	{
		type_list_item::iterator li = m_listItem.begin();
		for(; li != m_listItem.end(); )
		{
			if((*li)->RefCount <=0 )
			{
				delete (*li);
				li = m_listItem.erase(li);
			}
			else
				li++;
		}
	}
}

DWORD SEASON3B::CNewUIItemMng::GenerateItemKey()
{
	DWORD dwAvailableItemKey = FindAvailableKeyIndex(m_dwAvailableKeyStream);
	if(dwAvailableItemKey >= 0x8F000000)	//. 裹困 檬苞矫 犁八祸
	{
		m_dwAvailableKeyStream = 0;
		m_dwAlternate++;
		dwAvailableItemKey = FindAvailableKeyIndex(m_dwAvailableKeyStream);
	}
	return m_dwAvailableKeyStream = dwAvailableItemKey;
}

DWORD SEASON3B::CNewUIItemMng::FindAvailableKeyIndex(DWORD dwSeed)
{
	if(m_dwAlternate > 0)
	{
		type_list_item::iterator li = m_listItem.begin();
		for(; li != m_listItem.end(); li++)
		{
			ITEM* pItem = (*li);
			if(pItem->Key == dwSeed+1)	//. 乐促搁
				return FindAvailableKeyIndex(dwSeed+1);	//. 促矫 八祸
		}
	}
	return dwSeed+1; //. 绝栏搁 府畔
}

WORD SEASON3B::CNewUIItemMng::ExtractItemType(BYTE* pbyItemPacket)
{
	return pbyItemPacket[0] + (pbyItemPacket[3]&128)*2 + (pbyItemPacket[5]&240)*32;
}

void SEASON3B::CNewUIItemMng::SetItemAttr(ITEM* pItem, BYTE byLevel, BYTE byOption1, BYTE byOptionEx)
{
	//. 唱吝俊 咯扁肺 颗扁鲤!
	ItemConvert(pItem, byLevel, byOption1, byOptionEx);
}
