/* INCLUDES */

// Class header
#include "TechTreeWindow.h"

// Other includes
#include <mmsystem.h>
#include "Game.h"
#include "VanillaTechTreeRenderer.h"
#include "GameDataHandler.h"
#include "call_conventions.h"

/* STATIC WRAPPER FUNCTIONS */

static TechTreeWindow *THISCALL(TechTreeWindow_Constructor, TechTreeWindow *self, Window *underlyingWindow, Player* player, int selectedCivId) {
	return self->Constructor(underlyingWindow, player, selectedCivId);
}
static void THISCALL(TechTreeWindow_ScalarDeletingDestructor, TechTreeWindow *self, char mode) {
	return self->ScalarDeletingDestructor(mode);
}
static void THISCALL(TechTreeWindow_Draw, TechTreeWindow *self) {
	return self->Draw();
}
static int THISCALL(TechTreeWindow_HandleWindowsMessage, TechTreeWindow *self, HWND hWnd, signed int msg, WPARAM wParam, LPARAM lParam) {
	return self->HandleWindowsMessage(hWnd, msg, wParam, lParam);
}
static int THISCALL(TechTreeWindow_DoUpdate, TechTreeWindow *self) {
	return self->DoUpdate();
}
static int THISCALL(TechTreeWindow_HandleUserMessage, TechTreeWindow *self, WPARAM wParam, LPARAM lParam) {
	return self->HandleUserMessage(wParam, lParam);
}
static int THISCALL(TechTreeWindow_HandleMouseButtonDown, TechTreeWindow *self, int buttonId, int cursorPosX, int cursorPosY, int controlKeyPressed, int shiftKeyPressed) {
	return self->HandleMouseButtonDown(buttonId, cursorPosX, cursorPosY, controlKeyPressed, shiftKeyPressed);
}
static int THISCALL(TechTreeWindow_HandleMouseMove, TechTreeWindow *self, int cursorPosX, int cursorPosY, int controlKeyPressed, int shiftKeyPressed) {
	return self->HandleMouseMove(cursorPosX, cursorPosY, controlKeyPressed, shiftKeyPressed);
}
static int THISCALL(TechTreeWindow_HandleKeyDown2, TechTreeWindow *self, int keyDown, int lParam, int menuKeyDown, int controlKeyDown, int shiftKeyDown) {
	return self->HandleKeyDown2(keyDown, lParam, menuKeyDown, controlKeyDown, shiftKeyDown);
}
static int THISCALL(TechTreeWindow_HandleChildControlEvent, TechTreeWindow *self, Control *triggeringControl, int code, int data1, int data2) {
	return self->HandleChildControlEvent(triggeringControl, code, data1, data2);
}

/* VARIABLES */

// The "required" string for the popup label box.
char popupBoxRequiredString[128] = { 0 };

/* FUNCTIONS */

void TechTreeWindow::__Install()
{
	// First call base function
	base::__Install();

	// Replace class size statements in operator new() calls and install constructor
	int size = sizeof(TechTreeWindow);
	CopyBytesToAddr(0x004FDC45, reinterpret_cast<void *>(&size), 4);
	CopyBytesToAddr(0x00529318, reinterpret_cast<void *>(&size), 4);
	 CreateCodecave(0x004FDC6A, reinterpret_cast<void(*)()>(TechTreeWindow_Constructor));
	 CreateCodecave(0x00529347, reinterpret_cast<void(*)()>(TechTreeWindow_Constructor));

	// Install virtual function table entries
	CreateVTableEntry(0x00645118, reinterpret_cast<void(*)()>(TechTreeWindow_ScalarDeletingDestructor));
	CreateVTableEntry(0x00645148, reinterpret_cast<void(*)()>(TechTreeWindow_Draw));
	CreateVTableEntry(0x00645160, reinterpret_cast<void(*)()>(TechTreeWindow_HandleWindowsMessage));
	CreateVTableEntry(0x00645164, reinterpret_cast<void(*)()>(TechTreeWindow_DoUpdate));
	CreateVTableEntry(0x0064517C, reinterpret_cast<void(*)()>(TechTreeWindow_HandleUserMessage));
	CreateVTableEntry(0x00645188, reinterpret_cast<void(*)()>(TechTreeWindow_HandleMouseButtonDown));
	CreateVTableEntry(0x0064518C, reinterpret_cast<void(*)()>(TechTreeWindow_HandleMouseMove));
	CreateVTableEntry(0x006451D4, reinterpret_cast<void(*)()>(TechTreeWindow_HandleKeyDown2));
	CreateVTableEntry(0x006451DC, reinterpret_cast<void(*)()>(TechTreeWindow_HandleChildControlEvent));
}

