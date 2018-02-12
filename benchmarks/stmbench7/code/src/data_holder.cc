#include <cstdio>
#include <iostream>

#include "data_holder.h"
#include "parameters.h"
#include "sb7_exception.h"
#include "helpers.h"
#include "random.h"
#include "tm/tm_tx.h"

#include "struct/connection.h"

using namespace sb7;

void sb7::DataHolder::allocateData() {
	// initialize indexes
	m_atomicPartIdIndex = shared_atomic_part_int_index(
		new atomic_part_int_index());
	m_atomicPartBuildDateIndex = shared_atomic_part_set_int_index(
		new atomic_part_set_int_index());
	m_documentTitleIndex = shared_document_string_index(
		new document_string_index());
	m_compositePartIdIndex = shared_composite_part_int_index(
		new composite_part_int_index());
	m_baseAssemblyIdIndex = shared_base_assembly_int_index(
		new base_assembly_int_index());
	m_complexAssemblyIdIndex = shared_complex_assembly_int_index(
		new complex_assembly_int_index());

	// initialize id pools
	m_compositePartIdPool = sh_ptr<IdPool>(
		new IdPool(parameters.getMaxCompParts()));
	m_atomicPartIdPool = sh_ptr<IdPool>(
		new IdPool(parameters.getMaxAtomicParts()));
	m_moduleIdPool = sh_ptr<IdPool>(new IdPool(parameters.getNumModules()));
	m_documentIdPool = sh_ptr<IdPool>(new IdPool(
		parameters.getMaxCompParts()));
	m_manualIdPool = sh_ptr<IdPool>(new IdPool(parameters.getNumModules()));
	m_baseAssemblyIdPool = sh_ptr<IdPool>(
		new IdPool(parameters.getMaxBaseAssemblies()));
	m_complexAssemblyIdPool = sh_ptr<IdPool>(
		new IdPool(parameters.getMaxComplexAssemblies()));
}

void sb7::DataHolder::init() {
	allocateData();

	// first create composite parts and everything below them
	int initialTotalCompParts = parameters.getInitialTotalCompParts();
	sh_ptr<CompositePart> *designLibrary =
		new sh_ptr<CompositePart>[initialTotalCompParts];
	createCompositeParts(designLibrary, initialTotalCompParts);

	// create module
	m_module = createModule(0);
	// create assemblies
	sh_ptr<ComplexAssembly> designRoot = createAssemblies(0);

	// connect module and design root
	wr_ptr<Module> wr_mod(m_module,0);
	wr_mod->setDesignRoot(designRoot);

	// connect assemblies and composite parts
	connectAssembliesParts(designLibrary,
		initialTotalCompParts,
		parameters.getNumCompPerAssm(), 0);

	// delete design library
	delete [] designLibrary;
}

void sb7::DataHolder::initTx(int mode) {
	allocateData();

	// first create composite parts and everything below them
	int initialTotalCompParts = parameters.getInitialTotalCompParts();
	sh_ptr<CompositePart> *designLibrary =
		new sh_ptr<CompositePart>[initialTotalCompParts];
	createCompositePartsTx(designLibrary, initialTotalCompParts, mode);
	// create module
	m_module = createModuleTx(mode);
	// create assemblies
	sh_ptr<ComplexAssembly> designRoot = createAssembliesTx(mode);
	// connect module and design root
	connectModuleDesignRootTx(designRoot, mode);

	// connect assemblies and composite parts
	connectAssembliesPartsTx(designLibrary,
		initialTotalCompParts,
		parameters.getNumCompPerAssm(), mode);

	// delete design library
	delete [] designLibrary;
}

void sb7::DataHolder::connectModuleDesignRootTx(
		sh_ptr<ComplexAssembly> designRoot, int mode) {
	ConnectModuleDesignRootTxInnerParamStruct param =
		{ this, &designRoot };
	run_tx(connectModuleDesignRootTxInner, mode, 0, &param);
}

