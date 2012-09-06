/*
 * GatingHierarchy.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: wjiang2
 */

#include "include/GatingHierarchy.hpp"
#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/topological_sort.hpp>
#include <fstream>
#include <algorithm>


/*need to be careful that gate within each node of the GatingHierarchy is
 * dynamically allocated,even it GatingHierarchy gets copied before destroyed
 * these gates are already gone since the gate objects were already freed by
 * this destructor
 */


GatingHierarchy::~GatingHierarchy()
{
	//for each node

//	cout<<"entring the destructor of GatingHierarchy"<<endl;

	VertexID_vec vertices=getVertices(false);
	if(dMode>=GATING_HIERARCHY_LEVEL)
		cout<<"free the node properties from tree"<<endl;
	for (VertexID_vec::iterator it=vertices.begin();it!=vertices.end();it++)
	{
		nodeProperties * np=tree[*it];
		if(np==NULL)
			throw(domain_error("empty node properties!"));
		if(dMode>=POPULATION_LEVEL)
			cout<<"free "<<np->getName()<<endl;
		delete np;//free the property bundle
//			cout<<"free the indices"<<endl;
	}

}

//default constructor without argument
GatingHierarchy::GatingHierarchy()
{
	dMode=1;
	isLoaded=false;
	thisWs=NULL;
	nc=NULL;
	gTrans=NULL;
}
//constructor for sampleNode argument
//GatingHierarchy::GatingHierarchy(string sampleID,workspace * ws)
//{
//	thisWs=ws;
//
//	wsSampleNode curSampleNode=thisWs->getSample(sampleID);
//	wsRootNode root=thisWs->getRoot(curSampleNode);
//	VertexID pVerID=addRoot(thisWs->to_popNode(&root));
////	wsRootNode popNode=root;//getPopulation();
//	addPopulation(pVerID,&root);
//
//}
/*
 * Constructor that starts from a particular sampleNode from workspace to build a tree
 */
GatingHierarchy::GatingHierarchy(wsSampleNode curSampleNode,workspace * ws,bool isParseGate,ncdfFlow * _nc,trans_global_vec * _gTrans,unsigned short _dMode)
{
//	data=NULL;
	dMode=_dMode;
	isLoaded=false;
	thisWs=ws;
	nc=_nc;
	gTrans=_gTrans;
//	cout<<"get root node"<<endl;

	wsRootNode root=thisWs->getRoot(curSampleNode);
	if(isParseGate)
	{

		if(dMode>=GATING_HIERARCHY_LEVEL)
			cout<<endl<<"parsing compensation..."<<endl;
		comp=thisWs->getCompensation(curSampleNode);

		if(dMode>=GATING_HIERARCHY_LEVEL)
			cout<<endl<<"parsing trans flags..."<<endl;
		transFlag=thisWs->getTransFlag(curSampleNode);

		if(dMode>=GATING_HIERARCHY_LEVEL)
			cout<<endl<<"parsing transformation..."<<endl;
		trans=thisWs->getTransformation(root,comp,transFlag,gTrans);
	}
	if(dMode>=POPULATION_LEVEL)
		cout<<endl<<"parsing populations..."<<endl;
	VertexID pVerID=addRoot(thisWs->to_popNode(root));
	addPopulation(pVerID,&root,isParseGate);
//	if(isParseGate)
//		gating();

}
/*
 * add root node first before recursively add the other nodes
 * since root node does not have gates as the others do
 */
VertexID GatingHierarchy::addRoot(nodeProperties* rootNode)
{
	// Create  vertices in that graph
	VertexID u = boost::add_vertex(tree);

	tree[u]=rootNode;
	;

	return(u);
}

/*
 * recursively append the populations to the tree
 * when the boolean gates are encountered before its reference nodes
 * we still can add it as it is because gating path is stored as population names instead of actual VertexID.
 * Thus we will deal with the the boolean gate in the actual gating process
 */