TechTreeWindow *TechTreeWindow::Constructor(Window *underlyingWindow, Player* player, int selectedCivId)
{
	// Needed for window management and proper cleanup in other places
	base::Constructor("One Button Tech Tree Screen");

	// Save underlying window
	_underlyingWindow = underlyingWindow;

	// Save game data
	_player = player;

	// Set virtual function table address
	_VTable = reinterpret_cast<ControlVTable *>(0x00645118);

	// Cast virtual function table to be able to access all functions
	PanelVTable *vTable = static_cast<PanelVTable *>(_VTable);

	// Get design data
	_designData = _staticNewTechTreeDataObject->GetDesignData();

	// Load style file
	DirectDrawArea *backBuffer = underlyingWindow->_VTable->GetBackBufferData(underlyingWindow);
	if(!PrepareParentWindowAndLoadStyleData(backBuffer, "scr6", 50007, 1))
		_initializationError = 1;

	// Start with top left corner
	_horizontalScrollOffset = 0;
	_verticalScrollOffset = 0;
	_lastScrollActionTime = 0;

	// Create vanilla style tech tree renderer
	// TODO: May be changed to allow various renderers using completely different layout approaches
	_renderer = new VanillaTechTreeRenderer((*_staticGameObjectPointer)->GetGameDataHandler(), Size(_width1, _height1), player);

	// Get the renderer's age count
	_ageCount = _renderer->GetAgeCount();

	// Create close button and its buffer
	static_cast<PanelVTable *>(_VTable)->CreateButtonWithOneFontWithTextFromDll(this, this, &_closeButton, 20101, 0, _width1 - _designData->_closeButtonRelativeRectangle.X, _height1 - _designData->_closeButtonRelativeRectangle.Y, _designData->_closeButtonRelativeRectangle.Width, _designData->_closeButtonRelativeRectangle.Height, 9, 0, 0);
	_closeButton->_VTable->InvalidateAndRedrawControl1(_closeButton, 1);
	_closeButton->AssignIdToControlAndMoveInParentChildrenList(1, 0);
	_closeButton->SetHotKey(VK_ESCAPE, 0, 0, 0);
	_closeButtonDrawBuffer = new DirectDrawArea("TechTree Close Button Buffer", 1);
	_closeButtonDrawBuffer->InsertIntoBufferListAndCreateSurfaceAndDoUnknownStuff(_backBufferData->GetDirectDrawHandler(), _designData->_closeButtonRelativeRectangle.Width, _designData->_closeButtonRelativeRectangle.Height, 0, 0);
	_closeButtonDrawBuffer->SetClipRect2(0, 0, _designData->_closeButtonRelativeRectangle.Width, _designData->_closeButtonRelativeRectangle.Height);
	UpdateSelectedSubControl(_closeButton);

	// Create scroll buttons and their buffers
	static_cast<PanelVTable *>(_VTable)->CreateButton(this, this, &_scrollLeftButton, "<-", 0, _width1 - _designData->_scrollLeftButtonRelativeRectangle.X, _height1 - _designData->_scrollLeftButtonRelativeRectangle.Y, _designData->_scrollLeftButtonRelativeRectangle.Width, _designData->_scrollLeftButtonRelativeRectangle.Height, 9, 0, 0);
	_scrollLeftButton->_VTable->InvalidateAndRedrawControl1(_scrollLeftButton, 1);
	_scrollLeftButton->AssignIdToControlAndMoveInParentChildrenList(1, 0);
	_scrollLeftButtonDrawBuffer = new DirectDrawArea("TechTree ScrollLeft Button Buffer", 1);
	_scrollLeftButtonDrawBuffer->InsertIntoBufferListAndCreateSurfaceAndDoUnknownStuff(_backBufferData->GetDirectDrawHandler(), _designData->_scrollLeftButtonRelativeRectangle.Width, _designData->_scrollLeftButtonRelativeRectangle.Height, 0, 0);
	_scrollLeftButtonDrawBuffer->SetClipRect2(0, 0, _designData->_scrollLeftButtonRelativeRectangle.Width, _designData->_scrollLeftButtonRelativeRectangle.Height);
	static_cast<PanelVTable *>(_VTable)->CreateButton(this, this, &_scrollRightButton, "->", 0, _width1 - _designData->_scrollRightButtonRelativeRectangle.X, _height1 - _designData->_scrollRightButtonRelativeRectangle.Y, _designData->_scrollRightButtonRelativeRectangle.Width, _designData->_scrollRightButtonRelativeRectangle.Height, 9, 0, 0);
	_scrollRightButton->_VTable->InvalidateAndRedrawControl1(_scrollRightButton, 1);
	_scrollRightButton->AssignIdToControlAndMoveInParentChildrenList(1, 0);
	_scrollRightButtonDrawBuffer = new DirectDrawArea("TechTree ScrollRight Button Buffer", 1);
	_scrollRightButtonDrawBuffer->InsertIntoBufferListAndCreateSurfaceAndDoUnknownStuff(_backBufferData->GetDirectDrawHandler(), _designData->_scrollRightButtonRelativeRectangle.Width, _designData->_scrollRightButtonRelativeRectangle.Height, 0, 0);
	_scrollRightButtonDrawBuffer->SetClipRect2(0, 0, _designData->_scrollRightButtonRelativeRectangle.Width, _designData->_scrollRightButtonRelativeRectangle.Height);

	// Load arrow SLP for scroll buttons and assign frames to them
	_arrowSlp = new Shape(_designData->_scrollSlpFileName, _designData->_scrollSlpId);
	_scrollLeftButton->SetBackgroundSlpAtIndex(0, _arrowSlp, 1);
	_scrollLeftButton->SetDisplayMode(9);
	_scrollLeftButton->AssignLabelString(0, "");
	_scrollRightButton->SetBackgroundSlpAtIndex(0, _arrowSlp, 0);
	_scrollRightButton->SetDisplayMode(9);
	_scrollRightButton->AssignLabelString(0, "");

	// Create civ bonus label
	/*static_cast<PanelVTable *>(_VTable)->CreateLabelWithOneFontWithTextFromDll(this, this, &_civBonusLabel, 0, 2, 20, 100, 20, 10, 0, 0, 1);
	_civBonusLabel->_VTable->InvalidateAndRedrawControl1(_civBonusLabel, 0);
	_civBonusLabel->AssignIdToControlAndMoveInParentChildrenList(1, 0);
	_civBonusLabel->SetStyleText2Colors(0, 0);
	_civBonusLabel->SetStyle(0);*/
	FontData *civBonusLabelBaseFont = (*_staticGameObjectPointer)->GetFontWithIndex(10);
	HFONT civBonusLabelFonts[4];
	civBonusLabelFonts[0] = civBonusLabelBaseFont->GetFontHandle();
	civBonusLabelFonts[1] = (*_staticGameObjectPointer)->GetFontWithIndex(17)->GetFontHandle();
	civBonusLabelFonts[2] = (*_staticGameObjectPointer)->GetFontWithIndex(18)->GetFontHandle();
	civBonusLabelFonts[3] = (*_staticGameObjectPointer)->GetFontWithIndex(19)->GetFontHandle();
	_civBonusLabel = new LabelControl();
	_civBonusLabel->Setup(_backBufferData, this, 2, 20, 100, 20, civBonusLabelFonts, civBonusLabelBaseFont->GetAverageCharWidth(), civBonusLabelBaseFont->GetCharHeightWithRowSpace(), nullptr, 0, 0, 0, 0, 0, nullptr);
	_civBonusLabel->SetTextAlignment(3, 1);
	_civBonusLabel->SetWordWrap(1);
	_civBonusLabel->SetStyle(0);
	_civBonusLabel->SetStyleText2Colors(0x000000, 0x00E7E7);

	// Create civ selection combo box
	static_cast<PanelVTable *>(_VTable)->CreateComboBox(this, this, &_civSelectionComboBox, 230, 0, 2, 20, 200, 25, 10);
	_civSelectionComboBox->_VTable->InvalidateAndRedrawControl1(_civSelectionComboBox, 1);
	_civSelectionComboBox->SetDropDownListSortingMode(1);
	_civSelectionComboBox->AssignIdToControlAndMoveInParentChildrenList(1, 1);
	_civSelectionComboBox->SetStyleText2Colors(0, 0);
	_civSelectionComboBox->GetDropDownScrollBar()->SetFillBackground(-1);
	_civSelectionComboBox->GetValueLabel()->SetBackgroundSlp("tech_tile.slp", 50343);
	_civSelectionComboBox->GetDropDownList()->SetBackgroundSlp("tech_tile.slp", 50343);

	// Create "game civs" label
	static_cast<PanelVTable *>(_VTable)->CreateLabelWithOneFontWithTextFromDll(this, this, &_gameCivsLabel, 0, 2, 20, 200, 25, 10, 0, 0, 1);
	_gameCivsLabel->_VTable->InvalidateAndRedrawControl1(_gameCivsLabel, 0);
	_gameCivsLabel->AssignIdToControlAndMoveInParentChildrenList(1, 0);
	_gameCivsLabel->SetStyleText2Colors(0, 0);
	_gameCivsLabel->SetStyle(0);
	static_cast<LabelControlVTable *>(_gameCivsLabel->_VTable)->SetText(_gameCivsLabel, 20125);

	// Create "Not researched" label
	static_cast<PanelVTable *>(_VTable)->CreateLabelWithOneFontWithTextFromDll(this, this, &_legendNotResearchedLabel, 0, 2, 2, 160, 25, 13, 0, 0, 0);
	_legendNotResearchedLabel->_VTable->InvalidateAndRedrawControl1(_legendNotResearchedLabel, 0);
	_legendNotResearchedLabel->AssignIdToControlAndMoveInParentChildrenList(1, 0);
	_legendNotResearchedLabel->SetStyleText2Colors(0, 0);
	_legendNotResearchedLabel->SetStyle(0);
	static_cast<LabelControlVTable *>(_legendNotResearchedLabel->_VTable)->SetText(_legendNotResearchedLabel, 20124);

	// Create "Researched" label
	static_cast<PanelVTable *>(_VTable)->CreateLabelWithOneFontWithTextFromDll(this, this, &_legendResearchedLabel, 0, 2, 2, 160, 25, 13, 0, 0, 0);
	_legendResearchedLabel->_VTable->InvalidateAndRedrawControl1(_legendResearchedLabel, 0);
	_legendResearchedLabel->AssignIdToControlAndMoveInParentChildrenList(1, 0);
	_legendResearchedLabel->SetStyleText2Colors(0, 0);
	_legendResearchedLabel->SetStyle(0);
	static_cast<LabelControlVTable *>(_legendResearchedLabel->_VTable)->SetText(_legendResearchedLabel, 20128);

	// Create node type labels
	_legendNodeTypeLabels = new std::vector<LabelControl *>();
	int nodeTypeLabelCount = _renderer->GetLegendNodeTypeCount();
	for(int i = 0; i < nodeTypeLabelCount; ++i)
	{
		// Create label
		LabelControl *legendNodeTypeLabel;
		static_cast<PanelVTable *>(_VTable)->CreateLabelWithOneFontWithTextFromDll(this, this, &legendNodeTypeLabel, 0, 2, 2, 160, 25, 13, 0, 0, 0);
		legendNodeTypeLabel->_VTable->InvalidateAndRedrawControl1(legendNodeTypeLabel, 0);
		legendNodeTypeLabel->AssignIdToControlAndMoveInParentChildrenList(1, 0);
		legendNodeTypeLabel->SetStyleText2Colors(0, 0);
		legendNodeTypeLabel->SetStyle(0);
		static_cast<LabelControlVTable *>(legendNodeTypeLabel->_VTable)->SetText(legendNodeTypeLabel, _renderer->GetLegendNodeTypeLabelDllId(i));
		_legendNodeTypeLabels->push_back(legendNodeTypeLabel);
	}

	// Create "Disabled" label
	static_cast<PanelVTable *>(_VTable)->CreateLabelWithOneFontWithTextFromDll(this, this, &_legendDisabledLabel, 0, 2, 2, 160, 25, 13, 0, 0, 0);
	_legendDisabledLabel->_VTable->InvalidateAndRedrawControl1(_legendDisabledLabel, 0);
	_legendDisabledLabel->AssignIdToControlAndMoveInParentChildrenList(1, 0);
	_legendDisabledLabel->SetStyleText2Colors(0, 0);
	_legendDisabledLabel->SetStyle(0);
	static_cast<LabelControlVTable *>(_legendDisabledLabel->_VTable)->SetText(_legendDisabledLabel, 20119);

	// Fill civ selection combo box
	int civCount = (*_staticGameObjectPointer)->GetGameDataHandler()->_civCount;
	char civNameBuffer[128];
	char civNameSuffixBuffer[128];
	char civNameMergedBuffer[256];
	for(int c = 1; c < civCount; ++c)
	{
		// Clear strings
		civNameBuffer[0] = '\0';
		civNameSuffixBuffer[0] = '\0';
		civNameMergedBuffer[0] = '\0';

		// Load civ name
		(*_staticGameObjectPointer)->GetIndexedDllString(105, c, 0, civNameBuffer, sizeof(civNameBuffer));

		// Load civ suffix " (Player)" or " (Ally)", if neccessary
		if(_player != nullptr)
			if(_player->GetCivId() == c) // Check player
				(*_staticGameObjectPointer)->GetStringFromLanguageDllsWithBuffer(20126, civNameSuffixBuffer, sizeof(civNameSuffixBuffer));
			else
			{
				// Check allies
				GameDataHandler *gdh = (*_staticGameObjectPointer)->GetGameDataHandler();
				for(int p = 1; p < 9; ++p)
					if(_player->GetDiplomaticStance(p) == 4 && gdh->_players[p]->GetCivId() == c)
					{
						// Get suffix and break
						(*_staticGameObjectPointer)->GetStringFromLanguageDllsWithBuffer(20127, civNameSuffixBuffer, sizeof(civNameSuffixBuffer));
						break;
					}
			}

		// Merge civ name and suffix
		sprintf(civNameMergedBuffer, "%s%s", civNameBuffer, civNameSuffixBuffer);

		// Add civ to list
		_civSelectionComboBox->AddItem(civNameMergedBuffer, c);
	}
	_civSelectionComboBox->SetSelectedIndex(0);

	// Create age labels
	_ageLabels = new LabelControl*[_ageCount][2][2];
	for(int i = 0; i < _ageCount; ++i)
	{
		// Create 4 labels for this age
		for(int j = 0; j < 4; ++j)
		{
			// Create and initialize label
			LabelControl **currAgeLabelPointer = &_ageLabels[i][j >> 1][j & 1];
			static_cast<PanelVTable *>(_VTable)->CreateLabelWithOneFontWithTextFromDll(this, this, currAgeLabelPointer, 0, 2, 2, 150, 25, 11, 1, 0, 1);
			(*currAgeLabelPointer)->_VTable->InvalidateAndRedrawControl1(*currAgeLabelPointer, 0);
			(*currAgeLabelPointer)->AssignIdToControlAndMoveInParentChildrenList(1, 0);
			(*currAgeLabelPointer)->SetStyleText2Colors(0, 0);
			(*currAgeLabelPointer)->SetStyle(0);
		}

		// Set label texts
		static_cast<LabelControlVTable *>(_ageLabels[i][0][0]->_VTable)->SetText(_ageLabels[i][0][0], _designData->_firstLineBaseDllId + i);
		static_cast<LabelControlVTable *>(_ageLabels[i][0][1]->_VTable)->SetText(_ageLabels[i][0][1], _designData->_secondLineDllId + (_designData->_incrementSecondLineDllId ? i : 0));
		static_cast<LabelControlVTable *>(_ageLabels[i][1][0]->_VTable)->SetText(_ageLabels[i][1][0], _designData->_firstLineBaseDllId + i);
		static_cast<LabelControlVTable *>(_ageLabels[i][1][1]->_VTable)->SetText(_ageLabels[i][1][1], _designData->_secondLineDllId + (_designData->_incrementSecondLineDllId ? i : 0));
	}

	// Create popup label
	_popupLabelBaseFont = (*_staticGameObjectPointer)->GetFontWithIndex(10);
	HFONT popupLabelFonts[4];
	popupLabelFonts[0] = _popupLabelBaseFont->GetFontHandle();
	popupLabelFonts[1] = (*_staticGameObjectPointer)->GetFontWithIndex(17)->GetFontHandle();
	popupLabelFonts[2] = (*_staticGameObjectPointer)->GetFontWithIndex(18)->GetFontHandle();
	popupLabelFonts[3] = (*_staticGameObjectPointer)->GetFontWithIndex(19)->GetFontHandle();
	_popupLabel = new LabelControl();
	_popupLabel->Setup(_backBufferData, this, 0, 0, _designData->_popupLabelWidth, 50, popupLabelFonts, _popupLabelBaseFont->GetAverageCharWidth(), _popupLabelBaseFont->GetCharHeightWithRowSpace(), nullptr, 0, 0, 0, 0, 0, nullptr);
	_popupLabel->SetTextAlignment(3, 1);
	_popupLabel->SetWordWrap(1);
	_popupLabel->SetStyle(0);
	_popupLabel->SetStyleText2Colors(0x000000, 0x00E7E7);

	// Set current civ
	if(player != 0)
		selectedCivId = player->GetCivId();
	if(selectedCivId < 1)
		selectedCivId = 1;
	_civSelectionComboBox->SetSelectedItemId(selectedCivId);
	SetCurrentCiv(selectedCivId);

	// No element is selected
	_selectedElement = nullptr;
	_drawPopupLabelBox = false;
	_selectedElementChangedTime = 0;

	// Issue redraw
	_VTable->InvalidateAndRedrawControl2(this, 2);

	// Return reference to self
	return this;
}

