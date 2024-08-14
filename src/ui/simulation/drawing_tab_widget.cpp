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

#include "drawing_tab_widget.h"

#include <scgms/lang/dstrings.h>
#include <scgms/utils/QtUtils.h>
#include <scgms/rtl/referencedImpl.h>

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
static const std::array<const char*, static_cast<size_t>(scgms::TDrawing_Image_Type::count)> Default_Filename_For_Type = { {
	dsSave_Image_Default_Filename_Graph,
	dsSave_Image_Default_Filename_Day,
	dsSave_Image_Default_Filename_Parkes,
	dsSave_Image_Default_Filename_Clark,
	dsSave_Image_Default_Filename_AGP,
	dsSave_Image_Default_Filename_ECDF,
	dsSave_Image_Default_Filename_Profile_Glucose,
	dsSave_Image_Default_Filename_Profile_Carbs,
	dsSave_Image_Default_Filename_Profile_Insulin
} };

CDrawing_Graphics_View::CDrawing_Graphics_View()
	: QGraphicsView() {
	//
}

void CDrawing_Graphics_View::wheelEvent(QWheelEvent * event) {
	const auto delta = event->angleDelta();
	const int numSteps = (delta.x() + delta.y()) / 120;

	mNumScheduledScalings += numSteps;
	if (mNumScheduledScalings * numSteps < 0) {
		mNumScheduledScalings = numSteps;
	}

	QTimeLine *anim = new QTimeLine(350, this);
	anim->setUpdateInterval(20);

	connect(anim, SIGNAL(valueChanged(qreal)), SLOT(scalingTime(qreal)));
	connect(anim, SIGNAL(finished()), SLOT(animFinished()));
	anim->start();
}

void CDrawing_Graphics_View::scalingTime(qreal x) {
	qreal factor = 1.0 + qreal(mNumScheduledScalings) / 300.0;
	scale(factor, factor);
}

void CDrawing_Graphics_View::animFinished() {
	if (mNumScheduledScalings < 0) {
		mNumScheduledScalings++;
	}
	sender()->~QObject();
}

CDrawing_Tab_Widget::CDrawing_Tab_Widget(const scgms::TDrawing_Image_Type type, QWidget *parent)
	: CAbstract_Simulation_Tab_Widget(parent), mType(type), mItem(nullptr), mDiagnosis_Box(nullptr), mCurrent_Diagnosis(scgms::TDiagnosis::Type1) {

	mView = new CDrawing_Graphics_View();
	mScene = new QGraphicsScene(mView);
	mView->setScene(mScene);

	mView->setDragMode(QGraphicsView::ScrollHandDrag);
	mView->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	mRenderer = new QSvgRenderer();

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(mView, 0, 0);
	setLayout(mainLayout);

	// just parkes' grid has to disambiguate between diagnosis types (for now)
	if (type == scgms::TDrawing_Image_Type::Parkes) {
		mDiagnosis_Box = new QComboBox(this);
		mDiagnosis_Box->addItem(dsDiagnosis_T1D, static_cast<int>(scgms::TDiagnosis::Type1));
		mDiagnosis_Box->addItem(dsDiagnosis_T2D, static_cast<int>(scgms::TDiagnosis::Type2));
		mDiagnosis_Box->addItem(dsDiagnosis_Gestational, static_cast<int>(scgms::TDiagnosis::Gestational));
		// this should relocate the widget on top of the drawing to top left corner somewhere
		auto geom = mDiagnosis_Box->geometry();
		mDiagnosis_Box->setGeometry(20, 20, geom.width(), geom.height());

		connect(mDiagnosis_Box, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(On_Diagnosis_Changed(const QString&)));
	}

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(Show_Context_Menu(const QPoint&)));

	connect(this, SIGNAL(On_Redraw()), this, SLOT(Slot_Redraw()), Qt::QueuedConnection);
}

