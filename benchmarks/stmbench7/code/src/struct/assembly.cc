#include "assembly.h"
#include "composite_part.h"

#include "../parameters.h"

using namespace sb7;

// ComplexAssembly implementation
void sb7::ComplexAssembly::setLevel(int mode) {
	if(m_superAssembly != NULL) {
		rd_ptr<ComplexAssembly> rd_sa(m_superAssembly, mode);
		m_level = rd_sa->m_level - 1;
	} else {
		m_level = parameters.getNumAssmLevels();
	}
}

bool sb7::ComplexAssembly::addSubAssembly(sh_ptr<Assembly> assembly, int mode) const {
	wr_ptr<assembly_set> wr_sa(m_subAssemblies, mode);
	return wr_sa->add(assembly);
}

bool sb7::ComplexAssembly::removeSubAssembly(sh_ptr<Assembly> assembly, int mode) const {
	wr_ptr<assembly_set> wr_sa(m_subAssemblies, mode);
	return wr_sa->remove(assembly);
}

// BaseAssembly implementation
void sb7::BaseAssembly::addComponent(sh_ptr<CompositePart> cpart, int mode) const {
	wr_ptr<composite_part_bag> wr_cmp(m_components, mode);

	if(wr_cmp->add(cpart)) {
		sh_ptr<BaseAssembly> sh_this((BaseAssembly *)this);
		wr_ptr<CompositePart> wr_cpart(cpart, mode);

		wr_cpart->addAssembly(sh_this, mode);
	}
}

bool sb7::BaseAssembly::removeComponent(sh_ptr<CompositePart> cpart, int mode) const {
	wr_ptr<composite_part_bag> wr_cmp(m_components, mode);
	bool ret = wr_cmp->remove(cpart);

	if(ret) {
		wr_ptr<CompositePart> wr_cpart(cpart, mode);
		wr_cpart->removeAssembly(sh_ptr<BaseAssembly>((BaseAssembly *)this), mode);
	}

	return ret;
}