void TechTreeWindow::Destructor()
{
	// Free age labels
	for(int i = 0; i < _ageCount; ++i)
	{
		// Delete all labels for current age
		delete _ageLabels[i][0][0];
		delete _ageLabels[i][0][1];
		delete _ageLabels[i][1][0];
		delete _ageLabels[i][1][1];
	}
	delete[] _ageLabels;

	// Destroy renderer
	delete _renderer;
	_renderer = nullptr;

	// Destroy close button
	delete _closeButtonDrawBuffer;
	_closeButtonDrawBuffer = nullptr;
	delete _closeButton;
	_closeButton = nullptr;

	// Delete scroll buttons
	delete _scrollLeftButtonDrawBuffer;
	_scrollLeftButtonDrawBuffer = nullptr;
	delete _scrollLeftButton;
	_scrollLeftButton = nullptr;
	delete _scrollRightButtonDrawBuffer;
	_scrollRightButtonDrawBuffer = nullptr;
	delete _scrollRightButton;
	_scrollRightButton = nullptr;
	delete _arrowSlp;

	// Delete node type legend labels
	int nodeTypeLabelCount = _legendNodeTypeLabels->size();
	for(int i = 0; i < nodeTypeLabelCount; ++i)
		delete _legendNodeTypeLabels->at(i);
	delete _legendNodeTypeLabels;

	// Destroy various controls
	delete _civBonusLabel;
	_civBonusLabel = nullptr;
	delete _civSelectionComboBox;
	_civSelectionComboBox = nullptr;
	delete _gameCivsLabel;
	_gameCivsLabel = nullptr;
	delete _legendNotResearchedLabel;
	_legendNotResearchedLabel = nullptr;
	delete _legendResearchedLabel;
	_legendResearchedLabel = nullptr;
	delete _legendDisabledLabel;
	_legendDisabledLabel = nullptr;
	delete _popupLabel;
	_popupLabel = nullptr;

	// Remove underlying window reference
	_underlyingWindow = nullptr;

	// Finally call base class destructor
	base::Destructor();
}