sh_ptr<Document> sb7::DataHolder::createDocument(int cpartId, int mode) {
	wr_ptr<IdPool> wr_docIdPool(m_documentIdPool, mode);
	int docId = wr_docIdPool->getId();

	ITOA(cpartIdStr, cpartId);
	string docTitle = "Composite Part #" + (string)cpartIdStr;
	string txt = "I am the documentation for composite part #";
	txt += cpartIdStr;
	txt += '\n';
	string docText = createText(parameters.getDocumentSize(), txt);

	Document *doc = new Document(docId, docTitle, docText);
	sh_ptr<Document> ret(doc);

	wr_ptr<document_string_index> wr_docInd(m_documentTitleIndex, mode);
	wr_docInd->put(docTitle, ret);

	return ret;
}

sh_ptr<Manual> sb7::DataHolder::createManual(int moduleId, int mode) {
	wr_ptr<IdPool> wr_manIdPool(m_manualIdPool, mode);
	int manualId = wr_manIdPool->getId();

	ITOA(moduleIdStr, moduleId);
	string manTitle = "Manual for module #" + (string)moduleIdStr;
	string txt = "I am the manual for module #";
	txt += moduleIdStr;
	txt += '\n';
	string manText = createText(parameters.getManualSize(), txt);

	Manual *man = new Manual(manualId, manTitle, manText);
	sh_ptr<Manual> ret(man);
	return ret;
}

sb7::string sb7::DataHolder::createText(unsigned size, const string &txt) {
	string ret;

	while(ret.size() < size) {
		ret.append(txt);
	}

	return ret;
}

sb7::string sb7::DataHolder::createType() {
	int typeNum = get_random()->nextInt(parameters.getNumTypes());
	ITOA(itoa_buf, typeNum);
	string txt = "type #" + (string)itoa_buf;
	return txt;
}

int sb7::DataHolder::createBuildDate(int min, int max) {
	return min + get_random()->nextInt(max - min + 1);
}

void sb7::DataHolder::createCompositeParts(
		sh_ptr<CompositePart> designLibrary[], int dlSize) {

	//std::cout << "Creating composite parts:" << std::endl;

	for(int i = 0;i < dlSize;i++) {
		//std::cout << "\r";
		designLibrary[i] = createCompositePart(0);
		//std::cout << "Composite part " << i + 1 << " of " << dlSize << std::flush;
	}

	std::cout << std::endl << "Finished creating composite parts" << std::endl;
}

void sb7::DataHolder::createCompositePartsTx(
		sh_ptr<CompositePart> designLibrary[], int dlSize, int mode) {

	for(int i = 0;i < dlSize;i++) {
		designLibrary[i] = createCompositePartTx(mode);
	}

	std::cout << std::endl << "Finished creating composite parts" << std::endl;
}

sh_ptr<CompositePart> sb7::DataHolder::createCompositePart(int mode) {
	sh_ptr<CompositePart> ret;
	createCompositePart(&ret, mode);
	return ret;
}

void sb7::DataHolder::createCompositePart(
		sh_ptr<CompositePart> *retPtr, int mode) {
	// get composite part id
	wr_ptr<IdPool> wr_cpartIdPool(m_compositePartIdPool, mode);
	int id = wr_cpartIdPool->getId();

	// create other composite part elements
	string type = createType();

	// generate build date
	bool youngCPart = get_random()->nextInt(HUNDRED_PERCENT) <
		parameters.getYoungCompFrac();
	int min, max;

	if(youngCPart) {
		min = parameters.getMinYoungCompDate();
		max = parameters.getMaxYoungCompDate();
	} else {
		min = parameters.getMinOldCompDate();
		max = parameters.getMaxOldCompDate();
	}

	int buildDate = createBuildDate(min, max);

	// make document connected to this composite part
	sh_ptr<Document> doc = createDocument(id, mode);

	// make composite part using all data previously created
	CompositePart *cpart = new CompositePart(id, type, buildDate, doc);
	sh_ptr<CompositePart> sh_cpart(cpart);

	// connect document and composite part
	wr_ptr<Document> wr_doc(doc, mode);
	wr_doc->setPart(sh_cpart);

	// create atomic parts
	vector_apart aparts;
	createAtomicParts(aparts, parameters.getNumAtomicPerComp(), sh_cpart, mode);

	// create connections among atomic parts
	createConnections(aparts, parameters.getNumConnPerAtomic(), mode);

	// put composite part into index
	wr_ptr<composite_part_int_index> wr_cpartInd(m_compositePartIdIndex, mode);
	wr_cpartInd->put(id, sh_cpart);

	// finally return created composite part
	*retPtr = sh_cpart;
}

