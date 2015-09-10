
// DumpList.h	(was ewmList.h)
#ifndef	_DumpList_H
#define	_DumpList_H

#define  LE    LIST_ENTRY
#define  PLE   PLIST_ENTRY

#define	ListCount(ListHead)\
{\
	int	_icnt = 0;\
	PLIST_ENTRY _EX_Flink;\
	PLIST_ENTRY _EX_ListHead;\
	_EX_ListHead = (ListHead);\
	_EX_Flink = _EX_ListHead->Flink;\
	while( _EX_Flink != _EX_ListHead )\
	{\
		_icnt++;\
		_EX_Flink = _EX_Flink->Flink;\
	}\
}

//    ((ListHead)->Flink == (ListHead))
#define ListCount2(lh,pi) \
{\
    int             _icnt = 0;\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (lh);\
    _EX_Flink = _EX_ListHead->Flink;\
    while( _EX_Flink != _EX_ListHead )\
        {\
                _EX_Flink = _EX_Flink->Flink;\
                _icnt++;\
                if( ( _icnt == 0 ) ||\
                        ( !_EX_Flink ) )\
                {\
                                break;\
                }\
        }\
        *pi = _icnt;\
}


//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
// empty if FORWARD link points to SELF!
#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

//
//  PLIST_ENTRY
//  RemoveHeadList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY Entry
//      );
//

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _RT_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
	_RT_Flink = _EX_Flink->Flink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    _RT_Flink;}

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
// simple. the current BACKWARD link of the HEAD (EX_Blink)
// becomes this entries BACKWARDS link (entry)->Blink.
// and this entried FORWARD link is the HEAD
// this etry is store in the FORWARD link of the last HEAD Blink,
// and stored as the BACKWARDS link of the HEAD.

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

// Insert Entry as Blink of List
// =============================
// Extract the Blink from the List
// Extract the FLink from the Blink
// Insert  the Entry as the Flink
// Put the previous Flink and Entry Flink
#define InsertBefore(List,Entry) {\
    PLE _EX_Blink;\
    PLE _EX_Flink;\
    PLE _EX_List;\
    _EX_List = (List);\
    _EX_Blink = _EX_List->Blink;\
    _EX_Flink = _EX_Blink->Flink;\
    _EX_Blink->Flink = (Entry);\
    _EX_List->Blink = (Entry);\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_Blink;\
    }

// Insert Entry as Flink of List
// =============================
// extract Flink from list
// extract Blink from Flink
// make Entry the Flink of list
// and  Entry the Blink of Flink
// make Flink of list Flink of Entry
// and  Blink of Flink as Blink of Entry
#define InsertAfter(List,Entry) {\
    PLE _EX_Blink;\
    PLE _EX_Flink;\
    PLE _EX_List;\
    _EX_List = (List);\
    _EX_Flink = _EX_List->Flink;\
    _EX_Blink = _EX_Flink->Blink;\
    _EX_List->Flink = (Entry);\
    _EX_Flink->Blink = (Entry);\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_Blink;\
    }

//#define	Traverse_List2(ListHead,ListEntry)
//	for(PLIST_ENTRY _s_ListHead = (ListHead), _s_Flink = _s_ListHead->Flink, ListEntry = _s_Flink;_s_Flink != _s_ListHead; _s_Flink = _s_Flink->Flink, ListEntry = _s_Flink )
//static PLIST_ENTRY _SX_Flink;
//static PLIST_ENTRY _SX_ListHead;
//#define	Traverse_List(ListHead)	for(_SX_ListHead = (ListHead),_SX_Flink = _SX_ListHead->Flink;_SX_Flink != _SX_ListHead; _SX_Flink = _SX_Flink->Flink )
#define  Traverse_List(pListHead,pListNext)\
   for( pListNext = pListHead->Flink; pListNext != pListHead; pListNext = pListNext->Flink )

#define  InitLList(a)\
   {\
      (a)->Flink = a;\
      (a)->Blink = a;\
   }

#define  FreeLList(pListHead,pListNext)\
         while( !IsListEmpty( pListHead ) )\
         {\
            pListNext = RemoveHeadList(pListHead);\
            dMFREE(pListNext);\
         }

#define  KillLList(a)\
   {\
      PLE   _pNxt;\
      FreeLList(a,_pNxt);\
   }


#endif	// _DumpList_H
// eof - DumpList.h
