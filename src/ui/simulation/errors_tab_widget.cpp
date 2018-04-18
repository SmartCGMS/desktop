#include "errors_tab_widget.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/rtl/UILib.h"
#include "../../../../../common/QtUtils.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollBar>

#include <fstream>

#include <QtCore/QTimer>
#include <QtCore/QEventLoop>

#include "moc_errors_tab_widget.cpp"

constexpr int Error_Column_Count = glucose::Error_Misc_Count /* misc info */ + glucose::Error_Quantile_Count /* quantiles */;

// names of error types
const wchar_t* gError_Names[glucose::Error_Type_Count] = {
	dsError_Absolute,
	dsError_Relative
};
// ensure array length
static_assert((sizeof(gError_Names) / sizeof(const wchar_t*)) == glucose::Error_Type_Count, "Error type names count does not match error types defined");

// names of columns in table
const wchar_t* gError_Column_Names[] = {
	dsError_Column_Average,
	dsError_Column_StdDev,
	dsError_Column_AIC,
	dsError_Column_Minimum,
	dsError_Column_First_Quantile,
	dsError_Column_Median,
	dsError_Column_Third_Quantile,
	dsError_Column_95_Quantile,
	dsError_Column_99_Quantile,
	dsError_Column_Maximum
};
// ensure array length
static_assert((sizeof(gError_Column_Names) / sizeof(const wchar_t*)) == Error_Column_Count, "Column count does not match error columns defined");

CError_Table_Model::CError_Table_Model(QObject *parent)
	: QAbstractTableModel(parent), mMaxSignalRow(0)
{
}

int CError_Table_Model::rowCount(const QModelIndex &idx) const
{
	return mMaxSignalRow * glucose::Error_Type_Count;
}

int CError_Table_Model::columnCount(const QModelIndex &idx) const
{
	return Error_Column_Count;
}

QString Format_Error_String(glucose::NError_Type type, double val)
{
	// infinite, NaN, or other values like that results in empty string
	const auto cl = fpclassify(val);
	if (cl != FP_NORMAL && cl != FP_ZERO)
		return QString();

	switch (type)
	{
		case glucose::NError_Type::Absolute:
			return QString("%1").arg(val, 0, 'g', 4);
		case glucose::NError_Type::Relative:
			return QString("%1%").arg(val*100.0, 0, 'g', 4);
	}

	return QString::number(val);
}

QVariant CError_Table_Model::data(const QModelIndex &index, int role) const
{
	// content of error cells
	if (role == Qt::DisplayRole)
	{
		const int row = index.row();
		double val;

		if (index.column() < glucose::Error_Misc_Count)
			val = mErrors[row / glucose::Error_Type_Count][row % glucose::Error_Type_Count].misc[index.column()];
		else
			val = mErrors[row / glucose::Error_Type_Count][row % glucose::Error_Type_Count].q[index.column() - glucose::Error_Misc_Count];

		return Format_Error_String((glucose::NError_Type)(row % glucose::Error_Type_Count), val);
	}
	else if (role == Qt::BackgroundRole)
	{
		// use gray background for "average" column
		if (index.column() >= glucose::Error_Misc_Count)
			return QBrush(QColor(224, 224, 224));
	}
	return QVariant();
}

QVariant CError_Table_Model::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		// horizontal - use column names (fixed)
		if (orientation == Qt::Horizontal)
		{
			if (section >= 0 && section < Error_Column_Count)
				return StdWStringToQString(gError_Column_Names[section]);
		}
		// vertical - use signal names and error type names
		else if (orientation == Qt::Vertical)
			return StdWStringToQString(mSignalNameList[section / glucose::Error_Type_Count] + L" (" + gError_Names[section % glucose::Error_Type_Count] + L")");
	}

	return QVariant();
}

bool CError_Table_Model::insertRows(int position, int rows, const QModelIndex &index)
{
	beginInsertRows(QModelIndex(), position, position + rows - 1);

	mErrors.resize(position + 1);

	endInsertRows();

	return true;
}

bool CError_Table_Model::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid())
	{
		int row = index.row();

		if (index.column() < glucose::Error_Misc_Count)
			mErrors[row / glucose::Error_Type_Count][row % glucose::Error_Type_Count].misc[index.column()] = value.toDouble();
		else if (index.column() < Error_Column_Count)
			mErrors[row / glucose::Error_Type_Count][row % glucose::Error_Type_Count].q[index.column() - glucose::Error_Misc_Count] = value.toDouble();

		emit(dataChanged(index, index));

		return true;
	}

	return false;
}

void CError_Table_Model::Set_Error(const GUID& signal_id, std::wstring signal_name, const glucose::TError_Container& errors, const glucose::NError_Type type)
{
	bool added = false;
	int row;

	if (mSignalRow.find(signal_id) == mSignalRow.end())
	{
		row = mMaxSignalRow++;

		mSignalRow[signal_id] = row;
		mSignalNameList.push_back(signal_name);

		insertRows(row, glucose::Error_Type_Count, QModelIndex());
	}
	else
		row = mSignalRow[signal_id];

	int offset = (int)type;

	QModelIndex idx = index(row*glucose::Error_Type_Count + offset, 0, QModelIndex());
	for (int i = 0; i < glucose::Error_Misc_Count; i++)
	{
		idx = index(row*glucose::Error_Type_Count + offset, i, QModelIndex());
		setData(idx, errors.misc[i], Qt::EditRole);
	}
	for (int i = 0; i < glucose::Error_Quantile_Count; i++)
	{
		idx = index(row*glucose::Error_Type_Count + offset, i + glucose::Error_Misc_Count, QModelIndex());
		setData(idx, errors.q[i], Qt::EditRole);
	}
}




CErrors_Tab_Widget::CErrors_Tab_Widget(QWidget *parent)
	: CAbstract_Simulation_Tab_Widget(parent)
{
	QGridLayout *mainLayout = new QGridLayout();

	mTableView = new QTableView();
	mModel = new CError_Table_Model(nullptr);
	mTableView->setModel(mModel);
	mainLayout->addWidget(mTableView);

	auto models = glucose::get_model_descriptors();
	for (auto& model : models)
	{
		for (size_t i = 0; i < model.number_of_calculated_signals; i++)
			mSignalNames[model.calculated_signal_ids[i]] = model.description + std::wstring(L" - ") + model.calculated_signal_names[i];
	}

	for (size_t i = 0; i<glucose::signal_Virtual.size(); i++)		
		mSignalNames[glucose::signal_Virtual[i]] = dsSignal_Prefix_Virtual + std::wstring(L" ") + std::to_wstring(i);


	setLayout(mainLayout);
}

void CErrors_Tab_Widget::Update_Error_Metrics(const GUID& signal_id, glucose::TError_Container& container, glucose::NError_Type type)
{
	QEventLoop loop;
	Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this, signal_id, container, type]()
	{
		mModel->Set_Error(signal_id, mSignalNames[signal_id], container, type);

		mTableView->repaint();
	});
}

void CErrors_Tab_Widget::Reset()
{
	QEventLoop loop;
	Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this]()
	{
		auto newModel = new CError_Table_Model(nullptr);
		mTableView->setModel(newModel);

		delete mModel;
		mModel = newModel;
	});
}