void GatingHierarchy::addPopulation(VertexID parentID,wsNode * parentNode,bool isParseGate)
{


	wsPopNodeSet children =thisWs->getSubPop(parentNode);
	wsPopNodeSet::iterator it;
		for(it=children.begin();it!=children.end();it++)
		{
			//add boost node
			VertexID curChildID = boost::add_vertex(tree);
			wsPopNode curChildNode=(*it);
			//convert to the node format that GatingHierarchy understands
			nodeProperties *curChild=thisWs->to_popNode(curChildNode,isParseGate);
			if(dMode>=POPULATION_LEVEL)
				cout<<"node created:"<<curChild->getName()<<endl;
			//attach the populationNode to the boost node as property
			tree[curChildID]=curChild;
			//add relation between current node and parent node
			boost::add_edge(parentID,curChildID,tree);
			//update the node map for the easy query by pop name

			//recursively add its descendants
			addPopulation(curChildID,&curChildNode,isParseGate);
		}


}
/*
 * this is for semi-automated pipeline to add populations sequetially
 */
void GatingHierarchy::addGate(gate& g,string popName)
{

	typedef boost::graph_traits<populationTree>::vertex_descriptor vertex_t;

	// Create  vertices in that graph
//	vertex_t u = boost::add_vertex(tree);


//	vertex_t v = boost::add_vertex(g);

	// Create an edge conecting those two vertices
//	edge_t e; bool b;
//	boost::tie(e,b) = boost::add_edge(u,v,g);

//	boost::add_edge()
}
compensation GatingHierarchy::getCompensation(){
	return comp;
}

void GatingHierarchy::printLocalTrans(){
	cout<<endl<<"get trans from gating hierarchy"<<endl;
	map<string,transformation* > trans=this->trans.getTransMap();

	for (map<string,transformation* >::iterator it=trans.begin();it!=trans.end();it++)
	{
		transformation * curTrans=it->second;


		if(!curTrans->isInterpolated())
				throw(domain_error("non-interpolated calibration table:"+curTrans->getName()+curTrans->getChannel()+" from channel"+it->first));


		cout<<it->first<<curTrans->getName()<<" "<<curTrans->getChannel()<<endl;;

	}
}

/*
 * subset operation is done within R,so there is no need for this member function
 * to apply subsetting within c++ thus avoid unnecessary numeric operation in c++
 * Note: need to manually free memory pointed by flowData
 */

flowData GatingHierarchy::getData(string sampleName,VertexID nodeID)
{
//	cout<<"reading data from ncdf"<<endl;

	flowData res=nc->readflowData(sampleName);
	//subset the results by indices for non-root node
	if(nodeID>0)
	{
		throw(domain_error("accessing data through non-root node is not supported yet!"));
	}
	else
		return res;
}
/*
 * in-memory version
 */
flowData GatingHierarchy::getData(VertexID nodeID)
{
//	cout<<"reading data from ncdf"<<endl;

	flowData res=fdata;
	//subset the results by indices for non-root node
	if(nodeID>0)
	{
		throw(domain_error("accessing data through non-root node is not supported yet!"));
	}
	else
		return res;
}
/*
 * load data from ncdfFlow file
 * TODO:the memory for flowData was actually allocated by getData function, it may be safer to set flag within getData in future when
 * we decide to keep getData seperate from loadData
 */
void GatingHierarchy::loadData(string sampleName)
{

	if(!isLoaded)
	{
		if(dMode>=GATING_HIERARCHY_LEVEL)
					cout <<"loading data from cdf.."<<endl;
		fdata=getData(sampleName,0);
		isLoaded=true;
	}



}
/*
 * non-cdf version
*/
void GatingHierarchy::loadData(const flowData & _fdata)
{

	if(!isLoaded)
	{
		if(dMode>=GATING_HIERARCHY_LEVEL)
					cout <<"loading data from memory.."<<endl;
		fdata=_fdata;
		isLoaded=true;
	}



}

void GatingHierarchy::unloadData()
{

	if(isLoaded)
	{
		if(dMode>=GATING_HIERARCHY_LEVEL)
					cout <<"unloading raw data.."<<endl;
//		delete fdata.data;
		fdata.clear();
		isLoaded=false;
	}



}
/*
 * transform the data
 */
