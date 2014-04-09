#ifndef __DROPDOWN_H
#define __DROPDOWN_H

/** 
 * @file
 * @author Flugente (bears-pit.com)
 */

#include "LaptopSave.h"

/*
 * helpful draw functions that used to draw the boundaries of the dropdown. The old way of adding sti objects to paint boundaries is... lets say, inefficient
 */
void DisplaySmallLine( UINT16 usStartX, UINT16 usStartY, UINT16 EndX, UINT16 EndY, UINT16 usColor );
void Display2Line2ShadowVertical( UINT16 usStartX, UINT16 usStartY, UINT16 EndX, UINT16 usColor1, UINT16 usColor2 );
void Display2Line2ShadowHorizontal( UINT16 usStartX, UINT16 usStartY, UINT16 EndY, UINT16 usColor1, UINT16 usColor2 );

// the maximum number of entries that can be shown at once. Increase requires recompilation!
#define DROPDOWN_REGIONS		8

/*
 * As we cannot directly add our callbacks (we need a static callback due to the way MSYS_DefineRegion(...) works, which utterly fails if we have multiple instance of a class),
 * we use a very odd looking workaround.
 * We do not directly use DropDownBase. Instead we use DropDownTemplate, which is a template taking an int N. This template is a singleton. This means that for each N, there can only be one
 * instance of the class. This is necessary as we internally need a unique static self-pointer.
 * If you try to have two instances on the same screen, only one will show, which will be confusing. So simply use a separate N fore each instance. See IMP Prejudice.cpp to get an idea of how that is done.
 *
 * Apart from that, you will only have to implement void SetRefresh(). This function is necessary to refresh the screen after the popup-section is closed, otherwise it will still be shown.
 * 
 * As this class refreshes itself once you have properly implemented SetRefresh(), you can even move the box aroun onscreen, it will properly refresh, so moving it with a mouse will be possible
 *
 * You can get an instance by calling DropDownTemplate<int X>.Create(UINT16 usX, UINT16 usY);, the constructor itself is private (singleton).
  
 Usage:
	std::vector<std::pair<INT16, STR16> > entryvecDropDown_Appearance;
	for(UINT8 i = 0; i < NUM_APPEARANCES; ++i)
	entryvecDropDown_Appearance.push_back( std::make_pair<INT16, STR16>(i, szAppearanceText[i]) );
		
	DropDownTemplate<DROPDOWNNR_APPEARANCE>.SetEntries(entryvecDropDown_Appearance);
	DropDownTemplate<DROPDOWNNR_APPEARANCE>.Create(usX, usY);
	...

 */
class DropDownBase
{
public:
	DropDownBase();

	/*
	 * Create a Dropdown with upper left coordinates
	 */
	void Create(UINT16 sX, UINT16 sY);

	/*
	 * Destroy Dropdown, for example once a webiste isn't displayed anymore
	 */
	void Destroy();

	/*
	 * Display DropDownBase. Use this when refreshing
	 */
	void Display();

	/*
	 * Sometimes a refresh inside the class isn't enough - best example is closing the dropdown area, after whcih te previous background cannot be restored by this class.
	 * In that case, the background and this class has to be redrawn.
	 * This function is called internally whenver such a refreh is necessary.
	 * It has to be defined in derived classes and should notify your dialogues to redraw at apporpiate times
	 *
	 * This function has to be implemented!
	 */
	virtual void SetRefresh() = 0;

	// internal callback types
	enum {
		DROPDOWN_OPEN = 0,
		DROPDOWN_CLOSE,
		DROPDOWN_REGION,
		DROPDOWN_MOVEMENT,
		DROPDOWN_ARROW,
		DROPDOWN_SCROLL_REGION,
		DROPDOWN_SCROLL_MOVEMENT,
	};
			
	/*
	 * Set the content of a dropdown. Each entry consists of an INT16 key, by which you can later identify which entry was selected, and a STR16 that will be displayed.
	 * There can be multiple instances of the same key or name.
	 */
	void SetEntries( std::vector<std::pair<INT16, STR16> >& arEntryVec )	{ mEntryVector = arEntryVec; }

