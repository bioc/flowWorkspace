/*
 * flowJoWorkspace.cpp
 *
 *  Created on: Mar 15, 2012
 *      Author: wjiang2
 */
#include "flowJoWorkspace.hpp"
#include <string>
//#include <libxml/tree.h>
#include <libxml/parser.h>

#include <iostream>
using namespace std;


/*constructors of flowJoWorkspace for mac and win derived classes
 *
 */

macFlowJoWorkspace::macFlowJoWorkspace(xmlDoc * doc){
	cout<<"mac version of flowJo workspace recognized."<<endl;

	nodePath.group="/Workspace/Groups/GroupNode";
	nodePath.sampleRef=".//SampleRef";
	nodePath.sample="/Workspace/SampleList/Sample";
	nodePath.sampleNode="./SampleNode";
	nodePath.popNode="./Population";
	this->doc=doc;

}
winFlowJoWorkspace::winFlowJoWorkspace(xmlDoc * doc){
	cout<<"windows version of flowJo workspace recognized."<<endl;
//	xpath_sample="/Workspace/SampleList/Sample/DataSet";
	nodePath.group="/Workspace/Groups/GroupNode";
	nodePath.sampleRef=".//SampleRef";
	nodePath.sample="/Workspace/SampleList/Sample";
	nodePath.sampleNode="./SampleNode";
	nodePath.popNode="./Subpopulations";
	this->doc=doc;

}


/*get a vector of sampleID by the given groupID
 * keep the returned results in char * format in case the non-numeric sampleID is stored
 * make sure to free the memory of xmlChar * outside of the call
 */
vector<string> flowJoWorkspace::getSampleID(unsigned short groupID)
{

		xmlXPathContextPtr context = xmlXPathNewContext(doc);
		xmlXPathObjectPtr result = xmlXPathEval((xmlChar *)nodePath.group.c_str(), context);
		if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
			xmlXPathFreeObject(result);
			xmlXPathFreeContext(context);
	                cout<<"No Groups"<<endl;;
	                throw(2);
		}


		xmlNodePtr cur=result->nodesetval->nodeTab[groupID];
		context->node=cur;
		xmlXPathObjectPtr sids=xmlXPathEval((xmlChar *)nodePath.sampleRef.c_str(),context);
		vector<string> sampleID;
		xmlNodeSetPtr nodeSet=sids->nodesetval;
		int size=nodeSet->nodeNr;

		for(unsigned i=0;i<size;i++)
		{
			xmlNodePtr curNode=nodeSet->nodeTab[i];
			xmlChar * curSampleID= xmlGetProp(curNode,(xmlChar *)"sampleID");
			//to avoid memory leaking,store a copy of returned characters in string vector so that the dynamically allocated memory
			//can be freed right away in stead of later on.
			sampleID.push_back(string((const char *)curSampleID));
			xmlFree(curSampleID);
		}
//			;

		xmlXPathFreeObject (result);
		xmlXPathFreeContext(context);
		xmlXPathFreeObject (sids);
		return(sampleID);
}


//make sure to free the memory of xmlXPathObject outside of the call
wsSampleNode flowJoWorkspace::getSampleNode(string sampleID){
		string xpath=nodePath.sample;
		xpath.append("[@sampleID='");
		xpath.append(sampleID);
		xpath.append("']");
		xmlXPathContextPtr context=xmlXPathNewContext(doc);

		xmlXPathObjectPtr res=xmlXPathEval((xmlChar *)xpath.c_str(),context);
		wsSampleNode sample(res->nodesetval->nodeTab[0]);

		xmlXPathFreeContext(context);
		xmlXPathFreeObject(res);
		return sample;
}

//need to explicitly release the memory after this call
string flowJoWorkspace::getName(wsNode * node){

//	xmlChar * curSampleID= xmlGetProp(node->thisNode,(xmlChar *)"sampleID");
	return "test";

}
//xquery the "SampleNode" within "sample" context
wsRootNode flowJoWorkspace::getRoot(wsSampleNode sample)
{


	xmlXPathObjectPtr res=sample.xpathInNode(nodePath.sampleNode);
	wsRootNode node(res->nodesetval->nodeTab[0]);
	xmlXPathFreeObject(res);
	return node;
}


wsPopNodeSet flowJoWorkspace::getSubPop(wsNode * node)
{

	xmlXPathObjectPtr res=node->xpathInNode(nodePath.popNode);
	int nChildren=res->nodesetval->nodeNr;
//	wsPopNodeSet childenNodes(res->nodesetval->nodeTab,nChildren);
	wsPopNodeSet childenNodes;
	for(int i=0;i<nChildren;i++)
	{
		childenNodes.push_back(wsPopNode(res->nodesetval->nodeTab[i]));
	}

	xmlXPathFreeObject(res);

	return childenNodes;

}

populationNode flowJoWorkspace::to_popNode(wsRootNode const * node){
	xmlChar * popName=xmlGetProp(node->thisNode,(const xmlChar*)"name");

	populationNode pNode;
	pNode.setName((const char *)popName);

	xmlFree(popName);

	return pNode;
}

populationNode flowJoWorkspace::to_popNode(wsPopNode const * node){
	xmlChar * popName=xmlGetProp(node->thisNode,(const xmlChar*)"name");

	populationNode pNode;
	pNode.setName((const char *)popName);

	xmlFree(popName);

	return pNode;
}
