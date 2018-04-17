#pragma once

#include "../filter_config_window.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/rtl/manufactory.h"
#include "../../../../common/rtl/referencedImpl.h"
#include "../../../../common/rtl/FilterLib.h"
#include "../../../../common/rtl/UILib.h"
#include "../../../../../common/QtUtils.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>

/*
 * Template class for selection of object identified by GUID (id), having description and generic getter function
 * This is suitable for model, solver and metric comboboxes
 */
template <typename TDesc, typename std::vector<TDesc>(*G)()>
class CGUID_Entity_ComboBox : public QComboBox, public virtual filter_config_window::CContainer_Edit
{
	private:
		glucose::NParameter_Type mParamType;

	public:
		CGUID_Entity_ComboBox(QWidget *parent, glucose::NParameter_Type paramType) : QComboBox(parent), mParamType(paramType)
		{
			auto entities = G();

			// add entities retrieved using template function
			for (auto entity : entities)
				addItem(StdWStringToQString(entity.description), QVariant{ QByteArray(reinterpret_cast<const char*>(&entity.id)) });
		}

		glucose::TFilter_Parameter get_parameter() override
		{
			glucose::TFilter_Parameter result;
			result.type = mParamType;
			result.guid = *reinterpret_cast<const GUID*>(currentData().toByteArray().constData());
			return result;
		}

		void set_parameter(const glucose::TFilter_Parameter &param) override
		{
			auto entities = G();

			// find guid in items, set selected index
			for (int i = 0; i < count(); i++)
			{
				if (param.guid == *reinterpret_cast<const GUID*>(itemData(i).toByteArray().constData()))
				{
					setCurrentIndex(i);
					break;
				}
			}
		}
};

/*
 * Model-dependent signal ID selection combobox; connected with model selector (for appropriate signal selection)
 */
class CModel_Signal_Select_ComboBox : public QComboBox, public virtual filter_config_window::CContainer_Edit
{
	private:
		// connected model selector combobox
		const QComboBox *mModelSelector;

	protected:
		// refreshes combobox contents using model selector value
		void Refresh_Contents();

	public:
		CModel_Signal_Select_ComboBox(QWidget *parent, QComboBox *modelSelector);

		glucose::TFilter_Parameter get_parameter() override;
		void set_parameter(const glucose::TFilter_Parameter &param) override;
};

/*
 * All available signal ID selection combobox; connected with model selector (for appropriate signal selection)
 */
class CAvailable_Signal_Select_ComboBox : public QComboBox, public virtual filter_config_window::CContainer_Edit
{
	private:
		// we need to preserve order, so we use vector of pairs instead of map; we don't use advantages of map anyways
		std::vector<std::pair<GUID, std::wstring>> mSignalVector;

	public:
		CAvailable_Signal_Select_ComboBox(QWidget *parent);

		glucose::TFilter_Parameter get_parameter() override;
		void set_parameter(const glucose::TFilter_Parameter &param) override;
};
