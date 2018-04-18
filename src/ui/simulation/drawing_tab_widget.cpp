#include "drawing_tab_widget.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../../common/QtUtils.h"
#include "../../../../common/rtl/referencedImpl.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollBar>
#include <QtGui/QWheelEvent>

#include <fstream>

#include <QtCore/QTimer>
#include <QtCore/QEventLoop>

#include "moc_drawing_tab_widget.cpp"

constexpr qreal Minimum_Zoom = 1.1;
constexpr qreal Zoom_Step = 0.5;
constexpr qreal Maximum_Zoom = 10.0;

CDrawing_Tab_Widget::CDrawing_Tab_Widget(const TDrawing_Tab_Type type, QWidget *parent)
	: CAbstract_Simulation_Tab_Widget(parent), mType(type), mItem(nullptr)
{
	mView = new QGraphicsView();
	mScene = new QGraphicsScene(mView);
	mView->setScene(mScene);

	mRenderer = new QSvgRenderer();

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(mView);
	setLayout(mainLayout);
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

void CDrawing_Tab_Widget::Drawing_Callback(const TDrawing_Tab_Type type, const wchar_t* svg, bool type2)
{
	if (type != mType)
		return;

	std::unique_lock<std::mutex> lck(mDrawMtx);

	mSvgContents = std::string{ svg, svg + wcslen(svg) };

	QEventLoop loop;
	Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this]()
	{
		// lock scope
		{
			std::unique_lock<std::mutex> lck(mDrawMtx);

			mRenderer->load(QByteArray(mSvgContents.c_str()));
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
