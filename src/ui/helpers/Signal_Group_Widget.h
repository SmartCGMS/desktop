#pragma once

#include <map>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>

#include "../../../../common/rtl/guid.h"

class CSignal_Group_Widget : public QGroupBox
{
	private:
		QCheckBox *mCheckBox;
		GUID mSignal_Id;

	public:
		explicit CSignal_Group_Widget(const GUID& signalId, QWidget* parent = nullptr);
		virtual ~CSignal_Group_Widget();

		bool Is_Checked() const;
		const GUID& Get_Signal_Id() const;
};
