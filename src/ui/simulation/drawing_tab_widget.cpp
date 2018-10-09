/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Technicka 8
 * 314 06, Pilsen
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) For non-profit, academic research, this software is available under the
 *    GPLv3 license. When publishing any related work, user of this software
 *    must:
 *    1) let us know about the publication,
 *    2) acknowledge this software and respective literature - see the
 *       https://diabetes.zcu.cz/about#publications,
 *    3) At least, the user of this software must cite the following paper:
 *       Parallel software architecture for the next generation of glucose
 *       monitoring, Proceedings of the 8th International Conference on Current
 *       and Future Trends of Information and Communication Technologies
 *       in Healthcare (ICTH 2018) November 5-8, 2018, Leuven, Belgium
 * b) For any other use, especially commercial use, you must contact us and
 *    obtain specific terms and conditions for the use of the software.
 */

#include "drawing_tab_widget.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/utils/QtUtils.h"
#include "../../../../common/rtl/referencedImpl.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollBar>
#include <QtGui/QWheelEvent>

#include <iostream>
#include <fstream>

#include <QtCore/QTimer>
#include <QtCore/QEventLoop>
#include <QtWidgets/QMenu>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtCore/QTimeLine>

#include "moc_drawing_tab_widget.cpp"

// array of default names for image files by type
static const std::array<const char*, static_cast<size_t>(glucose::TDrawing_Image_Type::count)> Default_Filename_For_Type = { {
	dsSave_Image_Default_Filename_Graph,
	dsSave_Image_Default_Filename_Day,
	dsSave_Image_Default_Filename_Parkes,
	dsSave_Image_Default_Filename_Clark,
	dsSave_Image_Default_Filename_AGP,
	dsSave_Image_Default_Filename_ECDF
} };

CDrawing_Graphics_View::CDrawing_Graphics_View()
	: QGraphicsView()
{
	//
}

void CDrawing_Graphics_View::wheelEvent(QWheelEvent * event)
{
	const int numSteps = event->delta() / 120;

	mNumScheduledScalings += numSteps;
	if (mNumScheduledScalings * numSteps < 0)
		mNumScheduledScalings = numSteps;

	QTimeLine *anim = new QTimeLine(350, this);
	anim->setUpdateInterval(20);

	connect(anim, SIGNAL(valueChanged(qreal)), SLOT(scalingTime(qreal)));
	connect(anim, SIGNAL(finished()), SLOT(animFinished()));
	anim->start();
}

void CDrawing_Graphics_View::scalingTime(qreal x)
{
	qreal factor = 1.0 + qreal(mNumScheduledScalings) / 300.0;
	scale(factor, factor);
}

void CDrawing_Graphics_View::animFinished()
{
	if (mNumScheduledScalings < 0)
		mNumScheduledScalings++;
	sender()->~QObject();
}

CDrawing_Tab_Widget::CDrawing_Tab_Widget(const glucose::TDrawing_Image_Type type, QWidget *parent)
	: CAbstract_Simulation_Tab_Widget(parent), mType(type), mItem(nullptr), mDiagnosis_Box(nullptr), mCurrent_Diagnosis(glucose::TDiagnosis::Type1)
{
	mView = new CDrawing_Graphics_View();
	mScene = new QGraphicsScene(mView);
	mView->setScene(mScene);

	mView->setDragMode(QGraphicsView::ScrollHandDrag);
	mView->setRenderHints(QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing);

	mRenderer = new QSvgRenderer();

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(mView, 0, 0);
	setLayout(mainLayout);

	// just parkes' grid has to disambiguate between diagnosis types (for now)
	if (type == glucose::TDrawing_Image_Type::Parkes)
	{
		mDiagnosis_Box = new QComboBox(this);
		mDiagnosis_Box->addItem(dsDiagnosis_T1D, static_cast<int>(glucose::TDiagnosis::Type1));
		mDiagnosis_Box->addItem(dsDiagnosis_T2D, static_cast<int>(glucose::TDiagnosis::Type2));
		mDiagnosis_Box->addItem(dsDiagnosis_Gestational, static_cast<int>(glucose::TDiagnosis::Gestational));
		// this should relocate the widget on top of the drawing to top left corner somewhere
		auto geom = mDiagnosis_Box->geometry();
		mDiagnosis_Box->setGeometry(20, 20, geom.width(), geom.height());

		connect(mDiagnosis_Box, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(On_Diagnosis_Changed(const QString&)));
	}

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(Show_Context_Menu(const QPoint&)));
}