void TechTreeWindow::Draw()
{
	// The BeginDrawRun/EndDrawRun methods are not neccessary here, as we draw onto the whole window

	// Draw tech tree (including background)
	_renderer->Draw(_backBufferData, _horizontalScrollOffset, _verticalScrollOffset);

	// Draw Controls
	{
		// Close button
		RECT closeButtonRect = _closeButton->GetClientRectangle();
		_closeButtonDrawBuffer->Copy(_backBufferData, 0, 0, &closeButtonRect, 0);
		_closeButton->DrawSingle();

		// Scroll buttons
		RECT scrollLeftButtonRect = _scrollLeftButton->GetClientRectangle();
		_scrollLeftButtonDrawBuffer->Copy(_backBufferData, 0, 0, &scrollLeftButtonRect, 0);
		_scrollLeftButton->DrawSingle();
		RECT scrollRightButtonRect = _scrollRightButton->GetClientRectangle();
		_scrollRightButtonDrawBuffer->Copy(_backBufferData, 0, 0, &scrollRightButtonRect, 0);
		_scrollRightButton->DrawSingle();

		// Labels (iterate through child control list and draw each label)
		for(ControlListElement *currChildControlElement = _childControlListBegin; currChildControlElement; currChildControlElement = currChildControlElement->NextElement)
		{
			// Check type
			if(currChildControlElement->AssociatedControl->GetTypeIdentifier() != Control::ControlType::Label)
				continue;

			// Popup help text label is drawn later
			if(currChildControlElement->AssociatedControl == _popupLabel)
				continue;

			// Is the label on the outside of the rendering area?
			const Rect& currChildControlRect = currChildControlElement->AssociatedControl->GetPositionAndSizeData();
			if(currChildControlRect.X + currChildControlRect.Width < _positionX2 || currChildControlRect.X > _positionX2 + _width1
				|| currChildControlRect.Y + currChildControlRect.Height < _positionY2 || currChildControlRect.Y > _positionY2 + _height1)
				continue;

			// Label is visible, draw it
			static_cast<LabelControl *>(currChildControlElement->AssociatedControl)->ForceDraw();
		}

		// Civ selection combo box
		_civSelectionComboBox->Draw();
	}

	// Draw popup label
	if(_drawPopupLabelBox)
	{
		// Draw box
		_renderer->DrawPopupLabelBox(_backBufferData, _popupLabelBoxPosition.X, _popupLabelBoxPosition.Y);

		// Draw label
		_popupLabel->ForceDraw();
	}
}

