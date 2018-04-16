#pragma once

#include "general_container_edit.h"

#include "../../../../common/rtl/FilterLib.h"

#include <map>

#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLineEdit>

/*
 * Panel for selecting and setting model bounds and default parameters
 */
class CModel_Bounds_Panel : public QWidget, public virtual filter_config_window::CContainer_Edit
{
	Q_OBJECT

	protected:
		// connected model selector
		QComboBox *mModelSelector;
		// inner layout
		QGridLayout* mLayout;

		// stored edits
		std::vector<QLineEdit*> mLowerBoundEdits, mDefaultsEdits, mUpperBoundEdits;

		// refreshes contents (may use given parameter set)
		void Refresh_Contents(glucose::IModel_Parameter_Vector* inputs = nullptr);
		// resets user interface using given values
		void Reset_UI(const size_t parameter_count, const wchar_t** parameter_names, const double* lower_bounds, const double* defaults, const double* upper_bounds);
		// retrieves currently selected model; returns true on success
		bool Get_Current_Selected_Model(glucose::TModel_Descriptor& model);

	protected slots:
		void On_Reset_Lower();
		void On_Reset_Defaults();
		void On_Reset_Upper();

	public:
		CModel_Bounds_Panel(QComboBox* modelSelector, QWidget *parent);

		virtual glucose::TFilter_Parameter get_parameter();
		virtual void set_parameter(const glucose::TFilter_Parameter &param);
		virtual void apply();
};