void GatingHierarchy::transforming(bool updateCDF)
{
	if(dMode>=GATING_HIERARCHY_LEVEL)
		cout <<"start transforming data :"<<fdata.getSampleID()<<endl;
	if(!isLoaded)
		throw(domain_error("data is not loaded yet!"));

//	unsigned nEvents=fdata.nEvents;
//	unsigned nChannls=fdata.nChannls;
	vector<string> channels=fdata.getParams();

	/*
	 * transforming each marker
	 */
	for(vector<string>::iterator it1=channels.begin();it1!=channels.end();it1++)
	{

		string curChannel=*it1;

		transformation *curTrans=trans.getTran(curChannel);

		if(curTrans!=NULL)
		{
			if(curTrans->gateOnly())
				continue;

			valarray<double> x(this->fdata.subset(curChannel));
			if(dMode>=GATING_HIERARCHY_LEVEL)
				cout<<"transforming "<<curChannel<<" with func:"<<curTrans->getChannel()<<endl;

			curTrans->transforming(x);
			/*
			 * update fdata
			 */
			fdata.updateSlice(curChannel,x);
//			fdata.data[fdata.getSlice(curChannel)]=x;

		}


	}

	/*
	 * write the entire slice back to cdf
	 */
	if(updateCDF)
	{
		if(dMode>=GATING_HIERARCHY_LEVEL)
			cout<<"saving transformed data to CDF..."<<endl;
		nc->writeflowData(fdata);
	}
}
/*
 * extend gates if necessary
 */
void GatingHierarchy::extendGate(){
	if(dMode>=GATING_HIERARCHY_LEVEL)
			cout <<endl<<"start extending Gates for:"<<fdata.getSampleID()<<endl;

		if(!isLoaded)
				throw(domain_error("data is not loaded yet!"));

		VertexID_vec vertices=getVertices(false);

		for(VertexID_vec::iterator it=vertices.begin();it!=vertices.end();it++)
		{
			VertexID u=*it;
			nodeProperties * node=getNodeProperty(u);
			if(u!=0)
			{
				gate *g=node->getGate();
				if(g==NULL)
					throw(domain_error("no gate available for this node"));
				if(dMode>=POPULATION_LEVEL)
					cout <<node->getName()<<endl;
				if(g->getType()!=BOOLGATE)
					g->extend(fdata,dMode);
			}
		}
}

/*
 * traverse the tree to gate each pops
 * assuming data have already been compensated and transformed
 *
 */
void GatingHierarchy::gating()
{
	if(dMode>=GATING_HIERARCHY_LEVEL)
		cout <<endl<<"start gating:"<<fdata.getSampleID()<<endl;

	if(!isLoaded)
			throw(domain_error("data is not loaded yet!"));


	VertexID_vec vertices=getVertices(true);

	for(VertexID_vec::iterator it=vertices.begin();it!=vertices.end();it++)
	{
		VertexID u=*it;
		nodeProperties * node=getNodeProperty(u);
		if(u==0)
		{
			node->setIndices(vector<bool>(fdata.getEventsCount(),true));
			node->computeStats();
			continue;//skip gating for root node
		}
		/*
		 * check if current population is already gated (by boolGate)
		 *
		 */
		if(node->isGated())
			continue;
		else
			gating(u);

	}

	if(dMode>=GATING_HIERARCHY_LEVEL)
		cout <<"finish gating!"<<endl;



}
void GatingHierarchy::gating(VertexID u)
{
	nodeProperties * node=getNodeProperty(u);

	/*
	 * check if parent population is already gated
	 *
	 */
	VertexID_vec pids=getParent(u);
	if(pids.size()!=1) //we only allow one parent per node
		throw(domain_error("multiple parent nodes found!"));

	VertexID pid=pids.at(0);

	nodeProperties *parentNode =getNodeProperty(pid);
	if(!parentNode->isGated())
	{
		if(dMode>=POPULATION_LEVEL)
			cout <<"go to the ungated parent node:"<<parentNode->getName()<<endl;
		gating(pid);
	}



	if(dMode>=POPULATION_LEVEL)
		cout <<"gating on:"<<node->getName()<<endl;

	gate *g=node->getGate();

	if(g==NULL)
		throw(domain_error("no gate available for this node"));

	/*
	 * calculate the indices for the current node
	 */
	POPINDICES curIndices;
	if(g->getType()==BOOLGATE)
	{
		curIndices=boolGating(u);
	}
	else
	{
		/*
		 * transform gates if applicable
		 */

		g->transforming(trans,dMode);
		curIndices=g->gating(fdata);
	}

	//combine with parent indices
	transform (curIndices.begin(), curIndices.end(), parentNode->getIndices().begin(), curIndices.begin(),logical_and<bool>());
//	for(unsigned i=0;i<curIndices.size();i++)
//		curIndices.at(i)=curIndices.at(i)&parentNode->indices.at(i);
	node->setIndices(curIndices);
	node->computeStats();
}