CDrawing_Tab_Widget::~CDrawing_Tab_Widget() {
	if (mItem) {
		delete mItem;
	}

	mScene->clear();
	delete mRenderer;
}

void CDrawing_Tab_Widget::Update_View_Size() {
	mView->resetTransform();
}

CAbstract_Simulation_Tab_Widget* CDrawing_Tab_Widget::Clone() {
	CDrawing_Tab_Widget* cloned = new CDrawing_Tab_Widget(mType);
	for (auto& svg : mSvgContents) {
		cloned->Drawing_Callback(mType, svg.first, svg.second);
	}

	return cloned;
}

void CDrawing_Tab_Widget::Drawing_Callback(const scgms::TDrawing_Image_Type type, const scgms::TDiagnosis diagnosis, const std::string &svg) {
	if (type != mType) {
		return;
	}

	std::unique_lock<std::mutex> lck(mDrawMtx);

	mSvgContents[diagnosis] = svg;

	Redraw();
}

void CDrawing_Tab_Widget::Redraw() {
	if (!mDefered_Work) {
		mDefered_Work = true;
		emit On_Redraw();
	}
}

void CDrawing_Tab_Widget::Slot_Redraw() {
	// if the requested diagnosis image is not found, fall back to "Not Specified" - it's the default
	scgms::TDiagnosis diag = scgms::TDiagnosis::NotSpecified;
	if (mSvgContents.find(mCurrent_Diagnosis) != mSvgContents.end()) {
		diag = mCurrent_Diagnosis;
	}

	// lock scope
	{
		std::unique_lock<std::mutex> lck(mDrawMtx);

		mRenderer->load(QByteArray::fromStdString(mSvgContents[diag]));

		mDefered_Work = false;
	}

	if (mItem) {
		delete mItem;
	}

	mScene->clear();
	mView->viewport()->update();

	mItem = new QGraphicsSvgItem();
	mItem->setSharedRenderer(mRenderer);
	mItem->setFlags(QGraphicsItem::ItemClipsToShape);
	mItem->setCacheMode(QGraphicsItem::NoCache);
	mItem->setZValue(0);

	mScene->addItem(mItem);
	//mView->fitInView(mItem, Qt::AspectRatioMode::KeepAspectRatio);
}

void CDrawing_Tab_Widget::Show_Context_Menu(const QPoint& pos) {
	QPoint globalPos = mapToGlobal(pos);

	QMenu myMenu;
	myMenu.addAction(dsSave_Image_To_File, [this]() {
		auto path = QFileDialog::getSaveFileName(this, tr(dsSave_Image_To_File), Default_Filename_For_Type[static_cast<size_t>(mType)], tr(dsSave_Image_Ext_Spec));
		if (path.length() != 0) {
			std::ofstream fs(path.toStdString());
			fs << mSvgContents[mCurrent_Diagnosis].c_str();
		}
	});

	myMenu.addAction(dsSave_Viewport_To_File, [this]() {
		auto path = QFileDialog::getSaveFileName(this, tr(dsSave_Viewport_To_File), dsDefault_Viewport_File_Name, tr(dsSave_Viewport_Ext_Spec));
		if (path.length() != 0) {
			//QPixmap pixMap = QPixmap::grabWidget(mView->viewport());
			QPixmap pixMap = mView->grab();
			pixMap.save(path);
		}
	});

	myMenu.addSeparator();

	myMenu.addAction(dsReset_Zoom, [this]() {
		mView->resetTransform();
	});

	myMenu.exec(globalPos);
}

void CDrawing_Tab_Widget::On_Diagnosis_Changed(const QString& /*item*/) {
	int i = mDiagnosis_Box->currentIndex();
	if (i < 0) {
		return;
	}

	bool ok;
	int diagnosis = mDiagnosis_Box->itemData(i).toInt(&ok);
	if (!ok) {
		return;
	}

	mCurrent_Diagnosis = static_cast<scgms::TDiagnosis>(diagnosis);
	Redraw();
}
