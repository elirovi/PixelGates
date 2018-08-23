#include "LUTPlugin.h"

void LUTPlugin::updateUI(Ui::MainWindow* ui) const{
	QAction* lutAction= new QAction("LUT");

	Workspace* ws=(Workspace*)ui->workspace->scene();
	connect(lutAction,&QAction::triggered,ws,[=]{ws->addNode(Node::nodeMalloc("lut"));});

	ui->menuInsert->addAction(lutAction);

	ui->toolBox->addTool("lut","LUT","Color");
}

void LUTPlugin::init()const{
	Node::makeNodeMethods["lut"] = &LUTNode::makeNode;
}