sh_ptr<CompositePart> sb7::DataHolder::createCompositePartTx(int mode) {
	sh_ptr<CompositePart> ret;
	CreateCompositePartTxInnerParamStruct param = { this, &ret };
	run_tx(createCompositePartTxInner, mode,  0, &param);
	return ret;
}

void sb7::DataHolder::createAtomicParts(vector_apart &parts, int psize,
		sh_ptr<CompositePart> sh_cpart, int mode) {
	wr_ptr<CompositePart> wr_cpart(sh_cpart, mode);

	for(int i = 0;i < psize;i++) {
		sh_ptr<AtomicPart> sh_apart = createAtomicPart(mode);
		wr_cpart->addPart(sh_apart, mode);
		parts.push_back(sh_apart);
	}
}

sh_ptr<AtomicPart> sb7::DataHolder::createAtomicPart(int mode) {
	wr_ptr<IdPool> wr_apartIdPool(m_atomicPartIdPool, mode);
	int id = wr_apartIdPool->getId();
	string type = createType();
	int buildDate = createBuildDate(
		parameters.getMinAtomicDate(),
		parameters.getMaxAtomicDate());
	int x = get_random()->nextInt(parameters.getXYRange());
	int y = get_random()->nextInt(parameters.getXYRange());

	AtomicPart *apart = new AtomicPart(id, type, buildDate, x, y);
	sh_ptr<AtomicPart> sh_apart(apart);

	wr_ptr<atomic_part_int_index> wr_apartInd(m_atomicPartIdIndex, mode);
	wr_apartInd->put(id, sh_apart);
	addAtomicPartToBuildDateIndex(sh_apart, buildDate);

	return sh_apart;
}

void sb7::DataHolder::createConnections(vector_apart &parts, int outConn, int mode) {
	int size = parts.size();

	// connect all atomic parts in a ring in order to get fully
	// connected graph
	for(int i = 0;i < size - 1;i++) {
		connectAtomicParts(parts[i], parts[i + 1], mode);
	}

	connectAtomicParts(parts[size -1], parts[0], mode);

	// add random connections
	for(int i = 0;i < size;i++) {
		for(int j = 0;j < outConn;j++) {
			int connectToInd = get_random()->nextInt(size);
			connectAtomicParts(parts[i], parts[connectToInd], mode);
		}
	}
}

void sb7::DataHolder::connectAtomicParts(sh_ptr<AtomicPart> start,
		sh_ptr<AtomicPart> end, int mode) {
	string type = createType();
	int len = generateConnectionLength();

	wr_ptr<AtomicPart> wr_start(start, mode);
	wr_start->connectTo(end, type, len, mode);
}

int sb7::DataHolder::generateConnectionLength() {
	return get_random()->nextInt(parameters.getXYRange()) + 1;
}

void sb7::DataHolder::createModule(sh_ptr<Module> *retPtr, int mode) {
	// generate all required data elements
	wr_ptr<IdPool> wr_modIdPool(m_moduleIdPool, mode);
	int id = wr_modIdPool->getId();
	string type = createType();
	int buildDate = createBuildDate(
		parameters.getMinModuleDate(),
		parameters.getMaxModuleDate());

	// create manual
	sh_ptr<Manual> sh_man = createManual(id, mode);

	// create return value
	Module *mod = new Module(id, type, buildDate, sh_man);
	sh_ptr<Module> sh_mod(mod);

	// connect manual -> module
	rd_ptr<Module> rd_mod(sh_mod, mode);
	rd_mod->connectManual(mode);

	*retPtr = sh_mod;
}

