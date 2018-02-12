#include "manual.h"
#include "../string.h"

namespace sb7 {
	
    __attribute__((transaction_safe)) int Manual::countOccurences(char c) const {
		string::const_iterator curr;
		string::const_iterator end = m_text.end();
		int cnt = 0;

		for(curr = m_text.begin();curr != end;curr++) {
			if(*curr == c) {
				cnt++;
			}
		}

		return cnt;
	}

	__attribute__((transaction_safe)) bool Manual::checkFirstLastCharTheSame() const {
		return *(m_text.begin()) == *(m_text.rbegin());
	}

	bool Manual::startsWith(char c) const {
		return *(m_text.begin()) == c;
	}

	int Manual::replaceChar(char from, char to) {
		string::iterator curr;
		string::iterator end = m_text.end();
		int cnt = 0;

		for(curr = m_text.begin();curr != end;curr++) {
			char c = *curr;

			if(c == from) {
				*curr = to;
				cnt++;
			}
		}

		return cnt;
	}
}
