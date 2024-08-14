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

#include "drawing_v2_tab_widget.h"

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

#include "moc_drawing_v2_tab_widget.cpp"

CDrawing_v2_Graphics_View::CDrawing_v2_Graphics_View()
	: QGraphicsView() {
	//
}

void CDrawing_v2_Graphics_View::wheelEvent(QWheelEvent * event) {
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

void CDrawing_v2_Graphics_View::scalingTime(qreal x) {
	qreal factor = 1.0 + qreal(mNumScheduledScalings) / 300.0;
	scale(factor, factor);
}

void CDrawing_v2_Graphics_View::animFinished() {
	if (mNumScheduledScalings < 0) {
		mNumScheduledScalings++;
	}
	sender()->~QObject();
}

CDrawing_v2_Tab_Widget::CDrawing_v2_Tab_Widget(QWidget *parent)
	: CAbstract_Simulation_Tab_Widget(parent), mItem(nullptr) {

	mView = new CDrawing_v2_Graphics_View();
	mScene = new QGraphicsScene(mView);
	mView->setScene(mScene);

	mView->setDragMode(QGraphicsView::ScrollHandDrag);
	mView->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	mRenderer = new QSvgRenderer();

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(mView, 0, 0);
	setLayout(mainLayout);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(Show_Context_Menu(const QPoint&)));

	connect(this, SIGNAL(On_Redraw()), this, SLOT(Slot_Redraw()), Qt::QueuedConnection);
}

CDrawing_v2_Tab_Widget::~CDrawing_v2_Tab_Widget() {
	if (mItem) {
		delete mItem;
	}

	mScene->clear();
	delete mRenderer;
}

void CDrawing_v2_Tab_Widget::Update_View_Size() {
	mView->resetTransform();
}

CAbstract_Simulation_Tab_Widget* CDrawing_v2_Tab_Widget::Clone() {
	CDrawing_v2_Tab_Widget* cloned = new CDrawing_v2_Tab_Widget();
	cloned->Drawing_Callback(mSvgContents);

	return cloned;
}

void CDrawing_v2_Tab_Widget::Drawing_Callback(const std::string &svg) {
	std::unique_lock<std::mutex> lck(mDrawMtx);

	mSvgContents = svg;

	Redraw();
}

void CDrawing_v2_Tab_Widget::Redraw() {
	if (!mDefered_Work) {
		mDefered_Work = true;
		emit On_Redraw();
	}
}

void CDrawing_v2_Tab_Widget::Slot_Redraw() {
	// lock scope
	{
		std::unique_lock<std::mutex> lck(mDrawMtx);

		mRenderer->load(QByteArray::fromStdString(mSvgContents));

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

void CDrawing_v2_Tab_Widget::Show_Context_Menu(const QPoint& pos) {
	QPoint globalPos = mapToGlobal(pos);

	QMenu myMenu;
	myMenu.addAction(dsSave_Image_To_File, [this]() {
		auto path = QFileDialog::getSaveFileName(this, tr(dsSave_Image_To_File), "image", tr(dsSave_Image_Ext_Spec));
		if (path.length() != 0) {
			std::ofstream fs(path.toStdString());
			fs << mSvgContents.c_str();
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

void CDrawing_v2_Tab_Widget::Get_Canvas_Dimensions(int& _width, int& _height) {
	_width = width();
	_height = height();
}