	/*
	 * Set Color of boundary line
	 */
	void SetColorLine( UINT16 aCol )		{ mColorLine = aCol; }

	/*
	 * Set Color of boundary line shadow
	 */
	void SetColorLineShadow( UINT16 aCol )	{ mColorLineShadow = aCol; }

	/*
	 * Set background color of selected entry
	 */
	void SetColorMarked( UINT16 aCol )		{ mColorMarked = aCol; }

	/*
	 * Set Color of highlight of the scrollbar mover
	 */
	void SetColorHighLight( UINT16 aCol )	{ mColorHighLight = aCol; }

	/*
	 * Set help text decribing what can be selected
	 */
	void SetHelpText( STR16 aText )			{ swprintf( mHelpText, L"" ); wcscat( mHelpText, aText ); }

	/*
	 * Get key of selected entry
	 */
	INT16	GetSelectedEntryKey()		{ return mEntryVector[mSelectedEntry].first; }

	/*
	 * Get width of entire DropDownBase
	 */
	UINT16	GetTotalWidth()				{ return musWidth + musArrowWidth; }

	/*
	 * Get right x coordinate
	 */
	UINT16	GetLastX()					{ return musStartX + GetTotalWidth(); }

	/*
	 * Are we displayed?
	 */
	BOOLEAN	IsDisplayed()				{ return mfMouseRegionsCreated; }

	// call to open/close the drop down
	void OpenDropDownRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
	void CloseDropDownRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );

	void SelectDropDownRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
	void SelectDropDownMovementCallBack(MOUSE_REGION * pRegion, INT32 iReason );
	void SelectUpDownArrowOnScrollAreaRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );

	void SelectScrollAreaDropDownRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
	void SelectScrollAreaDropDownMovementCallBack(MOUSE_REGION * pRegion, INT32 iReason );

	/*
	 * This function is implemented again in DropDownTemplate
	 */
	virtual MOUSE_CALLBACK CallBackWrapper(void* pt2Object, UINT8 arg, void  (*pt2Function)(MOUSE_REGION * pRegion, INT32 iReason) )	{ return Dummyfunc; }

private:
	// declare but don't define
    DropDownBase(DropDownBase const&); 
    void operator=(DropDownBase const&);

	/*
	 * Initialise variables. Called after each creationm which allows moving a dropdown 
	 */
	void Init(UINT16 sX, UINT16 sY);

	/*
	 * Functions exclusively for the dropdown region, which is only shown after user interaction
	 */
	void Create_Drop();
	void Init_Drop();
	void Display_Drop();
	void Destroy_Drop();

	/*
	 * This dummy is needed internally and does nothing when called
	 */
	static void Dummyfunc(MOUSE_REGION * pRegion, INT32 iReason )	{}

	void DrawTopEntry();
	void DrawSelectedCity();
	void DrawGoldRectangle();

private:
	UINT16	musHeight;
	UINT16	musWidth;
	
	UINT16	musStartX;
	UINT16	musStartY;
	UINT16	musStartX_Drop;
	UINT16	musStartY_Drop;
	UINT16	musScrollAreaX;

	UINT16	musUpArrowX;
	UINT16	musUpArrowY;
	UINT16	musDownArrowX;
	UINT16	musDownArrowY;
	UINT16	musArrowWidth;
	UINT16	musAreaHeight;

	UINT16	musFontHeight;

	UINT16	mColorLine;			// color of boundary lines
	UINT16	mColorLineShadow;	// color of boundary line shadows
	UINT16	mColorMarked;		// color of marked entries
	UINT16	mColorHighLight;	// color of bar highlighting
	
	BOOLEAN mfMouseRegionsCreated;
	BOOLEAN mfMouseRegionsCreated_Drop;

	MOUSE_REGION	mSelectedOpenDropDownRegion;
	MOUSE_REGION	mSelectedCloseDropDownRegion;
	MOUSE_REGION	mBubbleHelpRegion;	// for a help text describing what a selection is good for, can be seen when hovering the mouse over the box when it is closed

	MOUSE_REGION	mDropDownRegion[DROPDOWN_REGIONS];
	MOUSE_REGION	mgSelectedUpDownArrowOnScrollAreaRegion[2];
	MOUSE_REGION	mSelectedScrollAreaDropDownRegion[DROPDOWN_REGIONS];

	UINT32	muiGoldArrowImages;

	std::vector<std::pair<INT16, STR16> > mEntryVector;

	CHAR16	mHelpText[200];

	UINT8	mSelectedEntry;				// keeps track of the currently selected city
	UINT8	mFirstShownEntry;			// top entry of the dropped region
	UINT8	mNumDisplayedEntries;		// number of entries displayed. Calculated internally, no need to change by user
};

