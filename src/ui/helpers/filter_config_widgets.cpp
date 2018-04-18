
#include "filter_config_widgets.h"

glucose::TFilter_Parameter CModel_Signal_Select_ComboBox::get_parameter()
{
	glucose::TFilter_Parameter result;
	result.type = glucose::NParameter_Type::ptModel_Signal_Id;
	result.guid = *reinterpret_cast<const GUID*>(currentData().toByteArray().constData());
	return result;
}

void CModel_Signal_Select_ComboBox::set_parameter(const glucose::TFilter_Parameter &param)
{
	auto models = glucose::get_model_descriptors();

	for (int i = 0; i < count(); i++)
	{
		if (param.guid == *reinterpret_cast<const GUID*>(itemData(i).toByteArray().constData()))
		{
			setCurrentIndex(i);
			break;
		}
	}
}

void CModel_Signal_Select_ComboBox::Refresh_Contents()
{
	// always clear contents
	clear();

	if (mModelSelector->currentIndex() >= 0)
	{
		// get selected model GUID
		glucose::TModel_Descriptor model{ 0 };
		const GUID selectedModelGUID = *reinterpret_cast<const GUID*>(mModelSelector->currentData().toByteArray().constData());

		// retrieve proper model
		if (glucose::get_model_descriptors_by_id(selectedModelGUID, model))
		{
			// add model signals to combobox
			for (size_t i = 0; i < model.number_of_calculated_signals; i++)
				addItem(StdWStringToQString(model.calculated_signal_names[i]), QVariant{ QByteArray(reinterpret_cast<const char*>(&model.calculated_signal_ids[i])) });
		}
	}
}

CModel_Signal_Select_ComboBox::CModel_Signal_Select_ComboBox(QWidget *parent, QComboBox *modelSelector)
	: QComboBox(parent), mModelSelector(modelSelector)
{
	Refresh_Contents();

	connect(mModelSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
		Refresh_Contents();
	});
}

CAvailable_Signal_Select_ComboBox::CAvailable_Signal_Select_ComboBox(QWidget *parent)
	: QComboBox(parent)
{
	// append measured signals
	std::wstring measSuffix = dsSignal_Suffix_Measured;
	measSuffix = L" (" + measSuffix + L")";

	//TO DO: enumerate input known signals in some global header
	mSignalVector.push_back({ glucose::signal_BG, dsSignal_Measured_BG + measSuffix });
	mSignalVector.push_back({ glucose::signal_IG, dsSignal_Measured_IG + measSuffix });
	mSignalVector.push_back({ glucose::signal_ISIG, dsSignal_Measured_ISIG + measSuffix });
	mSignalVector.push_back({ glucose::signal_Calibration, dsSignal_Measured_Calibration + measSuffix });
	mSignalVector.push_back({ glucose::signal_Insulin, dsSignal_Measured_Insulin + measSuffix });
	mSignalVector.push_back({ glucose::signal_Carb_Intake, dsSignal_Measured_Carb_Intake + measSuffix });
	mSignalVector.push_back({ glucose::signal_Health_Stress, dsSignal_Measured_Health_Stress + measSuffix });

	// append calculated signals of known models
	std::wstring calcSuffix = dsSignal_Suffix_Calculated;
	calcSuffix = L" (" + calcSuffix + L")";

	auto models = glucose::get_model_descriptors();
	for (auto& model : models)
	{
		for (size_t i = 0; i < model.number_of_calculated_signals; i++)
			mSignalVector.push_back({ model.calculated_signal_ids[i], model.description + std::wstring(L" - ") + model.calculated_signal_names[i] + calcSuffix });
	}

	// append virtual signals
	for (size_t i=0; i<glucose::signal_Virtual.size(); i++)
		mSignalVector.push_back({ glucose::signal_Virtual[i], dsSignal_Prefix_Virtual + std::wstring(L" ") + std::to_wstring(i) });

	// add all signals to combobox

	for (auto const& signal : mSignalVector)
		addItem(StdWStringToQString(signal.second), QVariant{ QByteArray(reinterpret_cast<const char*>(&signal.first)) });
}

glucose::TFilter_Parameter CAvailable_Signal_Select_ComboBox::get_parameter()
{
	glucose::TFilter_Parameter result;
	result.type = glucose::NParameter_Type::ptSignal_Id;
	result.guid = *reinterpret_cast<const GUID*>(currentData().toByteArray().constData());
	return result;
}

void CAvailable_Signal_Select_ComboBox::set_parameter(const glucose::TFilter_Parameter &param)
{
	auto models = glucose::get_model_descriptors();

	for (int i = 0; i < count(); i++)
	{
		if (param.guid == *reinterpret_cast<const GUID*>(itemData(i).toByteArray().constData()))
		{
			setCurrentIndex(i);
			break;
		}
	}
}
