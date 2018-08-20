#pragma once

#include <map>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>

class CTime_Segment_Group_Widget : public QGroupBox
{
	private:
		QCheckBox *mCheckBox;
		uint64_t mSegment_Id;

	public:
		explicit CTime_Segment_Group_Widget(uint64_t segmentId, QWidget* parent = nullptr);
		virtual ~CTime_Segment_Group_Widget();

		bool Is_Checked() const;
		void Set_Checked(bool state);
		uint64_t Get_Segment_Id() const;
};