sh_ptr<Module> sb7::DataHolder::createModule(int mode) {
	sh_ptr<Module> ret;
	createModule(&ret, mode);
	return ret;
}

sh_ptr<Module> sb7::DataHolder::createModuleTx(int mode) {
	sh_ptr<Module> ret;
	CreateModuleTxInnerParamStruct param = { this, &ret };
	run_tx(createModuleTxInner, mode, 0, &param);
	return ret;
}

void sb7::DataHolder::createAssemblies(
		sh_ptr<ComplexAssembly> *retPtr, int mode) {
	*retPtr = createComplexAssembly(sh_ptr<ComplexAssembly>(),
		parameters.getNumAssmPerAssm(), mode);
}

sh_ptr<ComplexAssembly> sb7::DataHolder::createAssemblies(int mode) {
	sh_ptr<ComplexAssembly> designRoot;
	createAssemblies(&designRoot, mode);
	return designRoot;
}

sh_ptr<ComplexAssembly> sb7::DataHolder::createAssembliesTx(int mode) {
	sh_ptr<ComplexAssembly> designRoot;
	CreateAssembliesTxInnerParamStruct param = { this, &designRoot };
	run_tx(createAssembliesTxInner, mode, 0, &param);
	printf("alaa\n");
	return designRoot;
}

void sb7::DataHolder::connectAssembliesParts(
		sh_ptr<CompositePart> designLibrary[], int partNum, int connNum, int mode) {
	rd_ptr<base_assembly_int_index> rd_bassInd(m_baseAssemblyIdIndex, mode);

	MapIterator<int, sh_ptr<BaseAssembly> > iter = rd_bassInd->getAll();

	while(iter.has_next()) {
		sh_ptr<BaseAssembly> sh_ba = iter.next();
		wr_ptr<BaseAssembly> wr_ba(sh_ba, mode);

		for(int i = 0;i < connNum;i++) {
			int partInd = get_random()->nextInt(partNum);
			wr_ba->addComponent(designLibrary[partInd], mode);
		}
	}
}

void sb7::DataHolder::connectAssembliesPartsTx(
		sh_ptr<CompositePart> designLibrary[], int partNum, int connNum, int mode) {
	ConnectAssembliesTxInnerParamStruct data =
		{ this, designLibrary, partNum, connNum };
	run_tx(connectAssembliesPartsTxInner, mode,  0, &data);
}

sh_ptr<ComplexAssembly> sb7::DataHolder::createComplexAssembly(
		sh_ptr<ComplexAssembly> parent, int childNum, int mode) {
	// create required data


	wr_ptr<IdPool> wr_cassmIdPool(m_complexAssemblyIdPool,mode);
	int id = wr_cassmIdPool->getId();
	printf("shady\n");
	string type = createType();
	int buildDate = createBuildDate(
		parameters.getMinAssmDate(),
		parameters.getMaxAssmDate());

	// construct complex assembly
	ComplexAssembly *cassm = new ComplexAssembly(id, type, buildDate,
		m_module, parent);
	sh_ptr<ComplexAssembly> sh_cassm(cassm);

	// set complex assembly level
	wr_ptr<ComplexAssembly> wr_cassm(sh_cassm, mode);
	wr_cassm->setLevel(mode);

	// add complex assembly to the index
	wr_ptr<complex_assembly_int_index> wr_cassmInd(m_complexAssemblyIdIndex, mode);
	wr_cassmInd->put(id, sh_cassm);

	// add complex assembly to parent
	if(parent != NULL) {
		wr_ptr<ComplexAssembly> wr_parent(parent, mode);
		sh_ptr<Assembly> sh_assm(cassm);
		wr_parent->addSubAssembly(sh_assm, mode);
	}

	// create children
	rd_ptr<ComplexAssembly> rd_cassm(sh_cassm, mode);
	bool createBase = rd_cassm->areChildrenBaseAssemblies();
	printf("is base %d\n",createBase);
	for(int i = 0;i < childNum;i++) {
		if(createBase) {
			createBaseAssembly(sh_cassm, mode);
		} else {
			createComplexAssembly(sh_cassm, childNum, mode);
		}
	}

	return sh_cassm;
}

