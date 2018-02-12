#ifndef SB7_DATA_HOLDER_H_
#define SB7_DATA_HOLDER_H_

#include "tm/tm_ptr.h"

#include <vector>

#include "containers.h"
#include "string.h"
#include "id_pool.h"

#include "struct/module.h"
#include "struct/atomic_part.h"
#include "struct/document.h"
#include "struct/composite_part.h"
#include "struct/assembly.h"

namespace sb7 {

	/**
	 * DataHolder object is a place to find sb7 data structure used in
	 * the benchmark. It also provides high level functions for manipulating
	 * objects that are used in the benchmark.
	 */
	class DataHolder {
		protected:
			// typedef shortcuts
			typedef Map<int, sh_ptr<AtomicPart> > atomic_part_int_index;
			typedef sh_ptr<atomic_part_int_index> shared_atomic_part_int_index;

			typedef Set<sh_ptr<AtomicPart> > atomic_part_set;
			typedef sh_ptr<atomic_part_set> shared_atomic_part_set;
			typedef Map<int, shared_atomic_part_set> atomic_part_set_int_index;
			typedef sh_ptr<atomic_part_set_int_index>
				shared_atomic_part_set_int_index;

			typedef Map<string, sh_ptr<Document> > document_string_index;
			typedef sh_ptr<document_string_index> shared_document_string_index;

			typedef Map<int, sh_ptr<CompositePart> > composite_part_int_index;
			typedef sh_ptr<composite_part_int_index>
				shared_composite_part_int_index;

			typedef Map<int, sh_ptr<BaseAssembly> > base_assembly_int_index;
			typedef sh_ptr<base_assembly_int_index>
				shared_base_assembly_int_index;

			typedef Map<int, sh_ptr<ComplexAssembly> >
				complex_assembly_int_index;
			typedef sh_ptr<complex_assembly_int_index>
				shared_complex_assembly_int_index;

			typedef std::vector<sh_ptr<AtomicPart>,
				Sb7Allocator<sh_ptr<AtomicPart> > > vector_apart;
		
		public:
			DataHolder() { }

			virtual ~DataHolder() { }

			/**
			 * Initialization of the whole structure.
			 */
			void init();
			void initTx(int mode);

			// getters for indexes and module
			shared_atomic_part_set_int_index getAtomicPartBuildDateIndex() {
				return m_atomicPartBuildDateIndex;
			}

			shared_atomic_part_int_index getAtomicPartIdIndex() {
				return m_atomicPartIdIndex;
			}

			shared_base_assembly_int_index getBaseAssemblyIdIndex() {
				return m_baseAssemblyIdIndex;
			}

			shared_complex_assembly_int_index getComplexAssemblyIdIndex() {
				return m_complexAssemblyIdIndex;
			}

			shared_composite_part_int_index getCompositePartIdIndex() {
				return m_compositePartIdIndex;
			}

			shared_document_string_index getDocumentTitleIndex() {
				return m_documentTitleIndex;
			}

			sh_ptr<Module> getModule() {
				return m_module;
			}

		private:
			void allocateData();

			string createText(unsigned size, const string &txt);
			string createType();
			int createBuildDate(int min, int max);

			void createCompositeParts(sh_ptr<CompositePart> designLibrary[],
				int dlSize);
			void createCompositePartsTx(sh_ptr<CompositePart> designLibrary[],
				int dlSize, int mode);

			void createAtomicParts(vector_apart &parts, int psize,
				sh_ptr<CompositePart> sh_cpart, int mode);
			sh_ptr<AtomicPart> createAtomicPart(int mode);

			void createConnections(vector_apart &parts, int outConn, int mode);
			void connectAtomicParts(sh_ptr<AtomicPart> ps,
				sh_ptr<AtomicPart> pe, int mode);
			int generateConnectionLength();

			void createModule(sh_ptr<Module> *retPtr, int mode);
			sh_ptr<Module> createModule(int mode);
			sh_ptr<Module> createModuleTx(int mode);

			void createAssemblies(sh_ptr<ComplexAssembly> *retPtr, int mode);
			sh_ptr<ComplexAssembly> createAssemblies(int mode);
			sh_ptr<ComplexAssembly> createAssembliesTx(int mode);
			
			sh_ptr<ComplexAssembly> createComplexAssembly(
				sh_ptr<ComplexAssembly> parent, int childNum, int mode);
			void connectAssembliesParts(sh_ptr<CompositePart> designLibrary[],
				int partNum, int connNum, int mode);

			// pass build date because during initialization opening of
			// all atomic parts for reading causes huge validation delay
			void addAtomicPartToBuildDateIndex(sh_ptr<AtomicPart> apart,
				int buildDate, int mode);

