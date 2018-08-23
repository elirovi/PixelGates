#include "Node.h"

Node::Socket::Socket(unsigned i, double y, Node *parent):QGraphicsObject(parent)
	,line(headSize,0,headSize,0),rank(i),iy(y),parent(parent){
	setZValue(parent->zValue()+1);
	setPos(-headSize-1,y);
	setAcceptHoverEvents(true);
	line.setParentItem(this);
	connect(this,&Node::Socket::xChanged,this,&Node::Socket::updateLine);
	connect(this,&Node::Socket::yChanged,this,&Node::Socket::updateLine);
}

Node::Socket::~Socket(){
	disconnect(this,&Node::Socket::yChanged,this,&Node::Socket::updateLine);
	disconnect(this,&Node::Socket::xChanged,this,&Node::Socket::updateLine);
}

QRectF Node::Socket::boundingRect()const{
	return QRectF(-headSize,-headSize,headSize*2+1,headSize*2+1);
}

void Node::Socket::updateLine(){
	line.setLine(QLine({headSize,0},(parentItem()->pos()-scenePos()-QPointF(0,-iy)).toPoint()));
}

void Node::Socket::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
	if(visible){
		painter->setPen(pen);
		painter->setRenderHint(QPainter::Antialiasing);
		painter->drawEllipse(boundingRect().center(),headSize,headSize);
	}
}

void Node::Socket::hoverEnterEvent(QGraphicsSceneHoverEvent *){
	pen.setWidth(2);
	update();
}

void Node::Socket::hoverLeaveEvent(QGraphicsSceneHoverEvent *){
	pen.setWidth(1);
	update();
}

Node* Node::Socket::collidesWithNode() const{
	for(auto& i:collidingItems())
		if(i->data(0)=="node")
			return (Node*)i;
	return nullptr;
}

void Node::Socket::mouseMoveEvent(QGraphicsSceneMouseEvent*event){
	Node*i=collidesWithNode();
	if(i&&!i->isLooping(parent)&&i->contains(event->scenePos()-i->scenePos())){
		hover=i;
		for(const auto& ii: i->collidingItems())
			if(i->zValue()<= ii->zValue())
				i->setZValue(ii->zValue()+1);
		parent->setZValue(i->zValue());
		setZValue(i->zValue());
		setPos(i->pos()+QPoint(i->boundingRect().width()+Socket::headSize-1,i->height/2)-parent->pos());
	}else{
		hover=nullptr;
		setPos(event->scenePos()- boundingRect().center()-parent->pos());
	}
}

void Node::Socket::connectToNode(Node* n){
	if(n==parent) return;
	if(parent->nbArgs>=rank+1 && !parent->iNodes[rank]){
		QRectF r= n->boundingRect();
		setPos(n->pos()+QPoint(r.width()-Socket::headSize,r.height()/2)-parent->pos());
		parent->iNodes[rank]=n;
		n->oConnections.push_back({parent,rank});
		connect(n,&Node::xChanged,this,&Node::Socket::updateLine);
		connect(n,&Node::yChanged,this,&Node::Socket::updateLine);
		setEnabled(false);
		visible=false;
		parent->updateConstant();
		emit sm.updateOutputs();
	}
}

void Node::Socket::disconnectNode(){
	if(parent->iNodes[rank]){
		for(auto l=parent->iNodes[rank]->oConnections.begin();
			l!=parent->iNodes[rank]->oConnections.end();++l)
			if(l->first==parent && l->second==rank){
				parent->iNodes[rank]->oConnections.erase(l);
				break;
			}
		disconnect(parent->iNodes[rank],&Node::xChanged,this,&Node::Socket::updateLine);
		disconnect(parent->iNodes[rank],&Node::yChanged,this,&Node::Socket::updateLine);
		parent->iNodes[rank]=nullptr;
		reset();
		parent->updateConstant();
		emit sm.updateOutputs();
	}
}

void Node::Socket::mousePressEvent(QGraphicsSceneMouseEvent*){}

void Node::Socket::mouseReleaseEvent(QGraphicsSceneMouseEvent*){
	if(hover) emit parent->connected(this,hover);
	else reset();
}

void Node::Socket::reset(){
	pen.setWidth(1);
	visible=true;
	setEnabled(true);
	setPos(-headSize-1,iy);
	line.setLine(headSize,0,headSize,0);
}

Node* Node::Socket::hover;
SignalManager Node::sm;

ulong Node::pixelID;
uint Node::widthByHeight;