POPINDICES GatingHierarchy::boolGating(VertexID u){

	nodeProperties * node=getNodeProperty(u);
	gate * g=node->getGate();

	//init the indices
	unsigned nEvents=fdata.getEventsCount();

	POPINDICES ind(nEvents,true);

	/*
	 * combine the indices of reference populations
	 */

	vector<BOOL_GATE_OP> boolOpSpec=g->getBoolSpec();
	for(vector<BOOL_GATE_OP>::iterator it=boolOpSpec.begin();it!=boolOpSpec.end();it++)
	{
		/*
		 * assume the reference node has already added during the parsing stage
		 */
		vector<string> nodePath=it->fullpath;
		VertexID nodeID=getNodeID(nodePath);
		nodeProperties * curPop=getNodeProperty(nodeID);
		if(!curPop->isGated())
		{
			if(dMode>=POPULATION_LEVEL)
				cout <<"go to the ungated reference node:"<<curPop->getName()<<endl;
			gating(nodeID);
		}


		POPINDICES curPopInd=curPop->getIndices();
		if(it->isNot)
			curPopInd.flip();


		switch(it->op)
		{
			case '&':
				transform (ind.begin(), ind.end(), curPopInd.begin(), ind.begin(),logical_and<bool>());
				break;
			case '|':
				transform (ind.begin(), ind.end(), curPopInd.begin(), ind.begin(),logical_or<bool>());
				break;
			default:
				throw(domain_error("not supported operator!"));
		}




//		for(unsigned i=0;i<ind.size();i++)
//		{
//
//
//			switch(it->op)
//			{
//				case '&':
//					ind.at(i)=ind.at(i)&curPopInd.at(i);
//					break;
//				case '|':
//					ind.at(i)=ind.at(i)|curPopInd.at(i);
//					break;
//				default:
//					throw(domain_error("not supported operator!"));
//			}
//		}

	}

	if(g->isNegate())
		ind.flip();

	return ind;

}

/*
 * current output the graph in dot format
 * and further covert it to gxl in order for Rgraphviz to read since it does not support dot directly
 * right now the data exchange is through file system,it would be nice to do it in memory
 */
void GatingHierarchy::drawGraph(string output)
{
	ofstream outputFile(output.c_str());

	boost::write_graphviz(outputFile,tree,OurVertexPropertyWriterR(tree));
	outputFile.close();


}

/*
 * retrieve the vertexIDs in topological order or in regular order
 */
VertexID_vec GatingHierarchy::getVertices(bool tsort=false){

	VertexID_vec res, vertices;
	if(tsort)
	{
		boost::topological_sort(tree,back_inserter(vertices));
		for(VertexID_vec::reverse_iterator it=vertices.rbegin();it!=vertices.rend();it++)
			res.push_back(*it);
	}
	else
	{
		VertexIt it_begin,it_end;
		tie(it_begin,it_end)=boost::vertices(tree);
		for(VertexIt it=it_begin;it!=it_end;it++)
			res.push_back((unsigned long)*it);
	}

	return(res);

}

/*
 * retrieve the VertexID by the gating path
 * this serves as a parser to convert generic gating path into internal node ID
 *
 */
VertexID GatingHierarchy::getNodeID(vector<string> gatePath){

	/*
	 * start from the root node
	 */
	VertexID parentID=0;

	for(vector<string>::iterator it=gatePath.begin();it!=gatePath.end();it++)
	{
		string nodeNameFromPath=*it;

		VertexID curNodeID=getChildren(parentID,nodeNameFromPath);
		if(curNodeID==-1)
		{
			string err="Node not found:";
			err.append(nodeNameFromPath);
			throw(domain_error(err));
		}
		else
			parentID=curNodeID;

	}
	return parentID;

}

/*
 * retrieve population names based on getVertices method
 * isPath flag indicates whether append the ancestor node names
 * the assumption is each node only has one parent
 */
