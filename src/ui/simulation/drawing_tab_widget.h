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
