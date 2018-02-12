#include "traversal_ops.h"

#include "../struct/module.h"
#include "../struct/connection.h"
#include "../sb7_exception.h"
#include "../parameters.h"

////////////////
// Traversal1 //
////////////////

int sb7::Traversal1::run(int mode) const {
	rd_ptr<Module> rd_mod(dataHolder->getModule(), mode);
	sh_ptr<ComplexAssembly> sh_designRoot = rd_mod->getDesignRoot();

	return traverse(sh_designRoot, mode);
}

int sb7::Traversal1::traverse(sh_ptr<ComplexAssembly> sh_cassm, int mode) const {
	int partsVisited = 0;

	rd_ptr<ComplexAssembly> rd_cassm(sh_cassm, mode);
	rd_ptr<Set<sh_ptr<Assembly> > > rd_subAssm(rd_cassm->getSubAssemblies(), mode);
	SetIterator<sh_ptr<Assembly> > iter = rd_subAssm->getIter();

	bool childrenAreBase = rd_cassm->areChildrenBaseAssemblies();

	// think about transforming this into a nicer oo design 
	while(iter.has_next()) {
		sh_ptr<Assembly> sh_assm = iter.next();

		if(!childrenAreBase) {
			partsVisited += traverse((sh_ptr<ComplexAssembly>)sh_assm, mode);
		} else {
			partsVisited += traverse((sh_ptr<BaseAssembly>)sh_assm, mode);
		}
	}

	return partsVisited;
}

int sb7::Traversal1::traverse(sh_ptr<BaseAssembly> bassm, int mode) const {
	int partsVisited = 0;

	rd_ptr<BaseAssembly> rd_bassm(bassm, mode);
	rd_ptr<Bag<sh_ptr<CompositePart> > > rd_cparts(rd_bassm->getComponents(), mode);
	BagIterator<sh_ptr<CompositePart> > iter = rd_cparts->getIter();

	while(iter.has_next()) {
		partsVisited += traverse(iter.next(), mode);
	}

	return partsVisited;
}

int sb7::Traversal1::traverse(sh_ptr<CompositePart> cpart, int mode) const {
	rd_ptr<CompositePart> rd_cpart(cpart, mode);
	sh_ptr<AtomicPart> sh_rootPart = rd_cpart->getRootPart();
	Set<sh_ptr<AtomicPart> > visitedPartSet;
	return traverse(sh_rootPart, visitedPartSet, mode);
}

int sb7::Traversal1::traverse(sh_ptr<AtomicPart> apart,
		Set<sh_ptr<AtomicPart> > &visitedPartSet, int mode) const {
	int ret;

	if(apart == NULL) {
		ret = 0;
	} else if(visitedPartSet.contains(apart)) {
		ret = 0;
	} else {
		ret = performOperationOnAtomicPart(apart, visitedPartSet, mode);
		visitedPartSet.add(apart);

		// visit all connected parts
		rd_ptr<AtomicPart> rd_apart(apart, mode);
		rd_ptr<Set<sh_ptr<Connection> > > rd_toConns(
			rd_apart->getToConnections(), mode);
		SetIterator<sh_ptr<Connection> > iter = rd_toConns->getIter();

		while(iter.has_next()) {
			rd_ptr<Connection> rd_conn(iter.next(), mode);
			ret += traverse(rd_conn->getDestination(), visitedPartSet, mode);
		}
	}

	return ret;
}

int sb7::Traversal1::performOperationOnAtomicPart(sh_ptr<AtomicPart> apart,
		Set<sh_ptr<AtomicPart> > &visitedPartSet, int mode) const {
	rd_ptr<AtomicPart> rd_apart(apart, mode);
	rd_apart->nullOperation();
	return 1;
}

/////////////////
// Traversal2a //
/////////////////

