#include "atomic_part.h"
#include "connection.h"

void sb7::AtomicPart::connectTo(sh_ptr<AtomicPart> dest,
		const string &type, int len, int mode) const {
	sh_ptr<AtomicPart> sh_this((AtomicPart *)this);
	Connection *conn = new Connection(sh_this, dest, type, len);
	sh_ptr<Connection> sh_conn(conn);

	wr_ptr<connection_set> wr_to(m_to, mode);
	wr_to->add(sh_conn);

	wr_ptr<AtomicPart> wr_dest(dest, mode);
	wr_dest->addConnectionFromOtherPart(sh_conn, mode);
}
