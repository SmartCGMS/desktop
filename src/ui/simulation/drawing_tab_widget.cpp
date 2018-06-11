#include "drawing_tab_widget.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../../common/QtUtils.h"
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
#include <QtWidgets/QFileDialog>

#include "moc_drawing_tab_widget.cpp"

constexpr qreal Minimum_Zoom = 1.1;
constexpr qreal Zoom_Step = 0.5;
constexpr qreal Maximum_Zoom = 10.0;

// array of default names for image files by type
static const std::array<const char*, static_cast<size_t>(glucose::TDrawing_Image_Type::count)> Default_Filename_For_Type = {
	dsSave_Image_Default_Filename_Graph,
	dsSave_Image_Default_Filename_Day,
	dsSave_Image_Default_Filename_Parkes,
	dsSave_Image_Default_Filename_Clark,
	dsSave_Image_Default_Filename_AGP,
	dsSave_Image_Default_Filename_ECDF
};

CDrawing_Tab_Widget::CDrawing_Tab_Widget(const glucose::TDrawing_Image_Type type, QWidget *parent)
	: CAbstract_Simulation_Tab_Widget(parent), mType(type), mItem(nullptr)
{
	mView = new QGraphicsView();
	mScene = new QGraphicsScene(mView);
	mView->setScene(mScene);

	mRenderer = new QSvgRenderer();

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(mView);
	setLayout(mainLayout);

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
	if (mItem)
		mView->fitInView(mItem, Qt::AspectRatioMode::KeepAspectRatio);
}

void CDrawing_Tab_Widget::Drawing_Callback(const glucose::TDrawing_Image_Type type, const glucose::TDiagnosis diagnosis, const std::string &svg)
{
	if (type != mType)
		return;

	std::unique_lock<std::mutex> lck(mDrawMtx);

	mSvgContents = svg;

	if (!mDefered_Work)
	{
		mDefered_Work = true;

		QEventLoop loop;
		Q_UNUSED(loop);
		QTimer::singleShot(0, this, [this]()
		{
			// lock scope
			{
				std::unique_lock<std::mutex> lck(mDrawMtx);

				mRenderer->load(QByteArray::fromStdString(mSvgContents));

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
			mView->fitInView(mItem, Qt::AspectRatioMode::KeepAspectRatio);
		});
	}
}

void CDrawing_Tab_Widget::Do_Zoom(bool in)
{
	if (in)
		mCurrZoom += Zoom_Step;
	else
		mCurrZoom -= Zoom_Step;

	if (mCurrZoom < Minimum_Zoom)
	{
		mCurrZoom = 1.0;
		mItem->setTransform(QTransform::fromScale(mCurrZoom, mCurrZoom));
		mView->fitInView(mItem, Qt::AspectRatioMode::KeepAspectRatio);

		return;
	}
	else if (mCurrZoom > Maximum_Zoom)
		mCurrZoom = Maximum_Zoom;

	mItem->setTransform(QTransform::fromScale(mCurrZoom, mCurrZoom));
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
			fs.write(mSvgContents.c_str(), mSvgContents.size());
		}
	});

	myMenu.exec(globalPos);
}