int TechTreeWindow::HandleWindowsMessage(HWND hWnd, signed int msg, WPARAM wParam, LPARAM lParam)
{
	// TODO
	return Control::HandleWindowsMessage(hWnd, msg, wParam, lParam);
}

int TechTreeWindow::DoUpdate()
{
	// Set selected element if time has elapsed
	if(_selectedElement != nullptr && timeGetTime() - _selectedElementChangedTime > _designData->_popupLabelDelay)
	{
		// Update selected element and redraw
		ApplySelectedElementAndRedraw();
	}

	// Time again for scrolling?
	if(timeGetTime() - _lastScrollActionTime > _designData->_mouseScrollDelay)
	{
		// Calculate scroll offsets depending on current mouse cursor position
		int scrollOffsetX = 0;
		if(_mouseCursorPosition.X <= _designData->_mouseScrollArea)
			scrollOffsetX = -_designData->_mouseScrollOffset;
		else if(_mouseCursorPosition.X > _width1 - _designData->_mouseScrollArea)
			scrollOffsetX = _designData->_mouseScrollOffset;
		int scrollOffsetY = 0;
		if(_mouseCursorPosition.Y <= _designData->_mouseScrollArea)
			scrollOffsetY = -_designData->_mouseScrollOffset;
		else if(_mouseCursorPosition.Y > _height1 - _designData->_mouseScrollArea)
			scrollOffsetY = _designData->_mouseScrollOffset;

		// => Scroll?
		if(scrollOffsetX != 0 || scrollOffsetY != 0)
		{
			// Scroll
			ApplyScrollOffset(scrollOffsetX, scrollOffsetY);

			// Update scroll timestamp
			_lastScrollActionTime = timeGetTime();
		}
	}

	// Update underlying window
	int result = base::DoUpdate();
	if(_underlyingWindow)
		_underlyingWindow->_VTable->DoUpdate(_underlyingWindow);
	return result;
}