void sb7::DataHolder::createSubAssembly(sh_ptr<ComplexAssembly> parent,
		int childNum, int mode) {
	rd_ptr<ComplexAssembly> rd_cassm(parent, mode);

	if(rd_cassm->areChildrenBaseAssemblies()) {
		createBaseAssembly(parent, mode);
	} else {
		createComplexAssembly(parent, childNum, mode);
	}
}

void sb7::DataHolder::createBaseAssembly(sh_ptr<ComplexAssembly> parent, int mode) {
	// create required data
	wr_ptr<IdPool> wr_bassmIdPool(m_baseAssemblyIdPool, mode);
	int id = wr_bassmIdPool->getId();
	string type = createType();
	int buildDate = createBuildDate(
		parameters.getMinAssmDate(),
		parameters.getMaxAssmDate());

	// construct object
	BaseAssembly *bassm = new BaseAssembly(id, type, buildDate,
		m_module, parent);
	sh_ptr<BaseAssembly> sh_bassm(bassm);

	// put into index
	wr_ptr<base_assembly_int_index> wr_bassmInd(m_baseAssemblyIdIndex, mode);
	wr_bassmInd->put(id, sh_bassm);

	// add to parent
	wr_ptr<ComplexAssembly> wr_parent(parent, mode);
	sh_ptr<Assembly> sh_assm(bassm);
	wr_parent->addSubAssembly(sh_assm, mode);
}

void sb7::DataHolder::addAtomicPartToBuildDateIndex(
		sh_ptr<AtomicPart> sh_apart, int buildDate, int mode) {
	// search for set containing aparts with corresponding build date
	rd_ptr<atomic_part_set_int_index> rd_apartSetInd(
		m_atomicPartBuildDateIndex, mode);
	atomic_part_set_int_index::Query query;
	query.key = buildDate;
	rd_apartSetInd->get(query);
	sh_ptr<atomic_part_set> sh_set;

	if(query.found) {
		sh_set = query.val;
	} else {
		sh_set = sh_ptr<atomic_part_set>(new atomic_part_set());
		wr_ptr<atomic_part_set_int_index> wr_apartSetInd(
			m_atomicPartBuildDateIndex, mode);
		wr_apartSetInd->put(buildDate, sh_set);
	}

	wr_ptr<atomic_part_set> wr_set(sh_set, mode);
	wr_set->add(sh_apart);
}

// just get build date and invoke function above
void sb7::DataHolder::addAtomicPartToBuildDateIndex(sh_ptr<AtomicPart> apart, int mode) {
	rd_ptr<AtomicPart> rd_apart(apart, mode);
	int buildDate = rd_apart->getBuildDate();
	addAtomicPartToBuildDateIndex(apart, buildDate, mode);
}

void sb7::DataHolder::removeAtomicPartFromBuildDateIndex(
		sh_ptr<AtomicPart> apart, int mode) {
	// get atomic part date
	rd_ptr<AtomicPart> rd_apart(apart, mode);

	// get set of atomic parts with the same build date
	rd_ptr<Map<int, shared_atomic_part_set> > rd_apartBuildDateSetMap(
		m_atomicPartBuildDateIndex, mode);
	Map<int, shared_atomic_part_set>::Query query;
	query.key = rd_apart->getBuildDate();
	rd_apartBuildDateSetMap->get(query);

	if(query.found) {
		wr_ptr<Set<sh_ptr<AtomicPart> > > wr_apartSet(query.val, mode);
		wr_apartSet->remove(apart);
	}
}


//////////////////////////////////////
// search for objects using indexes //
//////////////////////////////////////

sh_ptr<CompositePart> sb7::DataHolder::getCompositePart(int id, int mode) const {
	// make a search through an index
	rd_ptr<Map<int, sh_ptr<CompositePart> > > rd_ind(m_compositePartIdIndex, mode);
	Map<int, sh_ptr<CompositePart> >::Query query;
	query.key = id;
	rd_ind->get(query);

	// if object was found, return it
	if(query.found) {
		return query.val;
	}

	// otherwise return NULL
	return sh_ptr<CompositePart>();
}