template<int N>
class DropDownTemplate : public DropDownBase
{
public:
	static DropDownTemplate<N>& getInstance()
    {
        static DropDownTemplate<N>    instance; // Guaranteed to be destroyed.
                                // Instantiated on first use.
        return instance;
    }
	
	// this function has to be implemented!
	virtual void SetRefresh();
	
	static void OpenDropDown_DropDown(MOUSE_REGION * pRegion, INT32 iReason )					{ return static_cast<DropDownBase*>(mpSelf)->OpenDropDownRegionCallBack(pRegion, iReason); }
	static void CloseDropDown_DropDown(MOUSE_REGION * pRegion, INT32 iReason )					{ return static_cast<DropDownBase*>(mpSelf)->CloseDropDownRegionCallBack(pRegion, iReason); }
	static void SelectRegionDropDown_DropDown(MOUSE_REGION * pRegion, INT32 iReason )			{ return static_cast<DropDownBase*>(mpSelf)->SelectDropDownRegionCallBack(pRegion, iReason); }
	static void SelectMovementDropDown_DropDown(MOUSE_REGION * pRegion, INT32 iReason )			{ return static_cast<DropDownBase*>(mpSelf)->SelectDropDownMovementCallBack(pRegion, iReason); }
	static void SelectArrowDropDown_DropDown(MOUSE_REGION * pRegion, INT32 iReason )			{ return static_cast<DropDownBase*>(mpSelf)->SelectUpDownArrowOnScrollAreaRegionCallBack(pRegion, iReason); }
	static void SelectScrollRegionDropDown_DropDown(MOUSE_REGION * pRegion, INT32 iReason )		{ return static_cast<DropDownBase*>(mpSelf)->SelectScrollAreaDropDownRegionCallBack(pRegion, iReason); }
	static void SelectScrollMovementDropDown_DropDown(MOUSE_REGION * pRegion, INT32 iReason )	{ return static_cast<DropDownBase*>(mpSelf)->SelectScrollAreaDropDownMovementCallBack(pRegion, iReason); }

	MOUSE_CALLBACK
	CallBackWrapper(void* pt2Object, UINT8 arg, void  (*pt2Function)(MOUSE_REGION * pRegion, INT32 iReason) )
	{
		mpSelf = pt2Object;

		switch ( arg )
		{
		case DROPDOWN_OPEN:
			return &OpenDropDown_DropDown;
			break;
		case DROPDOWN_CLOSE:
			return &CloseDropDown_DropDown;
			break;
		case DROPDOWN_REGION:
			return &SelectRegionDropDown_DropDown;
			break;
		case DROPDOWN_MOVEMENT:
			return &SelectMovementDropDown_DropDown;
			break;
		case DROPDOWN_ARROW:
			return &SelectArrowDropDown_DropDown;
			break;
		case DROPDOWN_SCROLL_REGION:
			return &SelectScrollRegionDropDown_DropDown;
			break;
		case DROPDOWN_SCROLL_MOVEMENT:
			return &SelectScrollMovementDropDown_DropDown;
			break;
		}

		return *pt2Function;
	}

private:
	static void* mpSelf;
	
private:
	DropDownTemplate<N>() {};                   // private constructor, so we cannot create more instances
    
	// declare but don't define
    DropDownTemplate(DropDownTemplate const&); 
    void operator=(DropDownTemplate const&);
};

template <int N>
void DropDownTemplate<N>::SetRefresh();

template <int N>
void* DropDownTemplate<N>::mpSelf = NULL;

#endif