int TechTreeWindow::HandleUserMessage(WPARAM wParam, LPARAM lParam)
{
	// Call handler of underlying window
	return (_underlyingWindow ? _underlyingWindow->_VTable->HandleUserMessage(_underlyingWindow, wParam, lParam) : 0);
}

int TechTreeWindow::HandleMouseButtonDown(int buttonId, int cursorPosX, int cursorPosY, int controlKeyPressed, int shiftKeyPressed)
{
	// Left click?
	if(buttonId == 1)
	{
		// Force setting selected element
		if(_selectedElement != nullptr)
		{
			// Set selected element
			ApplySelectedElementAndRedraw();

			// Element shouldn't be deselected even after another mouse move
			_selectedElementChangedTime -= _designData->_popupLabelDelay;
		}
	}

	// Run base class mouse down handler
	return base::HandleMouseButtonDown(buttonId, cursorPosX, cursorPosY, controlKeyPressed, shiftKeyPressed);
}

int TechTreeWindow::HandleMouseMove(int cursorPosX, int cursorPosY, int controlKeyPressed, int shiftKeyPressed)
{
	// If middle mouse button is pressed, do manual scrolling
	if(GetAsyncKeyState(VK_MBUTTON) & 0x8000)
		ApplyScrollOffset(_mouseCursorPosition.X - cursorPosX, _mouseCursorPosition.Y - cursorPosY);

	// Save mouse cursor position
	_mouseCursorPosition = Point(cursorPosX, cursorPosY);

	// Update popup box visibility
	UpdatePopupBoxVisibility();

	// Run base class mouse move handler
	return base::HandleMouseMove(cursorPosX, cursorPosY, controlKeyPressed, shiftKeyPressed);
}

int TechTreeWindow::HandleKeyDown2(int keyDown, int lParam, int menuKeyDown, int controlKeyDown, int shiftKeyDown)
{
	// Scroll hotkeys
	if(keyDown == VK_LEFT)
	{
		// Scroll left
		ApplyScrollOffset(-_designData->_keyScrollOffset, 0);
		return 1;
	}
	else if(keyDown == VK_RIGHT)
	{
		// Scroll right
		ApplyScrollOffset(_designData->_keyScrollOffset, 0);
		return 1;
	}
	else if(keyDown == VK_PRIOR)
	{
		// Scroll up
		ApplyScrollOffset(0, -_designData->_keyScrollOffset);
		return 1;
	}
	else if(keyDown == VK_NEXT)
	{
		// Scroll down
		ApplyScrollOffset(0, _designData->_keyScrollOffset);
		return 1;
	}

	// TODO
	return 0;
}

int TechTreeWindow::HandleChildControlEvent(Control *triggeringControl, int code, int data1, int data2)
{
	// Close button up
	if(triggeringControl == _closeButton && code == 1)
	{
		// Close window, event is handled
		(*_staticGameObjectPointer)->SetPauseMode(0, 0);
		StaticControlContainer *staticControlContainer = StaticControlContainer::GetStaticHandle();
		staticControlContainer->SwitchToControl(_underlyingWindow->GetControlTypeName(), 0);
		staticControlContainer->RemoveControlFromListWithDestructorCall(_controlTypeName);
		return 1;
	}

	// Scroll button left, event is handled
	else if(triggeringControl == _scrollLeftButton && code == 1)
	{
		// Scroll left
		ApplyScrollOffset(-100, 0);
		return 1;
	}

	// Scroll button right, event is handled
	else if(triggeringControl == _scrollRightButton && code == 1)
	{
		// Scroll right
		ApplyScrollOffset(100, 0);
		return 1;
	}

	// Civ combo box selection change
	else if(triggeringControl == _civSelectionComboBox && code == 2)
	{
		// Update selection, but let execution flow go to default handling function
		SetCurrentCiv(_civSelectionComboBox->GetSelectedItemId());
	}

	// Call default function
	return base::HandleChildControlEvent(triggeringControl, code, data1, data2);
}