sh_ptr<BaseAssembly> sb7::DataHolder::getBaseAssembly(int id, int mode) const {
	rd_ptr<Map<int, sh_ptr<BaseAssembly> > > rd_ind(m_baseAssemblyIdIndex, mode);
	Map<int, sh_ptr<BaseAssembly> >::Query query;
	query.key = id;
	rd_ind->get(query);

	if(query.found) {
		return query.val;
	}

	return sh_ptr<BaseAssembly>();
}

sh_ptr<ComplexAssembly> sb7::DataHolder::getComplexAssembly(int id, int mode) const {
	rd_ptr<Map<int, sh_ptr<ComplexAssembly> > > rd_ind(
			m_complexAssemblyIdIndex, mode);
	Map<int, sh_ptr<ComplexAssembly> >::Query query;
	query.key = id;
	rd_ind->get(query);

	if(query.found) {
		return query.val;
	}

	return sh_ptr<ComplexAssembly>();
}

///////////////////////////////////////////
// Delete elements of the data structure //
///////////////////////////////////////////

void sb7::DataHolder::deleteDocument(sh_ptr<Document> doc, int mode) {
	rd_ptr<Document> rd_doc(doc, mode);
	int id = rd_doc->getDocumentId();

	// remove from document index
	wr_ptr<Map<string, sh_ptr<Document> > > wr_docInd(m_documentTitleIndex, mode);
	wr_docInd->remove(rd_doc->getTitle());

	// delete document object
	tx_delete(doc, mode);

	// return id to pool
	wr_ptr<IdPool> wr_docIdPool(m_documentIdPool, mode);
	wr_docIdPool->putId(id);
}

void sb7::DataHolder::deleteAtomicPart(sh_ptr<AtomicPart> apart, int mode) {
	rd_ptr<AtomicPart> rd_apart(apart, mode);
	int id = rd_apart->getId();

	// remove part from build date index
	removeAtomicPartFromBuildDateIndex(apart, mode);

	// remove atomic part from id index
	wr_ptr<Map<int, sh_ptr<AtomicPart> > > wr_apartInd(m_atomicPartIdIndex, mode);
	wr_apartInd->remove(id);

	// remove connections to other parts and delete them
	rd_ptr<Set<sh_ptr<Connection> > > rd_connToSet(
		rd_apart->getToConnections(), mode);
	SetIterator<sh_ptr<Connection> > iterTo = rd_connToSet->getIter();

	while(iterTo.has_next()) {
		sh_ptr<Connection> sh_conn(iterTo.next());

		// remove connection from destination part
		rd_ptr<Connection> rd_conn(sh_conn, mode);
		rd_ptr<AtomicPart> rd_apart(rd_conn->getDestination(), mode);
		rd_apart->removeConnectionFromOtherPart(sh_conn, mode);

		// delete connection
		tx_delete(sh_conn, mode);
	}

	// remove connections from other parts and delete them
	rd_ptr<Set<sh_ptr<Connection> > > rd_connFromSet(
		rd_apart->getFromConnections(), mode);
	SetIterator<sh_ptr<Connection> > iterFrom = rd_connFromSet->getIter();

	while(iterFrom.has_next()) {
		sh_ptr<Connection> sh_conn(iterFrom.next());

		// remove connection from destination part
		rd_ptr<Connection> rd_conn(sh_conn, mode);
		rd_ptr<AtomicPart> rd_apart(rd_conn->getSource(), mode);
		rd_apart->removeConnectionToOtherPart(sh_conn, mode);

		// delete connection
		tx_delete(sh_conn, mode);
	}

	// delete atomic part
	wr_ptr<AtomicPart> wr_apart(apart, mode);
	wr_apart->freeMemory(mode);
	tx_delete(apart, mode);

	// return id to the pool
	wr_ptr<IdPool> wr_apartIdPool(m_atomicPartIdPool, mode);
	wr_apartIdPool->putId(id);
}

