#include "mvDatePicker.h"
#include "mvContext.h"
#include <implot.h>
#include <implot_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include "mvItemRegistry.h"
#include "mvPythonExceptions.h"
#include "AppItems/fonts/mvFont.h"
#include "AppItems/themes/mvTheme.h"
#include "AppItems/containers/mvDragPayload.h"
#include "AppItems/widget_handlers/mvItemHandlerRegistry.h"

namespace Marvel {

	mvDatePicker::mvDatePicker(mvUUID uuid)
		: mvAppItem(uuid)
	{
	}

	void mvDatePicker::applySpecificTemplate(mvAppItem* item)
	{
		auto titem = static_cast<mvDatePicker*>(item);
		if (config.source != 0)
                { 
                    _value = titem->_value;
		    _imvalue = titem->_imvalue;
                }
		_level = titem->_level;
	}

	void mvDatePicker::draw(ImDrawList* drawlist, float x, float y)
	{

		//-----------------------------------------------------------------------------
		// pre draw
		//-----------------------------------------------------------------------------

		// show/hide
		if (!config.show)
			return;

		// focusing
		if (info.focusNextFrame)
		{
			ImGui::SetKeyboardFocusHere();
			info.focusNextFrame = false;
		}

		// cache old cursor position
		ImVec2 previousCursorPos = ImGui::GetCursorPos();

		// set cursor position if user set
		if (info.dirtyPos)
			ImGui::SetCursorPos(state.pos);

		// update widget's position state
		state.pos = { ImGui::GetCursorPosX(), ImGui::GetCursorPosY() };

		// set item width
		if (config.width != 0)
			ImGui::SetNextItemWidth((float)config.width);

		// set indent
		if (config.indent > 0.0f)
			ImGui::Indent(config.indent);

		// push font if a font object is attached
		if (font)
		{
			ImFont* fontptr = static_cast<mvFont*>(font.get())->getFontPtr();
			ImGui::PushFont(fontptr);
		}

		// themes
		apply_local_theming(this);

		//-----------------------------------------------------------------------------
		// draw
		//-----------------------------------------------------------------------------
		{

			ScopedID id(uuid);

			if (ImPlot::ShowDatePicker(info.internalLabel.c_str(), &_level, _imvalue.get(), _imvalue.get()))
			{
				ImPlot::GetGmtTime(*_imvalue, _value.get());
				{
					auto value = *_value;
					mvSubmitCallback([=]() {
						if(config.alias.empty())
							mvAddCallback(getCallback(false), uuid, ToPyTime(value), config.user_data);
						else
							mvAddCallback(getCallback(false), config.alias, ToPyTime(value), config.user_data);
						});
				}
			}
		}

		//-----------------------------------------------------------------------------
		// update state
		//-----------------------------------------------------------------------------
		UpdateAppItemState(state);

		//-----------------------------------------------------------------------------
		// post draw
		//-----------------------------------------------------------------------------

		// set cursor position to cached position
		if (info.dirtyPos)
			ImGui::SetCursorPos(previousCursorPos);

		if (config.indent > 0.0f)
			ImGui::Unindent(config.indent);

		// pop font off stack
		if (font)
			ImGui::PopFont();

		// handle popping themes
		cleanup_local_theming(this);

		if (handlerRegistry)
			handlerRegistry->checkEvents(&state);

		// handle drag & drop if used
		apply_drag_drop(this);
	}

	void mvDatePicker::handleSpecificKeywordArgs(PyObject* dict)
	{
		if (dict == nullptr)
			return;
		 
		if (PyObject* item = PyDict_GetItemString(dict, "level")) {
			_level = ToInt(item);
			if (_level > 2) _level = 0;
		}
	}

	void mvDatePicker::getSpecificConfiguration(PyObject* dict)
	{
		if (dict == nullptr)
			return;
		 
		PyDict_SetItemString(dict, "level", mvPyObject(ToPyLong((long)_level)));
	}

	PyObject* mvDatePicker::getPyValue()
	{
		return ToPyTime(*_value);
	}

	void mvDatePicker::setPyValue(PyObject* value)
	{

		if (value)
			*_value = ToTime(value);
		else
			_value = {};

		*_imvalue = ImPlot::MkGmtTime(_value.get());
	}
}