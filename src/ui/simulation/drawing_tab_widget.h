#pragma once

#include <QtWidgets/QListWidget>
#include <QtCore/QFileSystemWatcher>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QComboBox>
#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>

#include "../../../../common/desktop-console/filter_chain.h"
#include "../../../../common/iface/FilterIface.h"
#include "abstract_simulation_tab.h"

#include <mutex>
#include <map>

/*
 * Custom child of QGraphicsView to achieve zoom functionality
 */
class CDrawing_Graphics_View : public QGraphicsView
{
		Q_OBJECT

	protected slots:
		void scalingTime(qreal x);
		void animFinished();

	protected:
		int mNumScheduledScalings;

	public:
		CDrawing_Graphics_View();

		void wheelEvent(QWheelEvent * event);
};

/*
 * Drawing widget class
 */
class CDrawing_Tab_Widget : public CAbstract_Simulation_Tab_Widget
{
		Q_OBJECT

	protected:
		// maintained output type
		const glucose::TDrawing_Image_Type mType;

		// drawn item
		QGraphicsSvgItem* mItem;
		// SVG renderer
		QSvgRenderer* mRenderer;
		// main graphics view ("canvas")
		CDrawing_Graphics_View* mView;
		// main scene
		QGraphicsScene* mScene;
		// diagnosis combo box
		QComboBox* mDiagnosis_Box;

		// contents of SVG to be drawn
		std::map<glucose::TDiagnosis, std::string> mSvgContents;
		// draw mutex
		std::mutex mDrawMtx;

		// is there something to be drawn, but timer didn't hit yet?
		bool mDefered_Work = false;

		// currently selected diagnosis
		glucose::TDiagnosis mCurrent_Diagnosis;

	protected slots:
		void Show_Context_Menu(const QPoint& pos);
		void On_Diagnosis_Changed(const QString& /*item*/);

	public:
		explicit CDrawing_Tab_Widget(const glucose::TDrawing_Image_Type type, QWidget *parent = 0);
		virtual ~CDrawing_Tab_Widget();

		virtual void Update_View_Size() override;

		// when a new drawing is available
		void Drawing_Callback(const glucose::TDrawing_Image_Type type, const glucose::TDiagnosis diagnosis, const std::string &svg);

		void Redraw();
};