void sb7::DataHolder::deleteCompositePart(sh_ptr<CompositePart> cpart, int mode) {
	rd_ptr<CompositePart> rd_cpart(cpart, mode);
	int cpartId = rd_cpart->getId();

	// remove composite part from composite part index
	wr_ptr<Map<int, sh_ptr<CompositePart> > > wr_cpartInd(
		m_compositePartIdIndex, mode);
	wr_cpartInd->remove(cpartId);

	// delete document
	deleteDocument(rd_cpart->getDocumentation(), mode);

	// delete atomic parts
	rd_ptr<Set<sh_ptr<AtomicPart> > > rd_apartSet(rd_cpart->getParts(), mode);
	SetIterator<sh_ptr<AtomicPart> > iterApart = rd_apartSet->getIter();

	while(iterApart.has_next()) {
		deleteAtomicPart(iterApart.next(), mode);
	}

	// break connection with all base assemblies
	rd_ptr<Bag<sh_ptr<BaseAssembly> > > rd_bassmBag(rd_cpart->getUsedIn(), mode);
	BagIterator<sh_ptr<BaseAssembly> > iterBag = rd_bassmBag->getIter();
	Set<sh_ptr<BaseAssembly> > bassmSet;

	// first copy all base assemblies to separate set as bag will change
	// while breaking connection
	while(iterBag.has_next()) {
		bassmSet.add(iterBag.next());
	}

	// now use this set and break connections
	SetIterator<sh_ptr<BaseAssembly> > iterSet = bassmSet.getIter();

	while(iterSet.has_next()) {
		wr_ptr<BaseAssembly> wr_bassm(iterSet.next(), mode);

		while(wr_bassm->removeComponent(cpart, mode)) {
			// do nothing in the body
		}
	}

	// mark composite part for deletion
	wr_ptr<CompositePart> wr_cpart(cpart, mode);
	wr_cpart->freeMemory(mode);
	tx_delete(cpart, mode);

	// return id to the pool
	wr_ptr<IdPool> wr_cpartIdPool(m_compositePartIdPool, mode);
	wr_cpartIdPool->putId(cpartId);
}

void sb7::DataHolder::deleteBaseAssembly(sh_ptr<BaseAssembly> bassm, int mode) {
	rd_ptr<BaseAssembly> rd_bassm(bassm, mode);

	// remove from base assembly index
	int bassmId = rd_bassm->getId();
	wr_ptr<Map<int, sh_ptr<BaseAssembly> > > wr_bassmInd(
		m_baseAssemblyIdIndex, mode);
	wr_bassmInd->remove(bassmId);

	// remove from parent complex assembly
	wr_ptr<ComplexAssembly> wr_parent(rd_bassm->getSuperAssembly(), mode);
	wr_parent->removeSubAssembly((sh_ptr<Assembly>)bassm, mode);

	// remove links to all used components
	rd_ptr<Bag<sh_ptr<CompositePart> > > rd_cpartBag(
		rd_bassm->getComponents(), mode);
	Set<sh_ptr<CompositePart> > cpartSet;

	// copy component bag to a local one, to avoid changes to set we are
	// iterating through
	BagIterator<sh_ptr<CompositePart> > iterSh = rd_cpartBag->getIter();

	while(iterSh.has_next()) {
		cpartSet.add(iterSh.next());
	}

	// now go through local bag and remove all components in there
	SetIterator<sh_ptr<CompositePart> > iter = cpartSet.getIter();

	while(iter.has_next()) {
		sh_ptr<CompositePart> sh_cpart(iter.next());

		while(rd_bassm->removeComponent(sh_cpart, mode)) {
			// do nothing
		}
	}

	// delete base assembly
	wr_ptr<BaseAssembly> wr_bassm(bassm, mode);
	wr_bassm->freeMemory(mode);
	tx_delete(bassm, mode);

	// return id to pool
	wr_ptr<IdPool> wr_bassmIdPool(m_baseAssemblyIdPool, mode);
	wr_bassmIdPool->putId(bassmId);
}