void TechTreeWindow::ApplyScrollOffset(int horizontalScrollOffset, int verticalScrollOffset)
{
	// Apply offsets
	_horizontalScrollOffset += horizontalScrollOffset;
	_verticalScrollOffset += verticalScrollOffset;

	// Check horizontal bounds
	int treeWidth = _renderer->GetFullWidth();
	if(_horizontalScrollOffset < 0 || treeWidth - _width1 < 0)
		_horizontalScrollOffset = 0;
	else if(_horizontalScrollOffset >= treeWidth - _width1)
		_horizontalScrollOffset = treeWidth - _width1;

	// Check vertical bound
	int treeHeight = _renderer->GetFullHeight();
	if(_verticalScrollOffset < 0 || treeHeight - _height1 < 0)
		_verticalScrollOffset = 0;
	else if(_verticalScrollOffset >= treeHeight - _height1)
		_verticalScrollOffset = treeHeight - _height1;

	// Update civ bonus label position
	const Rect *civBonusLabelRectangle = _renderer->GetCivBonusLabelRectangle();
	_civBonusLabel->UpdatePositionAndSizeData(
		civBonusLabelRectangle->X - _horizontalScrollOffset,
		civBonusLabelRectangle->Y - _verticalScrollOffset,
		civBonusLabelRectangle->Width,
		civBonusLabelRectangle->Height);

	// Update civ selection combobox position
	const Rect *civSelectionComboBoxRectangle = _renderer->GetCivSelectionComboBoxRectangle();
	_civSelectionComboBox->UpdatePositionAndSizeData(
		civSelectionComboBoxRectangle->X - _horizontalScrollOffset,
		civSelectionComboBoxRectangle->Y - _verticalScrollOffset,
		civSelectionComboBoxRectangle->Width,
		civSelectionComboBoxRectangle->Height);

	// Update "game civs" label position
	const Rect *gameCivsLabelRectangle = _renderer->GetGameCivsLabelRectangle();
	_gameCivsLabel->UpdatePositionAndSizeData(
		gameCivsLabelRectangle->X - _horizontalScrollOffset,
		gameCivsLabelRectangle->Y - _verticalScrollOffset,
		gameCivsLabelRectangle->Width,
		gameCivsLabelRectangle->Height);

	// Update "not researched" label position
	const Rect *legendNotResearchedLabelRectangle = _renderer->GetLegendNotResearchedLabelRectangle();
	_legendNotResearchedLabel->UpdatePositionAndSizeData(
		legendNotResearchedLabelRectangle->X - _horizontalScrollOffset,
		legendNotResearchedLabelRectangle->Y - _verticalScrollOffset,
		legendNotResearchedLabelRectangle->Width,
		legendNotResearchedLabelRectangle->Height);

	// Update "researched" label position
	const Rect *legendResearchedLabelRectangle = _renderer->GetLegendResearchedLabelRectangle();
	_legendResearchedLabel->UpdatePositionAndSizeData(
		legendResearchedLabelRectangle->X - _horizontalScrollOffset,
		legendResearchedLabelRectangle->Y - _verticalScrollOffset,
		legendResearchedLabelRectangle->Width,
		legendResearchedLabelRectangle->Height);

	// Update legend label positions
	for(int i = 0; i < _legendNodeTypeLabels->size(); ++i)
	{
		// Get rectangle and update position
		const Rect *currLegendLabelRectangle = _renderer->GetLegendNodeTypeLabelRectangle(i);
		_legendNodeTypeLabels->at(i)->UpdatePositionAndSizeData(
			currLegendLabelRectangle->X - _horizontalScrollOffset,
			currLegendLabelRectangle->Y - _verticalScrollOffset,
			currLegendLabelRectangle->Width,
			currLegendLabelRectangle->Height);
	}

	// Update "disabled" label position
	const Rect *legendDisabledLabelRectangle = _renderer->GetLegendDisabledLabelRectangle();
	_legendDisabledLabel->UpdatePositionAndSizeData(
		legendDisabledLabelRectangle->X - _horizontalScrollOffset,
		legendDisabledLabelRectangle->Y - _verticalScrollOffset,
		legendDisabledLabelRectangle->Width,
		legendDisabledLabelRectangle->Height);

	// Update age label positions
	for(int i = 0; i < _ageCount; ++i)
		for(int j = 0; j < 4; ++j)
		{
			const Rect *currRect = _renderer->GetAgeLabelRectangle(i, (j >> 1) == 1, (j & 1) == 1);
			_ageLabels[i][j >> 1][j & 1]->UpdatePositionAndSizeData(currRect->X - _horizontalScrollOffset, currRect->Y - _verticalScrollOffset, currRect->Width, currRect->Height);
		}

	// Update popup label (maybe the element was left when scrolling)
	UpdatePopupBoxVisibility();

	// Redraw
	_VTable->InvalidateAndRedrawControl2(this, 1);
}

void TechTreeWindow::SetCurrentCiv(int civId)
{
	// Save ID
	_currentCivId = civId;

	// Update tech tree
	_renderer->SetCurrentCiv(_currentCivId);

	// Update control positions
	ApplyScrollOffset(0, 0);

	// Update description label
	char civString[2048];
	(*_staticGameObjectPointer)->GetStringFromLanguageDllsWithBuffer(20149 + _currentCivId, civString, sizeof(civString));
	_civBonusLabel->SetTextFormat(civString, -1, 0);

	// Redraw
	_VTable->InvalidateAndRedrawControl2(this, 1);
}

