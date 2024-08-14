/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Copyright (c) since 2018 University of West Bohemia.
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Univerzitni 8, 301 00 Pilsen
 * Czech Republic
 * 
 * 
 * Purpose of this software:
 * This software is intended to demonstrate work of the diabetes.zcu.cz research
 * group to other scientists, to complement our published papers. It is strictly
 * prohibited to use this software for diagnosis or treatment of any medical condition,
 * without obtaining all required approvals from respective regulatory bodies.
 *
 * Especially, a diabetic patient is warned that unauthorized use of this software
 * may result into severe injure, including death.
 *
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under these license terms is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) This file is available under the Apache License, Version 2.0.
 * b) When publishing any derivative work or results obtained using this software, you agree to cite the following paper:
 *    Tomas Koutny and Martin Ubl, "SmartCGMS as a Testbed for a Blood-Glucose Level Prediction and/or 
 *    Control Challenge with (an FDA-Accepted) Diabetic Patient Simulation", Procedia Computer Science,  
 *    Volume 177, pp. 354-362, 2020
 */

#pragma once

#include <QtWidgets/QListWidget>
#include <QtCore/QFileSystemWatcher>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QComboBox>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtSvg/QGraphicsSvgItem>
#else
#include <QtSvgWidgets/QGraphicsSvgItem>
#endif
#include <QtSvg/QSvgRenderer>

#include <scgms/iface/FilterIface.h>
#include "abstract_simulation_tab.h"

#include <mutex>
#include <map>

/*
 * Custom child of QGraphicsView to achieve zoom functionality
 */
class CDrawing_Graphics_View : public QGraphicsView {
	Q_OBJECT

	protected slots:
		void scalingTime(qreal x);
		void animFinished();

	protected:
		int mNumScheduledScalings = 0;

	public:
		CDrawing_Graphics_View();

		void wheelEvent(QWheelEvent * event);
};

/*
 * Drawing widget class
 */
class CDrawing_Tab_Widget : public CAbstract_Simulation_Tab_Widget {
	Q_OBJECT

	protected:
		// maintained output type
		const scgms::TDrawing_Image_Type mType;

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
		std::map<scgms::TDiagnosis, std::string> mSvgContents;
		// draw mutex
		std::mutex mDrawMtx;

		// is there something to be drawn, but timer didn't hit yet?
		bool mDefered_Work = false;

		// currently selected diagnosis
		scgms::TDiagnosis mCurrent_Diagnosis;

	signals:
		void On_Redraw();

	protected slots:
		void Show_Context_Menu(const QPoint& pos);
		void On_Diagnosis_Changed(const QString& /*item*/);

		void Slot_Redraw();

	public:
		explicit CDrawing_Tab_Widget(const scgms::TDrawing_Image_Type type, QWidget *parent = 0);
		virtual ~CDrawing_Tab_Widget();

		virtual void Update_View_Size() override;
		virtual CAbstract_Simulation_Tab_Widget* Clone() override;

		// when a new drawing is available
		void Drawing_Callback(const scgms::TDrawing_Image_Type type, const scgms::TDiagnosis diagnosis, const std::string &svg);

		void Redraw();
};