void sb7::DataHolder::deleteComplexAssembly(sh_ptr<ComplexAssembly> cassm, int mode) {
	rd_ptr<ComplexAssembly> rd_cassm(cassm, mode);

	// check if this is the root assembly
	sh_ptr<ComplexAssembly> sh_superAssm(rd_cassm->getSuperAssembly());

	if(sh_superAssm == NULL) {
		throw Sb7Exception(
			"DeleteComplexAssembly: root Complex Assembly cannot be removed!");
	}

	// remove assembly from assembly index
	int cassmId = rd_cassm->getId();

	wr_ptr<Map<int, sh_ptr<ComplexAssembly> > > wr_cassmInd(
		m_complexAssemblyIdIndex, mode);
	wr_cassmInd->remove(cassmId);

	// remove link from parent to this complex assembly
	rd_ptr<ComplexAssembly> rd_superAssm(sh_superAssm, mode);
	rd_superAssm->removeSubAssembly(cassm, mode);

	// delete subtree under this assembly
	rd_ptr<Set<sh_ptr<Assembly> > > rd_subAssmSet(
		rd_cassm->getSubAssemblies(), mode);
	Set<sh_ptr<Assembly> > subAssmSet;

	// copy set to a local one as it will be changing while removing
	// sub assemblies
	SetIterator<sh_ptr<Assembly> > iterSubAssmSetRd = rd_subAssmSet->getIter();

	while(iterSubAssmSetRd.has_next()) {
		subAssmSet.add(iterSubAssmSetRd.next());
	}

	// now delete all sub assemblies
	SetIterator<sh_ptr<Assembly> > iter = subAssmSet.getIter();
	bool childrenAreBase = rd_cassm->areChildrenBaseAssemblies();

	while(iter.has_next()) {
		if(childrenAreBase) {
			deleteBaseAssembly((sh_ptr<BaseAssembly>)iter.next(), mode);
		} else {
			deleteComplexAssembly((sh_ptr<ComplexAssembly>)iter.next(), mode);
		}
	}

	// delete asembly
	wr_ptr<ComplexAssembly> wr_cassm(cassm, mode);
	wr_cassm->freeMemory(mode);
	tx_delete(cassm, mode);

	// return id to the id pool
	wr_ptr<IdPool> wr_cassmIdPool(m_complexAssemblyIdPool, mode);
	wr_cassmIdPool->putId(cassmId);
}


//////////////////////////////
// tx method implementation //
//////////////////////////////

void *sb7::createModuleTxInner(void *data, int mode) {
	CreateModuleTxInnerParamStruct *param =
		(CreateModuleTxInnerParamStruct *)data;
	param->dataHolder->createModule(param->sh_module, mode);
	return NULL;
}

void *sb7::createCompositePartTxInner(void *data, int mode) {
	CreateCompositePartTxInnerParamStruct *param =
		(CreateCompositePartTxInnerParamStruct *)data;
	param->dataHolder->createCompositePart(param->sh_compositePart, mode);
	return NULL;
}

void *sb7::createAssembliesTxInner(void *data, int mode) {
	CreateAssembliesTxInnerParamStruct *param =
		(CreateAssembliesTxInnerParamStruct *)data;
	param->dataHolder->createAssemblies(param->sh_designRoot, mode);
	return NULL;
}

void *sb7::connectModuleDesignRootTxInner(void *data, int mode) {
	ConnectModuleDesignRootTxInnerParamStruct *param =
		(ConnectModuleDesignRootTxInnerParamStruct *)data;
	sh_ptr<ComplexAssembly> *designRoot = param->sh_designRoot;
	wr_ptr<Module> wr_mod(param->dataHolder->m_module, mode);
	wr_mod->setDesignRoot(*designRoot);
	return NULL;
}

void *sb7::connectAssembliesPartsTxInner(void *data, int mode) {
	ConnectAssembliesTxInnerParamStruct *param =
		(ConnectAssembliesTxInnerParamStruct *)data;
	param->dataHolder->connectAssembliesParts(
		param->designLibrary, param->partNum, param->connNum, mode);
	return NULL;
}