vector<string> GatingHierarchy::getPopNames(bool tsort,bool isPath){

	VertexID_vec vertices=getVertices(tsort);
	vector<string> res;
	for(VertexID_vec::iterator it=vertices.begin();it!=vertices.end();it++)
	{
		VertexID u=*it;

		string nodeName=getNodeProperty(u)->getName();



		/*
		 * append ancestors on its way of tracing back to the root node
		 */
		if(isPath)
		{
			while(u>0)//if u==0, it is a root vertex
			{
				nodeName="/"+nodeName;
				VertexID_vec parents=getParent(u);
				if(parents.size()>1)
				{
					cout<<"multiple parent nodes."<<endl;
					break;
				}
				else
				{
					u=parents.at(0);
					if(u>0)//don't append the root node
						nodeName=getNodeProperty(u)->getName()+nodeName;
				}

			}


		}

		res.push_back(nodeName);

	}
	return res;
}
/*
 * using boost in_edges out_edges to retrieve adjacent vertices
 */
VertexID_vec GatingHierarchy::getParent(VertexID target){
	VertexID_vec res;
	if(target>=0&&target<=boost::num_vertices(tree)-1)
	{
//		cout<<"getting parent of "<<target<<"."<<tree[target].getName()<<endl;

		EdgeID e;
		boost::graph_traits<populationTree>::in_edge_iterator in_i, in_end;

		for (tie(in_i, in_end) = in_edges(target,tree);
			         in_i != in_end; ++in_i)
		{
		  e = *in_i;
		  VertexID  sarg = boost::source(e, tree);
		  res.push_back(sarg);
		}


	}
	else
	{
		cout<<"Warning:invalid vertexID:"<<target<<endl;
//		  res.push_back(0);

	}
	return(res);
}
/*
 * retrieve all children nodes
 */
VertexID_vec GatingHierarchy::getChildren(VertexID source){

	VertexID_vec res;
	if(source>=0&&source<=boost::num_vertices(tree)-1)
	{

		EdgeID e;
		boost::graph_traits<populationTree>::out_edge_iterator out_i, out_end;

		for (tie(out_i, out_end) = out_edges(source,tree);
				 out_i != out_end; ++out_i)
			{
			  e = *out_i;
			  VertexID  targ = target(e, tree);
			  res.push_back(targ);
			}
	}
	else
	{
		cout<<"invalid vertexID:"<<source<<endl;
//		res.push_back(0);
	}
	return(res);
}
/*
 * retrieve single child node by parent id and child name
 */
VertexID GatingHierarchy::getChildren(VertexID source,string childName){

	VertexID curNodeID=-1;
	VertexID_vec children=getChildren(source);
	VertexID_vec::iterator it;
	for(it=children.begin();it!=children.end();it++)
	{
		curNodeID=*it;
		if(getNodeProperty(curNodeID)->getName().compare(childName)==0)
			break;
	}

	return(curNodeID);

}
/*
 * returning the reference of the vertex bundle
 */
nodeProperties * GatingHierarchy::getNodeProperty(VertexID u){


	if(u>=0&&u<=boost::num_vertices(tree)-1)
		return(tree[u]);
	else
	{
		throw(out_of_range("returning empty node due to the invalid vertexID:"+u));

	}
}
//void GatingHierarchy::reset(){
//	isGated=false;
//	isLoaded=false;
//	nc=NULL;
//	thisWs=NULL;
//	fdata.data.resize(0);
//}
/*
 *TODO:to deal with trans copying (especially how to sync with gTrans)
  update to caller to free the memory
 */
GatingHierarchy * GatingHierarchy::clone(const trans_map & _trans,trans_global_vec * _gTrans){

	GatingHierarchy * res=new GatingHierarchy();


	res->trans.setTransMap(_trans);
	res->gTrans=_gTrans;
	res->comp=comp;

	/*
	 * copy bgl tree and update property bundle pointer for each node
	 * TODO:explore deep copying facility of bgl library,especially for node property bundle as the pointer
	 */
	res->tree=tree;
	VertexID_vec vertices=res->getVertices(false);
	for(VertexID_vec::iterator it=vertices.begin();it!=vertices.end();it++)
	{
		/*
		 * update the pointer for nodeProperties
		 */
		VertexID u=*it;
		nodeProperties * node=res->getNodeProperty(u);
		node=node->clone();
		//update the tree node
		res->tree[u]=node;
	}


	return res;
}
