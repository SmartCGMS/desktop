#include "Model_Bounds_Panel.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/rtl/UILib.h"

#include <QtWidgets/QLabel>
#include <QtGui/QDoubleValidator>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>

#include "moc_Model_Bounds_Panel.cpp"

CModel_Bounds_Panel::CModel_Bounds_Panel(QComboBox* modelSelector, QWidget * parent)
	: QWidget(parent), mModelSelector(modelSelector)
{
	QVBoxLayout* layout = new QVBoxLayout();
	setLayout(layout);

	QWidget* contents = new QWidget();
	mLayout = new QGridLayout();
	contents->setLayout(mLayout);

	layout->addWidget(contents);
	layout->addStretch();

	connect(mModelSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
		Refresh_Contents();
	});

	Refresh_Contents();
}

glucose::TFilter_Parameter CModel_Bounds_Panel::get_parameter()
{
	std::vector<double> values;
	bool ok;

	// "serialize" fields
	for (auto& container : { mLowerBoundEdits, mDefaultsEdits, mUpperBoundEdits })
	{
		for (size_t i = 0; i < container.size(); i++)
		{
			double dbl = container[i]->text().toDouble(&ok);
			values.push_back(ok ? dbl : 0.0);
		}
	}

	glucose::TFilter_Parameter result;
	result.type = glucose::NParameter_Type::ptModel_Bounds;
	result.parameters = refcnt::Create_Container<double>(values.data(), values.data() + values.size());
	return result;
}

void CModel_Bounds_Panel::set_parameter(const glucose::TFilter_Parameter &param)
{
	if (param.type != glucose::NParameter_Type::ptModel_Bounds)
		return;

	Refresh_Contents(param.parameters);
}

void CModel_Bounds_Panel::apply()
{
}

void CModel_Bounds_Panel::Reset_UI(const size_t parameter_count, const wchar_t** parameter_names, const double* lower_bounds, const double* defaults, const double* upper_bounds)
{
	// clear layout
	int colums = mLayout->columnCount();
	int rows = mLayout->rowCount();

	for (int j = 0; j < rows; j++)
	{
		for (int k = 0; k < colums; k++)
		{
			QLayoutItem* item = mLayout->itemAtPosition(j, k);

			if (!item)
				continue;

			mLayout->removeItem(item);
			delete item->widget();
			delete item;
		}
	}

	// add column headers
	mLayout->addWidget(new QLabel(dsLower_Bounds), 0, 1);
	mLayout->addWidget(new QLabel(dsDefault_Parameters), 0, 2);
	mLayout->addWidget(new QLabel(dsUpper_Bounds), 0, 3);

	auto create_edit = [](double val) -> QLineEdit* {
		QLineEdit* fld = new QLineEdit(QString::number(val, 'f'));
		auto validator = new QDoubleValidator();
		validator->setLocale(QLocale(QLocale::English));
		fld->setValidator(validator);
		return fld;
	};

	mLowerBoundEdits.clear();
	mDefaultsEdits.clear();
	mUpperBoundEdits.clear();

	for (size_t i = 0; i < parameter_count; i++)
	{
		mLowerBoundEdits.push_back(create_edit(lower_bounds[i]));
		mDefaultsEdits.push_back(create_edit(defaults[i]));
		mUpperBoundEdits.push_back(create_edit(upper_bounds[i]));
	}

	for (size_t i = 0; i < parameter_count; i++)
	{
		mLayout->addWidget(new QLabel(QString::fromWCharArray(parameter_names[i])), (int)i + 1, 0);

		mLayout->addWidget(mLowerBoundEdits[i], (int)i + 1, 1);
		mLayout->addWidget(mDefaultsEdits[i], (int)i + 1, 2);
		mLayout->addWidget(mUpperBoundEdits[i], (int)i + 1, 3);
	}

	// add reset buttons
	QPushButton* btn;

	btn = new QPushButton(dsReset_Bounds);
	connect(btn, SIGNAL(clicked()), this, SLOT(On_Reset_Lower()));
	mLayout->addWidget(btn, (int)parameter_count + 1, 1);

	btn = new QPushButton(dsReset_Bounds);
	connect(btn, SIGNAL(clicked()), this, SLOT(On_Reset_Defaults()));
	mLayout->addWidget(btn, (int)parameter_count + 1, 2);

	btn = new QPushButton(dsReset_Bounds);
	connect(btn, SIGNAL(clicked()), this, SLOT(On_Reset_Upper()));
	mLayout->addWidget(btn, (int)parameter_count + 1, 3);

	for (size_t i = 0; i < parameter_count + 2; i++)
		mLayout->setRowStretch((int)i, 1);
}

void CModel_Bounds_Panel::On_Reset_Lower()
{
	glucose::TModel_Descriptor model{ 0 };
	if (!Get_Current_Selected_Model(model))
		return;

	for (size_t i = 0; i < model.number_of_parameters; i++)
		mLowerBoundEdits[i]->setText(QString::number(model.lower_bound[i], 'f'));
}

void CModel_Bounds_Panel::On_Reset_Defaults()
{
	glucose::TModel_Descriptor model{ 0 };
	if (!Get_Current_Selected_Model(model))
		return;

	for (size_t i = 0; i < model.number_of_parameters; i++)
		mDefaultsEdits[i]->setText(QString::number(model.default_values[i], 'f'));
}

void CModel_Bounds_Panel::On_Reset_Upper()
{
	glucose::TModel_Descriptor model{ 0 };
	if (!Get_Current_Selected_Model(model))
		return;

	for (size_t i = 0; i < model.number_of_parameters; i++)
		mUpperBoundEdits[i]->setText(QString::number(model.upper_bound[i], 'f'));
}

bool CModel_Bounds_Panel::Get_Current_Selected_Model(glucose::TModel_Descriptor& model)
{
	if (mModelSelector->currentIndex() >= 0)
	{
		// get selected model GUID
		const GUID selectedModelGUID = *reinterpret_cast<const GUID*>(mModelSelector->currentData().toByteArray().constData());

		if (glucose::get_model_descriptor_by_id(selectedModelGUID, model))
			return true;
	}

	return false;
}

void CModel_Bounds_Panel::Refresh_Contents(glucose::IModel_Parameter_Vector* inputs)
{
	glucose::TModel_Descriptor model{ 0 };

	if (Get_Current_Selected_Model(model))
	{
		const double* lb = model.lower_bound;
		const double* def = model.default_values;
		const double* ub = model.upper_bound;

		double *beg, *end;

		if (inputs && inputs->get(&beg, &end) == S_OK)
		{
			if (std::distance(beg, end) == model.number_of_parameters * 3)
			{
				lb = beg;
				def = beg + model.number_of_parameters;
				ub = beg + 2 * model.number_of_parameters;
			}
		}

		Reset_UI(model.number_of_parameters, model.parameter_ui_names, lb, def, ub);
	}
}
