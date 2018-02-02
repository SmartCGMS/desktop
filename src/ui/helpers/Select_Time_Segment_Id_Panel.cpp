#include "Select_Time_Segment_Id_Panel.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/rtl/FilterLib.h"

#include "moc_Select_Time_Segment_Id_Panel.cpp"

CSelect_Time_Segment_Id_Panel::CSelect_Time_Segment_Id_Panel(const std::vector<glucose::TFilter_Parameter>& configuration, QWidget * parent) : QTableView(parent), mConfiguration(configuration) {
}


glucose::TFilter_Parameter CSelect_Time_Segment_Id_Panel::get_parameter() {
	std::vector<int64_t> segment_ids;

	if (mDb && mDb->isOpen()) {
		//prepare Qt iterators
		auto selindexes = selectionModel()->selectedIndexes();
		auto iterend = selindexes.end();
		auto iterbegin = selindexes.begin();
		auto selcol = iterbegin->column();


		//and get all selected time segment ids
		for (auto iter = iterbegin; iter < iterend; iter++) {
			if (iter->column() == selcol) {
				segment_ids.push_back(mSegmentsModel->data(mSegmentsModel->index(iter->row(), 0)).toInt());
			}
		}
	}

	//then, convert these ids into the array list

	glucose::TFilter_Parameter result;
	result.type = glucose::NParameter_Type::ptSelect_Time_Segment_ID;	
	auto container = glucose::Create_Parameter_Container<int64_t>(segment_ids.data(), segment_ids.data() + segment_ids.size());
	result.select_time_segment_id = container.get();
	result.select_time_segment_id->AddRef();
	return result;
}

void CSelect_Time_Segment_Id_Panel::set_parameter(const glucose::TFilter_Parameter &param) {
	if (!mDb) apply(); //try to connect first

	int64_t *begin, *end;
	if (param.select_time_segment_id->get(&begin, &end) == S_OK) {
		auto is_in_selection = [begin, end](const int id)->bool {
			for (auto iter = begin; iter != end; iter++)
				if (*iter == id) return true;
			return false;
		};

		if (mSegmentsModel)
			for (int data_row = 0;  data_row<mSegmentsModel->rowCount(); data_row++) {
				if (is_in_selection(mSegmentsModel->data(mSegmentsModel->index(data_row, 0)).toInt()))
					selectRow(data_row);
			}
		
		
	}
}

void CSelect_Time_Segment_Id_Panel::apply() {
	auto current_selection = get_parameter();

	auto get_attr = [this](const wchar_t* attr_name)->std::wstring {

		for (const auto &param : mConfiguration) {
			if (WChar_Container_Equals_WString(param.config_name, attr_name)) {				
				return WChar_Container_To_WString(param.wstr);
			}
		}

		return std::wstring{};	
	};

	mSegmentsModel.reset(nullptr);
	mSegmentsQuery.reset(nullptr);
	if (mDb) {
		QString connection;
		connection = mDb->connectionName();		
		mDb.reset(nullptr);
		QSqlDatabase::removeDatabase(connection);

	}

	mDb = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase(QString::fromStdWString(get_attr(rsDb_Provider)), mDb_Connection_Name));
	mDb->setHostName(QString::fromStdWString(get_attr(rsDb_Host)));
	mDb->setDatabaseName(QString::fromStdWString(get_attr(rsDb_Name)));
	mDb->setUserName(QString::fromStdWString(get_attr(rsDb_User_Name)));
	mDb->setPassword(QString::fromStdWString(get_attr(rsDb_Password)));
	
	if (mDb->open()) {

		mSegmentsQuery = std::make_unique<QSqlQuery>(*mDb.get());
		mSegmentsQuery->prepare(rsSelect_Subjects_And_Segments_For_Db_Reader_Filter);
		mSegmentsQuery->exec();

		mSegmentsModel = std::make_unique<QSqlQueryModel>();
		mSegmentsModel->setQuery(*mSegmentsQuery.get());

		//and fix the UI
		setModel(mSegmentsModel.get());
		setSelectionMode(QAbstractItemView::MultiSelection);
		setSelectionBehavior(QAbstractItemView::SelectRows);

		hideColumn(0); //segment it id
		mSegmentsModel->setHeaderData(1, Qt::Horizontal, tr(dsSubject));
		mSegmentsModel->setHeaderData(2, Qt::Horizontal, tr(dsSegment));

		set_parameter(current_selection);		
	}
}