void TechTreeWindow::UpdatePopupBoxVisibility()
{
	// Get selected element
	TechTreeElement *newSelectedElement = _renderer->GetElementAtPosition(_mouseCursorPosition.X + _horizontalScrollOffset, _mouseCursorPosition.Y + _verticalScrollOffset);
	if(newSelectedElement != _selectedElement && newSelectedElement != nullptr)
		_selectedElementChangedTime = timeGetTime();

	// If the selected element changed, or there is no one, hide popup label
	if((newSelectedElement != _selectedElement || newSelectedElement == nullptr) && _drawPopupLabelBox)
	{
		// Hide label
		_drawPopupLabelBox = false;
		_renderer->SetSelectedElement(nullptr);
		_currPopupBoxElement = nullptr;
		static_cast<LabelControlVTable *>(_popupLabel->_VTable)->SetTextFormat(_popupLabel, " ", 0, 0);
	}

	// Update selected element
	_selectedElement = newSelectedElement;

	// Recalculate popup label box position and size
	UpdatePopupLabelDrawData();

	// Redraw
	_VTable->InvalidateAndRedrawControl2(this, 1);
}

void TechTreeWindow::ApplySelectedElementAndRedraw()
{
	// Element changed?
	// If not, no (expensive) update is necessary
	if(_currPopupBoxElement == _selectedElement)
		return;
	_currPopupBoxElement = _selectedElement;
	
	// Update selected element in renderer
	_renderer->SetSelectedElement(_selectedElement);

	// Get description text
	int dllIndex = -1;
	if(_selectedElement->_elementType == TechTreeElement::ItemType::Building || _selectedElement->_elementType == TechTreeElement::ItemType::Creatable)
		dllIndex = (*_staticGameObjectPointer)->GetGameDataHandler()->_civs[_currentCivId]->_units[_selectedElement->_elementObjectID]->_languageDllNameId + 21000;
	else if(_selectedElement->_elementType == TechTreeElement::ItemType::Research)
		dllIndex = (*_staticGameObjectPointer)->GetGameDataHandler()->_researches->_researches[_selectedElement->_elementObjectID]._languageDLLName1 + 21000;
	char descriptionTextBuffer[2048];
	(*_staticGameObjectPointer)->GetStringFromLanguageDllsWithBuffer(dllIndex, descriptionTextBuffer, sizeof(descriptionTextBuffer));

	// Are there required elements?
	if(_selectedElement->_requiredElements.size() > 0)
	{
		// Load "required" string
		if(popupBoxRequiredString[0] == '\0')
			(*_staticGameObjectPointer)->GetStringFromLanguageDllsWithBuffer(27003, popupBoxRequiredString, sizeof(popupBoxRequiredString));

		// Concat texts
		strcat(descriptionTextBuffer, popupBoxRequiredString);
	}

	// Pass text to popup label
	static_cast<LabelControlVTable *>(_popupLabel->_VTable)->SetTextFormat(_popupLabel, descriptionTextBuffer, _selectedElement->_elementObjectID, (_selectedElement->_elementType == TechTreeElement::ItemType::Research ? 1 : 0));

	// Calculate and apply popup label size
	Size popupLabelSize(_popupLabel->GetWidth(), _popupLabel->GetLineCount() * _popupLabelBaseFont->GetCharHeightWithRowSpace() + 30);
	_popupLabel->_VTable->UpdatePositionAndSizeAndOtherData(_popupLabel, 0, 0, 0, 0, 0, popupLabelSize.X, popupLabelSize.X, popupLabelSize.Y, popupLabelSize.Y, 0, 0, 0, 0);

	// Calculate popup label box size
	Rect popupLabelBoxDrawData = _renderer->UpdateAndGetPopupLabelBoxDrawData(popupLabelSize);
	_popupLabelRelativeDrawPosition = Point(popupLabelBoxDrawData.X, popupLabelBoxDrawData.Y);
	_popupLabelBoxSize = Size(popupLabelBoxDrawData.Width, popupLabelBoxDrawData.Height);

	// Recalculate popup label box position
	_drawPopupLabelBox = true;
	UpdatePopupLabelDrawData();

	// Redraw
	_VTable->InvalidateAndRedrawControl2(this, 1);
}

void TechTreeWindow::UpdatePopupLabelDrawData()
{
	// Calculate popup label box position (prefer position top left of the mouse cursor)
	_popupLabelBoxPosition.X = _mouseCursorPosition.X;
	if(_popupLabelBoxPosition.X > _popupLabelBoxSize.X)
		_popupLabelBoxPosition.X -= _popupLabelBoxSize.X;
	_popupLabelBoxPosition.Y = _mouseCursorPosition.Y;
	if(_popupLabelBoxPosition.Y > _popupLabelBoxSize.Y)
		_popupLabelBoxPosition.Y -= _popupLabelBoxSize.Y;

	// Update popup label position
	int popupLabelWidth = _popupLabel->GetWidth();
	int popupLabelHeight = _popupLabel->GetHeight();
	int popupLabelPosX = _popupLabelBoxPosition.X + _popupLabelRelativeDrawPosition.X;
	int popupLabelPosY = _popupLabelBoxPosition.Y + _popupLabelRelativeDrawPosition.Y;
	_popupLabel->_VTable->UpdatePositionAndSizeAndOtherData(_popupLabel, 0, popupLabelPosX, popupLabelPosY, 0, 0, popupLabelWidth, popupLabelWidth, popupLabelHeight, popupLabelHeight, 0, 0, 0, 0);
}