Node::Node(const std::string &type, unsigned w, unsigned h, QColor c, uint n, bool spec):
	width(w),height(h),_type(type),special(spec),color(c),pen(QPen(Qt::black,1)),nbArgs(n){
	setCursor(Qt::OpenHandCursor);
	if(!spec)setData(0,"node");
	setData(1,"countMe");
	for(uint i=0; i<nbArgs;i++){
		sockets.push_back(new Socket(i,height*((i+1.0)/(nbArgs+1.0)),this));
		iNodes.push_back(nullptr);
	}
	setFlags(GraphicsItemFlag::ItemIsMovable| GraphicsItemFlag::ItemIsSelectable);
	connect(this,SIGNAL(xChanged()),this,SLOT(updateLines()));
	connect(this,SIGNAL(yChanged()),this,SLOT(updateLines()));
	actionDelete=new QAction("Delete");
}

Node::~Node(){
	delete actionDelete;
	disconnect(this,SIGNAL(yChanged()),this,SLOT(updateLines()));
	disconnect(this,SIGNAL(xChanged()),this,SLOT(updateLines()));
	qDeleteAll(sockets);
}

QRectF Node::boundingRect()const{
	if(special) return QRectF(0,0,width+socketSize,height);
	return QRectF(0,0,width+2*socketSize,height);
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem*,QWidget*){
	QRectF rect=boundingRect();
	if(!special)rect.setRight(rect.right()-socketSize);
	rect.setLeft(rect.left()+socketSize);
	painter->setRenderHint(QPainter::Antialiasing);
	QPainterPath path;
	path.addRoundedRect(rect, 10, 10);
	pen.setWidth(isSelected()?2:1);
	painter->setPen(pen);
	painter->fillPath(path, color);
	painter->drawPath(path);
	for(uint i=1; i<=nbArgs; i++)//draw input sockets
		painter->drawLine(rect.topLeft()+QPointF(0,i*height/(nbArgs+1.0)),rect.topLeft()+QPointF(-socketSize,i*height/(nbArgs+1.0)));
	if(!special)//draw output socket
		painter->drawLine(rect.center()+QPointF(rect.width()/2,0),rect.center()+QPointF(rect.width()/2+socketSize,0));
}

void Node::updateLines()const{
	for(int i = 0; i<iNodes.size();i++)
		if(iNodes[i]){
			QRectF r= iNodes[i]->boundingRect();
			sockets[i]->setPos(iNodes[i]->pos()-pos()+QPointF(r.width()-Socket::headSize,r.height()/2));
		}
	for(auto l=oConnections.begin(); l!=oConnections.end();++l)
		l->first->sockets[l->second]->setPos(pos()-l->first->pos()+QPointF(boundingRect().width()-Socket::headSize,height/2));
}

QPointF Node::tmpPos;

void Node::mousePressEvent(QGraphicsSceneMouseEvent* event){
	if(event->button()==Qt::LeftButton){
		setCursor(Qt::ClosedHandCursor);
		setZValue(INT32_MAX);
	}else
		setCursor(Qt::ArrowCursor);
	tmpPos=pos();
	QGraphicsItem::mousePressEvent(event);
}
void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent*event){
	scene()->setSceneRect(scene()->itemsBoundingRect());
	setCursor(Qt::OpenHandCursor);
	setZValue(0);
	for(const auto& i: collidingItems())
		if(zValue()<= i->zValue())
			setZValue(i->zValue()+1);
	if(pos()-tmpPos!=QPointF(0,0))
		emit moved();
	QGraphicsItem::mouseReleaseEvent(event);
}

void Node::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){
	if(!menu) menu= new QMenu;
	for(unsigned i = 0; i<nbArgs; i++){
		QAction *a= menu->addAction(QString("Disconnect ")+QString::number(i+1));
		a->setEnabled(iNodes[i]!=nullptr);
		connect(a,&QAction::triggered,this,[=](){emit disconnected(sockets[i]);});
	}
	menu->addSeparator();
	menu->addAction(actionDelete);
	menu->exec(event->screenPos());
	delete menu;
	menu=nullptr;
}

Node::operator bool()const{
	for(auto g:iNodes)
		if(g==nullptr || !(*g))
			return false;
	return true;
}

void Node::drawIcon(QPainter *painter, QString filename){
	QImage icon=QImage(filename);
	painter->drawImage(width/2-icon.width()/2+socketSize,height/2-icon.height()/2,icon);
}

void Node::updateVal(){
	if(constant){
		cache=kernel();
		for(const auto& i : oConnections)
			i.first->updateVal();
	}
}

