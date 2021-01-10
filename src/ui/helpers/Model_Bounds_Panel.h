/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Copyright (c) since 2018 University of West Bohemia.
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Univerzitni 8, 301 00 Pilsen
 * Czech Republic
 * 
 * 
 * Purpose of this software:
 * This software is intended to demonstrate work of the diabetes.zcu.cz research
 * group to other scientists, to complement our published papers. It is strictly
 * prohibited to use this software for diagnosis or treatment of any medical condition,
 * without obtaining all required approvals from respective regulatory bodies.
 *
 * Especially, a diabetic patient is warned that unauthorized use of this software
 * may result into severe injure, including death.
 *
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under these license terms is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) For non-profit, academic research, this software is available under the
 *      GPLv3 license.
 * b) For any other use, especially commercial use, you must contact us and
 *       obtain specific terms and conditions for the use of the software.
 * c) When publishing work with results obtained using this software, you agree to cite the following paper:
 *       Tomas Koutny and Martin Ubl, "Parallel software architecture for the next generation of glucose
 *       monitoring", Procedia Computer Science, Volume 141C, pp. 279-286, 2018
 */

#pragma once

#include "general_container_edit.h"

#include "../../../../common/rtl/FilterLib.h"


#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QItemDelegate>

#include <QtWidgets/QTableView>
#include <QtCore/QAbstractTableModel>

namespace CModel_Bounds_Panel_internal {

	class CParameters_Table_Model : public QAbstractTableModel {
		Q_OBJECT
	protected:		
		std::vector<QString> mNames;
		double *Get_Data(const int col);
	public:
		std::vector<scgms::NModel_Parameter_Value> mTypes;
		std::vector<double> mLower_Bounds, mDefault_Values, mUpper_Bounds;

	public:
		explicit CParameters_Table_Model(QObject *parent = 0) noexcept;

		virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
		virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
		virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
		virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
		virtual Qt::ItemFlags flags(const QModelIndex & index) const override;
		
		void Load_Parameters(const scgms::TModel_Descriptor& model, const double* lower_bounds, const double* defaults, const double* upper_bounds);
		std::vector<double> Store_Parameters();
	};

	class CParameter_Value_Delegate : public QItemDelegate {
		Q_OBJECT
	protected:
		std::vector<scgms::NModel_Parameter_Value> &mTypes;
		std::vector<double> &mLower_Bounds, &mDefault_Values, &mUpper_Bounds;
	public:
		CParameter_Value_Delegate(std::vector<scgms::NModel_Parameter_Value> &types,
			std::vector<double> &lower, std::vector<double> &default_values, std::vector<double> &upper,
			QObject *parent);

		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
			const QModelIndex &index) const override;
		void setEditorData(QWidget *editor, const QModelIndex &index) const override;
		void setModelData(QWidget *editor, QAbstractItemModel *model,
			const QModelIndex &index) const override;
	};
}


/*
 * Panel for selecting and setting model bounds and default parameters
 */
class CModel_Bounds_Panel : public QWidget, public virtual filter_config_window::CContainer_Edit {
	Q_OBJECT
protected:
	// connected model selector
	QComboBox *mModelSelector;
	const GUID mFixed_Model;	//in a case that mModelSelector is nullptr
	// inner layout
	QVBoxLayout* mLayout;

	QTableView* mTableView;
	// table model for error metrics
	CModel_Bounds_Panel_internal::CParameters_Table_Model* mModel;

	// retrieves currently selected model; returns true on success
	bool Get_Currently_Selected_Model(scgms::TModel_Descriptor& model);

	void Reset_Parameters(std::vector<double> &values, std::function<const double*(const scgms::TModel_Descriptor&)> get_bounds);
protected slots:
	void On_Reset_Lower();
	void On_Reset_Defaults();
	void On_Reset_Upper();

public:
		//modelSelector can be nullptr, but fixed_model must be a valid model ID then
	CModel_Bounds_Panel(scgms::SFilter_Parameter parameter, QComboBox* modelSelector, const GUID &fixed_model, QWidget *parent);

	virtual void fetch_parameter() override;
	virtual void store_parameter() override;
};