int sb7::Traversal2a::performOperationOnAtomicPart(sh_ptr<AtomicPart> apart,
		Set<sh_ptr<AtomicPart> > &visitedPartSet, int mode) const {
	int ret;

	if(visitedPartSet.size() == 0) {
		wr_ptr<AtomicPart> wr_apart(apart, mode);
		wr_apart->swapXY();
		ret = 1;
	} else {
		rd_ptr<AtomicPart> rd_apart(apart, mode);
		rd_apart->nullOperation();
		ret = 1;
	}

	return ret;
}

/////////////////
// Traversal2b //
/////////////////

int sb7::Traversal2b::performOperationOnAtomicPart(sh_ptr<AtomicPart> apart,
		Set<sh_ptr<AtomicPart> > &visitedPartSet, int mode) const {
	wr_ptr<AtomicPart> wr_apart(apart, mode);
	wr_apart->swapXY();
	return 1;
}

/////////////////
// Traversal2c //
/////////////////

int sb7::Traversal2c::performOperationOnAtomicPart(sh_ptr<AtomicPart> apart,
		Set<sh_ptr<AtomicPart> > &visitedPartSet, int mode) const {
	wr_ptr<AtomicPart> wr_apart(apart, mode);

	wr_apart->swapXY();
	wr_apart->swapXY();
	wr_apart->swapXY();
	wr_apart->swapXY();

	return 4;
}

/////////////////
// Traversal3a //
/////////////////

int sb7::Traversal3a::performOperationOnAtomicPart(sh_ptr<AtomicPart> apart,
		Set<sh_ptr<AtomicPart> > &visitedPartSet, int mode) const {
	int ret;

	if(visitedPartSet.size() == 0) {
		updateBuildDate(apart, mode);
		ret = 1;
	} else {
		rd_ptr<AtomicPart> rd_apart(apart, mode);
		rd_apart->nullOperation();
		ret = 1;
	}
	
	return ret;
}

void sb7::Traversal3a::updateBuildDate(sh_ptr<AtomicPart> apart, int mode) const {
	dataHolder->removeAtomicPartFromBuildDateIndex(apart, mode);
	wr_ptr<AtomicPart> wr_apart(apart, mode);
	wr_apart->updateBuildDate();
	dataHolder->addAtomicPartToBuildDateIndex(apart, mode);
}

/////////////////
// Traversal3b //
/////////////////

int sb7::Traversal3b::performOperationOnAtomicPart(sh_ptr<AtomicPart> apart,
		Set<sh_ptr<AtomicPart> > &visitedPartSet, int mode) const {
	updateBuildDate(apart, mode);
	return 1;
}

/////////////////
// Traversal3c //
/////////////////

int sb7::Traversal3c::performOperationOnAtomicPart(sh_ptr<AtomicPart> apart,
		Set<sh_ptr<AtomicPart> > &visitedPartSet, int mode) const {
	updateBuildDate(apart, mode);
	updateBuildDate(apart, mode);
	updateBuildDate(apart, mode);
	updateBuildDate(apart, mode);
	return 4;
}

////////////////
// Traversal4 //
////////////////

int sb7::Traversal4::traverse(sh_ptr<CompositePart> cpart, int mode) const {
	rd_ptr<CompositePart> rd_cpart(cpart, mode);
	return traverse(rd_cpart->getDocumentation(), mode);
}

int sb7::Traversal4::traverse(sh_ptr<Document> doc, int mode) const {
	rd_ptr<Document> rd_doc(doc, mode);
	return rd_doc->searchText('I');
}

int sb7::Traversal4::traverse(sh_ptr<AtomicPart> part,
		Set<sh_ptr<AtomicPart> > &setOfVisitedParts) const {
	throw Sb7Exception("T4: traverse(AtomicPart, HashSet<AtomicPart>) called!");
}

int sb7::Traversal4::performOperationOnAtomicPart(sh_ptr<AtomicPart> apart,
		Set<sh_ptr<AtomicPart> > &setOfVisitedPartIds) const {
	throw Sb7Exception("T4: performOperationInAtomicPart(..) called!");
}

////////////////
// Traversal5 //
////////////////

