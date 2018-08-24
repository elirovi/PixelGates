#include "mainwindow.h"
#include "ui_mainwindow.h"

const int MainWindow::_magic_number;
const int MainWindow::_version;
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow){

	ui->setupUi(this);

	QWidget::setWindowTitle("Emergence");
	QWidget::setWindowIcon(QIcon(":/emgc.ico"));

	ui->workspace->setScene(scene=new Workspace);
	ui->workspace->setAcceptDrops(true);
	ui->workspace->setDragMode(QGraphicsView::DragMode::RubberBandDrag);

	connect(ui->actionCopy,SIGNAL(triggered(bool)),scene,SLOT(copy()));
	connect(ui->actionPaste,SIGNAL(triggered(bool)),scene,SLOT(paste()));
	connect(ui->actionCut,SIGNAL(triggered(bool)),scene,SLOT(cut()));
	connect(ui->actionDelete,SIGNAL(triggered(bool)),scene,SLOT(delete_selected()));
	connect(ui->actionSelect_all,SIGNAL(triggered(bool)),scene,SLOT(select_all()));

	connect(ui->actionSave_as,SIGNAL(triggered(bool)),this,SLOT(save()));
	connect(ui->actionOpen,SIGNAL(triggered(bool)),this,SLOT(load()));

	connect(ui->actionUndo,SIGNAL(triggered(bool)),&scene->undoStack,SLOT(undo()));
	connect(ui->actionRedo,SIGNAL(triggered(bool)),&scene->undoStack,SLOT(redo()));
	ui->actionUndo->setEnabled(false); ui->actionRedo->setEnabled(false);
	connect(&scene->undoStack,SIGNAL(canUndoChanged(bool)),ui->actionUndo,SLOT(setEnabled(bool)));
	connect(&scene->undoStack,SIGNAL(canRedoChanged(bool)),ui->actionRedo,SLOT(setEnabled(bool)));

//	connect(scene,SIGNAL(selectionChanged()),this,SLOT(updateActions()));
//	updateActions();
//	ui->actionPaste->setEnabled(QApplication::clipboard()->mimeData()->text()=="Emergence_Nodes");
//	connect(ui->actionCopy,SIGNAL(triggered(bool)),ui->actionPaste,SLOT(setDisabled(bool)));
	zoomIN.setShortcut(QKeySequence::ZoomIn);
	zoomOUT.setShortcut(QKeySequence::ZoomOut);
	connect(&zoomIN,SIGNAL(triggered(bool)),this,SLOT(zoomIn()));
	connect(&zoomOUT,SIGNAL(triggered(bool)),this,SLOT(zoomOut()));
	addAction(&zoomIN);
	addAction(&zoomOUT);
	pluginManager= new PluginManager(ui);
	if(!pluginManager->loadPlugins()){
		ui->menuInsert->setEnabled(false);
		delete ui->toolboxDock;
	}
}

MainWindow::~MainWindow(){
	delete ui;
}
void MainWindow::zoomIn()const{
	ui->workspace->scale(Workspace::scaleFactor,Workspace::scaleFactor);
}
void MainWindow::zoomOut()const{
	ui->workspace->scale(1/Workspace::scaleFactor,1/Workspace::scaleFactor);
}
void MainWindow::save()const{
	QString fileName= QFileDialog::getSaveFileName(ui->workspace,"Save as...",".","Node Files (*.emrg)");
	if(fileName.isEmpty()) return;
	if(!fileName.endsWith(".emrg"))
		fileName.append(".emrg");
	std::ofstream ofs(fileName.toStdString());

	ofs.write(reinterpret_cast<const char*>(&_magic_number),4);
	ofs.write(reinterpret_cast<const char*>(&_version),4);

	pluginManager->save(ofs);

	ofs << *scene;
	ofs.close();
}

void MainWindow::load(){
	QString filename= QFileDialog::getOpenFileName(parentWidget(),"Open File",".","Node Files (*.emrg)");
	if(filename.isNull()) return;

	std::ifstream ifs(filename.toStdString());
	int tmp;
	ifs.read(reinterpret_cast<char*>(&tmp),4);
	if(tmp!=_magic_number){
		QMessageBox::warning(0,"Wrong format","Bad File Format");
		ifs.close();
		return;
	}
	ifs.read(reinterpret_cast<char*>(&tmp),4);
	if(tmp<_version){
		QMessageBox::warning(0,"Wrong version","Sorry, this save file is too old.");
		ifs.close();
		return;
	}
	pluginManager->load(ifs);

	scene->select_all();
	scene->undoStack.beginMacro("load");
	scene->delete_selected();
	ifs >> *scene;
	scene->undoStack.endMacro();
	scene->clearSelection();
	ifs.close();
}

void MainWindow::on_actionExit_triggered(){
	close();
}
//void MainWindow::updateActions(){
//	ui->actionExport->setEnabled(false);
//	bool someNodeIsSelected=false;
//	for(auto& n: scene->selectedItems()){
//		if(((Node*)n)->id!=Node::OUTPUT_G &&((Node*)n)->id!=Node::INPUT_G)
//			someNodeIsSelected=true;
//		if(((Node*)n)->id==Node::RENDER_G){
//			ui->actionExport->setEnabled(true);
//			break;
//		}
//	}
//	ui->actionCut->setEnabled(someNodeIsSelected);
//	ui->actionCopy->setEnabled(someNodeIsSelected);
//	ui->actionDelete->setEnabled(someNodeIsSelected);
//}