			sh_ptr<Document> createDocument(int cpartId, int mode);
			sh_ptr<Manual> createManual(int moduleId, int mode);

		//////////////////////////////////////////////////////////////////////
		// Some functions that are used outside of data holder, but is good //
		// to have them defined in one place.                               //
		//////////////////////////////////////////////////////////////////////

		public:
			void addAtomicPartToBuildDateIndex(sh_ptr<AtomicPart> apart, int mode);
			void removeAtomicPartFromBuildDateIndex(sh_ptr<AtomicPart> apart, int mode);

			sh_ptr<CompositePart> createCompositePart(int mode);
			void deleteCompositePart(sh_ptr<CompositePart> cpart, int mode);

			void createBaseAssembly(sh_ptr<ComplexAssembly> parent, int mode);
			void deleteBaseAssembly(sh_ptr<BaseAssembly> bassm, int mode);

			void createSubAssembly(sh_ptr<ComplexAssembly> parent,
				int childNum, int mode);

			void deleteComplexAssembly(sh_ptr<ComplexAssembly> bassm, int mode);

		protected:
			sh_ptr<CompositePart> createCompositePartTx(int mode);
			void createCompositePart(sh_ptr<CompositePart> *retPtr, int mode);
			void connectModuleDesignRootTx(sh_ptr<ComplexAssembly> designRoot, int mode);
			void connectAssembliesPartsTx(
				sh_ptr<CompositePart> designLibrary[],
				int partNum, int connNum, int mode);

		protected:
			void deleteDocument(sh_ptr<Document> doc, int mode);
			void deleteAtomicPart(sh_ptr<AtomicPart> apart, int mode);

		////////////////////////////////////////////////////////////////
		// Here are some functions for searching objects belonging to //
		// different classes in indexes.                              //
		////////////////////////////////////////////////////////////////

		public:
			sh_ptr<CompositePart> getCompositePart(int id, int mode) const;
			sh_ptr<BaseAssembly> getBaseAssembly(int id, int mode) const;
			sh_ptr<ComplexAssembly> getComplexAssembly(int id, int mode) const;

		private:
			/**
			 * The only module used in the data structure.
			 */
		    sh_ptr<Module> m_module;

			// definition of indexes
			shared_atomic_part_int_index m_atomicPartIdIndex;
			shared_atomic_part_set_int_index m_atomicPartBuildDateIndex;
			shared_document_string_index m_documentTitleIndex;
			shared_composite_part_int_index m_compositePartIdIndex;
			shared_base_assembly_int_index m_baseAssemblyIdIndex;
			shared_complex_assembly_int_index m_complexAssemblyIdIndex;

			// id pools
			sh_ptr<IdPool> m_compositePartIdPool;
			sh_ptr<IdPool> m_atomicPartIdPool;
			sh_ptr<IdPool> m_moduleIdPool;
			sh_ptr<IdPool> m_documentIdPool;
			sh_ptr<IdPool> m_manualIdPool;
			sh_ptr<IdPool> m_baseAssemblyIdPool;
			sh_ptr<IdPool> m_complexAssemblyIdPool;

		// tx functions
		friend void *createModuleTxInner(void *data, int mode);
		friend void *createAssembliesTxInner(void *data, int mode);
		friend void *createCompositePartTxInner(void *param, int mode);
		friend void *connectModuleDesignRootTxInner(void *data, int mode);
		friend void *connectAssembliesPartsTxInner(void *data, int mode);
	};

	struct CreateModuleTxInnerParamStruct {
		DataHolder *dataHolder;
		sh_ptr<Module> *sh_module;
	};

	void *createModuleTxInner(void *data, int mode);

	struct CreateAssembliesTxInnerParamStruct {
		DataHolder *dataHolder;
		sh_ptr<ComplexAssembly> *sh_designRoot;
	};

	void *createAssembliesTxInner(void *data, int mode);

	struct ConnectAssembliesTxInnerParamStruct {
		DataHolder *dataHolder;
		sh_ptr<CompositePart> *designLibrary;
		int partNum;
		int connNum;
	};

	void *connectAssembliesPartsTxInner(void *data, int mode);

	struct CreateCompositePartTxInnerParamStruct {
		DataHolder *dataHolder;
		sh_ptr<CompositePart> *sh_compositePart;
	};

	void *createCompositePartTxInner(void *param, int mode);

	struct ConnectModuleDesignRootTxInnerParamStruct {
		DataHolder *dataHolder;
		sh_ptr<ComplexAssembly> *sh_designRoot;
	};

	void *connectModuleDesignRootTxInner(void *data, int mode);
}

#endif /*SB7_DATA_HOLDER_H_*/