bool Node::isLooping(Node* n)const{
	if(this==n) return true;
	for(const auto& node:iNodes)
		if(node && node->isLooping(n))
				return true;
	return false;
}
void Node::updateConstant(){
	constant=true;
	for(const auto& node: iNodes)
		if(!node || !node->constant)
			constant=false;
	if(constant)
		cache=kernel();
	for(const auto& i : oConnections)
		i.first->updateConstant();
}

QList<const QString> Node::knownTypes;
std::map<const QString, Node*(*)(void*)> Node::makeNodeMethods;

Node* Node::nodeMalloc(const QString& type, void* arg){
	if(makeNodeMethods.find(type)!= makeNodeMethods.end())
		return makeNodeMethods[type](arg);
	return nullptr;
//	case Node::FUNC_G:{
////		Function *f;
//		if(arg) return new FunctionNode((Function*)arg);
//		return new FunctionNode;
////		else{
////			f=FunctionManager::getFunction();
////			if(!f)return nullptr;
////		}
//	}
//	case Node::OUTPUT_G:	return new Function::OutputNode;
//	case Node::INPUT_G:		return new Function::InputNode(*(uint*)arg);
}

data_t Node::eval(){
	if(constant) return cache;
	if(pixelID==lastPixelID) return cache;
	lastPixelID=pixelID;
	return cache=kernel();
}

void Node::toBin(std::ostream&)const{}
void Node::fromBin(std::istream &, void*)const{}

std::ostream& operator<<(std::ostream& out, const Node&){
//	out << Node::knownTypes.indexOf(n._type);
//	float tmp=n.scenePos().x();
//	out.write(reinterpret_cast<char*>(&tmp),4);
//	tmp=n.scenePos().y();
//	out.write(reinterpret_cast<char*>(&tmp),4);

//	n.toBin(out);
//	case Node::DOUBLE_G:
//		out << n.cache.d << '\n';
//		break;
//	case Node::COLOR_G:
//		out.write(reinterpret_cast<const char*>(&n.cache.clr),sizeof(data_t::color));
//		break;
//	case Node::FUNC_G:
//		out << FunctionManager::indexOf(((FunctionNode*)&n)->func) << '\n';
//		break;
//	case Node::INPUT_G:
//		out << ((Function::InputNode*)&n)->_rank<< '\n';
//		break;
//	case Node::BITMAP_G:
//		out << ((BitmapNode*)&n)->path << "\n";
//		break;
	return out;
}
std::ostream& operator<<(std::ostream& out,const QList<Node*>&nodes){
	out << nodes.size()<< '\n';
	for(const auto& n:nodes)
		out << *n;
	for(const auto& n:nodes)
		for(const auto& nn:n->iNodes)
			out << nodes.indexOf(nn) << '\n';
	return out;
}

std::istream& operator>>(std::istream& in, QList<Node*>&nodes){
	int tmp;
	in>>tmp;
	for(int i=0;i<tmp;i++){
		Node* n;
		int id;
		in>>id;
		float xx,yy;
		in.read(reinterpret_cast<char*>(&xx),4);
		in.read(reinterpret_cast<char*>(&yy),4);
//		case Node::DOUBLE_G:
//			in >> tmpD;
//			n=Node::nodeMalloc((Node::Type)id,&tmpD);
//			break;
//		case Node::COLOR_G:
//			in.read(reinterpret_cast<char*>(&tmpC),sizeof(data_t::color));
//			n=Node::nodeMalloc((Node::Type)id,&tmpC);
//			break;
//		case Node::FUNC_G:
//			in >> tmpI;
//			n=Node::nodeMalloc((Node::Type)id,FunctionManager::functionAt(tmpI));
//			break;
//		case Node::INPUT_G:
//			in >> tmpI;
//			n=Node::nodeMalloc((Node::Type)id,&tmpI);
//			break;
//		case Node::BITMAP_G:
//			in >> tmpS;
//			n=Node::nodeMalloc((Node::Type)id,&tmpS);
//			break;
		void* arg=0;
		n->fromBin(in,arg);
		n=Node::nodeMalloc(Node::knownTypes.at(id),arg);
		n->setPos(xx,yy);
		nodes.push_back(n);
	}
	for(const auto& n:nodes)
		for(uint i=0; i<n->nbArgs; i++){
			int tmp;
			in>>tmp;
			if(tmp>=0)
				n->sockets[i]->connectToNode(nodes.at(tmp));
		}
	return in;
}
