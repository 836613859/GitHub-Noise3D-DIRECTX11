#include "stdafx.h"
#include "QtSHLightingUtApp.h"
#include <QMessageBox>

QtSHLightingUtApp::QtSHLightingUtApp(QWidget *parent)
	: QMainWindow(parent)
{
	mUI.setupUi(this);

	//"ע��"signal&slot�������� setupUi ����֮��
	connect(mUI.btn_SelectSphericalMap, &QPushButton::clicked, this, &QtSHLightingUtApp::Slot_LoadSphericalTexture);

	mMain3dApp.InitNoise3D(mUI.renderCanvas->winId);
}


void QtSHLightingUtApp::Slot_LoadSphericalTexture()
{
	QMessageBox::information(this, tr("jigehahaah"), tr("jigeha66666"));
}

void QtSHLightingUtApp::Slot_LoadCubeMap()
{
}
