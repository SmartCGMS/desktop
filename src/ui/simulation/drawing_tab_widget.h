#pragma once

#include <QtWidgets/QListWidget>
#include <QtCore/QFileSystemWatcher>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>

#include "../../../../common/desktop-console/filter_chain.h"
#include "../../../../common/iface/FilterIface.h"
#include "abstract_simulation_tab.h"

#include <mutex>

/*
 * Types of drawing widgets
 */
enum class TDrawing_Tab_Type
{
	Graph,
	Day,
	Parkes,
	Clark,
	Agp
};

/*
 * Drawing widget class
 */
class CDrawing_Tab_Widget : public CAbstract_Simulation_Tab_Widget
{
		Q_OBJECT

	protected:
		// maintained output type
		const TDrawing_Tab_Type mType;

		// drawn item
		QGraphicsSvgItem* mItem;
		// SVG renderer
		QSvgRenderer* mRenderer;
		// main graphics view ("canvas")
		QGraphicsView* mView;
		// main scene
		QGraphicsScene* mScene;

		// contents of SVG to be drawn
		std::string mSvgContents;
		// draw mutex
		std::mutex mDrawMtx;

		// current zoom applied
		qreal mCurrZoom = 1.0;

	public:
		explicit CDrawing_Tab_Widget(const TDrawing_Tab_Type type, QWidget *parent = 0);
		virtual ~CDrawing_Tab_Widget();

		virtual void Update_View_Size() override;

		void Do_Zoom(bool in);

		// when a new drawing is available
		void Drawing_Callback(const TDrawing_Tab_Type type, const wchar_t* svg, bool type2);
};