int sb7::Traversal5::traverse(sh_ptr<Document> doc, int mode) const {
	int ret;
	wr_ptr<Document> wr_doc(doc, mode);

	if(wr_doc->textBeginsWith("I am")) {
		ret = wr_doc->replaceText("I am", "This is");
	} else if(wr_doc->textBeginsWith("This is")) {
		ret = wr_doc->replaceText("This is", "I am");
	} else {
		throw Sb7Exception(
			("T5: illegal document text: " + wr_doc->getText()).c_str());
	}

	if(!ret) {
		throw Sb7Exception("T5: concurrent modification!");
	}

	return ret;
}

////////////////
// Traversal6 //
////////////////

int sb7::Traversal6::traverse(sh_ptr<CompositePart> cpart, int mode) const {
	rd_ptr<CompositePart> rd_cpart(cpart, mode);
	rd_ptr<AtomicPart> rd_rootPart(rd_cpart->getRootPart(), mode);
	rd_rootPart->nullOperation();
	return 1;
}

//////////////////////////////////////////////
// Traversal 7 - actually a short traversal //
//////////////////////////////////////////////

int sb7::Traversal7::run(int mode) const {
	int apartInd = get_random()->nextInt(parameters.getMaxAtomicParts()) + 1;
	rd_ptr<Map<int, sh_ptr<AtomicPart> > > rd_apartInd(
			dataHolder->getAtomicPartIdIndex(), mode);

	Map<int, sh_ptr<AtomicPart> >::Query query;
	query.key = apartInd;
	rd_apartInd->get(query);

	if(!query.found) {
		throw Sb7Exception();
	}

	rd_ptr<AtomicPart> rd_apart(query.val, mode);
	return traverse(rd_apart->getPartOf(), mode);
}

int sb7::Traversal7::traverse(sh_ptr<CompositePart> cpart, int mode) const {
	Set<sh_ptr<Assembly> > visitedAssemblies;
	int ret = 0;

	rd_ptr<CompositePart> rd_cpart(cpart, mode);
	rd_ptr<Bag<sh_ptr<BaseAssembly> > > rd_bag(rd_cpart->getUsedIn(), mode);
	BagIterator<sh_ptr<BaseAssembly> > iter = rd_bag->getIter();

	while(iter.has_next()) {
		sh_ptr<Assembly> sh_assembly(iter.next());
		ret += traverse(sh_assembly, visitedAssemblies, mode);
	}

	return ret;
}

int sb7::Traversal7::traverse(sh_ptr<Assembly> assembly,
		Set<sh_ptr<Assembly> > &visitedAssemblies, int mode) const {
	int ret;

	if(assembly == NULL) {
		ret = 0;
	} else if(visitedAssemblies.contains(assembly)) {
		ret = 0;
	} else {
		visitedAssemblies.add(assembly);
		performOperationOnAssembly(assembly, mode);
		rd_ptr<Assembly> rd_assembly(assembly, mode);
    	ret = traverse(rd_assembly->getSuperAssembly(), visitedAssemblies, mode) + 1;
	}

	return ret;
}

void sb7::Traversal7::performOperationOnAssembly(
		sh_ptr<Assembly> assembly, int mode) const {
	rd_ptr<Assembly> rd_assembly(assembly, mode);
	rd_assembly->nullOperation();
}

//////////////////////////////////////////
// Traversal8 - actually a ro operation //
//////////////////////////////////////////

int sb7::Traversal8::run(int mode) const {
	rd_ptr<Module> rd_mod(dataHolder->getModule(), mode);
	return traverse(rd_mod->getManual(), mode);
}

int sb7::Traversal8::traverse(sh_ptr<Manual> manual, int mode) const {
	rd_ptr<Manual> rd_manual(manual, mode);
	return rd_manual->countOccurences('I');
}

//////////////////////////////////////////
// Traversal9 - actually a ro operation //
//////////////////////////////////////////

int sb7::Traversal9::traverse(sh_ptr<Manual> manual, int mode) const {
	rd_ptr<Manual> rd_manual(manual, mode);
	return rd_manual->checkFirstLastCharTheSame();
}