CDrawing_Tab_Widget::~CDrawing_Tab_Widget()
{
	if (mItem)
		delete mItem;

	mScene->clear();
	delete mRenderer;
}

void CDrawing_Tab_Widget::Update_View_Size()
{
	mView->resetMatrix();
}

CAbstract_Simulation_Tab_Widget* CDrawing_Tab_Widget::Clone()
{
	CDrawing_Tab_Widget* cloned = new CDrawing_Tab_Widget(mType);
	for (auto& svg : mSvgContents)
		cloned->Drawing_Callback(mType, svg.first, svg.second);

	return cloned;
}

void CDrawing_Tab_Widget::Drawing_Callback(const glucose::TDrawing_Image_Type type, const glucose::TDiagnosis diagnosis, const std::string &svg)
{
	if (type != mType)
		return;

	std::unique_lock<std::mutex> lck(mDrawMtx);

	mSvgContents[diagnosis] = svg;

	Redraw();
}

void CDrawing_Tab_Widget::Redraw()
{
	if (!mDefered_Work)
	{
		mDefered_Work = true;

		QEventLoop loop;
		Q_UNUSED(loop);
		QTimer::singleShot(0, this, [this]()
		{
			// if the requested diagnosis image is not found, fall back to "Not Specified" - it's the default
			glucose::TDiagnosis diag = glucose::TDiagnosis::NotSpecified;
			if (mSvgContents.find(mCurrent_Diagnosis) != mSvgContents.end())
				diag = mCurrent_Diagnosis;

			// lock scope
			{
				std::unique_lock<std::mutex> lck(mDrawMtx);

				mRenderer->load(QByteArray::fromStdString(mSvgContents[diag]));

				mDefered_Work = false;
			}

			if (mItem)
				delete mItem;

			mScene->clear();
			mView->viewport()->update();

			mItem = new QGraphicsSvgItem();
			mItem->setSharedRenderer(mRenderer);
			mItem->setFlags(QGraphicsItem::ItemClipsToShape);
			mItem->setCacheMode(QGraphicsItem::NoCache);
			mItem->setZValue(0);

			mScene->addItem(mItem);
			//mView->fitInView(mItem, Qt::AspectRatioMode::KeepAspectRatio);
		});
	}
}

void CDrawing_Tab_Widget::Show_Context_Menu(const QPoint& pos)
{
	QPoint globalPos = mapToGlobal(pos);

	QMenu myMenu;
	myMenu.addAction(dsSave_Image_To_File, [this]() {
		auto path = QFileDialog::getSaveFileName(this, tr(dsSave_Image_To_File), Default_Filename_For_Type[static_cast<size_t>(mType)], tr(dsSave_Image_Ext_Spec));
		if (path.length() != 0)
		{
			std::ofstream fs(path.toStdString());
			fs << mSvgContents[mCurrent_Diagnosis].c_str();
		}
	});
	myMenu.addAction(dsSave_Viewport_To_File, [this]() {
		auto path = QFileDialog::getSaveFileName(this, tr(dsSave_Viewport_To_File), dsDefault_Viewport_File_Name, tr(dsSave_Viewport_Ext_Spec));
		if (path.length() != 0)
		{
			QPixmap pixMap = QPixmap::grabWidget(mView->viewport());
			pixMap.save(path);
		}
	});
	myMenu.addSeparator();
	myMenu.addAction(dsReset_Zoom, [this]() {
		mView->resetMatrix();
	});

	myMenu.exec(globalPos);
}

void CDrawing_Tab_Widget::On_Diagnosis_Changed(const QString& /*item*/)
{
	int i = mDiagnosis_Box->currentIndex();
	if (i < 0)
		return;

	bool ok;
	int diagnosis = mDiagnosis_Box->itemData(i).toInt(&ok);
	if (!ok)
		return;

	mCurrent_Diagnosis = static_cast<glucose::TDiagnosis>(diagnosis);
	Redraw();
}
